// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_VANISHING_VANISHING_COMMITTED_H_
#define TACHYON_ZK_PLONK_VANISHING_VANISHING_COMMITTED_H_

#include <utility>

#include "tachyon/zk/base/entities/entity_ty.h"

namespace tachyon::zk {

template <EntityTy EntityType, typename PCSTy>
class VanishingCommitted;

template <typename PCSTy>
class VanishingCommitted<EntityTy::kProver, PCSTy> {
 public:
  VanishingCommitted() = default;
  VanishingCommitted(typename PCSTy::Poly&& random_poly,
                     typename PCSTy::Field&& random_blind)
      : random_poly_(std::move(random_poly)),
        random_blind_(std::move(random_blind)) {}

  const typename PCSTy::Poly& random_poly() { return random_poly_; }

  typename PCSTy::Poly&& TakeRandomPoly() && { return std::move(random_poly_); }
  typename PCSTy::Field&& TakeRandomBlind() && {
    return std::move(random_blind_);
  }

 private:
  typename PCSTy::Poly random_poly_;
  typename PCSTy::Field random_blind_;
};

template <typename PCSTy>
class VanishingCommitted<EntityTy::kVerifier, PCSTy> {
 public:
  VanishingCommitted() = default;
  explicit VanishingCommitted(
      typename PCSTy::Commitment&& random_poly_commitment)
      : random_poly_commitment_(std::move(random_poly_commitment)) {}

  typename PCSTy::Commitment&& TakeRandomPolyCommitment() && {
    return std::move(random_poly_commitment_);
  }

 private:
  typename PCSTy::Commitment random_poly_commitment_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_VANISHING_VANISHING_COMMITTED_H_
