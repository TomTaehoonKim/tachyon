// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_VANISHING_VANISHING_EVALUATED_H_
#define TACHYON_ZK_PLONK_VANISHING_VANISHING_EVALUATED_H_

#include <utility>
#include <vector>

#include "tachyon/zk/base/entities/entity_ty.h"
#include "tachyon/zk/plonk/vanishing/vanishing_committed.h"

namespace tachyon::zk {

template <EntityTy EntityType, typename PCSTy>
class VanishingEvaluated;

template <typename PCSTy>
class VanishingEvaluated<EntityTy::kProver, PCSTy> {
 public:
  VanishingEvaluated() = default;
  VanishingEvaluated(typename PCSTy::Poly&& h_poly,
                     typename PCSTy::Field&& h_blind,
                     VanishingCommitted<EntityTy::kProver, PCSTy>&& committed)
      : h_poly_(std::move(h_poly)),
        h_blind_(std::move(h_blind)),
        committed_(std::move(committed)) {}

  typename PCSTy::Poly&& TakeHPoly() && { return std::move(h_poly_); }
  typename PCSTy::Field&& TakeHBlind() && { return std::move(h_blind_); }
  VanishingCommitted<EntityTy::kProver, PCSTy>&& TakeCommitted() && {
    return std::move(committed_);
  }

 private:
  typename PCSTy::Poly h_poly_;
  typename PCSTy::Field h_blind_;
  VanishingCommitted<EntityTy::kProver, PCSTy> committed_;
};

template <typename PCSTy>
class VanishingEvaluated<EntityTy::kVerifier, PCSTy> {
 public:
  VanishingEvaluated() = default;
  VanishingEvaluated(typename PCSTy::Commitment&& h_commitment,
                     typename PCSTy::Commitment&& random_poly_commitment,
                     typename PCSTy::Field&& expected_h_eval,
                     typename PCSTy::Field&& random_eval)
      : h_commitment_(std::move(h_commitment)),
        random_poly_commitment_(std::move(random_poly_commitment)),
        expected_h_eval_(std::move(expected_h_eval)),
        random_eval_(std::move(random_eval)) {}

  const typename PCSTy::Commitment& h_commitment() const {
    return h_commitment_;
  }
  const typename PCSTy::Commitment& random_poly_commitment() const {
    return std::move(random_poly_commitment_);
  }
  typename PCSTy::Field&& TakeExpectedHEval() && {
    return std::move(expected_h_eval_);
  }
  typename PCSTy::Field&& TakeRandomEval() && {
    return std::move(random_eval_);
  }

 private:
  typename PCSTy::Commitment h_commitment_;
  typename PCSTy::Commitment random_poly_commitment_;
  typename PCSTy::Field expected_h_eval_;
  typename PCSTy::Field random_eval_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_VANISHING_VANISHING_EVALUATED_H_
