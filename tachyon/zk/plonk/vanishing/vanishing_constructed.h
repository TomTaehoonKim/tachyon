// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_VANISHING_VANISHING_CONSTRUCTED_H_
#define TACHYON_ZK_PLONK_VANISHING_VANISHING_CONSTRUCTED_H_

#include <utility>
#include <vector>

#include "tachyon/zk/base/entities/entity_ty.h"
#include "tachyon/zk/plonk/vanishing/vanishing_committed.h"

namespace tachyon::zk {

template <EntityTy EntityType, typename PCSTy>
class VanishingConstructed;

template <typename PCSTy>
class VanishingConstructed<EntityTy::kProver, PCSTy> {
 public:
  using F = typename PCSTy::Field;
  using Poly = typename PCSTy::Poly;
  using ExtendedPoly = typename PCSTy::ExtendedPoly;

  VanishingConstructed() = default;
  VanishingConstructed(ExtendedPoly&& h_poly, Poly&& h_blinds_poly,
                       VanishingCommitted<EntityTy::kProver, PCSTy>&& committed)
      : h_poly_(std::move(h_poly)),
        h_blinds_poly_(std::move(h_blinds_poly)),
        committed_(std::move(committed)) {}

  const ExtendedPoly& h_poly() const { return h_poly_; }
  const Poly& h_blinds_poly() const { return h_blinds_poly_; }

  VanishingCommitted<EntityTy::kProver, PCSTy>&& TakeCommitted() && {
    return std::move(committed_);
  }

 private:
  ExtendedPoly h_poly_;
  Poly h_blinds_poly_;
  VanishingCommitted<EntityTy::kProver, PCSTy> committed_;
};

template <typename PCSTy>
class VanishingConstructed<EntityTy::kVerifier, PCSTy> {
 public:
  using Commitment = typename PCSTy::Commitment;

  VanishingConstructed() = default;
  VanishingConstructed(std::vector<Commitment>&& h_commitments,
                       Commitment&& random_poly_commitment)
      : h_commitments_(std::move(h_commitments)),
        random_poly_commitment_(std::move(random_poly_commitment)) {}

  std::vector<Commitment>&& TakeHCommitments() && {
    return std::move(h_commitments_);
  }
  Commitment&& TakeRandomPolyCommitment() && {
    return std::move(random_poly_commitment_);
  }

 private:
  std::vector<Commitment> h_commitments_;
  Commitment random_poly_commitment_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_VANISHING_VANISHING_CONSTRUCTED_H_
