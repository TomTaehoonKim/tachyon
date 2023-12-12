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
  VanishingConstructed() = default;
  VanishingConstructed(typename PCSTy::ExtendedPoly&& h_poly,
                       std::vector<typename PCSTy::Field>&& h_blinds,
                       VanishingCommitted<EntityTy::kProver, PCSTy>&& committed)
      : h_poly_(std::move(h_poly)),
        h_blinds_(std::move(h_blinds)),
        committed_(std::move(committed)) {}

  const typename PCSTy::ExtendedPoly& h_poly() const { return h_poly_; }
  const std::vector<typename PCSTy::Field>& h_blinds() const {
    return h_blinds_;
  }

  VanishingCommitted<EntityTy::kProver, PCSTy>&& TakeCommitted() && {
    return std::move(committed_);
  }

 private:
  typename PCSTy::ExtendedPoly h_poly_;
  std::vector<typename PCSTy::Field> h_blinds_;
  VanishingCommitted<EntityTy::kProver, PCSTy> committed_;
};

template <typename PCSTy>
class VanishingConstructed<EntityTy::kVerifier, PCSTy> {
 public:
  VanishingConstructed() = default;
  VanishingConstructed(std::vector<typename PCSTy::Commitment>&& h_commitments,
                       typename PCSTy::Commitment&& random_poly_commitment)
      : h_commitments_(std::move(h_commitments)),
        random_poly_commitment_(std::move(random_poly_commitment)) {}

  std::vector<typename PCSTy::Commitment>&& TakeHCommitments() && {
    return std::move(h_commitments_);
  }
  typename PCSTy::Commitment&& TakeRandomPolyCommitment() && {
    return std::move(random_poly_commitment_);
  }

 private:
  std::vector<typename PCSTy::Commitment> h_commitments_;
  typename PCSTy::Commitment random_poly_commitment_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_VANISHING_VANISHING_CONSTRUCTED_H_
