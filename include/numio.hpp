#pragma once

// *****************************************************************************

#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

#if defined(__STDCPP_FLOAT16_T__) || defined(__STDCPP_FLOAT32_T__) || defined(__STDCPP_FLOAT64_T__) || defined(__STDCPP_FLOAT128_T__) || defined(__STDCPP_BFLOAT16_T__)
  #include <stdfloat>
#endif

#if (defined(NUMIO_ENABLE_EXCEPTIONS) && __cpp_exceptions == 199711)
  #include <stdexcept>
#else
  #undef NUMIO_ENABLE_EXCEPTIONS
#endif

// *****************************************************************************

///
/// @brief Flexible C++17 set of template classes for platform-agnostic integer and float data I/O.
///
/// @param NUMIO_DEFAULT_ENDIAN_V `numio::Endian` macro parameter setting the default endianness for (un)packing data
///   when not explicitly specified by the user. Defaults to `numio::Endian::LITTLE`.
/// @param NUMIO_DEFAULT_ALIGN_V Boolean macro parameter setting the default byte alignment for (un)packing data when
///   not explicitly specified by the user. Defaults to `false`.
/// @param NUMIO_SYSTEM_ENDIANNESS_V Integer macro parameter defining the target system endianness. Defaults to the
///   endianness of the compiling system when not specified.
///   Possible values are:
///    - `LITTLE_ENDIAN` or as integer value `1234`
///    - `BIG_ENDIAN` or as integer value `4321`
/// @param NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V Boolean macro parameter alternative to `NUMIO_SYSTEM_ENDIANNESS_V` for
///   defining the target system endianness.
/// @param NUMIO_ENABLE_EXCEPTIONS Defining this macro enables runtime checking and exceptions, which should be used
///   mainly for debug or test builds.
///
namespace numio {

// *****************************************************************************

namespace {
  // Compile-time way to check the endianness of the system that is executing the compiler.
  // It's impossible to determine the endianness of the *target* system, so it's necessary to
  // define NUMIO_SYSTEM_ENDIANNESS_V or NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V when cross-compiling.
  static constexpr bool __IS_SYSTEM_LITTLE_ENDIAN = []
  {
    // There's some GCC and Clang/LLVM header that sets these, but are not defined on MSVC. Therefore
    // define these macros temporarily ourselves if not defined.
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
          "This message can be suppressed by including \"numio/native.hpp\" or by defining NUMIO_IGNORE_AUTO_ENDIAN."\
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

  template<typename FLOAT_T> struct __FloatIO_container_type { typedef void type; };
  template<> struct __FloatIO_container_type<float > { typedef std::uint32_t type; };
  template<> struct __FloatIO_container_type<double> { typedef std::uint64_t type; };
  #if defined(__STDCPP_FLOAT16_T__)
    template<> struct __FloatIO_container_type<std::float16_t> { typedef std::uint16_t type; };
  #endif
  #if defined(__STDCPP_FLOAT32_T__)
    template<> struct __FloatIO_container_type<std::float32_t> { typedef std::uint32_t type; };
  #endif
  #if defined(__STDCPP_FLOAT64_T__)
    template<> struct __FloatIO_container_type<std::float64_t> { typedef std::uint64_t type; };
  #endif
  #if defined(__STDCPP_FLOAT128_T__)
    #if __SIZEOF_INT128__ >= 16
      template<> struct __FloatIO_container_type<std::float128_t> { typedef __uint128_t type; };
    #elif defined(BOOST_INT128_UINT128_C)
      template<> struct __FloatIO_container_type<std::float128_t> { typedef boost::int128::uint128_t type; };
    #endif
  #endif
  #if defined(__STDCPP_BFLOAT16_T__)
    template<> struct __FloatIO_container_type<std::bfloat16_t> { typedef std::uint16_t type; };
  #endif

  // Quick and dirty constexpr ceil function for positive numbers since `ceil()`
  // is not a standardized constexpr function before C++23.
  static constexpr int __positive_ceil(float f)
  {
    auto i = static_cast<int>(f);
    return static_cast<bool>(f != i)
      ? i + 1
      : i;
  };

  template<typename INT_T>
  static constexpr INT_T __unsigned_max(unsigned int n_bits) {
    static_assert(std::is_integral_v<INT_T>, "Template parameter INT_T must be an integer type!");
    return (static_cast<INT_T>(1) << (n_bits-1)) - 1 + (static_cast<INT_T>(1) << (n_bits-1));
  }
}

// *****************************************************************************

#ifdef NUMIO_ENABLE_EXCEPTIONS
  struct NumIOError : std::runtime_error {
    using std::runtime_error::runtime_error;
  };
#endif

///
/// @brief Endian modes supported by numio.
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

// *****************************************************************************

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

