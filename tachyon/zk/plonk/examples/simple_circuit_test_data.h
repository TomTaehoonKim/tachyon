#ifndef TACHYON_ZK_PLONK_EXAMPLES_SIMPLE_CIRCUIT_TEST_DATA_H_
#define TACHYON_ZK_PLONK_EXAMPLES_SIMPLE_CIRCUIT_TEST_DATA_H_

#include <stdint.h>

#include <array>

#include "tachyon/zk/plonk/base/column_key.h"
#include "tachyon/zk/plonk/constraint_system/selector.h"

namespace tachyon::zk::halo2::simple_circuit {

constexpr uint8_t kExpectedProof[] = {
    206, 109, 139, 136, 181, 35,  204, 231, 212, 93,  105, 116, 154, 77,  204,
    23,  71,  148, 11,  151, 126, 145, 6,   150, 171, 185, 254, 230, 41,  136,
    76,  141, 132, 227, 154, 206, 134, 35,  253, 67,  8,   186, 228, 143, 116,
    139, 145, 119, 85,  253, 127, 208, 95,  153, 195, 112, 209, 116, 172, 45,
    15,  175, 128, 142, 206, 109, 139, 136, 181, 35,  204, 231, 212, 93,  105,
    116, 154, 77,  204, 23,  71,  148, 11,  151, 126, 145, 6,   150, 171, 185,
    254, 230, 41,  136, 76,  141, 132, 227, 154, 206, 134, 35,  253, 67,  8,
    186, 228, 143, 116, 139, 145, 119, 85,  253, 127, 208, 95,  153, 195, 112,
    209, 116, 172, 45,  15,  175, 128, 142, 240, 229, 40,  5,   109, 36,  59,
    227, 58,  205, 89,  157, 199, 193, 252, 51,  168, 195, 186, 126,
};

const std::array<AdviceColumnKey, 2> kExpectedAdvice = {
    AdviceColumnKey(0),
    AdviceColumnKey(1),
};
const InstanceColumnKey kExpectedInstance = InstanceColumnKey(0);
const Selector kExpectedSelector = Selector::Simple(0);

}  // namespace tachyon::zk::halo2::simple_circuit

#endif  // TACHYON_ZK_PLONK_EXAMPLES_SIMPLE_CIRCUIT_TEST_DATA_H_
