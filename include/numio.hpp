#ifndef NUMIO_H
#define NUMIO_H

// ****************************************************************************

#include <climits>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

// ****************************************************************************

///
/// @brief Flexible C++17 set of template classes for platform-agnostic integer and float data I/O.
///
/// @param NUMIO_DEFAULT_ENDIAN_V `NumIO::Endian` macro parameter setting the default endianness for (un)packing data
///        when not explicitly specified by the user. Defaults to `NumIO::Endian::LITTLE`.
/// @param NUMIO_DEFAULT_ALIGN_V Boolean macro parameter setting the default byte alignment for (un)packing data when
///        not explicitly specified by the user. Defaults to `false`.
/// @param NUMIO_SYSTEM_ENDIANNESS_V Integer macro parameter defining the target system endianness. Defaults to the
///        endianness of the compiling system when not specified.
///        Possible values are:
///         - `LITTLE_ENDIAN` or as integer value `1234`
///         - `BIG_ENDIAN` or as integer value `4321`
/// @param NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V Boolean macro parameter alternative to `NUMIO_SYSTEM_ENDIANNESS_V` for
///        defining the target system endianness.
///
namespace NumIO
{
    namespace {
        // Quick and dirty constexpr ceil function for positive numbers, since ceil is officially not
        // constexpr before C++23 but GCC implemented it anyways already
        static constexpr int __positive_ceil(float f)
        {
            int i = static_cast<int>(f);
            if (f != static_cast<int>(i)) {
                i++;
            }
            return i;
        };

        // https://stackoverflow.com/a/23782939
        static constexpr unsigned int __floorlog2(unsigned int x)
        {
            return x == 1 ? 0 : 1+__floorlog2(x >> 1);
        }

        template <typename FLOAT_T>
        static constexpr unsigned int __get_n_bits_exponent_for_typename() {
            static_assert(std::is_floating_point_v<FLOAT_T>, "Template parameter FLOAT_T must be a float type!");
            return __floorlog2(std::numeric_limits<FLOAT_T>::max_exponent) + 1;
        }

        template <typename FLOAT_T>
        static constexpr unsigned int __get_n_bits_fraction_for_typename() {
            static_assert(std::is_floating_point_v<FLOAT_T>, "Template parameter FLOAT_T must be a float type!");
            return std::numeric_limits<FLOAT_T>::digits - 1;
        }

        // Compile-time way to check the endianness of the system that is executing the compiler.
        // It's impossible to determine the endianness of the *target* system, so it's necessary to
        // define NUMIO_SYSTEM_ENDIANNESS_V or NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V when cross-compiling.
        static constexpr bool __IS_SYSTEM_LITTLE_ENDIAN = []
        {
            // There's some GCC and Clang/LLVM header that sets these, but are not defined on MSVC. Therefore
            // define these macros temporarily ourselves if not defined
            #if !(defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN))
                #define LITTLE_ENDIAN 1234
                #define BIG_ENDIAN 4321
                #define NUMIO_UNDEFINE_ENDIANNESS
            #endif

            #if defined(NUMIO_SYSTEM_ENDIANNESS_V)
                #if NUMIO_SYSTEM_ENDIANNESS_V == LITTLE_ENDIAN
                    return true;
                #elif NUMIO_SYSTEM_ENDIANNESS_V == BIG_ENDIAN
                    return false;
                #else
                    #error "NUMIO_SYSTEM_ENDIANNESS_V contains an unsupported value!"
                    return false;
                #endif
            #elif defined(NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V)
                static_assert(std::is_same<decltype(NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V), bool>::value, "NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V must be a bool type!");
                return NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V;
            #else
                #ifndef NUMIO_IGNORE_AUTO_ENDIAN
                    #pragma message(\
                        "NUMIO_SYSTEM_ENDIANNESS_V or NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V was not set. The endianness of the current system is used instead."\
                        "This message can be surpressed by including \"numio/native.hpp\" or by defining NUMIO_IGNORE_AUTO_ENDIAN."\
                    )
                #endif
                const int value = 1;
                return static_cast<const unsigned char&>(value) == 1;
            #endif

