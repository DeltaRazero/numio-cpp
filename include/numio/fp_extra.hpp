#pragma once

// *****************************************************************************

#include <cstdint>
#include "../numio.hpp"

namespace numio
{
  /// @brief Google Brain bfloat16 (brain floating point)
  using bf16_io = FloatIO<float, std::uint16_t, 8, 7>;

  /// @brief NVidia TensorFloat
  using nv_tf32_io = FloatIO<float, std::uint32_t, 8, 10>;

  /// @brief AMD fp24
  using amd_fp24_io = FloatIO<float, std::uint32_t, 7, 16>;

  /// @brief Pixar PXR24
  using pxr24_io = FloatIO<float, std::uint32_t, 8, 15>;
}
