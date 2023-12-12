// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_VANISHING_PROVER_VANISHING_ARGUMENT_H_
#define TACHYON_ZK_PLONK_VANISHING_PROVER_VANISHING_ARGUMENT_H_

#include <algorithm>
#include <utility>
#include <vector>

#include "tachyon/base/parallelize.h"
#include "tachyon/crypto/transcripts/transcript.h"
#include "tachyon/zk/base/entities/entity_ty.h"
#include "tachyon/zk/base/entities/prover.h"
#include "tachyon/zk/base/prover_query.h"
#include "tachyon/zk/plonk/vanishing/vanishing_committed.h"
#include "tachyon/zk/plonk/vanishing/vanishing_constructed.h"
#include "tachyon/zk/plonk/vanishing/vanishing_evaluated.h"
#include "tachyon/zk/plonk/vanishing/vanishing_utils.h"

namespace tachyon::zk {

template <typename PCSTy>
[[nodiscard]] bool VanishingCommit(
    Prover<PCSTy>* prover, VanishingCommitted<EntityTy::kProver, PCSTy>* out) {
  // Sample a random polynomial of degree n - 1
  const typename PCSTy::Evals random_eval =
      PCSTy::Evals::One(prover->pcs().N());
  typename PCSTy::Poly random_poly = prover->domain()->IFFT(random_eval);

  // Sample a random blinding factor
  // TODO(TomTaehoonKim): Figure out why it is named |random_blind|.
  // See
  // https://github.com/kroma-network/halo2/blob/7d0a36990452c8e7ebd600de258420781a9b7917/halo2_proofs/src/plonk/vanishing/prover.rs#L55-L56
  typename PCSTy::Field random_blind = PCSTy::Field::Zero();

  if (!prover->Commit(random_poly)) return false;

  *out = {std::move(random_poly), std::move(random_blind)};
  return true;
}

template <typename PCSTy, typename ExtendedEvals>
[[nodiscard]] bool VanishingConstruct(
    Prover<PCSTy>* prover,
    VanishingCommitted<EntityTy::kProver, PCSTy>&& committed,
    const ExtendedEvals& linear_combination_of_gates,
    VanishingConstructed<EntityTy::kProver, PCSTy>* constructed_out) {
  // Divide by t(X) = X^{params.n} - 1.
  ExtendedEvals h_evals = DivideByVanishingPoly<typename PCSTy::Field>(
      linear_combination_of_gates, prover->extended_domain(), prover->domain());

  // Obtain final h(X) polynomial
  typename PCSTy::ExtendedPoly h_poly =
      ExtendedToCoeff<typename PCSTy::Field, typename PCSTy::ExtendedPoly>(
          h_evals, prover->extended_domain());

  // Truncate it to match the size of the quotient polynomial; the
  // evaluation domain might be slightly larger than necessary because
  // it always lies on a power-of-two boundary.
  std::vector<typename PCSTy::Field> h_coeffs =
      h_poly.coefficients().coefficients();
  h_coeffs.resize(prover->extended_domain()->size(), PCSTy::Field::Zero());

  // Compute commitments to each h(X) piece
  const size_t kCommitmentNum = h_coeffs.size() / prover->pcs().N();

  std::vector<bool> results = base::ParallelizeMapByChunkSize(
      h_coeffs, prover->pcs().N(),
      [prover](absl::Span<const typename PCSTy::Field> h_piece,
               size_t chunk_index) { return prover->Commit(h_piece); });
  if (std::any_of(results.begin(), results.end(),
                  [](bool result) { return result == false; })) {
    return false;
  }

  // FIXME(TomTaehoonKim): Remove this if possible.
  std::vector<typename PCSTy::Field> h_blinds = base::CreateVector(
      kCommitmentNum, [prover]() { return prover->blinder().Generate(); });

  *constructed_out = {std::move(h_poly), std::move(h_blinds),
                      std::move(committed)};
  return true;
}

template <typename PCSTy, typename F, typename Commitment>
[[nodiscard]] bool VanishingEvaluate(
    const PCSTy& pcs,
    VanishingConstructed<EntityTy::kProver, PCSTy>&& constructed,
    const crypto::Challenge255<F>& x, const F& x_n,
    crypto::TranscriptWriter<Commitment>* writer,
    VanishingEvaluated<EntityTy::kProver, PCSTy>* evaluated_out) {
  typename PCSTy::Poly h_poly = PCSTy::Poly::Zero();
  auto h_chunks = base::Chunked(
      constructed.h_poly().coefficients().coefficients(), pcs.N());
  auto h_pieces =
      base::Map(h_chunks.begin(), h_chunks.end(),
                [](const absl::Span<const F>& h_piece) { return h_piece; });
  for (absl::Span<const F> h_piece : base::Reversed(h_pieces)) {
    std::vector<F> h_vec(h_piece.begin(), h_piece.end());
    h_poly = h_poly * x_n +
             typename PCSTy::Poly(
                 typename PCSTy::Poly::Coefficients(std::move(h_vec)));
  }

  F h_blind = std::accumulate(constructed.h_blinds().rbegin(),
                              constructed.h_blinds().rend(), F::Zero(),
                              [&x_n](F& acc, const F& eval) {
                                acc *= x_n;
                                return acc + eval;
                              });

  VanishingCommitted<EntityTy::kProver, PCSTy> committed =
      std::move(std::move(constructed).TakeCommitted());
  F random_eval = committed.random_poly().Evaluate(x.ChallengeAsScalar());
  if (!writer->WriteToProof(random_eval)) return false;

  *evaluated_out = {std::move(h_poly), std::move(h_blind),
                    std::move(committed)};
  return true;
}

template <typename PCSTy, typename F>
std::vector<ProverQuery<PCSTy>> VanishingOpen(
    VanishingEvaluated<EntityTy::kProver, PCSTy>&& evaluated,
    const crypto::Challenge255<F>& x) {
  F x_scalar = x.ChallengeAsScalar();
  VanishingCommitted<EntityTy::kProver, PCSTy>&& committed =
      std::move(evaluated).TakeCommitted();
  return {{x_scalar, BlindedPolynomial<typename PCSTy::Poly>(
                         std::move(evaluated).TakeHPoly(),
                         std::move(evaluated).TakeHBlind())
                         .ToRef()},
          {x_scalar, BlindedPolynomial<typename PCSTy::Poly>(
                         std::move(committed).TakeRandomPoly(),
                         std::move(committed).TakeRandomBlind())
                         .ToRef()}};
}

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_VANISHING_PROVER_VANISHING_ARGUMENT_H_
