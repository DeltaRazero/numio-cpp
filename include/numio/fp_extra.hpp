#ifndef FP_EXTRA_H
#define FP_EXTRA_H

// ****************************************************************************

#include "../numio.hpp"
#include <cstdint>

namespace NumIO
{
    /// @brief Google Brain bfloat16 (brain floating point)
    using bfloat16_IO = FloatIO<float, std::uint16_t, 8, 7>;

    /// @brief NVidia TensorFloat
    using nv_tf32_IO = FloatIO<float, std::uint32_t, 8, 10>;

    /// @brief AMD fp24
    using amd_fp24_IO = FloatIO<float, std::uint32_t, 7, 16>;

    /// @brief Pixar PXR24
    using pxr24_IO = FloatIO<float, std::uint32_t, 8, 15>;
}

// ****************************************************************************

#endif /* FP_EXTRA_H */
