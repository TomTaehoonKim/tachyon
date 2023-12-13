// Copyright 2022 arkworks contributors
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.arkworks and the LICENCE-APACHE.arkworks
// file.

#ifndef TACHYON_MATH_POLYNOMIALS_UNIVARIATE_UNIVARIATE_EVALUATIONS_H_
#define TACHYON_MATH_POLYNOMIALS_UNIVARIATE_UNIVARIATE_EVALUATIONS_H_

#include <stddef.h>

#include <string>
#include <utility>
#include <vector>

#include "absl/hash/hash.h"

#include "tachyon/base/buffer/copyable.h"
#include "tachyon/base/containers/container_util.h"
#include "tachyon/base/logging.h"
#include "tachyon/base/strings/string_util.h"
#include "tachyon/math/polynomials/polynomial.h"
#include "tachyon/math/polynomials/univariate/univariate_evaluation_domain_forwards.h"

namespace tachyon {
namespace math {
namespace internal {

template <typename F, size_t MaxDegree>
class UnivariateEvaluationsOp;

}  // namespace internal

// UnivariateEvaluations represents a univariate polynomial in evaluation form.
// For a univariate polynomial like 3x² + 2x + 1, it can be represented as [(0,
// 1), (1, 6), (2, 17)]. Using Lagrange interpolation, we can easily convert it
// into coefficient form, which is [1, 2, 3]. UnivariateEvaluations only stores
// the y-coordinates, such as [1, 6, 17] in this example. Depending on its
// evaluation domain, the univariate polynomial can vary.
// For more information, refer to
// https://en.wikipedia.org/wiki/Lagrange_polynomial
template <typename F, size_t MaxDegree>
class UnivariateEvaluations final
    : public Polynomial<UnivariateEvaluations<F, MaxDegree>> {
 public:
  constexpr static size_t kMaxDegree = MaxDegree;

  using Field = F;

  constexpr UnivariateEvaluations() = default;
  constexpr explicit UnivariateEvaluations(const std::vector<F>& evaluations)
      : evaluations_(evaluations) {
    CHECK_LE(Degree(), MaxDegree);
  }
  constexpr explicit UnivariateEvaluations(std::vector<F>&& evaluations)
      : evaluations_(std::move(evaluations)) {
    CHECK_LE(Degree(), MaxDegree);
  }

  constexpr static bool IsCoefficientForm() { return false; }

  constexpr static bool IsEvaluationForm() { return true; }

  // NOTE(chokobole): The zero polynomial can be represented in two forms:
  // 1. An empty vector.
  // 2. A vector filled with |F::Zero()|.
  constexpr static UnivariateEvaluations Zero() {
    return UnivariateEvaluations();
  }

  // NOTE(chokobole): This creates polynomial that contains |F::Zero()| up to
  // |degree| + 1.
  constexpr static UnivariateEvaluations UnsafeZero(size_t degree) {
    UnivariateEvaluations ret;
    ret.evaluations_ = base::CreateVector(degree + 1, F::Zero());
    return ret;
  }

  constexpr static UnivariateEvaluations One(size_t degree) {
    UnivariateEvaluations ret;
    ret.evaluations_ = base::CreateVector(degree + 1, F::One());
    return ret;
  }

  constexpr static UnivariateEvaluations Random(size_t degree) {
    return UnivariateEvaluations(
        base::CreateVector(degree + 1, []() { return F::Random(); }));
  }

  constexpr const std::vector<F>& evaluations() const { return evaluations_; }
  constexpr std::vector<F>& evaluations_ref() { return evaluations_; }

  // NOTE(chokobole): Sometimes, this degree doesn't match with the exact
  // degree of the coefficients that is produced by IFFT. I leave it for
  // consistency with another polynomial.
  // For example, [0, 0, 0, 0] gives you 3, but it's 0 in reality.
  constexpr size_t Degree() const {
    if (evaluations_.empty()) return 0;
    return evaluations_.size() - 1;
  }

  constexpr size_t NumElements() const { return evaluations_.size(); }

  constexpr bool IsZero() const {
    if (evaluations_.empty()) return true;
    return std::all_of(evaluations_.begin(), evaluations_.end(),
                       [](const F& value) { return value.IsZero(); });
  }

  constexpr bool IsOne() const {
    if (evaluations_.empty()) return false;
    return std::all_of(evaluations_.begin(), evaluations_.end(),
                       [](const F& value) { return value.IsOne(); });
  }

  constexpr bool operator==(const UnivariateEvaluations& other) const {
    if (evaluations_.empty()) {
      return other.IsZero();
    }
    if (other.evaluations_.empty()) {
      return IsZero();
    }
    return evaluations_ == other.evaluations_;
  }

  constexpr bool operator!=(const UnivariateEvaluations& other) const {
    return !operator==(other);
  }

  constexpr F* operator[](size_t i) {
    return const_cast<F*>(std::as_const(*this)[i]);
  }

  constexpr const F* operator[](size_t i) const {
    if (i < evaluations_.size()) {
      return &evaluations_[i];
    }
    return nullptr;
  }

  std::string ToString() const { return base::VectorToString(evaluations_); }

  // AdditiveSemigroup methods
  UnivariateEvaluations& AddInPlace(const UnivariateEvaluations& other) {
    return internal::UnivariateEvaluationsOp<F, MaxDegree>::AddInPlace(*this,
                                                                       other);
  }

  // AdditiveGroup methods
  UnivariateEvaluations& SubInPlace(const UnivariateEvaluations& other) {
    return internal::UnivariateEvaluationsOp<F, MaxDegree>::SubInPlace(*this,
                                                                       other);
  }

  UnivariateEvaluations& NegInPlace() {
    return internal::UnivariateEvaluationsOp<F, MaxDegree>::NegInPlace(*this);
  }

  // MultiplicativeSemigroup methods
  UnivariateEvaluations& MulInPlace(const UnivariateEvaluations& other) {
    return internal::UnivariateEvaluationsOp<F, MaxDegree>::MulInPlace(*this,
                                                                       other);
  }

  UnivariateEvaluations& MulInPlace(const F& scalar) {
    return internal::UnivariateEvaluationsOp<F, MaxDegree>::MulInPlace(*this,
                                                                       scalar);
  }

  UnivariateEvaluations& DivInPlace(const UnivariateEvaluations& other) {
    return internal::UnivariateEvaluationsOp<F, MaxDegree>::DivInPlace(*this,
                                                                       other);
  }

  UnivariateEvaluations& DivInPlace(const F& scalar) {
    return internal::UnivariateEvaluationsOp<F, MaxDegree>::DivInPlace(*this,
                                                                       scalar);
  }

  constexpr UnivariateEvaluations operator/(
      const UnivariateEvaluations& other) const {
    UnivariateEvaluations poly = *this;
    return poly.DivInPlace(other);
  }

  constexpr UnivariateEvaluations& operator/=(
      const UnivariateEvaluations& other) {
    return DivInPlace(other);
  }

  constexpr UnivariateEvaluations operator/(const F& scalar) const {
    UnivariateEvaluations poly = *this;
    return poly.DivInPlace(scalar);
  }

  constexpr UnivariateEvaluations& operator/=(const F& scalar) {
    return DivInPlace(scalar);
  }

 private:
  friend class internal::UnivariateEvaluationsOp<F, MaxDegree>;
  friend class Radix2EvaluationDomain<F, MaxDegree>;
  friend class MixedRadixEvaluationDomain<F, kMaxDegree>;

  std::vector<F> evaluations_;
};

template <typename F, size_t MaxDegree>
class PolynomialTraits<UnivariateEvaluations<F, MaxDegree>> {
 public:
  constexpr static bool kIsEvaluationForm = true;
};

template <typename H, typename F, size_t MaxDegree>
H AbslHashValue(H h, const UnivariateEvaluations<F, MaxDegree>& evals) {
  // NOTE(chokobole): We shouldn't hash only with a non-zero term.
  // See https://abseil.io/docs/cpp/guides/hash#the-abslhashvalue-overload
  size_t degree = 0;
  for (const F& eval : evals.evaluations()) {
    h = H::combine(std::move(h), eval);
    ++degree;
  }
  F zero = F::Zero();
  for (size_t i = degree; i < MaxDegree + 1; ++i) {
    h = H::combine(std::move(h), zero);
  }
  return h;
}

}  // namespace math

namespace base {

template <typename F, size_t MaxDegree>
class Copyable<math::UnivariateEvaluations<F, MaxDegree>> {
 public:
  static bool WriteTo(const math::UnivariateEvaluations<F, MaxDegree>& evals,
                      Buffer* buffer) {
    return buffer->Write(evals.evaluations());
  }

  static bool ReadFrom(const Buffer& buffer,
                       math::UnivariateEvaluations<F, MaxDegree>* evals) {
    std::vector<F> evals_vec;
    if (!buffer.Read(&evals_vec)) return false;
    *evals = math::UnivariateEvaluations<F, MaxDegree>(evals_vec);
    return true;
  }

  static size_t EstimateSize(
      const math::UnivariateEvaluations<F, MaxDegree>& evals) {
    return base::EstimateSize(evals.evaluations());
  }
};

}  // namespace base
}  // namespace tachyon

#include "tachyon/math/polynomials/univariate/univariate_evaluations_ops.h"

#endif  // TACHYON_MATH_POLYNOMIALS_UNIVARIATE_UNIVARIATE_EVALUATIONS_H_
