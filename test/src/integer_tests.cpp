#include <bitset>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <gtest/gtest.h>

#include "../include/numio/native.hpp"
using namespace numio;

// ****************************************************************************

TEST(NumioIntegerTests, StandardIntegerTests)
{
  // Little endian standard integer.
  {
    #define TEST_ENDIANNESS Endian::LITTLE
    #define TEST_INT_BITS 32
    #define TEST_ALIGN true

    using i32_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0x4F; // LSB
      ibuffer << (char)0x1A;
      ibuffer << (char)0xAD;
      ibuffer << (char)0x05; // MSB

      auto value = i32_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 95230543);

      std::stringstream obuffer;
      i32_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Negative.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0xB1; // LSB
      ibuffer << (char)0xE5;
      ibuffer << (char)0x52;
      ibuffer << (char)0xFA; // MSB

      auto value = i32_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -95230543);

      std::stringstream obuffer;
      i32_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    #undef TEST_ENDIANNESS
    #undef TEST_INT_BITS
    #undef TEST_ALIGN
  }

  // Big endian standard integer.
  {
    #define TEST_ENDIANNESS Endian::BIG
    #define TEST_INT_BITS 32
    #define TEST_ALIGN true

    using i32_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0x05; // MSB
      ibuffer << (char)0xAD;
      ibuffer << (char)0x1A;
      ibuffer << (char)0x4F; // LSB

      auto value = i32_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 95230543);

      std::stringstream obuffer;
      i32_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Negative.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0xFA; // MSB
      ibuffer << (char)0x52;
      ibuffer << (char)0xE5;
      ibuffer << (char)0xB1; // LSB

      auto value = i32_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -95230543);

      std::stringstream obuffer;
      i32_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    #undef TEST_ENDIANNESS
    #undef TEST_INT_BITS
    #undef TEST_ALIGN
  }
}

TEST(NumioIntegerTests, _24BitIntegerTests)
{
  // Little endian 24-bit aligned integer.
  {
    #define TEST_ENDIANNESS Endian::LITTLE
    #define TEST_INT_BITS 24
    #define TEST_ALIGN true

    using i24a_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b01111111; // MSB (value)
      ibuffer << (char)0b00000000; // MSB (container)

      auto value = i24a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8388607);

      std::stringstream obuffer;
      i24a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Negative integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b11111111; // MSB (value)
      ibuffer << (char)0b00000000; // MSB (container)

      auto value = i24a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream obuffer;
      i24a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Garbage data in MSB.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0x2B; // LSB
      ibuffer << (char)0xF0;
      ibuffer << (char)0x7F; // MSB (value)
      ibuffer << (char)0x42; // MSB (container)

      auto value = i24a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8384555);

      std::stringstream tmp_buffer;
      i24a_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24a_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    #undef TEST_ENDIANNESS
    #undef TEST_INT_BITS
    #undef TEST_ALIGN
  }

  // Big endian 24-bit aligned integer.
  {
    #define TEST_ENDIANNESS Endian::BIG
    #define TEST_INT_BITS 24
    #define TEST_ALIGN true

    using i24a_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b00000000; // MSB (container)
      ibuffer << (char)0b01111111; // MSB (value)
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b11111111; // LSB

      auto value = i24a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8388607);

      std::stringstream obuffer;
      i24a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Negative integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b00000000; // MSB (container)
      ibuffer << (char)0b11111111; // MSB (value)
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b11111111; // LSB

      auto value = i24a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream obuffer;
      i24a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Garbage data in MSB (container).
    {
      std::stringstream ibuffer;
      ibuffer << (char)0x42; // MSB (container)
      ibuffer << (char)0x7F; // MSB (value)
      ibuffer << (char)0xF0;
      ibuffer << (char)0x2B; // LSB

      auto value = i24a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8384555);

      std::stringstream tmp_buffer;
      i24a_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24a_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    #undef TEST_ENDIANNESS
    #undef TEST_INT_BITS
    #undef TEST_ALIGN
  }

  // Little endian 24-bit non-aligned integer.
  {
    #define TEST_ENDIANNESS Endian::LITTLE
    #define TEST_INT_BITS 24
    #define TEST_ALIGN false

    using i24_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b01111111; // MSB
      ibuffer << (char)0b00000000; // Padding

      auto value = i24_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8388607);

      std::stringstream tmp_buffer;
      i24_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    // Negative integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b11111111; // MSB
      ibuffer << (char)0b00000000; // Padding

      auto value = i24_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream tmp_buffer;
      i24_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    // Garbage data in padding.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0x2B; // LSB
      ibuffer << (char)0xF0;
      ibuffer << (char)0x7F; // MSB
      ibuffer << (char)0x42; // Padding

      auto value = i24_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8384555);

      std::stringstream tmp_buffer;
      i24_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
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

    using i24_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b01111111; // MSB
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b00000000; // Padding

      auto value = i24_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8388607);

      std::stringstream tmp_buffer;
      i24_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    // Negative integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // MSB
      ibuffer << (char)0b11111111;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b00000000; // Padding

      auto value = i24_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream tmp_buffer;
      i24_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    // Garbage data in padding.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0x7F; // MSB
      ibuffer << (char)0xF0;
      ibuffer << (char)0x2B; // LSB
      ibuffer << (char)0x42; // Padding

      auto value = i24_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 8384555);

      std::stringstream tmp_buffer;
      i24_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i24_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i24_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    #undef TEST_ENDIANNESS
    #undef TEST_INT_BITS
    #undef TEST_ALIGN
  }
}

