
# numio-cpp

<!-- BADGES -->
<div align="left">
    <!--
        C++ standard
    --->
    <img src="https://img.shields.io/badge/C++-17+-informational.svg?labelColor=363d45&logo=cplusplus&logoColor=white"
    alt="C++17"/>
    <!--
        Library tag version
    --->
    <a href="https://github.com/deltarazero/numio-cpp/tags">
        <img src="https://img.shields.io/github/v/tag/deltarazero/numio-cpp?labelColor=363d45&logo=github&logoColor=white"
        alt="Latest release tag version"/></a>
    <!--
        License
    --->
    <a href="https://choosealicense.com/licenses/bsd-3-clause/">
        <img src="https://img.shields.io/github/license/DeltaRazero/numio-cpp?labelColor=363d45&color=informational"
        alt="BSD 3-Clause license"/></a>
</div>

numio-cpp is a C++17 header-only library that provides a set of template classes for flexible and platform-agnostic integer and float data I/O in a customizable and endian-safe manner.


## Getting Started

Copy the files from the `include` directory into your project's `include` directory.

You can then include `numio.hpp` or `numio/native.hpp` if you are targetting a system that has the same endianness as your current system.


## Usage

The `NumIO` namespace provides two main template classes: `IntIO` for integer data I/O and `FloatIO` for floating-point data I/O.

Following is a short description of features.


### Unpacking and Packing from Vectors

```cpp
std::vector<std::uint8_t> data_bytes = {0x01, 0x02, 0x03, 0x04};
int unpacked_value = NumIO::IntIO<std::int32_t>::unpack(data_bytes);

std::vector<std::uint8_t> data_bytes;
NumIO::IntIO<std::int32_t>::pack(1234, data_bytes);
```

### Reading/Writing from Streams

```cpp
std::ifstream input_file("input.bin", std::ios::binary);
std::int32_t value = NumIO::IntIO<std::int32_t>::read(input_file);

std::ofstream output_file("output.bin", std::ios::binary);
NumIO::IntIO<std::int32_t>::write(value, output_file);
```

### Endianness

The `ENDIANNESS_V` template parameter is used to specify the byte order of the data when (un)packing. The data is written correctly regardless of the system's native endianness. Expects a value from the enum class `NumIO::Endian`, which defines the following values:

* `Endian::LITTLE`: Specifies little-endian byte order, where the least significant byte is stored first (common on x86 and x86-64 architectures).
* `Endian::BIG`: Specifies big-endian byte order, where the most significant byte is stored first (common on some older architectures like Motorola 68k and in network protocols).
* `Endian::NATIVE`: Uses the byte order of the **system running the compiler**.
* `Endian::NETWORK`: Equivalent to `Endian::BIG` and is commonly used for network protocol data where big-endian byte order is prevalent.

> [!IMPORTANT]
> #### Ensuring Data Portability when Cross-Compiling
>
> When cross-compiling, it's crucial to specify the target system's endianness explicitly, since it is not possible to detect the target system's endianness at compile-time. Defining the system endianness explicitly ensures that data is processed correctly on target systems with a different endianness than the current system that is compiling.
>
> You can use either of these macros:
>
> * `NUMIO_SYSTEM_ENDIANNESS_V`: Sets the endianness based on macro variable name or integer value. Useful in particular when using CMake's [CMAKE\_\<LANG\>\_BYTE_ORDER](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_BYTE_ORDER.html) or equivalent. Takes one of the following values:
>   * `LITTLE_ENDIAN` or as integer value `1234`
>   * `BIG ENDIAN` or as integer value `4321`
> * `NUMIO_IS_SYSTEM_LITTLE_ENDIAN_V`: Set the endianness based on a boolean value.

### Predefined Types

The `numio/std.hpp` header provides standardized aliases for common data types. It allows you to work with integer and floating-point data without the need to specify custom bit widths or alignments. Here's a quick overview of the provided aliases:

#### Integer Types

| **Length**       | **Signed Type**  | **Unsigned Type**  |
|------------------|------------------|--------------------|
| 8-bit            | `NumIO::i8_IO`   | `NumIO::u8_IO`     |
| 16-bit           | `NumIO::i16_IO`  | `NumIO::u16_IO`    |
| 24-bit (packed)  | `NumIO::i24_IO`  | `NumIO::u24_IO`    |
| 24-bit (aligned) | `NumIO::i24a_IO` | `NumIO::u24a_IO`   |
| 32-bit           | `NumIO::i32_IO`  | `NumIO::u32_IO`    |
| 64-bit           | `NumIO::i64_IO`  | `NumIO::u64_IO`    |