  // :: PRIVATE ATTRIBUTES :: //
  private:

  static constexpr int _N_CONTAINER_BITS = []{
    auto bits = std::numeric_limits<INT_T>::digits;
    if (std::is_signed_v<INT_T>)
      bits++;
    return bits;
  }();
  static_assert(_N_CONTAINER_BITS >= N_BITS, "N_BITS cannot be bigger than the amount of bits that integer container INT_T can store!");

  static constexpr int _N_DATA_BYTES = __positive_ceil(N_BITS / 8.0);

  // Amount of bytes if we need to pad.
  static constexpr int _N_ALIGN_BYTES = []{
    if (N_BITS <= 8) {
      return 0;
    }
    return ALIGNED_V
      ? static_cast<int>(_N_CONTAINER_BITS - N_BITS) / 8
      : 0;
  }();

  static constexpr INT_T _VALUE_MASK = []{
    if (_N_CONTAINER_BITS == N_BITS) {
      return static_cast<INT_T>(~static_cast<INT_T>(0));
    }
    return __unsigned_max<INT_T>(N_BITS);
  }();

  static constexpr int _get_endianness_offset(Endian endianness)
  {
    return (endianness == Endian::BIG) ^ !__IS_SYSTEM_LITTLE_ENDIAN
      ? AMOUNT_IO_BYTES - 1
      : 0;
  }

  // :: PUBLIC ATTRIBUTES :: //
  public:

  ///
  /// @brief The amount of bytes used for the packed data.
  ///
  static constexpr int AMOUNT_IO_BYTES = _N_DATA_BYTES + _N_ALIGN_BYTES;

  // :: UNPACKING FUNCTIONS :: //
  public:

  ///
  /// @brief Unpacks an integer from an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param bytes Array of bytes to read from.
  /// @param offset Offset in bytes to extract from of the array.
  /// @return Integer value.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static INT_T unpack(unsigned char* bytes, std::size_t offset=0)
  {
    INT_T result = 0;

    static constexpr auto ENDIANNESS_OFFSET = _get_endianness_offset(ENDIANNESS_V);

    // Copy the input bytes into the result.
    if constexpr (!ENDIANNESS_OFFSET)
    {
      for (int i=0; i<_N_DATA_BYTES; i++) {
        result |= static_cast<INT_T>(bytes[i+offset]) << (i * 8);
      }
    }
    // Reversed order.
    else
    {
      for (int i=0; i<_N_DATA_BYTES; i++) {
        result |= static_cast<INT_T>(bytes[ENDIANNESS_OFFSET-i+offset]) << (i * 8);
      }
    }

    if constexpr (N_BITS != _N_CONTAINER_BITS)
    {
      // Bitmask to clear out unwanted/garbage data in the final read byte.
      result &= _VALUE_MASK;

      if constexpr (std::is_signed_v<INT_T>)
      {
        // Sign extend the result number if number should be negative.
        static constexpr unsigned int MSB = ENDIANNESS_OFFSET ? _N_ALIGN_BYTES : _N_DATA_BYTES - 1;
        static constexpr unsigned int SIGN_BIT_MASK = (1 << ((N_BITS % 8) + 7) % 8);
        result |= (~_VALUE_MASK) * static_cast<bool>(bytes[MSB+offset] & SIGN_BIT_MASK);
      }
    }

    return result;
  }

  ///
  /// @brief Unpacks an integer from an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param bytes Array of bytes to read from.
  /// @param offset Offset in bytes to extract from of the array.
  /// @return Integer value.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static INT_T unpack(char* bytes, std::size_t offset=0)
  { return unpack<ENDIANNESS_V>(reinterpret_cast<unsigned char*>(bytes), offset); }

