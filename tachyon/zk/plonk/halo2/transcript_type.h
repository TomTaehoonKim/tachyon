#ifndef TACHYON_ZK_PLONK_HALO2_TRANSCRIPT_TYPE_H_
#define TACHYON_ZK_PLONK_HALO2_TRANSCRIPT_TYPE_H_

#include <stdint.h>

#include <string>

#include "absl/strings/substitute.h"

#include "tachyon/base/flag/flag_value_traits.h"

namespace tachyon {
namespace zk::plonk::halo2 {

// THE ORDER OF ELEMENTS ARE IMPORTANT!! DO NOT CHANGE!
// This order matches with the constants of c APIs.
// Pleas refer to tachyon/c/zk/plonk/halo2/constants.h for details.
enum class TranscriptType : uint8_t {
  kBlake2b,
  kPoseidon,
  kSha256,
};

}  // namespace zk::plonk::halo2

namespace base {

template <>
class FlagValueTraits<zk::plonk::halo2::TranscriptType> {
 public:
  static bool ParseValue(std::string_view input,
                         zk::plonk::halo2::TranscriptType* value,
                         std::string* reason) {
    if (input == "blake2b") {
      *value = zk::plonk::halo2::TranscriptType::kBlake2b;
    } else {
      *reason = absl::Substitute("Unknown test set: $0", input);
      return false;
    }
    return true;
  }
};

}  // namespace base
}  // namespace tachyon

#endif  // TACHYON_ZK_PLONK_HALO2_TRANSCRIPT_TYPE_H_
