// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_ASSIGNED_CELL_H_
#define TACHYON_ZK_PLONK_CIRCUIT_ASSIGNED_CELL_H_

#include <string>

#include "tachyon/zk/plonk/circuit/cell.h"
#include "tachyon/zk/value.h"

namespace tachyon::zk {

// An assigned cell.
template <typename F>
class AssignedCell {
 public:
  AssignedCell() = default;
  AssignedCell(const Cell& cell, const Value<F>& value)
      : cell_(cell), value_(value) {}

  const Cell& cell() const { return cell_; }
  const Value<F>& value() const { return value_; }

  std::string ToString() const {
    return absl::Substitute("{cell: $0, value: $1}", cell_.ToString(),
                            value_.ToString());
  }

 private:
  Cell cell_;
  Value<F> value_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_ASSIGNED_CELL_H_
