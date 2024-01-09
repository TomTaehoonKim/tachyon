#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_IS_ZERO_CHIP_H_
#define TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_IS_ZERO_CHIP_H_

#include <memory>
#include <utility>

#include "tachyon/zk/expressions/expression_factory.h"
#include "tachyon/zk/plonk/circuit/region.h"
#include "tachyon/zk/plonk/constraint_system.h"

namespace tachyon::zk {

template <typename F>
class IsZeroConfig {
 public:
  using Field = F;

  IsZeroConfig(const AdviceColumnKey& value_inv,
               std::unique_ptr<Expression<F>> is_zero_expr)
      : value_inv(value_inv), is_zero_expr(std::move(is_zero_expr)) {}

  const Expression<F>* expr() const { return is_zero_expr.get(); }

 private:
  AdviceColumnKey value_inv;
  std::unique_ptr<Expression<F>> is_zero_expr;
};

template <typename F>
class IsZeroChip {
 public:
  explicit IsZeroChip(IsZeroConfig<F>&& config) : config_(std::move(config)) {}

  static IsZeroConfig<F> Configure(
      ConstraintSystem<F>& meta,
      base::OnceCallback<std::unique_ptr<Expression<F>>(VirtualCells<F>&)>
          q_enable,
      base::OnceCallback<std::unique_ptr<Expression<F>>(VirtualCells<F>&)>
          value,
      const AdviceColumnKey& value_inv) {
    std::unique_ptr<Expression<F>> is_zero_expr =
        ExpressionFactory<F>::Constant(F::Zero());

    meta.CreateGate("is_zero", [&q_enable, &value, &value_inv,
                                &is_zero_expr](VirtualCells<F>& meta) {
      // clang-format off
      //
      // q_enable | value |  value_inv |  1 - value * value_inv | value * (1 - value * value_inv)
      // ---------+-------+------------+------------------------+-------------------------------
      //   yes    |   x   |    1/x     |         0              |         0
      //   no     |   x   |    0       |         1              |         x
      //   yes    |   0   |    0       |         1              |         0
      //   yes    |   0   |    y       |         1              |         0
      //
      // clang-format on
      std::unique_ptr<Expression<F>> q_enable = q_enable.Run(meta);
      std::unique_ptr<Expression<F>> value = value.Run(meta);
      std::unique_ptr<Expression<F>> value_inv =
          meta.QueryAdvice(value_inv, Rotation::Cur());

      std::unique_ptr<Expression<F>> is_zero_expr =
          ExpressionFactory<F>::Constant(F::One() - value * value_inv);
      std::vector<Constraint<F>> constraints;
      constraints.emplace_back(std::move(q_enable) * std::move(value) *
                               std::move(is_zero_expr));
      return constraints;
    });

    return IsZeroConfig<F>(value_inv, std::move(is_zero_expr));
  }

  void Assign(Region<F>& region, size_t offset, const Value<F>& value) const {
    const F value_inv = value.IsOne() ? F::Zero() : value.value().Inverse();
    region.AssignAdvice("value inv", config_.value_inv, offset,
                        [&value_inv]() { return value_inv; });
  }

 private:
  IsZeroConfig<F> config_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_IS_ZERO_CHIP_H_
