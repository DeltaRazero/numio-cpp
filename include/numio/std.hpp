#ifndef NUMIO_STD_H
#define NUMIO_STD_H

// ****************************************************************************

#include <cstdint>
#include "../numio.hpp"

namespace NumIO
{
    using  i8_IO  = IntIO<std:: int8_t>;
    using i16_IO  = IntIO<std::int16_t>;
    using i24_IO  = IntIO<std::int32_t, 24, false>;
    using i24a_IO = IntIO<std::int32_t, 24, true>;
    using i32_IO  = IntIO<std::int32_t>;
    using i64_IO  = IntIO<std::int64_t>;

    using  u8_IO  = IntIO<std:: uint8_t>;
    using u16_IO  = IntIO<std::uint16_t>;
    using u24_IO  = IntIO<std::uint32_t, 24, false>;
    using u24a_IO = IntIO<std::uint32_t, 24, true>;
    using u32_IO  = IntIO<std::uint32_t>;
    using u64_IO  = IntIO<std::uint64_t>;

    using fp16_IO = FloatIO<float,  std::uint16_t, 5, 10>;
    using fp32_IO = FloatIO<float,  std::uint32_t>;
    using fp64_IO = FloatIO<double, std::uint64_t>;
}

// ****************************************************************************

#endif /* NUMIO_STD_H */
