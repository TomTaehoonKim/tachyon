// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_ASSIGNMENT_H_
#define TACHYON_ZK_PLONK_CIRCUIT_ASSIGNMENT_H_

#include <stddef.h>

#include <optional>
#include <string>

#include "tachyon/base/functional/callback.h"
#include "tachyon/math/base/rational_field.h"
#include "tachyon/zk/plonk/circuit/challenge.h"
#include "tachyon/zk/plonk/circuit/column.h"
#include "tachyon/zk/plonk/circuit/selector.h"
#include "tachyon/zk/plonk/error.h"

namespace tachyon::zk {

template <typename F>
class Assignment {
 public:
  using AnnotationCallback = base::OnceCallback<std::string()>;
  using AssignCallback = base::OnceCallback<math::RationalField<F>()>;

  // Creates a new region and enters into it.
  //
  // Panics if we are currently in a region (if |ExitRegion()| was not called).
  //
  // Not intended for downstream consumption; use |Layouter::AssignRegion()|
  // instead.
  virtual void EnterRegion(AnnotationCallback annotation) {}

  // Allows the developer to include an annotation for a specific column
  // within a |Region|.
  //
  // This is usually useful for debugging circuit failures.
  virtual void AnnotateColumn(AnnotationCallback annotation,
                              const AnyColumn& column) {}

  // Exits the current region.
  //
  // Panics if we are not currently in a region (if |EnterRegion()| was not
  // called).
  //
  // Not intended for downstream consumption; use |Layouter::AssignRegion()|
  // instead.
  virtual void ExitRegion() {}

  // Enables a selector at the given row.
  virtual Error EnableSelector(AnnotationCallback annotation,
                               const Selector& selector, size_t row) {
    return Error::kNone;
  }

  // Queries the cell of an instance column at a particular absolute row.
  //
  // Returns |Error::kNone| and populates |instance| with the cell's value, if
  // known.
  virtual Error QueryInstance(const InstanceColumn& column, size_t row,
                              Value<F>* instance) {
    return Error::kNone;
  }

  // Assign an advice column value (witness).
  virtual Error AssignAdvice(AnnotationCallback annotation,
                             const AdviceColumn& column, size_t row,
                             AssignCallback assign) {
    return Error::kNone;
  }

  // Assign a fixed value.
  virtual Error AssignFixed(AnnotationCallback annotation,
                            const FixedColumn& column, size_t row,
                            AssignCallback assign) {
    return Error::kNone;
  }

  // Assign two cells to have the same value
  virtual Error Copy(const AnyColumn& left_column, size_t left_row,
                     const AnyColumn& right_column, size_t right_row) {
    return Error::kNone;
  }

  // Fills a fixed |column| starting from the given |row| with value |assign|.
  virtual Error FillFromRow(const FixedColumn& column, size_t row,
                            AssignCallback assign) {
    return Error::kNone;
  }

  // Queries the value of the given challenge.
  //
  // Returns |Error::kChallengeUnavailable| if the current synthesis phase is
  // before the challenge can be queried. Otherwise, it populates |value| with
  // the challenge.
  virtual Error GetChallenge(const Challenge& challenge, Value<F>* value) {
    *value = Value<F>::Unknown();
    return Error::kNone;
  }

  // Creates a new (sub)namespace and enters into it.
  //
  // Not intended for downstream consumption; use |Layouter::namespace()|
  // instead.
  virtual void PushNamespace(AnnotationCallback annotation) {}

  // Exits out of the existing namespace.
  //
  // Not intended for downstream consumption; use |Layouter::namespace()|
  // instead.
  virtual void PopNamespace(const std::optional<std::string>& gadget_name) {}
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_ASSIGNMENT_H_
