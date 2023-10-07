#ifndef NUMIO_H
#define NUMIO_H

// ****************************************************************************

#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
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
        constexpr static int __positive_ceil(float f)
        {
            int i = static_cast<int>(f);
            if (f != static_cast<int>(i)) {
                i++;
            }
            return i;
        };

        // Compile-time way to check the endianness of the system that is executing the compiler.
        // It's impossible to determine the endianness of the *target* system, so it's necessary to
        // define NUMIO_SYSTEM_ENDIANNESS_V or NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V when cross-compiling.
        constexpr static bool __IS_SYSTEM_LITTLE_ENDIAN = []
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
    template <typename INT_T, int N_BITS=sizeof(INT_T)*CHAR_BIT, bool ALIGNED_V=NUMIO_DEFAULT_ALIGN_V>
    class IntIO
    {
        static_assert(std::is_integral_v<INT_T>, "Template parameter INT_T must be an integer type!");


        // :: PRIVATE ATTRIBUTES & HELPER FUNCTIONS :: //
        private:

        constexpr static short _N_CONTAINER_BITS = []{
            auto bits = std::numeric_limits<INT_T>::digits;
            if (std::is_signed_v<INT_T>)
                bits++;
            return bits;
        }();
        static_assert(_N_CONTAINER_BITS >= N_BITS, "N_BITS cannot be bigger than the amount of bits that integer container INT_T can store!");

        constexpr static short _N_DATA_BYTES = __positive_ceil(N_BITS / 8.0);

        // For padding bytes
        constexpr static short _N_ALIGN_BYTES = []{
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

        constexpr static short _get_endianness_offset(Endian endianness)
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
        constexpr static int N_IO_BYTES = _N_DATA_BYTES + _N_ALIGN_BYTES;


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

            constexpr auto endianness_offset = _get_endianness_offset(ENDIANNESS_V);

            // Copy the input bytes into the result
            if constexpr (endianness_offset) // Reversed order
            {
                for (short i=0; i<_N_DATA_BYTES; i++)
                    result |= static_cast<INT_T>(bytes[endianness_offset-i+offset]) << (i * 8);
            }
            else
            {
                for (short i=0; i<_N_DATA_BYTES; i++)
                    result |= static_cast<INT_T>(bytes[i+offset]) << (i * 8);
            }

            // Bitmask to clear out unwanted/garbage data
            constexpr unsigned int VALUE_MASK = (1LL << N_BITS) - 1;
            result &= VALUE_MASK;

            if constexpr (N_BITS % _N_CONTAINER_BITS != 0 && std::is_signed_v<INT_T>)
            {
                // Sign extend the result number if number should be negative
                constexpr unsigned int msb = endianness_offset ? _N_ALIGN_BYTES : _N_DATA_BYTES - 1;
                constexpr std::uint8_t SIGN_BIT_MASK = (1LL << ((N_BITS % 8) + 7) % 8);
                if (bytes[msb+offset] & SIGN_BIT_MASK)
                    result |= ~VALUE_MASK;
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

            // Create a bitmask to isolate the bits that we're interested in
            constexpr unsigned int VALUE_MASK = (1LL << N_BITS) - 1;
            value &= VALUE_MASK;

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
    /// @brief Template class for doing float data I/O.
    ///
    /// @tparam FLOAT_T Float type to (un)pack.
    ///
    template <typename FLOAT_T>
    class FloatIO
    {
        static_assert(std::is_floating_point_v<FLOAT_T>, "Template parameter FLOAT_T must be a float type!");


        // :: PRIVATE ATTRIBUTES & HELPER FUNCTIONS ::
        private:

        constexpr static short _N_DATA_BYTES = sizeof(FLOAT_T);

        constexpr static short _get_endianness_offset(Endian endianness)
        {
            return (endianness == Endian::BIG) ^ !__IS_SYSTEM_LITTLE_ENDIAN
                ? _N_DATA_BYTES - 1
                : 0;
        }


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
            FLOAT_T result = 0;
            std::uint8_t* result_bytes = reinterpret_cast<std::uint8_t*>(&result);

            constexpr auto endianness_offset = _get_endianness_offset(ENDIANNESS_V);

            // Copy the input bytes into the result
            if constexpr (endianness_offset)
            {
                for (short i=0; i<_N_DATA_BYTES; i++)
                    result_bytes[i] = static_cast<std::uint8_t>(bytes[endianness_offset-i+offset]);
            }
            else
            {
                for (short i=0; i<_N_DATA_BYTES; i++)
                    result_bytes[i] = static_cast<std::uint8_t>(bytes[i+offset]);
            }

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
            // Extend vector for packed data
            auto offset = bytes.size();
            bytes.resize(offset+_N_DATA_BYTES);

            std::uint8_t* value_bytes = reinterpret_cast<std::uint8_t*>(&value);

            constexpr auto endianness_offset = _get_endianness_offset(ENDIANNESS_V);

            // Copy the bits into the byte vector
            if constexpr (endianness_offset)
            {
                for (short i=0; i<_N_DATA_BYTES; i++)
                    bytes[endianness_offset-i+offset] = static_cast<std::uint8_t>(value_bytes[i]);
            }
            else
            {
                for (short i=0; i<_N_DATA_BYTES; i++)
                    bytes[i+offset] = static_cast<std::uint8_t>(value_bytes[i]);
            }

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
            auto n_io_bytes_ = n_io_bytes();
            std::vector<std::uint8_t> buffer(n_io_bytes_);
            s.read(reinterpret_cast<char*>(buffer.data()), n_io_bytes_);
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
            auto n_io_bytes_ = n_io_bytes();
            std::vector<std::uint8_t> buffer;
            buffer.reserve(n_io_bytes_);
            pack<ENDIANNESS_V>(value, buffer);
            s.write(reinterpret_cast<char*>(&buffer[0]), n_io_bytes_);
            return;
        }
    };

}

// ****************************************************************************

#endif /* NUMIO_H */
