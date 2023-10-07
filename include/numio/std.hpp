#ifndef NUMIO_STD_H
#define NUMIO_STD_H

// ****************************************************************************

#include "../numio.hpp"
#include <cstdint>

namespace NumIO
{
    using  i8_IO = IntIO<std:: int8_t>;
    using i16_IO = IntIO<std::int16_t>;
    using i24_IO = IntIO<std::int32_t, 24>;
    using i32_IO = IntIO<std::int32_t>;
    using i64_IO = IntIO<std::int64_t>;

    using  u8_IO = IntIO<std:: uint8_t>;
    using u16_IO = IntIO<std::uint16_t>;
    using u24_IO = IntIO<std::uint32_t, 24>;
    using u32_IO = IntIO<std::uint32_t>;
    using u64_IO = IntIO<std::uint64_t>;

    using f32_IO = FloatIO<float>;
    using f64_IO = FloatIO<double>;
}

// ****************************************************************************

#endif /* NUMIO_STD_H */
