// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_CELL_H_
#define TACHYON_ZK_PLONK_CIRCUIT_CELL_H_

#include <stddef.h>

#include <string>

#include "absl/strings/substitute.h"

#include "tachyon/export.h"
#include "tachyon/zk/plonk/circuit/column.h"

namespace tachyon::zk {

// A pointer to a cell within a circuit.
class TACHYON_EXPORT Cell {
 public:
  Cell() = default;
  Cell(size_t region_index, size_t row_offset, const AnyColumn& column)
      : region_index_(region_index), row_offset_(row_offset), column_(column) {}

  size_t region_index() const { return region_index_; }
  size_t row_offset() const { return row_offset_; }
  const AnyColumn& column() const { return column_; }

  std::string ToString() const {
    return absl::Substitute("{region_index: $0, row_offset: $1, column: $2}",
                            region_index_, row_offset_, column_.ToString());
  }

 private:
  // Identifies the region in which this cell resides.
  size_t region_index_;
  // The relative offset of this cell within its region.
  size_t row_offset_;
  // The column of this cell.
  AnyColumn column_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_CELL_H_
