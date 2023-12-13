// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_VANISHING_VANISHING_UTILS_H_
#define TACHYON_ZK_PLONK_VANISHING_VANISHING_UTILS_H_

#include <utility>
#include <vector>

#include "tachyon/base/parallelize.h"

namespace tachyon::zk {

// Calculate ζ⁻¹ = g^((2ˢ * T) / 3).
// NOTE(TomTaehoonKim): Halo2 calculates ζ = g^((2ˢ * T) / 3)². So in need of
// ζ, square the result of this function.
template <typename F>
constexpr F GetZetaInv() {
  CHECK_EQ(F::Config::kTrace % math::BigInt<F::kLimbNums>(3),
           math::BigInt<F::kLimbNums>(0));
  return F::FromMontgomery(F::Config::kSubgroupGenerator)
      .Pow(F(2).Pow(F::Config::kTwoAdicity).ToBigInt() * F::Config::kTrace /
           math::BigInt<F::kLimbNums>(3));
}

// This divides the polynomial (in the extended domain) by the vanishing
// polynomial of the 2ᵏ size domain.
template <typename F, typename Domain, typename ExtendedDomain,
          typename ExtendedEvals>
ExtendedEvals& DivideByVanishingPolyInPlace(
    ExtendedEvals& evals, const ExtendedDomain* extended_domain,
    const Domain* domain) {
  CHECK_EQ(evals.NumElements(), extended_domain->size());

  F zeta = GetZetaInv<F>().Square();

  // Compute the evaluations of t(X) = Xⁿ - 1 in the coset evaluation domain.
  // We don't have to compute all of them, because it will repeat.
  std::vector<F> t_evaluations;
  t_evaluations.reserve(size_t{1} << (extended_domain->log_size_of_group() -
                                      domain->log_size_of_group()));
  F orig = zeta.Pow(domain->size());
  F step = extended_domain->group_gen().Pow(domain->size());
  F cur = orig;
  do {
    t_evaluations.push_back(cur);
    cur *= step;
  } while (cur != orig);
  CHECK_EQ(t_evaluations.size(),
           size_t{1} << (extended_domain->log_size_of_group() -
                         domain->log_size_of_group()));

  // Subtract 1 from each to give us t_evaluations[i] = t(zeta *
  // extended_omegaⁱ)
  // TODO(TomTaehoonKim): Consider implementing "translate" function.
  base::Parallelize(t_evaluations, [](absl::Span<F> chunk) {
    for (F& coeff : chunk) {
      coeff -= F::One();
    }
  });

  F::BatchInverseInPlace(t_evaluations);

  // Multiply the inverse to obtain the quotient polynomial in the coset
  // evaluation domain.
  std::vector<F>& evaluations = evals.evaluations_ref();
  base::Parallelize(evaluations,
                    [t_evaluations](absl::Span<F> chunk, size_t chunk_idx,
                                    size_t chunk_size) {
                      size_t index = chunk_idx * chunk_size;
                      for (F& h : chunk) {
                        h *= t_evaluations[index % t_evaluations.size()];
                        ++index;
                      }
                    });

  return evals;
}

// This divides the polynomial (in the extended domain) by the vanishing
// polynomial of the 2ᵏ size domain.
template <typename F, typename Domain, typename ExtendedDomain,
          typename ExtendedEvals>
ExtendedEvals DivideByVanishingPoly(const ExtendedEvals& evals,
                                    const ExtendedDomain* extended_domain,
                                    const Domain* domain) {
  ExtendedEvals ret = evals;
  DivideByVanishingPolyInPlace<F>(ret, extended_domain, domain);
  return ret;
}

// Given a |poly| of coefficients  [a₀, a₁, a₂, ...], this returns
// [a₀, ζa₁, ζ²a₂, a₃, ζa₄, ζ²a₅, a₆, ...], where ζ is a cube root of unity in
// the multiplicative subgroup with order (p - 1), i.e. ζ³ = 1.
//
// |into_coset| should be set to true when moving into the coset, and false
// when moving out. This toggles the choice of ζ.
template <typename F, typename ExtendedPoly>
void DistributePowersZeta(ExtendedPoly& poly, bool into_coset) {
  F zeta_inv = GetZetaInv<F>();
  F zeta = zeta_inv.Square();
  std::vector<F> coset_powers{into_coset ? zeta : zeta_inv,
                              into_coset ? zeta_inv : zeta};

  std::vector<F> coeffs = poly.coefficients().coefficients();
  base::Parallelize(coeffs,
                    [&coset_powers](absl::Span<F> chunk, size_t chunk_idx,
                                    size_t chunk_size) {
                      size_t i = chunk_idx * chunk_size;
                      for (F& a : chunk) {
                        // Distribute powers to move into/from coset
                        size_t j = i % (coset_powers.size() + 1);
                        if (j != 0) {
                          a *= coset_powers[j - 1];
                        }
                        ++i;
                      }
                    });
}

// This takes us from the extended evaluation domain and gets us the quotient
// polynomial coefficients.
//
// This function will panic if the provided vector is not the correct length.
template <typename F, typename ExtendedPoly, typename ExtendedEvals,
          typename ExtendedDomain>
ExtendedPoly ExtendedToCoeff(const ExtendedEvals& evals,
                             const ExtendedDomain* extended_domain) {
  CHECK_EQ(evals.NumElements(), extended_domain->size());

  ExtendedPoly poly = extended_domain->IFFT(evals);

  // Distribute powers to move from coset; opposite from the
  // transformation we performed earlier.
  DistributePowersZeta<F>(poly, false);

  return poly;
}

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_VANISHING_VANISHING_UTILS_H_
