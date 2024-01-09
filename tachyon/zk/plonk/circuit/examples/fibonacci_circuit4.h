#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT4_H_S
#define TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT4_H_

#include <memory>
#include <utility>

#include "tachyon/zk/plonk/circuit/assigned_cell.h"
#include "tachyon/zk/plonk/circuit/circuit.h"
#include "tachyon/zk/plonk/circuit/virtual_cells.h"

namespace tachyon::zk {

template <typename F>
struct FibonacciConfig4 {
  using Field = F;

  FibonacciConfig4(std::array<AdviceColumnKey, 3>&& advice,
                   const Selector& s_add, const Selector& s_xor,
                   std::array<LookupTableColumn, 3>&& xor_table,
                   const InstanceColumnKey& instance)
      : advice(std::move(advice)),
        s_add(s_add),
        s_xor(s_xor),
        xor_table(std::move(xor_table)),
        instance(instance) {}

  std::array<AdviceColumnKey, 3> advice;
  Selector s_add;
  Selector s_xor;
  std::array<LookupTableColumn, 3> xor_table;
  InstanceColumnKey instance;
};

template <typename F>
class FibonacciChip4 {
 public:
  explicit FibonacciChip4(FibonacciConfig4<F>&& config)
      : config_(std::move(config)) {}