  ///
  /// @brief Unpacks an integer from a vector of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param bytes Vector of bytes to read from.
  /// @param offset Offset in bytes to extract from of the vector.
  /// @return Integer value.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static INT_T unpack(std::vector<unsigned char>& bytes, std::size_t offset=0)
  {
    #ifdef NUMIO_ENABLE_EXCEPTIONS
      if (bytes.size() + offset + AMOUNT_IO_BYTES > bytes.size()) {
        NumIOError("Out of bounds access of input vector `bytes`!");
      }
    #endif
    return unpack<ENDIANNESS_V>(bytes.data(), offset);
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
  static INT_T unpack(std::vector<char>& bytes, std::size_t offset=0)
  { return unpack<ENDIANNESS_V>(reinterpret_cast<unsigned char*>(bytes.data()), offset); }

  // :: PACKING FUNCTIONS :: //
  public:

  ///
  /// @brief Packs an integer to an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input integer value.
  /// @param bytes Array of bytes to write to.
  /// @param offset Offset in bytes where to write to the array.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(INT_T value, unsigned char* bytes, std::size_t offset=0)
  {
    // Isolate the bits that we're interested in.
    value &= _VALUE_MASK;

    constexpr auto ENDIANNESS_OFFSET = _get_endianness_offset(ENDIANNESS_V);

    // Copy the bits into the byte vector.
    if constexpr (!ENDIANNESS_OFFSET)
    {
      for (int i=0; i<AMOUNT_IO_BYTES; i++) {
        bytes[i+offset] = static_cast<unsigned char>((value >> (i * 8)) & 0xFF);
      }
    }
    // Reversed order.
    else
    {
      for (int i=0; i<_N_DATA_BYTES; i++) {
        bytes[ENDIANNESS_OFFSET-i+offset] = static_cast<unsigned char>((value >> (i * 8)) & 0xFF);
      }
    }

    return;
  }

  ///
  /// @brief Packs an integer to an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input integer value.
  /// @param bytes Array of bytes to write to.
  /// @param offset Offset in bytes where to write to the array.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(INT_T value, char* bytes, std::size_t offset=0)
  { pack<ENDIANNESS_V>(value, reinterpret_cast<unsigned char*>(bytes), offset); }

  ///
  /// @brief Packs an integer to a vector of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input integer value.
  /// @param bytes Vector of bytes to write to.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(INT_T value, std::vector<unsigned char>& bytes)
  {
    // Extend vector for packed data.
    auto offset = bytes.size();
    bytes.resize(offset+AMOUNT_IO_BYTES);
    pack<ENDIANNESS_V>(value, bytes.data(), offset);
  }

  ///
  /// @brief Packs an integer to a vector of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input integer value.
  /// @param bytes Vector of bytes to write to.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(INT_T value, std::vector<char>& bytes)
  { pack<ENDIANNESS_V>(value, *reinterpret_cast<std::vector<unsigned char>*>(&bytes)); }

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
    unsigned char buffer[AMOUNT_IO_BYTES] = { 0 };
    #ifdef NUMIO_ENABLE_EXCEPTIONS
      auto current_pos = s.tellg();
      s.read(reinterpret_cast<char*>(&buffer[0]), AMOUNT_IO_BYTES);
      if (s.tellg() - current_pos < AMOUNT_IO_BYTES) {
        NumIOError("Unexpected end of stream!");
      }
    #else
      s.read(reinterpret_cast<char*>(&buffer[0]), AMOUNT_IO_BYTES);
    #endif
    return unpack<ENDIANNESS_V>(&buffer[0], 0);
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
    unsigned char buffer[AMOUNT_IO_BYTES] = { 0 };
    pack<ENDIANNESS_V>(value, &buffer[0], 0);
    s.write(reinterpret_cast<char*>(&buffer[0]), AMOUNT_IO_BYTES);
    return;
  }
};

// *****************************************************************************

///
/// @brief Template class for doing floating point data I/O.
///
/// @tparam FLOAT_T Float type to (un)pack.
/// @tparam N_BITS_EXPONENT Specifies the amount of bits of the exponent part of the floating point data. Defaults to an
///         automatically calculated value if `FLOAT_T` is a built-in type or implements `std::numeric_limits<FLOAT_T>`.
/// @tparam N_BITS_FRACTION Specifies the amount of bits of the fraction part of the floating point data. Defaults to an
///         automatically calculated value if `FLOAT_T` is a built-in type or implements `std::numeric_limits<FLOAT_T>`.
/// @tparam ALIGNED_V Specifies if the data to (un)pack is aligned to match up with the amount of bytes as used by
///         the intermediate storage type `UINT_IO_T`.
/// @tparam UINT_IO_T Unsigned integer type used as intermediate storage for I/O retrieval and storage.
///
template <
  typename FLOAT_T,
  unsigned int N_BITS_EXPONENT=(sizeof(FLOAT_T)*8 - (std::numeric_limits<FLOAT_T>::digits-1) - 1),
  unsigned int N_BITS_FRACTION=(std::numeric_limits<FLOAT_T>::digits-1),
  bool ALIGNED_V=NUMIO_DEFAULT_ALIGN_V,
  typename UINT_IO_T=typename __FloatIO_container_type<FLOAT_T>::type
>
class FloatIO
{
  static_assert(std::is_floating_point_v<FLOAT_T>, "Template parameter FLOAT_T must be a float type!");

