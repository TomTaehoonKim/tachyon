#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT3_H_
#define TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT3_H_

#include <memory>
#include <utility>

#include "tachyon/zk/plonk/circuit/assigned_cell.h"
#include "tachyon/zk/plonk/circuit/circuit.h"
#include "tachyon/zk/plonk/circuit/examples/is_zero_chip.h"
#include "tachyon/zk/plonk/circuit/virtual_cells.h"

namespace tachyon::zk {

template <typename F>
struct FibonacciConfig3 {
  using Field = F;

  FibonacciConfig3(const Selector& selector, const AdviceColumnKey& a,
                   const AdviceColumnKey& b, const AdviceColumnKey& c,
                   IsZeroConfig<F>&& a_equals_b, const AdviceColumnKey& output)
      : selector(selector),
        a(a),
        b(b),
        c(c),
        a_equals_b(std::move(a_equals_b)),
        output(output) {}

  Selector selector;
  AdviceColumnKey a;
  AdviceColumnKey b;
  AdviceColumnKey c;
  IsZeroConfig<F> a_equals_b;
  AdviceColumnKey output;
};

template <typename F>
class FibonacciChip3 {
 public:
  explicit FibonacciChip3(FibonacciConfig3<F>&& config)
      : config_(std::move(config)) {}

  static FibonacciConfig3<F> Configure(ConstraintSystem<F>& meta) {
    Selector selector = meta.CreateSimpleSelector();
    AdviceColumnKey a = meta.CreateAdviceColumn();
    AdviceColumnKey b = meta.CreateAdviceColumn();
    AdviceColumnKey c = meta.CreateAdviceColumn();
    AdviceColumnKey output = meta.CreateAdviceColumn();

    AdviceColumnKey is_zero_advice_column = meta.CreateAdviceColumn();
    IsZeroConfig<F> a_equals_b = IsZeroChip<F>::Configure(
        meta,
        [&selector](VirtualCells<F>& meta) {
          return meta.QuerySelector(selector);
        },
        [&a, &b](VirtualCells<F>& meta) {
          return meta.QueryAdvice(a, Rotation::Cur()) -
                 meta.QueryAdvice(b, Rotation::Cur());
        },
        is_zero_advice_column);

    meta.CreateGate(
        "f(a, b, c) = if a == b {c} else {a - b}",
        [&selector, &a, &b, &c, &output, &a_equals_b](VirtualCells<F>& meta) {
          std::unique_ptr<Expression<F>> s = meta.QuerySelector(selector);
          std::unique_ptr<Expression<F>> a =
              meta.QueryAdvice(a, Rotation::Cur());
          std::unique_ptr<Expression<F>> b =
              meta.QueryAdvice(b, Rotation::Cur());
          std::unique_ptr<Expression<F>> c =
              meta.QueryAdvice(c, Rotation::Cur());
          std::unique_ptr<Expression<F>> output =
              meta.QueryAdvice(output, Rotation::Cur());

          std::vector<Constraint<F>> constraints;
          constraints.emplace_back(std::move(s.Clone()) *
                                   (a_equals_b.expr() * (output - c)));
          constraints.emplace_back(
              std::move(s) *
              (ExpressionFactory<F>::Constant(F::One()) - a_equals_b.expr()) *
              (output - (a - b)));
          return constraints;
        });

    return FibonacciConfig3<F>(a, b, c, selector, a_equals_b, output);
  }

  AssignedCell<F> Assign(Layouter<F>* layouter, const F& a, const F& b,
                         const F& c) const {
    AssignedCell<F> ret;
    layouter->AssignRegion(
        "f(a, b, c) = if a == b {c} else {a - b}",
        [this, &ret, &a, &b, &c](Region<F>& region) {
          config_.selector.Enable(region, 0);

          AssignedCell<F> a_cell = region.AssignAdvice(
              "a", config_.a, 0, [&a]() { return Value<F>::Known(a); });
          AssignedCell<F> b_cell = region.AssignAdvice(
              "b", config_.b, 0, [&b]() { return Value<F>::Known(b); });
          AssignedCell<F> c_cell = region.AssignAdvice(
              "c", config_.c, 0, [&c]() { return Value<F>::Known(c); });

          IsZeroChip<F> is_zero_chip(std::move(config_.a_equals_b));
          is_zero_chip.Assign(region, 0, Value::Known(a - b));

          F output = a == b ? c : a - b;
          AssignedCell<F> output_cell = region.AssignAdvice(
              "output", config_.output, 0,
              [&output]() { return Value<F>::Known(output); });

          ret = output_cell;
        });
    return ret;
  }

 private:
  FibonacciConfig3<F> config_;
};

template <typename F, template <typename> class _FloorPlanner>
class FibonacciCircuit3 : public Circuit<FibonacciConfig3<F>> {
 public:
  using FloorPlanner = _FloorPlanner<FibonacciCircuit3<F, _FloorPlanner>>;

  FloorPlanner& floor_planner() { return floor_planner_; }

  std::unique_ptr<Circuit<FibonacciConfig3<F>>> WithoutWitness()
      const override {
    return std::make_unique<FibonacciCircuit3>();
  }

  static FibonacciConfig3<F> Configure(ConstraintSystem<F>& meta) {
    return FibonacciChip3<F>::Configure(meta);
  }

  void Synthesize(FibonacciConfig3<F>&& config,
                  Layouter<F>* layouter) const override {
    FibonacciChip3<F> fibonacci_chip3(std::move(config));
    fibonacci_chip3.Assign(layouter, a_, b_, c_);
  }

 private:
  FloorPlanner floor_planner_;
  F a_;
  F b_;
  F c_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT3_H_
