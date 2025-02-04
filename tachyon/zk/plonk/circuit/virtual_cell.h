#ifndef TACHYON_ZK_PLONK_CIRCUIT_VIRTUAL_CELL_H_
#define TACHYON_ZK_PLONK_CIRCUIT_VIRTUAL_CELL_H_

#include <string>

#include "tachyon/export.h"
#include "tachyon/zk/plonk/circuit/column.h"
#include "tachyon/zk/plonk/circuit/rotation.h"

namespace tachyon::zk {

class TACHYON_EXPORT VirtualCell {
 public:
  VirtualCell() = default;
  VirtualCell(const AnyColumn& column, Rotation rotation)
      : column_(column), rotation_(rotation) {}

  const AnyColumn& column() const { return column_; }
  Rotation rotation() const { return rotation_; }

  std::string ToString() const {
    return absl::Substitute("column: $0, rotation: $1", column_.ToString(),
                            rotation_.ToString());
  }

 private:
  AnyColumn column_;
  Rotation rotation_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_VIRTUAL_CELL_H_
