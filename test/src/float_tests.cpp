#include <bitset>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <gtest/gtest.h>

#include "../include/numio/native.hpp"
using namespace numio;

// ****************************************************************************

// https://www.h-schmidt.net/FloatConverter/IEEE754.html
TEST(NumioFloatTests, _FloatFloatTests)
{
  using f32_io = FloatIO<float>;

  // Little endian.
  {
    #define TEST_ENDIANNESS Endian::LITTLE

    std::stringstream ibuffer;
    ibuffer << (char)0x52; // LSB
    ibuffer << (char)0x06;
    ibuffer << (char)0x9E;
    ibuffer << (char)0x3F; // MSB

    auto value = f32_io::read<TEST_ENDIANNESS>(ibuffer);
    float comparison = 1.23456789f;
    EXPECT_FLOAT_EQ(value, comparison);

    // std::cout << std::bitset<32>( 42 ) << std::endl;

    std::stringstream obuffer;
    f32_io::write<TEST_ENDIANNESS>(value, obuffer);
    ibuffer.seekg(0);
    EXPECT_EQ(ibuffer.str(), obuffer.str());

    #undef TEST_ENDIANNESS
  }

  // Big endian.
  {
    #define TEST_ENDIANNESS Endian::BIG

    std::stringstream ibuffer;
    ibuffer << (char)0x3F; // MSB
    ibuffer << (char)0x9E;
    ibuffer << (char)0x06;
    ibuffer << (char)0x52; // LSB

    auto value = f32_io::read<TEST_ENDIANNESS>(ibuffer);
    float comparison = 1.23456789f;
    EXPECT_FLOAT_EQ(value, comparison);

    std::stringstream obuffer;
    f32_io::write<TEST_ENDIANNESS>(value, obuffer);
    ibuffer.seekg(0);
    EXPECT_EQ(ibuffer.str(), obuffer.str());

    #undef TEST_ENDIANNESS
  }
}


TEST(NumioIntegerTests, _DoubleFloatTests)
{
  using f64_io = FloatIO<double>;

  // Little endian Denormalized.
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

    auto value = f64_io::read<TEST_ENDIANNESS>(ibuffer);
    double comparison = -0.12345678987654321;
    EXPECT_FLOAT_EQ(value, comparison);

    std::stringstream obuffer;
    f64_io::write<TEST_ENDIANNESS>(value, obuffer);
    ibuffer.seekg(0);
    EXPECT_EQ(ibuffer.str(), obuffer.str());

    #undef TEST_ENDIANNESS
  }

  // Little endian Denormalized.
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

    auto value = f64_io::read<TEST_ENDIANNESS>(ibuffer);
    double comparison = 1.234567898765432;
    EXPECT_FLOAT_EQ(value, comparison);

    std::stringstream obuffer;
    f64_io::write<TEST_ENDIANNESS>(value, obuffer);
    ibuffer.seekg(0);
    EXPECT_EQ(ibuffer.str(), obuffer.str());

    #undef TEST_ENDIANNESS
  }
}
