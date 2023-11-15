
// TODO: Make a test pipeline used by a badge on the README displaying if succesful

// ****************************************************************************

#include <bitset>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../include/numio/native.hpp"
using namespace NumIO;

// ****************************************************************************

// Debug program
int main()
{
    // Standard integer
    {
        // Little endian standard integer
        {
            #define TEST_ENDIANNESS Endian::LITTLE
            #define TEST_INT_BITS 32
            #define TEST_ALIGN true

            using i32_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive
            {
                std::stringstream ibuffer;
                ibuffer << (char)0x4F; // LSB
                ibuffer << (char)0x1A;
                ibuffer << (char)0xAD;
                ibuffer << (char)0x05; // MSB

                auto value = i32_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 95230543);

                std::stringstream obuffer;
                i32_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Negative
            {
                std::stringstream ibuffer;
                ibuffer << (char)0xB1; // LSB
                ibuffer << (char)0xE5;
                ibuffer << (char)0x52;
                ibuffer << (char)0xFA; // MSB

                auto value = i32_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -95230543);

                std::stringstream obuffer;
                i32_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN

            // std::cout << std::bitset<TEST_01_INT_BITS>(value) << std::endl;
            // std::cout << value << std::endl;
        }

        // Big endian standard integer
        {
            #define TEST_ENDIANNESS Endian::BIG
            #define TEST_INT_BITS 32
            #define TEST_ALIGN true

            using i32_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive
            {
                std::stringstream ibuffer;
                ibuffer << (char)0x05; // MSB
                ibuffer << (char)0xAD;
                ibuffer << (char)0x1A;
                ibuffer << (char)0x4F; // LSB

                auto value = i32_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 95230543);

                std::stringstream obuffer;
                i32_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Negative
            {
                std::stringstream ibuffer;
                ibuffer << (char)0xFA; // MSB
                ibuffer << (char)0x52;
                ibuffer << (char)0xE5;
                ibuffer << (char)0xB1; // LSB

                auto value = i32_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -95230543);

                std::stringstream obuffer;
                i32_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN
        }
    }

    // 24-bit integer
    {
        // Little endian 24-bit aligned integer
        {
            #define TEST_ENDIANNESS Endian::LITTLE
            #define TEST_INT_BITS 24
            #define TEST_ALIGN true

            using i24a_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b01111111; // MSB (value)
                ibuffer << (char)0b00000000; // MSB (container)

                auto value = i24a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8388607);

                std::stringstream obuffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Negative integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b11111111; // MSB (value)
                ibuffer << (char)0b00000000; // MSB (container)

                auto value = i24a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream obuffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Garbage data in MSB
            {
                std::stringstream ibuffer;
                ibuffer << (char)0x2B; // LSB
                ibuffer << (char)0xF0;
                ibuffer << (char)0x7F; // MSB (value)
                ibuffer << (char)0x42; // MSB (container)

                auto value = i24a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8384555);

                std::stringstream tmp_buffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24a_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN
        }

        // Big endian 24-bit aligned integer
        {
            #define TEST_ENDIANNESS Endian::BIG
            #define TEST_INT_BITS 24
            #define TEST_ALIGN true

            using i24a_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b00000000; // MSB (container)
                ibuffer << (char)0b01111111; // MSB (value)
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b11111111; // LSB

                auto value = i24a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8388607);

                std::stringstream obuffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Negative integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b00000000; // MSB (container)
                ibuffer << (char)0b11111111; // MSB (value)
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b11111111; // LSB

                auto value = i24a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream obuffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Garbage data in MSB (container)
            {
                std::stringstream ibuffer;
                ibuffer << (char)0x42; // MSB (container)
                ibuffer << (char)0x7F; // MSB (value)
                ibuffer << (char)0xF0;
                ibuffer << (char)0x2B; // LSB

                auto value = i24a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8384555);

                std::stringstream tmp_buffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24a_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN
        }

        // Little endian 24-bit non-aligned integer
        {
            #define TEST_ENDIANNESS Endian::LITTLE
            #define TEST_INT_BITS 24
            #define TEST_ALIGN false

            using i24_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b01111111; // MSB
                ibuffer << (char)0b00000000; // Padding

                auto value = i24_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8388607);

                std::stringstream tmp_buffer;
                i24_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            // Negative integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b11111111; // MSB
                ibuffer << (char)0b00000000; // Padding

                auto value = i24_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream tmp_buffer;
                i24_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            // Garbage data in padding
            {
                std::stringstream ibuffer;
                ibuffer << (char)0x2B; // LSB
                ibuffer << (char)0xF0;
                ibuffer << (char)0x7F; // MSB
                ibuffer << (char)0x42; // Padding

                auto value = i24_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8384555);

                std::stringstream tmp_buffer;
                i24_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN
        }

        // Big endian 24-bit non-aligned integer
        {
            #define TEST_ENDIANNESS Endian::BIG
            #define TEST_INT_BITS 24
            #define TEST_ALIGN false

            using i24_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b01111111; // MSB
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b00000000; // Padding

                auto value = i24_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8388607);

                std::stringstream tmp_buffer;
                i24_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            // Negative integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // MSB
                ibuffer << (char)0b11111111;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b00000000; // Padding

                auto value = i24_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream tmp_buffer;
                i24_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            // Garbage data in padding
            {
                std::stringstream ibuffer;
                ibuffer << (char)0x7F; // MSB
                ibuffer << (char)0xF0;
                ibuffer << (char)0x2B; // LSB
                ibuffer << (char)0x42; // Padding

                auto value = i24_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 8384555);

                std::stringstream tmp_buffer;
                i24_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i24_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i24_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN
        }
    }

    // 13-bit integer
    {
        // Little endian 13-bit aligned integer
        {
            #define TEST_ENDIANNESS Endian::LITTLE
            #define TEST_INT_BITS 13
            #define TEST_ALIGN true

            using i13a_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b00001111; // MSB
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00000000; // Padding

                auto value = i13a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 4095);

                std::stringstream obuffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Negative integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b00011111; // MSB
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00000000; // Padding

                auto value = i13a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream obuffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Garbage data in MSB
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b11111111; // LSB
                ibuffer << (char)0b01011111; // MSB
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00000000; // Padding

                auto value = i13a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream tmp_buffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i13a_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN
        }

        // Big endian 13-bit aligned integer
        {
            #define TEST_ENDIANNESS Endian::BIG
            #define TEST_INT_BITS 13
            #define TEST_ALIGN true

            using i13a_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00001111; // MSB
                ibuffer << (char)0b11111111; // LSB

                auto value = i13a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == 4095);

                std::stringstream obuffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Negative integer
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00011111; // MSB
                ibuffer << (char)0b11111111; // LSB

                auto value = i13a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream obuffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());
            }

            // Garbage data in MSB
            {
                std::stringstream ibuffer;
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b00000000; // Padding
                ibuffer << (char)0b01011111; // MSB
                ibuffer << (char)0b11111111; // LSB

                auto value = i13a_IO::read<TEST_ENDIANNESS>(ibuffer);
                assert(value == -1);

                std::stringstream tmp_buffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, tmp_buffer);
                value = i13a_IO::read<TEST_ENDIANNESS>(tmp_buffer);

                std::stringstream obuffer;
                i13a_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(tmp_buffer.str() == obuffer.str());
            }

            #undef TEST_ENDIANNESS
            #undef TEST_INT_BITS
            #undef TEST_ALIGN
        }
    }

    // Float
    {
        // Float
        {
            using f32_IO = FloatIO<float, std::int32_t>;
            using i32_IO = IntIO<std::int32_t>;

            // Little endian
            {
                #define TEST_ENDIANNESS Endian::LITTLE

                std::stringstream ibuffer;
                ibuffer << (char)0x52; // LSB
                ibuffer << (char)0x06;
                ibuffer << (char)0x9E;
                ibuffer << (char)0x3F; // MSB

                auto value = f32_IO::read<TEST_ENDIANNESS>(ibuffer);
                float comparison = 1.23456789;
                assert(value == comparison);

                std::stringstream obuffer;
                f32_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());

                #undef TEST_ENDIANNESS
            }

            // Big endian
            {
                #define TEST_ENDIANNESS Endian::BIG

                std::stringstream ibuffer;
                ibuffer << (char)0x3F; // MSB
                ibuffer << (char)0x9E;
                ibuffer << (char)0x06;
                ibuffer << (char)0x52; // LSB

                auto value = f32_IO::read<TEST_ENDIANNESS>(ibuffer);
                float comparison = 1.23456789;
                assert(value == comparison);

                std::stringstream obuffer;
                f32_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());

                #undef TEST_ENDIANNESS
            }
        }

        // Double
        {
            using f64_IO = FloatIO<double, std::uint64_t>;

            // Little endian Denormalized
            {
                #define TEST_ENDIANNESS Endian::LITTLE

                std::stringstream ibuffer;
                ibuffer << (char)0x5A; // LSB
                ibuffer << (char)0x28;
                ibuffer << (char)0xFD;
                ibuffer << (char)0x3A;
                ibuffer << (char)0xDD;
                ibuffer << (char)0x9A;
                ibuffer << (char)0xBF;
                ibuffer << (char)0xBF; // MSB

                auto value = f64_IO::read<TEST_ENDIANNESS>(ibuffer);
                double comparison = -0.12345678987654321;
                assert(value == comparison);

                std::stringstream obuffer;
                f64_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());

                #undef TEST_ENDIANNESS
            }

            // Little endian Denormalized
            {
                #define TEST_ENDIANNESS Endian::LITTLE

                std::stringstream ibuffer;
                ibuffer << (char)0x38; // LSB
                ibuffer << (char)0x39;
                ibuffer << (char)0xDE;
                ibuffer << (char)0x44;
                ibuffer << (char)0xCA;
                ibuffer << (char)0xC0;
                ibuffer << (char)0xF3;
                ibuffer << (char)0x3F; // MSB

                auto value = f64_IO::read<TEST_ENDIANNESS>(ibuffer);
                double comparison = 1.234567898765432;
                assert(value == comparison);

                std::stringstream obuffer;
                f64_IO::write<TEST_ENDIANNESS>(value, obuffer);
                ibuffer.seekg(0);
                assert(ibuffer.str() == obuffer.str());

                #undef TEST_ENDIANNESS
            }
        }
    }

    // std::cout << std::bitset<32>(value) << std::endl;
    // std::cout << value << std::endl;

    return 0;
}