TEST(NumioIntegerTests, _13BitIntegerTests)
{
  // Little endian 13-bit aligned integer.
  {
    #define TEST_ENDIANNESS Endian::LITTLE
    #define TEST_INT_BITS 13
    #define TEST_ALIGN true

    using i13a_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b00001111; // MSB
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00000000; // Padding

      auto value = i13a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 4095);

      std::stringstream obuffer;
      i13a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Negative integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b00011111; // MSB
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00000000; // Padding

      auto value = i13a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream obuffer;
      i13a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Garbage data in MSB.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b11111111; // LSB
      ibuffer << (char)0b01011111; // MSB
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00000000; // Padding

      auto value = i13a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream tmp_buffer;
      i13a_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i13a_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i13a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    #undef TEST_ENDIANNESS
    #undef TEST_INT_BITS
    #undef TEST_ALIGN
  }

  // Big endian 13-bit aligned integer.
  {
    #define TEST_ENDIANNESS Endian::BIG
    #define TEST_INT_BITS 13
    #define TEST_ALIGN true

    using i13a_io = IntIO<std::int32_t, TEST_INT_BITS, TEST_ALIGN>;

    // Positive integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00001111; // MSB
      ibuffer << (char)0b11111111; // LSB

      auto value = i13a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, 4095);

      std::stringstream obuffer;
      i13a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Negative integer.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00011111; // MSB
      ibuffer << (char)0b11111111; // LSB

      auto value = i13a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream obuffer;
      i13a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(ibuffer.str(), obuffer.str());
    }

    // Garbage data in MSB.
    {
      std::stringstream ibuffer;
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b00000000; // Padding
      ibuffer << (char)0b01011111; // MSB
      ibuffer << (char)0b11111111; // LSB

      auto value = i13a_io::read<TEST_ENDIANNESS>(ibuffer);
      EXPECT_EQ(value, -1);

      std::stringstream tmp_buffer;
      i13a_io::write<TEST_ENDIANNESS>(value, tmp_buffer);
      value = i13a_io::read<TEST_ENDIANNESS>(tmp_buffer);

      std::stringstream obuffer;
      i13a_io::write<TEST_ENDIANNESS>(value, obuffer);
      ibuffer.seekg(0);
      EXPECT_EQ(tmp_buffer.str(), obuffer.str());
    }

    #undef TEST_ENDIANNESS
    #undef TEST_INT_BITS
    #undef TEST_ALIGN
  }
}
