#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT_H_
#define TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT_H_

#include <memory>
#include <utility>

#include "tachyon/zk/plonk/circuit/assigned_cell.h"
#include "tachyon/zk/plonk/circuit/circuit.h"
#include "tachyon/zk/plonk/circuit/virtual_cells.h"

namespace tachyon::zk {

template <typename F>
struct FibonacciConfig {
  using Field = F;

  FibonacciConfig(const AdviceColumnKey& col_a, const AdviceColumnKey& col_b,
                  const AdviceColumnKey& col_c, const Selector& selector,
                  const InstanceColumnKey& instance)
      : col_a(col_a),
        col_b(col_b),
        col_c(col_c),
        selector(selector),
        instance(instance) {}

  AdviceColumnKey col_a;
  AdviceColumnKey col_b;
  AdviceColumnKey col_c;
  Selector selector;
  InstanceColumnKey instance;
};

template <typename F>
class FibonacciChip {
 public:
  explicit FibonacciChip(FibonacciConfig<F>&& config)
      : config_(std::move(config)) {}

  static FibonacciConfig<F> Configure(ConstraintSystem<F>& meta) {
    AdviceColumnKey col_a = meta.CreateAdviceColumn();
    AdviceColumnKey col_b = meta.CreateAdviceColumn();
    AdviceColumnKey col_c = meta.CreateAdviceColumn();
    Selector selector = meta.CreateSimpleSelector();
    InstanceColumnKey instance = meta.CreateInstanceColumn();

    meta.EnableEquality(col_a);
    meta.EnableEquality(col_b);
    meta.EnableEquality(col_c);
    meta.EnableEquality(instance);

    meta.CreateGate(
        "add", [&selector, &col_a, &col_b, &col_c](VirtualCells<F>& meta) {
          //
          // col_a | col_b | col_c | selector
          //   a      b        c       s
          //
          std::unique_ptr<Expression<F>> s = meta.QuerySelector(selector);
          std::unique_ptr<Expression<F>> a =
              meta.QueryAdvice(col_a, Rotation::Cur());
          std::unique_ptr<Expression<F>> b =
              meta.QueryAdvice(col_b, Rotation::Cur());
          std::unique_ptr<Expression<F>> c =
              meta.QueryAdvice(col_c, Rotation::Cur());

          std::vector<Constraint<F>> constraints;
          constraints.emplace_back(
              std::move(s) * (std::move(a) + std::move(b) - std::move(c)));
          return constraints;
        });

    return FibonacciConfig<F>(std::move(col_a), std::move(col_b),
                              std::move(col_c), std::move(selector),
                              std::move(instance));
  }

  std::tuple<AssignedCell<F>, AssignedCell<F>, AssignedCell<F>> AssignFirstRow(
      Layouter<F>* layouter) const {
    std::tuple<AssignedCell<F>, AssignedCell<F>, AssignedCell<F>> ret;
    layouter->AssignRegion("first row", [this, &ret](Region<F>& region) {
      config_.selector.Enable(region, 0);

      const AssignedCell<F> a_cell = region.AssignAdviceFromInstance(
          "f(0)", config_.instance, 0, config_.col_a, 0);
      const AssignedCell<F> b_cell = region.AssignAdviceFromInstance(
          "f(1)", config_.instance, 1, config_.col_b, 0);
      const AssignedCell<F> c_cell = region.AssignAdvice(
          "a + b", config_.col_c, 0,
          [&a_cell, &b_cell]() { return a_cell.value() + b_cell.value(); });

      ret = std::make_tuple(a_cell, b_cell, c_cell);
    });
    return ret;
  }

  AssignedCell<F> AssignRow(Layouter<F>* layouter,
                            const AssignedCell<F>& prev_b,
                            const AssignedCell<F>& prev_c) const {
    AssignedCell<F> ret;
    layouter->AssignRegion(
        "next row", [this, &ret, &prev_b, &prev_c](Region<F>& region) {
          config_.selector.Enable(region, 0);

          // Copy the value from b & c in previous row to a & b in current row
          const AssignedCell<F> a_cell =
              prev_b.CopyAdvice("a", region, config_.col_a, 0);
          const AssignedCell<F> b_cell =
              prev_c.CopyAdvice("b", region, config_.col_b, 0);

          const AssignedCell<F> c_cell = region.AssignAdvice(
              "a + b", config_.col_c, 0,
              [&a_cell, &b_cell]() { return a_cell.value() + b_cell.value(); });

          ret = c_cell;
        });
    return ret;
  }

  void ExposePublic(Layouter<F>* layouter, const AssignedCell<F>& cell,
                    size_t row) const {
    layouter->ConstrainInstance(cell.cell(), config_.instance(), row);
  }

 private:
  FibonacciConfig<F> config_;
};

template <typename F, template <typename> class _FloorPlanner>
class FibonacciCircuit : public Circuit<FibonacciConfig<F>> {
 public:
  using FloorPlanner = _FloorPlanner<FibonacciCircuit<F, _FloorPlanner>>;

  FloorPlanner& floor_planner() { return floor_planner_; }

  std::unique_ptr<Circuit<FibonacciConfig<F>>> WithoutWitness() const override {
    return std::make_unique<FibonacciCircuit>();
  }

  static FibonacciConfig<F> Configure(ConstraintSystem<F>& meta) {
    return FibonacciChip<F>::Configure(meta);
  }

  void Synthesize(FibonacciConfig<F>&& config,
                  Layouter<F>* layouter) const override {
    FibonacciChip<F> fibonacci_chip(std::move(config));

    std::tuple<AssignedCell<F>, AssignedCell<F>, AssignedCell<F>> first_row =
        fibonacci_chip.AssignFirstRow(layouter->Namespace("first row").get());

    AssignedCell<F> prev_b = std::get<1>(first_row);
    AssignedCell<F> prev_c = std::get<2>(first_row);

    for (size_t i = 3; i < 10; ++i) {
      AssignedCell<F> c_cell = fibonacci_chip.AssignRow(
          layouter->Namespace("next row"), prev_b, prev_c);
      prev_b = prev_c;
      prev_c = c_cell;
    }

    fibonacci_chip.ExposePublic(layouter->Namespace("out").get(), prev_c, 2);
  }

 private:
  FloorPlanner floor_planner_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT_H_