  static_assert(!std::is_same<UINT_IO_T, void>::value, "Template parameter UINT_IO_T must be manually assigned for non-standard float type!");
  static_assert(std::is_integral_v<UINT_IO_T>, "Template parameter UINT_IO_T must be an integer type!");
  static_assert(std::is_unsigned_v<UINT_IO_T>, "Template parameter UINT_IO_T must be an unsigned integer type!");

  // :: PRIVATE ATTRIBUTES ::
  private:

  // Takes care of asserting amount of bits not being more than being able to be stored by FLOAT_T and UINT_IO_T.
  using _INTIO_TYPE = IntIO<UINT_IO_T, (1+N_BITS_EXPONENT+N_BITS_FRACTION), ALIGNED_V>;

  static constexpr int _N_DATA_BYTES = sizeof(FLOAT_T);

  static constexpr int _EXPONENT_MASK = (static_cast<int>(1) << N_BITS_EXPONENT) - 1;
  static constexpr UINT_IO_T _FRACTION_MASK = (static_cast<UINT_IO_T>(1) << N_BITS_FRACTION) - 1;

  static constexpr int _EXPONENT_BIAS = (static_cast<int>(1) << (N_BITS_EXPONENT - 1)) - 1;
  static constexpr UINT_IO_T _FRACTION_DENOMINATOR = _FRACTION_MASK + 1;

  static constexpr int _EXPONENT_MAX = +_EXPONENT_BIAS;
  static constexpr int _EXPONENT_MIN = -_EXPONENT_BIAS + 1;

  static constexpr int _MIN_VAL_EXPONENT_NORMALIZED = _EXPONENT_MIN - N_BITS_FRACTION - 1;

  // :: PUBLIC ATTRIBUTES :: //
  public:

  ///
  /// @brief The amount of bytes used for the packed data.
  ///
  const static int AMOUNT_IO_BYTES = _N_DATA_BYTES;

  // :: UNPACKING FUNCTIONS :: //
  public:

  ///
  /// @brief Unpacks a float from an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param bytes Array of bytes to read from.
  /// @param offset Offset in bytes to extract from of the array.
  /// @return Float value.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static FLOAT_T unpack(unsigned char* bytes, std::size_t offset=0)
  {
    UINT_IO_T binary_data = _INTIO_TYPE::template unpack<ENDIANNESS_V>(bytes, offset);

    UINT_IO_T fraction_numerator = binary_data & _FRACTION_MASK;
    int exponent = static_cast<int>(binary_data >> N_BITS_FRACTION) & _EXPONENT_MASK;

    // Multiply result with this to make negative if sign bit set.
    int apply_sign = 1 - (
      ((binary_data >> (N_BITS_FRACTION + N_BITS_EXPONENT)) & 1)
      * 2
    );

    // Special values.
    if (exponent == _EXPONENT_MASK) {
      return fraction_numerator == 0
        ? std::numeric_limits<FLOAT_T>::infinity() * apply_sign
        : std::numeric_limits<FLOAT_T>::quiet_NaN();
    }

    // If the exponent is all zeros, but the mantissa is not then the value is a denormalized number.
    // This means this number does not have an assumed leading one before the binary point.
    int denormalized_adjust = (exponent != 0) & 1;

    FLOAT_T result = (
      std::pow(2.0, -_EXPONENT_BIAS + exponent + 1 - denormalized_adjust) // or use math.exp2(x).
      * (denormalized_adjust + static_cast<FLOAT_T>(fraction_numerator)/_FRACTION_DENOMINATOR)
      * apply_sign
    );

    return result;
  }