            #ifdef NUMIO_UNDEFINE_ENDIANNESS
                #undef LITTLE_ENDIAN
                #undef BIG_ENDIAN
            #endif
        }();
    }

    ///
    /// @brief Endian modes supported by NumIO.
    ///
    enum class Endian
    {
        LITTLE  = 0,
        BIG     = 1,
        NATIVE  = !__IS_SYSTEM_LITTLE_ENDIAN,
        NETWORK = BIG,
    };

    #ifndef NUMIO_DEFAULT_ENDIAN_V
        #define NUMIO_DEFAULT_ENDIAN_V Endian::LITTLE
    #endif
    #ifndef NUMIO_DEFAULT_ALIGN_V
        #define NUMIO_DEFAULT_ALIGN_V false
    #endif

    ///
    /// @brief Template class for doing integer data I/O.
    ///
    /// @tparam INT_T Integer type to (un)pack.
    /// @tparam N_BITS Specifies the data to (un)pack as an integer with a given amount of bits. `INT_T` will become
    ///         the container to store the value in.
    /// @tparam ALIGNED_V Specifies if the data to (un)pack is aligned to match up with the amount of bytes as used by
    ///         the container type `INT_T`.
    ///
    template <typename INT_T, unsigned int N_BITS=sizeof(INT_T)*8, bool ALIGNED_V=NUMIO_DEFAULT_ALIGN_V>
    class IntIO
    {
        static_assert(std::is_integral_v<INT_T>, "Template parameter INT_T must be an integer type!");


        // :: PRIVATE ATTRIBUTES & HELPER FUNCTIONS :: //
        private:

        static constexpr short _N_CONTAINER_BITS = []{
            auto bits = std::numeric_limits<INT_T>::digits;
            if (std::is_signed_v<INT_T>)
                bits++;
            return bits;
        }();
        static_assert(_N_CONTAINER_BITS >= N_BITS, "N_BITS cannot be bigger than the amount of bits that integer container INT_T can store!");

        static constexpr short _N_DATA_BYTES = __positive_ceil(N_BITS / 8.0);

        // For padding bytes
        static constexpr short _N_ALIGN_BYTES = []{
            if constexpr (N_BITS > 8)
            {
                // constexpr short N_ALIGN_BYTES = (_N_CONTAINER_BITS - N_BITS) % 16 / 8;
                constexpr short N_ALIGN_BYTES = (_N_CONTAINER_BITS - N_BITS) / 8;
                return ALIGNED_V
                    ? N_ALIGN_BYTES
                    : 0;
            }
            else
                return 0;
        }();

        static constexpr INT_T _VALUE_MASK = []{
            if (_N_CONTAINER_BITS == N_BITS) {
                return ~static_cast<INT_T>(0);
            }
            INT_T mask = 0;
            for (int i=0; i<N_BITS; i++) {
                mask |= (static_cast<INT_T>(1) << i);
            }
            return mask;
        }();

        static constexpr short _get_endianness_offset(Endian endianness)
        {
            return (endianness == Endian::BIG) ^ !__IS_SYSTEM_LITTLE_ENDIAN
                ? N_IO_BYTES - 1
                : 0;
        }


        // :: PUBLIC ATTRIBUTES :: //
        public:

        ///
        /// @brief The amount of bytes used for the packed data.
        ///
        static constexpr int N_IO_BYTES = _N_DATA_BYTES + _N_ALIGN_BYTES;


        // :: UNPACKING FUNCTIONS :: //
        public:

        ///
        /// @brief Unpacks an integer from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param bytes Vector of bytes to read from.
        /// @param offset Offset in bytes to extract from of the vector.
        /// @return Integer value.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static INT_T unpack(std::vector<std::uint8_t>& bytes, const unsigned int offset=0)
        {
            INT_T result = 0;

            static constexpr auto endianness_offset = _get_endianness_offset(ENDIANNESS_V);

            // Copy the input bytes into the result
            if constexpr (endianness_offset) // Reversed order
            {
                for (short i=0; i<_N_DATA_BYTES; i++) {
                    result |= static_cast<INT_T>(bytes[endianness_offset-i+offset]) << (i * 8);
                }
            }
            else
            {
                for (short i=0; i<_N_DATA_BYTES; i++) {
                    result |= static_cast<INT_T>(bytes[i+offset]) << (i * 8);
                }
            }

            if constexpr (N_BITS != _N_CONTAINER_BITS)
            {
                // Bitmask to clear out unwanted/garbage data in the final read byte
                result &= _VALUE_MASK;

                if constexpr (std::is_signed_v<INT_T>)
                {
                    // Sign extend the result number if number should be negative
                    static constexpr unsigned int MSB = endianness_offset ? _N_ALIGN_BYTES : _N_DATA_BYTES - 1;
                    static constexpr std::uint8_t SIGN_BIT_MASK = (1 << ((N_BITS % 8) + 7) % 8);
                    if (bytes[MSB+offset] & SIGN_BIT_MASK)
                        result |= ~_VALUE_MASK;
                }
            }

            return result;
        }

        ///
        /// @brief Unpacks an integer from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param bytes Vector of bytes to read from.
        /// @param offset Offset in bytes to extract from of the vector.
        /// @return Integer value.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static INT_T unpack(std::vector<std::int8_t>& bytes, const unsigned int offset=0)
        { return unpack<ENDIANNESS_V>(*reinterpret_cast<std::vector<std::uint8_t>*>(&bytes), offset); }


        // :: PACKING FUNCTIONS :: //
        public:

        ///
        /// @brief Packs an integer from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param value Input integer value.
        /// @param bytes Vector of bytes to write to.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static void pack(INT_T value, std::vector<std::uint8_t>& bytes)
        {
            // Extend vector for packed data
            auto offset = bytes.size();
            bytes.resize(offset+N_IO_BYTES);

            // Isolate the bits that we're interested in
            value &= _VALUE_MASK;

            constexpr auto endianness_offset = _get_endianness_offset(ENDIANNESS_V);

            // Copy the bits into the byte vector
            if constexpr (endianness_offset) // Reversed order
            {
                for (short i=0; i<_N_DATA_BYTES; i++)
                    bytes[endianness_offset-i] = static_cast<std::uint8_t>((value >> (i * 8)) & 0xFF);
            }
            else
            {
                for (short i=0; i<N_IO_BYTES; i++)
                    bytes[i] = static_cast<std::uint8_t>((value >> (i * 8)) & 0xFF);
            }

            return;
        }

        ///
        /// @brief Packs an integer from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param value Input integer value.
        /// @param bytes Vector of bytes to write to.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static void pack(INT_T value, std::vector<std::int8_t>& bytes)
        { pack<ENDIANNESS_V>(value, *reinterpret_cast<std::vector<std::uint8_t>*>(&bytes)); }


        // :: I/O FUNCTIONS :: //
        public:

        ///
        /// @brief Reads an integer from a binary stream.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param s Binary stream to read from.
        /// @return Integer value.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static INT_T read(std::istream& s)
        {
            std::vector<std::uint8_t> buffer(N_IO_BYTES);
            s.read(reinterpret_cast<char*>(buffer.data()), N_IO_BYTES);
            return unpack<ENDIANNESS_V>(buffer, 0);
        }

        ///
        /// @brief Writes an integer to a binary stream.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param value Input integer value.
        /// @param s Binary stream to write to.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static void write(INT_T value, std::ostream& s)
        {
            std::vector<std::uint8_t> buffer;
            buffer.reserve(N_IO_BYTES);
            pack<ENDIANNESS_V>(value, buffer);
            s.write(reinterpret_cast<char*>(&buffer[0]), N_IO_BYTES);
            return;
        }
    };


    ///
    /// @brief Template class for doing floating point data I/O.
    ///
    /// @tparam FLOAT_T Float type to (un)pack.
    /// @tparam INT_IO_T Integer type used as intermediate storage for I/O retrieval and storage.
    /// @tparam N_BITS_EXPONENT Specifies the amount of bits of the exponent part of the floating point data. Defaults to an
    ///         automatically calculated value if `FLOAT_T` is a built-in type or implements `std::numeric_limits<FLOAT_T>`.
    /// @tparam N_BITS_FRACTION Specifies the amount of bits of the fraction part of the floating point data. Defaults to an
    ///         automatically calculated value if `FLOAT_T` is a built-in type or implements `std::numeric_limits<FLOAT_T>`.
    /// @tparam ALIGNED_V Specifies if the data to (un)pack is aligned to match up with the amount of bytes as used by
    ///         the intermediate storage type `INT_IO_T`.
    ///
    template <typename FLOAT_T,
              typename INT_IO_T,
              unsigned int N_BITS_EXPONENT=__get_n_bits_exponent_for_typename<FLOAT_T>(),
              unsigned int N_BITS_FRACTION=__get_n_bits_fraction_for_typename<FLOAT_T>(),
              bool ALIGNED_V=NUMIO_DEFAULT_ALIGN_V
             >
    class FloatIO
    {
        static_assert(std::is_floating_point_v<FLOAT_T>, "Template parameter FLOAT_T must be a float type!");
        static_assert(std::is_integral_v<INT_IO_T>, "Template parameter INT_IO_T must be an integer type!");


        // :: PRIVATE ATTRIBUTES & HELPER FUNCTIONS ::
        private:

        // Takes care of asserting amount of bits not being more than being able to be stored by FLOAT_T and INT_IO_T
        using _INTIO_TYPE = IntIO<INT_IO_T, (1+N_BITS_EXPONENT+N_BITS_FRACTION), ALIGNED_V>;

        static constexpr short _N_DATA_BYTES = sizeof(FLOAT_T);

        static constexpr int EXPONENT_MASK = (static_cast<int>(1) << N_BITS_EXPONENT) - 1;
        static constexpr INT_IO_T FRACTION_MASK = (static_cast<INT_IO_T>(1) << N_BITS_FRACTION) - 1;

        static constexpr int EXPONENT_BIAS = (static_cast<int>(1) << (N_BITS_EXPONENT - 1)) - 1;
        static constexpr INT_IO_T FRACTION_DENOMINATOR = FRACTION_MASK + 1;

        static constexpr int EXPONENT_MAX = EXPONENT_BIAS;
        static constexpr int EXPONENT_MIN = -EXPONENT_BIAS + 1;

        static constexpr int MIN_VAL_EXPONENT_NORMALIZED = EXPONENT_MIN - N_BITS_FRACTION - 1; // -1 for normalized


        // :: PUBLIC ATTRIBUTES :: //
        public:

        ///
        /// @brief The amount of bytes used for the packed data.
        ///
        const static int N_IO_BYTES = _N_DATA_BYTES;


        // :: UNPACKING FUNCTIONS :: //
        public:

        ///
        /// @brief Unpacks a float from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param bytes Vector of bytes to read from.
        /// @param offset Offset in bytes to extract from of the vector.
        /// @return Float value.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static FLOAT_T unpack(std::vector<std::uint8_t>& bytes, const unsigned int offset=0)
        {
            INT_IO_T binary_data = _INTIO_TYPE::template unpack<ENDIANNESS_V>(bytes, offset);

            INT_IO_T fraction_numerator = binary_data & FRACTION_MASK;
            int exponent = (binary_data >> N_BITS_FRACTION) & EXPONENT_MASK;

            // Multiply result with this to make negative if sign bit set
            int apply_sign = 1 - (
                // unsigned char sign = (binary_data >> (FRACTION_BITS + EXPONENT_BITS)) & 1;
                ((binary_data >> (N_BITS_FRACTION + N_BITS_EXPONENT)) & 1)
                * 2
            );

            // Special values
            if (exponent == EXPONENT_MASK) {
                if (fraction_numerator == 0) {
                    return std::numeric_limits<FLOAT_T>::infinity() * apply_sign;
                }
                // Else
                return std::numeric_limits<FLOAT_T>::quiet_NaN();
            }

            // If the exponent is all zeros, but the mantissa is not then the value is a denormalized number.
            // This means this number does not have an assumed leading one before the binary point.
            int denormalized_adjust = (exponent != 0) & 1;


            FLOAT_T result = (
                std::pow(2.0, -EXPONENT_BIAS + exponent + 1 - denormalized_adjust) // or use math.exp2(x)
                * (denormalized_adjust + static_cast<FLOAT_T>(fraction_numerator)/FRACTION_DENOMINATOR)
                * apply_sign
            );

            return result;
        }

        ///
        /// @brief Unpacks a float from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param bytes Vector of bytes to read from.
        /// @param offset Offset in bytes to extract from of the vector.
        /// @return Float value.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static FLOAT_T unpack(std::vector<std::int8_t>& bytes, const unsigned int offset=0)
        { return unpack<ENDIANNESS_V>(*reinterpret_cast<std::vector<std::uint8_t>*>(&bytes), offset); }


        // :: PACKING FUNCTIONS :: //
        public:

        ///
        /// @brief Packs a float from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param value Input float value.
        /// @param bytes Vector of bytes to write to.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static void pack(FLOAT_T value, std::vector<std::uint8_t>& bytes)
        {
            int sign = 0;
            int exponent = 0; // int since frexp() expects int as argument. No floating point format comes close to needing more than 32 bits for exponent
            INT_IO_T fraction_numerator = 0;

            // Special values
            if (std::isinf(value)) {
                exponent = EXPONENT_MASK;
                sign = (value < 0) & 1;
            }
            else if (std::isnan(value)) {
                exponent = EXPONENT_MASK;
                // Topmost bit of the fraction is used to set non-signaling/quiet NaN. Signaling NaNs would've
                // caused exceptions in the program, handled by the FPU. All other bits stay 0
                fraction_numerator = FRACTION_DENOMINATOR >> 1;
            }
            // Note that this condition captures both 0.0 and -0.0
            else if (value == 0.0) {
                // Use copysign to differentiate between 0.0 and -0.0
                sign = (std::copysign(1.0, value) < 0) & 1; // Ruby .negative? or similar also works
            }
            // Non-special values
            else
            {
                // Extract sign bit and make input value absolute if negative
                // Note that
                if (value < 0.0) {
                    sign = 1;
                    value = -value;
                }
                else {
                    sign = 0;
                }

                FLOAT_T fraction = std::frexp(value, &exponent);
                if (fraction < 0.5 || fraction >= 1.0) {
                    throw std::runtime_error("frexp() result out of range!");
                }

                // Normalize fraction to be in the range [1.0, 2.0]
                fraction *= 2.0;
                exponent -= 1;

                if (exponent > EXPONENT_MAX) {
                    throw std::runtime_error("The floating point value is too large to be packed into the designated format!");
                }
                else if (exponent < MIN_VAL_EXPONENT_NORMALIZED) {
                    // Underflow to zero
                    fraction = 0;
                    exponent = 0;
                }
                else if (exponent < EXPONENT_MIN) {
                    // Gradual underflow
                    fraction = std::ldexp(fraction, -EXPONENT_MIN + exponent);
                    exponent = 0;
                }
                else if (!(exponent == 0 && fraction == 0.0)) {
                    exponent += EXPONENT_BIAS;
                    fraction -= 1.0; // Get rid of leading 1
                }
                else {
                    throw std::runtime_error("Reached an invalid or unsupported scenario while packing floating point value!");
                }

                fraction *= FRACTION_DENOMINATOR; // Turn into fractional numerator
                fraction_numerator = static_cast<INT_IO_T>(fraction); // Truncate numerator, can also use floor() but probably slower
                // Round to even
                if ((fraction - fraction_numerator > 0.5) ||
                    ((fraction - fraction_numerator == 0.5) && (fraction_numerator % 2 == 1))
                ) {
                    fraction_numerator += 1;
                    if (fraction_numerator == FRACTION_DENOMINATOR) {
                        // Fraction overflows, carry to exponent
                        fraction_numerator = 0;
                        exponent += 1;
                        if (exponent >= EXPONENT_MASK) {
                            throw std::runtime_error("The floating point value is too large to be packed into the designated format!");
                        }
                    }
                }
            }

            INT_IO_T binary_data = (static_cast<INT_IO_T>(sign) << (N_BITS_EXPONENT + N_BITS_FRACTION)) |
                                   (static_cast<INT_IO_T>(exponent & EXPONENT_MASK) << N_BITS_FRACTION) |
                                   (fraction_numerator & FRACTION_MASK);

            _INTIO_TYPE::template pack<ENDIANNESS_V>(binary_data, bytes);

            return;
        }

        ///
        /// @brief Packs a float from a vector of bytes.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param value Input float value.
        /// @param bytes Vector of bytes to write to.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static void pack(FLOAT_T value, std::vector<std::int8_t>& bytes)
        { pack<ENDIANNESS_V>(value, *reinterpret_cast<std::vector<std::uint8_t>*>(&bytes)); }


        // :: I/O FUNCTIONS :: //
        public:

        ///
        /// @brief Reads a float from a binary stream.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param s Binary stream to read from.
        /// @return Float value.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static FLOAT_T read(std::istream& s)
        {
            std::vector<std::uint8_t> buffer(N_IO_BYTES);
            s.read(reinterpret_cast<char*>(buffer.data()), N_IO_BYTES);
            return unpack<ENDIANNESS_V>(buffer, 0);
        }

        ///
        /// @brief Writes an float to a binary stream.
        ///
        /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
        /// @param value Input float value.
        /// @param s Binary stream to write to.
        ///
        template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
        static void write(FLOAT_T value, std::ostream& s)
        {
            std::vector<std::uint8_t> buffer;
            buffer.reserve(N_IO_BYTES);
            pack<ENDIANNESS_V>(value, buffer);
            s.write(reinterpret_cast<char*>(&buffer[0]), N_IO_BYTES);
            return;
        }
    };

}

// ****************************************************************************

#endif /* NUMIO_H */
