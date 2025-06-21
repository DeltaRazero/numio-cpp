#ifndef NUMIO_STD_H
#define NUMIO_STD_H

// ****************************************************************************

#include <cstdint>
#include "../numio.hpp"

namespace numio
{
    using  i8_io  = IntIO<std:: int8_t>;
    using i16_io  = IntIO<std::int16_t>;
    using i24_io  = IntIO<std::int32_t, 24, false>;
    using i24a_io = IntIO<std::int32_t, 24, true>;
    using i32_io  = IntIO<std::int32_t>;
    using i64_io  = IntIO<std::int64_t>;

    using  u8_io  = IntIO<std:: uint8_t>;
    using u16_io  = IntIO<std::uint16_t>;
    using u24_io  = IntIO<std::uint32_t, 24, false>;
    using u24a_io = IntIO<std::uint32_t, 24, true>;
    using u32_io  = IntIO<std::uint32_t>;
    using u64_io  = IntIO<std::uint64_t>;

    using f16_io = FloatIO<float,  std::uint16_t, 5, 10>;
    using f32_io = FloatIO<float,  std::uint32_t>;
    using f64_io = FloatIO<double, std::uint64_t>;
}

// ****************************************************************************

#endif /* NUMIO_STD_H */