  ///
  /// @brief Unpacks a float from an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param bytes Array of bytes to read from.
  /// @param offset Offset in bytes to extract from of the array.
  /// @return Float value.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static FLOAT_T unpack(char* bytes, std::size_t offset=0)
  { return unpack<ENDIANNESS_V>(reinterpret_cast<unsigned char*>(bytes), offset); }

  ///
  /// @brief Unpacks a float from a vector of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param bytes Vector of bytes to read from.
  /// @param offset Offset in bytes to extract from of the vector.
  /// @return Float value.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static FLOAT_T unpack(std::vector<unsigned char>& bytes, std::size_t offset=0)
  {
    #ifdef NUMIO_ENABLE_EXCEPTIONS
      if (bytes.size() + offset + AMOUNT_IO_BYTES > bytes.size()) {
        NumIOError("Out of bounds access of input vector `bytes`!");
      }
    #endif
    return unpack<ENDIANNESS_V>(bytes.data(), offset);
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
  static FLOAT_T unpack(std::vector<char>& bytes, std::size_t offset=0)
  { return unpack<ENDIANNESS_V>(reinterpret_cast<unsigned char*>(bytes.data()), offset); }

  // :: PACKING FUNCTIONS :: //
  public:

  ///
  /// @brief Packs a float to an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input float value.
  /// @param bytes Array of bytes to write to.
  /// @param offset Offset in bytes where to write to the array.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(FLOAT_T value, unsigned char* bytes, std::size_t offset=0)
  {
    int sign = 0;
    // frexp() expects int as argument. No floating point format comes close to
    // needing more than 32 bits for exponent.
    int exponent = 0;
    UINT_IO_T fraction_numerator = 0;

    // Special values.
    if (std::isinf(value)) {
      exponent = _EXPONENT_MASK;
      sign = (value < 0) & 1;
    }
    else if (std::isnan(value)) {
      exponent = _EXPONENT_MASK;
      // Topmost bit of the fraction is used to set non-signaling/quiet NaN. Signaling NaNs would've
      // already caused exceptions in the program, handled by the FPU. All other bits stay 0.
      fraction_numerator = _FRACTION_DENOMINATOR >> 1;
    }
    // NOTE: This condition captures both +0.0 and -0.0.
    else if (value == 0.0) {
      // Use copysign() to differentiate between +0.0 and -0.0.
      sign = (std::copysign(1.0, value) < 0) & 1; // Ruby .negative? or similar also works.
    }
    // Normal values.
    else
    {
      // Extract sign bit and make input value absolute if negative.
      if (value < 0.0) {
        sign = 1;
        value = -value;
      }

      FLOAT_T fraction = std::frexp(value, &exponent);
      if (fraction < 0.5 || fraction >= 1.0) {
        #ifdef NUMIO_ENABLE_EXCEPTIONS
          throw NumIOError("frexp() result out of range!");
        #else
          std::abort();
        #endif
      }

      // Normalize fraction to be in the range [1.0, 2.0].
      fraction *= 2.0;
      exponent -= 1;

      if (exponent > _EXPONENT_MAX) {
        #ifdef NUMIO_ENABLE_EXCEPTIONS
          throw NumIOError("The floating point value is too large to be packed into the designated format!");
        #else
          std::abort();
        #endif
      }
      else if (exponent < _MIN_VAL_EXPONENT_NORMALIZED) {
        // Underflow to zero.
        fraction = 0;
        exponent = 0;
      }
      else if (exponent < _EXPONENT_MIN) {
        // Gradual underflow.
        fraction = std::ldexp(fraction, -_EXPONENT_MIN + exponent);
        exponent = 0;
      }
      else if (!(exponent == 0 && fraction == 0.0)) {
        exponent += _EXPONENT_BIAS;
        // Get rid of leading 1.
        fraction -= 1.0;
      }
      else {
        #ifdef NUMIO_ENABLE_EXCEPTIONS
          throw NumIOError("Reached an invalid or unsupported scenario while packing floating point value!");
        #else
          std::abort();
        #endif
      }

      // Turn into fractional numerator.
      fraction *= _FRACTION_DENOMINATOR;
      // // Truncate numerator, can also use floor() but probably slower.
      fraction_numerator = static_cast<UINT_IO_T>(fraction);
      // Round to even.
      if (
        (fraction - fraction_numerator > 0.5) ||
        ((fraction - fraction_numerator == 0.5) && (fraction_numerator % 2 == 1))
      ) {
        fraction_numerator += 1;
        if (fraction_numerator == _FRACTION_DENOMINATOR) {
          // Fraction overflows, carry to exponent.
          fraction_numerator = 0;
          exponent += 1;
          if (exponent >= _EXPONENT_MASK) {
            #ifdef NUMIO_ENABLE_EXCEPTIONS
              throw NumIOError("The floating point value is too large to be packed into the designated format!");
            #else
              std::abort();
            #endif
          }
        }
      }
    }

    UINT_IO_T binary_data =
      (static_cast<UINT_IO_T>(sign) << (N_BITS_EXPONENT + N_BITS_FRACTION))  |
      (static_cast<UINT_IO_T>(exponent & _EXPONENT_MASK) << N_BITS_FRACTION) |
      (fraction_numerator & _FRACTION_MASK);

    _INTIO_TYPE::template pack<ENDIANNESS_V>(binary_data, bytes, offset);

    return;
  }

  ///
  /// @brief Packs a float to an array of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input float value.
  /// @param bytes Array of bytes to write to.
  /// @param offset Offset in bytes where to write to the array.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(FLOAT_T value, char* bytes, std::size_t offset=0)
  { pack<ENDIANNESS_V>(value, reinterpret_cast<unsigned char*>(bytes), offset); }

  ///
  /// @brief Packs a float from a vector of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input float value.
  /// @param bytes Vector of bytes to write to.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(FLOAT_T value, std::vector<unsigned char>& bytes)
  {
    // Extend vector for packed data.
    auto offset = bytes.size();
    bytes.resize(offset+AMOUNT_IO_BYTES);
    pack<ENDIANNESS_V>(value, bytes.data(), offset);
  }

  ///
  /// @brief Packs a float from a vector of bytes.
  ///
  /// @tparam ENDIANNESS_V Defines the endianness of the data to process.
  /// @param value Input float value.
  /// @param bytes Vector of bytes to write to.
  ///
  template<Endian ENDIANNESS_V=NUMIO_DEFAULT_ENDIAN_V>
  static void pack(FLOAT_T value, std::vector<char>& bytes)
  { pack<ENDIANNESS_V>(value, *reinterpret_cast<std::vector<unsigned char>*>(&bytes)); }

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
    unsigned char buffer[AMOUNT_IO_BYTES] = { 0 };
    #ifdef NUMIO_ENABLE_EXCEPTIONS
      auto current_pos = s.tellg();
      s.read(reinterpret_cast<char*>(&buffer[0]), AMOUNT_IO_BYTES);
      if (s.tellg() - current_pos < AMOUNT_IO_BYTES) {
        NumIOError("Unexpected end of stream!");
      }
    #else
      s.read(reinterpret_cast<char*>(&buffer[0]), AMOUNT_IO_BYTES);
    #endif
    return unpack<ENDIANNESS_V>(&buffer[0], 0);
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
    unsigned char buffer[AMOUNT_IO_BYTES] = { 0 };
    pack<ENDIANNESS_V>(value, &buffer[0], 0);
    s.write(reinterpret_cast<char*>(&buffer[0]), AMOUNT_IO_BYTES);
    return;
  }
};

// *****************************************************************************

} /* namespace numio */