  static FibonacciConfig4<F> Configure(ConstraintSystem<F>& meta) {
    AdviceColumnKey col_a = meta.CreateAdviceColumn();
    AdviceColumnKey col_b = meta.CreateAdviceColumn();
    AdviceColumnKey col_c = meta.CreateAdviceColumn();
    Selector s_add = meta.CreateSimpleSelector();
    Selector s_xor = meta.CreateComplexSelector();
    InstanceColumnKey instance = meta.CreateInstanceColumn();

    std::array<LookupTableColumn, 3> xor_table = {
        meta.CreateLookupTableColumn(),
        meta.CreateLookupTableColumn(),
        meta.CreateLookupTableColumn(),
    };

    meta.EnableEquality(col_a);
    meta.EnableEquality(col_b);
    meta.EnableEquality(col_c);
    meta.EnableEquality(instance);

    meta.CreateGate(
        "add", [&s_add, &col_a, &col_b, &col_c](VirtualCells<F>& meta) {
          //
          // col_a | col_b | col_c | s_add
          //   a      b        c       s
          //
          std::unique_ptr<Expression<F>> s = meta.QuerySelector(s_add);
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

    meta.Lookup("lookup", [&s_xor, &col_a, &col_b, &col_c,
                           &xor_table](VirtualCells<F>& meta) {
      std::unique_ptr<Expression<F>> s = meta.QuerySelector(s_xor);
      std::unique_ptr<Expression<F>> lhs =
          meta.QueryAdvice(col_a, Rotation::Cur());
      std::unique_ptr<Expression<F>> rhs =
          meta.QueryAdvice(col_b, Rotation::Cur());
      std::unique_ptr<Expression<F>> out =
          meta.QueryAdvice(col_c, Rotation::Cur());

      std::vector<std::pair<std::unique_ptr<Expression<F>>, LookupTableColumn>>
          ret;
      ret.emplace_back(std::move(s) * std::move(lhs), xor_table[0]);
      ret.emplace_back(std::move(s) * std::move(rhs), xor_table[1]);
      ret.emplace_back(std::move(s) * std::move(out), xor_table[2]);
      return ret;
    });

    return FibonacciConfig4<F>(std::move({col_a, col_b, col_c}),
                               std::move(s_add), std::move(s_xor),
                               std::move(xor_table), std::move(instance));
  }

  void LoadTable(Layouter<F>* layouter) const {
    layouter->AssignLookupTable("xor_table", [this](LookupTable<F>& table) {
      size_t idx = 0;
      for (size_t lhs = 0; lhs < 32; ++lhs) {
        for (size_t rhs = 0; rhs < 32; ++rhs) {
          table.AssignCell("lhs", config_.xor_table[0], idx,
                           [lhs]() { return Value<F>::Known(F::From(lhs)); });
          table.AssignCell("rhs", config_.xor_table[1], idx,
                           [rhs]() { return Value<F>::Known(F::From(rhs)); });
          table.AssignCell(
              "lhs ^ rhs", config_.xor_table[2], idx,
              [lhs, rhs]() { return Value<F>::Known(F::From(lhs ^ rhs)); });
          ++idx;
        }
      }
    });
  }

  AssignedCell<F> Assign(Layouter<F>* layouter, size_t n_rows) const {
    layouter->AssignRegion("entire circuit", [this, n_rows](Region<F>& region) {
      config_.s_add.Enable(region, 0);

      // assign first row
      AssignedCell<F> a_cell = region.AssignAdviceFromInstance(
          "1", config_.instance, 0, config_.advice[0], 0);
      AssignedCell<F> b_cell = region.AssignAdviceFromInstance(
          "1", config_.instance, 1, config_.advice[1], 0);
      AssignedCell<F> c_cell = region.AssignAdvice(
          "add", config_.advice[2], 0,
          [a_cell, b_cell]() { return a_cell.value() + b_cell.value(); });

      // assign the rest of rows
      for (size_t row = 1; row < n_rows; ++row) {
        b_cell.CopyAdvice("a", region, config_.advice[0], row);
        c_cell.CopyAdvice("b", region, config_.advice[1], row);

        AssignedCell<F> new_c_cell;
        if (row % 2 == 0) {
          config_.s_add.Enable(region, row);
          new_c_cell = region.AssignAdvice(
              "advice", config_.advice[2], row,
              [b_cell, c_cell]() { return b_cell.value() + c_cell.value(); });
        } else {
          config_.s_xor.Enable(region, row);
          new_c_cell = region.AssignAdvice(
              "advice", config_.advice[2], row, [b_cell, c_cell]() {
                return b_cell.value().AndThen(
                    [&b = c_cell.value()](const F& a) {
                      uint64_t a_val = a[F::N - 1];
                      uint64_t b_val = b[F::N - 1];
                      return F(a_val ^ b_val);
                    });
              });
        }

        b_cell = c_cell;
        c_cell = new_c_cell;
      }

      return c_cell;
    });
  }

  void ExposePublic(Layouter<F>* layouter, const AssignedCell<F>& cell,
                    size_t row) const {
    layouter->ConstrainInstance(cell.cell(), config_.instance(), row);
  }

 private:
  FibonacciConfig4<F> config_;
};

template <typename F, template <typename> class _FloorPlanner>
class FibonacciCircuit4 : public Circuit<FibonacciConfig4<F>> {
 public:
  using FloorPlanner = _FloorPlanner<FibonacciCircuit4<F, _FloorPlanner>>;

  FloorPlanner& floor_planner() { return floor_planner_; }

  std::unique_ptr<Circuit<FibonacciConfig4<F>>> WithoutWitness()
      const override {
    return std::make_unique<FibonacciCircuit4>();
  }

  static FibonacciConfig4<F> Configure(ConstraintSystem<F>& meta) {
    return FibonacciChip4<F>::Configure(meta);
  }

  void Synthesize(FibonacciConfig4<F>&& config,
                  Layouter<F>* layouter) const override {
    FibonacciChip4<F> fibonacci_chip4(std::move(config));
    fibonacci_chip4.LoadTable(layouter->Namespace("lookup table").get());
    AssignedCell<F> out_cell =
        fibonacci_chip4.Assign(layouter->Namespace("entire table").get(), 8);
    fibonacci_chip4.ExposePublic(layouter->Namespace("out").get(), out_cell, 2);
  }

 private:
  FloorPlanner floor_planner_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXAMPLES_FIBONACCI_CIRCUIT4_H_