#### Floating-Point Types (IEEE 754)

| **Precision**     | **Type**         |
|-------------------|------------------|
| 16-bit (`float`)  | `NumIO::fp16_IO` |
| 32-bit (`float`)  | `NumIO::fp32_IO` |
| 64-bit (`double`) | `NumIO::fp64_IO` |

#### Floating-Point Extra Types

Extra floating-point types, defined in `numio/fp_extra.hpp`.

| **Precision**                   | **Type**             |
|---------------------------------|----------------------|
| Google Brain bfloat16 (`float`) | `NumIO::bf16_IO`     |
| NVidia TensorFloat (`float`)    | `NumIO::nv_tf32_IO`  |
| AMD fp24 (`float`)              | `NumIO::amd_fp24_IO` |
| Pixar PXR24 (`float`)           | `NumIO::pxr24_IO`    |

### Custom Formats

In addition to the default integer types supported in C++, NumIO supports custom integer and floating-point formats with specified bit widths and alignment. This allows you to work with non-standard type representations. To work with custom formats, you can use the IntIO/FloatIO class and provide the necessary template parameters.

#### Integer Formats

```cpp
template <typename INT_T, unsigned int N_BITS, bool ALIGNED_V>
class IntIO;
```
* `INT_T`: This will be the container to store the value in.
* `N_BITS`: Specifies the data to (un)pack as an integer with a given amount of bits.
* `ALIGNED_V`: Specifies if the data to (un)pack is aligned to match up with the amount of bytes as used by the container type `INT_T`.

Example with a 12-bit integer:

```cpp
// Define I/O for unsigned 12-bit integer and aligned to 4 bytes
using uint12_IO = NumIO::IntIO<uint32_t, 12, true>;

std::vector<uint8_t> data_bytes = {0x00, 0x00, 0x12, 0x34}; // Represents a 12-bit integer value
uint32_t unpacked_value = uint12_IO::unpack(data_bytes);
```

#### Floating-Point Formats

```cpp
template <typename FLOAT_T, typename INT_IO_T, unsigned int N_BITS_EXPONENT, unsigned int N_BITS_FRACTION, bool ALIGNED_V>
class FloatIO;
```
* `FLOAT_T`: This will be the container to store the value in.
* `INT_IO_T`: Integer type used as intermediate storage for I/O retrieval and storage.
* `N_BITS_EXPONENT`: Specifies the amount of bits of the exponent part of the floating point data. Defaults to an automatically calculated value if `FLOAT_T` is a built-in type or implements `std::numeric_limits<FLOAT_T>`.
* `N_BITS_FRACTION`: Specifies the amount of bits of the fraction part of the floating point data. Defaults to an automatically calculated value if `FLOAT_T` is a built-in type or implements `std::numeric_limits<FLOAT_T>`.
* `ALIGNED_V`: Specifies if the data to (un)pack is aligned to match up with the amount of bytes as used by the intermediate storage type `INT_IO_T`.

Example with bfloat16:

```cpp
// Define I/O for a floating-point number with 8 bits for the exponent part and 7 bits for the fraction. Retrieved and stored in a 16-bit integer
using bfloat16_IO = FloatIO<float, std::uint16_t, 8, 7>;

std::vector<uint8_t> data_bytes = {0x3E, 0x20}; // Represents a bfloat16 value
float unpacked_value = bfloat16_IO::unpack(data_bytes);
```

### Customize Defaults

You can customize default behaviour by defining the following macros before including `numio.hpp`:

* `NUMIO_DEFAULT_ENDIAN_V`: Set the default endianness for (un)packing data when not explicitly setting the method template parameter `ENDIANNESS_V` (default is `NumIO::Endian::LITTLE`).
* `NUMIO_DEFAULT_ALIGN_V`: Set the default byte alignment for (un)packing data when not explicitly setting the class template parameter `ALIGNED_V` (default is `false`).


## License

© 2023 DeltaRazero. All rights reserved.

All included scripts, modules, etc. are licensed under the terms of the  [BSD 3-Clause license](https://github.com/deltarazero/numio-cpp/LICENSE), unless stated otherwise in the respective files.

### Acknowledgments

Sourcecode of CPython's implementation of floating-point data routines for the struct module served as basis and for insights resource for the implementation of packing floating-point data in this library. In particaluar, I thank the contributions of Eli Stevens, Mark Dickinson, and Victor Stinner.
