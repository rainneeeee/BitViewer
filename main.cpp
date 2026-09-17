/*
    - build with -DTYPE (define TYPE)
        example:
            g++ -c -std=c++20 -DINT32_T main.cpp
    - will use int8_t by default (without defining TYPE)
    - types = {INT16_T, INT32_T, INT64_T, FLOAT_T, DOUBLE_T}
*/

#include <iostream>
#include <iomanip>

#include <string>
#include <sstream>

#include <array>
#include <algorithm>

#include <limits>
#include <stdexcept>
#include <type_traits>

#include <cstdlib>
#include <cstdint>

using namespace std::string_literals;
using byte_t = uint8_t;

#ifndef SET_TYPE
    #define SET_TYPE(t, id)     \
        typedef t TYPE;         \
        auto TYPEID = id;

    #ifdef INT16_T
        SET_TYPE(int16_t, "16-bit")
    #elif defined(INT32_T)
        SET_TYPE(int32_t, "32-bit")
    #elif defined(INT64_T)
        SET_TYPE(int64_t, "64-bit")
    #elif defined(FLOAT_T)
        SET_TYPE(float, "single-precision"s)
    #elif defined(DOUBLE_T)
        SET_TYPE(double, "double-precision"s)
    #else
        #define DEFAULT_TYPE
    #endif
#endif

auto ignore_newl_from_cin(void) -> void
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template <typename T>
auto check_cin_failure(void) -> void
{
    static_assert(std::is_arithmetic_v<T>, "Template must be an arithmetic type!");

    if (std::cin.fail()) {
        std::cin.clear();
        ignore_newl_from_cin();

        auto error_stream = std::ostringstream{};
        error_stream << "Expected an element of [";
        /*
            - adding '+' prefix to fix int8_t printing issue in lowest() and max()
            - issue: printing ASCII characters when TYPE is int8_t
        */
        error_stream << +std::numeric_limits<T>::lowest() << ", ";
        error_stream << +std::numeric_limits<T>::max() << "].";

        throw std::runtime_error(error_stream.str());
    }
}

auto byte_to_bitstring(byte_t b) -> std::string
{
    auto bit_stream = std::ostringstream{};
    constexpr auto BYTE_MSB = byte_t{128U};

    /*
        - convert byte to binary by reading bits from MSB to LSB
        - starting with MSB to convert from little-endian to big-endian format
    */
    for (int i = 0; i < 8; ++i) {
        bit_stream << ((b & BYTE_MSB) != 0U);
        b <<= 1;
    }

    return bit_stream.str();
}

template <typename N>
union base10_number_wrapper {
    N data;
    struct {
        std::array<byte_t, sizeof(N)> byte_array;

        auto to_string(int radix = 2) -> std::string
        {
            auto byte_stream = std::ostringstream{};

            // convert bytes to the target radix format
            std::for_each(byte_array.rbegin(), byte_array.rend(), [=, &byte_stream](byte_t b) -> void {
                switch (radix) {
                    case 16 : {
                        byte_stream.width(2);
                        byte_stream.fill('0');
                        byte_stream << std::hex << static_cast<int>(b) << ' ';
                    } break;
                    case 8 : {
                        byte_stream.width(3);
                        byte_stream.fill('0');
                        byte_stream << std::oct << static_cast<int>(b) << ' ';
                    } break;
                    case 2 : {
                        byte_stream << byte_to_bitstring(b) << ' ';
                    } break;
                    default : {
                        throw std::invalid_argument("Expected an element of {2, 8, 16}.");
                    };
                }
            });

            return byte_stream.str();
        }
    } bytes;
};

auto main(void) -> int
{
    #ifdef DEBUG
        std::clog << "\n[WARNING] Program running in DEBUG mode.\n\n";
    #endif

    #ifdef DEFAULT_TYPE
        typedef int8_t TYPE;
        auto TYPEID = "8-bit"s;
    #endif

    try {
        std::cout << TYPEID << " base10 number: ";
        auto temp_base10_n = TYPE{};
        if constexpr (std::is_same_v<TYPE, int8_t>) {
            // had to do this because int8_t is being treated as a char type
            int16_t expanded_buffer{};
            std::cin >> expanded_buffer;

            if (expanded_buffer > INT8_MAX || expanded_buffer < INT8_MIN) {
                std::cin.setstate(std::ios_base::failbit);
            }

            temp_base10_n = static_cast<int8_t>(expanded_buffer);
        } else {
            std::cin >> temp_base10_n;
        }

        check_cin_failure<TYPE>();
        ignore_newl_from_cin();

        std::cout << "radix: ";
        auto temp_radix = int{};
        std::cin >> temp_radix;

        check_cin_failure<int>();
        ignore_newl_from_cin();

        base10_number_wrapper<TYPE> base10_n;
        base10_n.data = temp_base10_n;

        std::cout << base10_n.bytes.to_string(temp_radix) << '\n';
        if constexpr (std::is_integral_v<TYPE>) {
            auto base10_n_unsigned = static_cast<std::make_unsigned_t<TYPE>>(base10_n.data);
            std::cout << "unsigned representation: " << base10_n_unsigned << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << '\n' << e.what() << '\n';
    }

    return EXIT_SUCCESS;
}