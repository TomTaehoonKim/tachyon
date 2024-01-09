#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT2_H_
#define TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT2_H_

#include <memory>
#include <utility>

#include "tachyon/zk/plonk/circuit/assigned_cell.h"
#include "tachyon/zk/plonk/circuit/circuit.h"
#include "tachyon/zk/plonk/circuit/virtual_cells.h"

namespace tachyon::zk {

template <typename F>
struct FibonacciConfig2 {
  using Field = F;

  FibonacciConfig2(const AdviceColumnKey& advice, const Selector& selector,
                   const InstanceColumnKey& instance)
      : advice(advice), selector(selector), instance(instance) {}

  AdviceColumnKey advice;
  Selector selector;
  InstanceColumnKey instance;
};

template <typename F>
class FibonacciChip2 {
 public:
  explicit FibonacciChip2(FibonacciConfig2<F>&& config)
      : config_(std::move(config)) {}

  static FibonacciConfig2<F> Configure(ConstraintSystem<F>& meta,
                                       const AdviceColumnKey& advice,
                                       const InstanceColumnKey& instance) {
    Selector selector = meta.CreateSimpleSelector();

    meta.EnableEquality(advice);
    meta.EnableEquality(instance);

    meta.CreateGate("add", [&selector, &advice](VirtualCells<F>& meta) {
      //
      // advice | selector
      //   a    |   s
      //   b    |
      //   c    |
      //
      std::unique_ptr<Expression<F>> s = meta.QuerySelector(selector);
      std::unique_ptr<Expression<F>> a =
          meta.QueryAdvice(advice, Rotation::Cur());
      std::unique_ptr<Expression<F>> b =
          meta.QueryAdvice(advice, Rotation::Next());
      std::unique_ptr<Expression<F>> c = meta.QueryAdvice(advice, Rotation(2));
      std::vector<Constraint<F>> constraints;
      constraints.emplace_back(std::move(s) *
                               (std::move(a) + std::move(b) - std::move(c)));
      return constraints;
    });

    return FibonacciConfig2<F>(advice, selector, instance);
  }

  AssignedCell<F> Assign(Layouter<F>* layouter, size_t n_rows) const {
    AssignedCell<F> ret;
    layouter->AssignRegion(
        "entire fibonacci table", [this, &ret, n_rows](Region<F>& region) {
          config_.selector.Enable(region, 0);
          config_.selector.Enable(region, 1);

          AssignedCell<F> a_cell = region.AssignAdviceFromInstance(
              "1", config_.instance, 0, config_.advice, 0);
          AssignedCell<F> b_cell = region.AssignAdviceFromInstance(
              "1", config_.instance, 1, config_.advice, 1);

          for (size_t row = 2; row < n_rows; ++row) {
            if (row < n_rows - 2) {
              config_.selector.Enable(region, row);
            }

            const AssignedCell<F> c_cell = region.AssignAdvice(
                "advice", config_.advice, row, [&a_cell, &b_cell]() {
                  return a_cell.value() + b_cell.value();
                });

            a_cell = b_cell;
            b_cell = c_cell;
          }

          ret = b_cell;
        });
    return ret;
  }

  void ExposePublic(Layouter<F>* layouter, const AssignedCell<F>& cell,
                    size_t row) const {
    layouter->ConstrainInstance(cell.cell(), config_.instance(), row);
  }

 private:
  FibonacciConfig2<F> config_;
};

template <typename F, template <typename> class _FloorPlanner>
class FibonacciCircuit2 : public Circuit<FibonacciConfig2<F>> {
 public:
  using FloorPlanner = _FloorPlanner<FibonacciCircuit2<F, _FloorPlanner>>;

  FloorPlanner& floor_planner() { return floor_planner_; }

  std::unique_ptr<Circuit<FibonacciConfig2<F>>> WithoutWitness()
      const override {
    return std::make_unique<FibonacciCircuit2>();
  }

  static FibonacciConfig2<F> Configure(ConstraintSystem<F>& meta) {
    AdviceColumnKey advice = meta.CreateAdviceColumn();
    InstanceColumnKey instance = meta.CreateInstanceColumn();
    return FibonacciChip2<F>::Configure(meta, advice, instance);
  }

  void Synthesize(FibonacciConfig2<F>&& config,
                  Layouter<F>* layouter) const override {
    FibonacciChip2<F> fibonacci_chip2(std::move(config));

    AssignedCell<F> out_cell =
        fibonacci_chip2.Assign(layouter->Namespace("entire table").get(), 10);

    fibonacci_chip2.ExposePublic(layouter->Namespace("out").get(), out_cell, 2);
  }

 private:
  FloorPlanner floor_planner_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT2_H_
