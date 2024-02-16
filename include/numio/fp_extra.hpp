#ifndef NUMIO_FP_EXTRA_H
#define NUMIO_FP_EXTRA_H

// ****************************************************************************

#include <cstdint>
#include "../numio.hpp"

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

#endif /* NUMIO_FP_EXTRA_H */
