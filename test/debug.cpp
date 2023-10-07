
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
                std::stringstream buffer;
                buffer << (char)0x4F; // LSB
                buffer << (char)0x1A;
                buffer << (char)0xAD;
                buffer << (char)0x05; // MSB

                auto value = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>::read<TEST_ENDIANNESS>(buffer);
                assert(value == 95230543);
            }

            // Negative
            {
                std::stringstream buffer;
                buffer << (char)0xB1; // LSB
                buffer << (char)0xE5;
                buffer << (char)0x52;
                buffer << (char)0xFA; // MSB

                auto value = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>::read<TEST_ENDIANNESS>(buffer);
                assert(value == -95230543);
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
                std::stringstream buffer;
                buffer << (char)0x05; // MSB
                buffer << (char)0xAD;
                buffer << (char)0x1A;
                buffer << (char)0x4F; // LSB

                auto value = i32_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 95230543);
            }

            // Negative
            {
                std::stringstream buffer;
                buffer << (char)0xFA; // MSB
                buffer << (char)0x52;
                buffer << (char)0xE5;
                buffer << (char)0xB1; // LSB

                auto value = i32_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -95230543);
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
                std::stringstream buffer;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b11111111;
                buffer << (char)0b01111111; // MSB (value)
                buffer << (char)0b00000000; // MSB (container)

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8388607);
            }

            // Negative integer
            {
                std::stringstream buffer;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b11111111;
                buffer << (char)0b11111111; // MSB (value)
                buffer << (char)0b00000000; // MSB (container)

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
            }

            // Garbage data in MSB
            {
                std::stringstream buffer;
                buffer << (char)0x2B; // LSB
                buffer << (char)0xF0;
                buffer << (char)0x7F; // MSB (value)
                buffer << (char)0x42; // MSB (container)

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8384555);
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
                std::stringstream buffer;
                buffer << (char)0b00000000; // MSB (container)
                buffer << (char)0b01111111; // MSB (value)
                buffer << (char)0b11111111;
                buffer << (char)0b11111111; // LSB

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8388607);
            }

            // Negative integer
            {
                std::stringstream buffer;
                buffer << (char)0b00000000; // MSB (container)
                buffer << (char)0b11111111; // MSB (value)
                buffer << (char)0b11111111;
                buffer << (char)0b11111111; // LSB

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
            }

            // Garbage data in MSB (container)
            {
                std::stringstream buffer;
                buffer << (char)0x42; // MSB (container)
                buffer << (char)0x7F; // MSB (value)
                buffer << (char)0xF0;
                buffer << (char)0x2B; // LSB

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8384555);
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

            using i24a_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream buffer;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b11111111;
                buffer << (char)0b01111111; // MSB
                buffer << (char)0b00000000; // Padding

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8388607);
            }

            // Negative integer
            {
                std::stringstream buffer;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b11111111;
                buffer << (char)0b11111111; // MSB
                buffer << (char)0b00000000; // Padding

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
            }

            // Garbage data in padding
            {
                std::stringstream buffer;
                buffer << (char)0x2B; // LSB
                buffer << (char)0xF0;
                buffer << (char)0x7F; // MSB
                buffer << (char)0x42; // Padding

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8384555);
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

            using i24a_IO = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

            // Positive integer
            {
                std::stringstream buffer;
                buffer << (char)0b01111111; // MSB
                buffer << (char)0b11111111;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b00000000; // Padding

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8388607);
            }

            // Negative integer
            {
                std::stringstream buffer;
                buffer << (char)0b11111111; // MSB
                buffer << (char)0b11111111;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b00000000; // Padding

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
            }

            // Garbage data in padding
            {
                std::stringstream buffer;
                buffer << (char)0x7F; // MSB
                buffer << (char)0xF0;
                buffer << (char)0x2B; // LSB
                buffer << (char)0x42; // Padding

                auto value = i24a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 8384555);
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
                std::stringstream buffer;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b00001111; // MSB
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00000000; // Padding

                auto value = i13a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 4095);
            }

            // Negative integer
            {
                std::stringstream buffer;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b00011111; // MSB
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00000000; // Padding

                auto value = i13a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
            }

            // Garbage data in MSB
            {
                std::stringstream buffer;
                buffer << (char)0b11111111; // LSB
                buffer << (char)0b01011111; // MSB
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00000000; // Padding

                auto value = i13a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
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
                std::stringstream buffer;
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00001111; // MSB
                buffer << (char)0b11111111; // LSB

                auto value = i13a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == 4095);
            }

            // Negative integer
            {
                std::stringstream buffer;
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00011111; // MSB
                buffer << (char)0b11111111; // LSB

                auto value = i13a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
            }

            // Garbage data in MSB
            {
                std::stringstream buffer;
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b00000000; // Padding
                buffer << (char)0b01011111; // MSB
                buffer << (char)0b11111111; // LSB

                auto value = i13a_IO::read<TEST_ENDIANNESS>(buffer);
                assert(value == -1);
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
            using f32_IO = FloatIO<float>;

            // Little endian
            {
                #define TEST_ENDIANNESS Endian::LITTLE

                std::stringstream buffer;
                buffer << (char)0x52; // LSB
                buffer << (char)0x06;
                buffer << (char)0x9E;
                buffer << (char)0x3F; // MSB

                auto value = f32_IO::read<TEST_ENDIANNESS>(buffer);
                float comparison = 1.23456789;
                assert(value == comparison);

                #undef TEST_ENDIANNESS
            }

            // Big endian
            {
                #define TEST_ENDIANNESS Endian::BIG

                std::stringstream buffer;
                buffer << (char)0x3F; // MSB
                buffer << (char)0x9E;
                buffer << (char)0x06;
                buffer << (char)0x52; // LSB

                auto value = f32_IO::read<TEST_ENDIANNESS>(buffer);
                float comparison = 1.23456789;
                assert(value == comparison);

                #undef TEST_ENDIANNESS
            }
        }

        // Double
        {
            using f64_IO = FloatIO<double>;

            // Little endian
            {
                #define TEST_ENDIANNESS Endian::LITTLE

                std::stringstream buffer;
                buffer << (char)0x1B; // LSB
                buffer << (char)0xDE;
                buffer << (char)0x83;
                buffer << (char)0x42;
                buffer << (char)0xCA;
                buffer << (char)0xC0;
                buffer << (char)0xF3;
                buffer << (char)0x3F;  // MSB

                auto value = f64_IO::read<TEST_ENDIANNESS>(buffer);
                double comparison = 1.23456789;
                assert(value == comparison);

                #undef TEST_ENDIANNESS
            }

            // Big endian
            {
                #define TEST_ENDIANNESS Endian::BIG

                std::stringstream buffer;
                buffer << (char)0x3F;  // MSB
                buffer << (char)0xF3;
                buffer << (char)0xC0;
                buffer << (char)0xCA;
                buffer << (char)0x42;
                buffer << (char)0x83;
                buffer << (char)0xDE;
                buffer << (char)0x1B; // LSB

                auto value = f64_IO::read<TEST_ENDIANNESS>(buffer);
                double comparison = 1.23456789;
                assert(value == comparison);

                #undef TEST_ENDIANNESS
            }
        }
    }

    // std::cout << std::bitset<32>(value) << std::endl;
    // std::cout << value << std::endl;

    return 0;
}
