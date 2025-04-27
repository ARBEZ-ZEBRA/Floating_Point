#ifndef FTYPE_HPP_
#define FTYPE_HPP_

#include <cmath>
#include <iostream>
#include "FloatingPoint_1.hpp"

using Half = FloatingPoint<5, 10>;    // 16-bit floating point (5 exponent, 10 mantissa)
using Float = FloatingPoint<8, 23>;   // 32-bit floating point (8 exponent, 23 mantissa)
using Double = FloatingPoint<11, 52>; // 64-bit floating point (11 exponent, 52 mantissa)
using CA25 = FloatingPoint<32, 31>;   // Custom 64-bit floating point (32 exponent, 31 mantissa)

#endif // FTYPE_HPP_