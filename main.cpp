/*
    build with -DTYPE
    example:
        g++ -c -std=c++20 -DINT32_T main.cpp

    types = {INT16_T, INT32_T, INT64_T, FLOAT_T, DOUBLE_T, default=INT8_T}
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
using byte = uint8_t;

#if !defined(SET_TYPE)
    #define SET_TYPE(t, id)         \
        typedef t TYPE;             \
        auto TYPEID = id;

    #ifdef INT16_T
        SET_TYPE(int16_t, "16-bit"s)
    #elif defined(INT32_T)
        SET_TYPE(int32_t, "32-bit"s)
    #elif defined(INT64_T)
        SET_TYPE(int64_t, "64-bit"s)
    #elif defined(FLOAT_T)
        SET_TYPE(float, "single-precision"s)
    #elif defined(DOUBLE_T)
        SET_TYPE(double, "double-precision"s)
    #else
        #define NO_TYPE
    #endif
#endif

auto ignore_newl_from_cin(void) -> void
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

auto check_cin_failure(const std::string& error_msg) -> void
{
    if (std::cin.fail()) {
        std::cin.clear();
        ignore_newl_from_cin();
        throw std::runtime_error(error_msg);
    }
}

auto byte_to_bitstring(byte b) -> std::string
{
    auto oss = std::ostringstream{};
    constexpr auto BYTE_MSB = byte{128};

    for (int i = 0; i < 8; ++i) {
        oss << ((b & BYTE_MSB) != 0);
        b <<= 1;
    }

    return oss.str();
}

template <typename N>
union number_wrapper {
    N data;
    struct {
        std::array<byte, sizeof(N)> block;

        auto to_string(int base = 2) -> std::string
        {
            auto oss = std::ostringstream{};

            std::for_each(block.rbegin(), block.rend(), [=, &oss](byte b) -> void {
                switch (base) {
                    case 16 : {
                        oss.width(2);
                        oss.fill('0');
                        oss << std::hex << static_cast<int>(b) << ' ';
                    } break;
                    case 10 : {
                        oss << std::dec << static_cast<int>(b) << ' ';
                    } break;
                    case 8 : {
                        oss.width(3);
                        oss.fill('0');
                        oss << std::oct << static_cast<int>(b) << ' ';
                    } break;
                    case 2 : {
                        oss << byte_to_bitstring(b) << ' ';
                    } break;
                    default : {
                        throw std::invalid_argument("Expected an element of {2, 8, 10, 16}.");
                    };
                }
            });

            return oss.str();
        }
    } bytes;
};

auto main(void) -> int
{
    #ifdef DEBUG
        std::clog << "[WARNING] Program running in DEBUG mode.\n";
    #endif

    #ifdef NO_TYPE
        typedef int8_t TYPE;
        auto TYPEID = "8-bit"s;
    #endif

    try {
        std::cout << TYPEID << " number: ";
        auto n_buffer = TYPE{};
        if constexpr (std::is_same_v<TYPE, int8_t>) {
            int16_t int_buffer{};
            std::cin >> int_buffer;
            n_buffer = static_cast<TYPE>(int_buffer);
        } else {
            std::cin >> n_buffer;
        }

        check_cin_failure("Input must be a number."s);
        ignore_newl_from_cin();

        std::cout << "base: ";
        auto base_buffer = int{};
        std::cin >> base_buffer;

        check_cin_failure("Expected an element of [-2^(31), 2^(31)-1]."s);
        ignore_newl_from_cin();

        number_wrapper<TYPE> n;
        n.data = static_cast<TYPE>(n_buffer);

        std::cout << n.bytes.to_string(base_buffer) << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return EXIT_SUCCESS;
}