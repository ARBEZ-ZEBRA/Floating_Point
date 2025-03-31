#ifndef FLOATINGPOINT_HPP_
#define FLOATINGPOINT_HPP_

#include <cstdint>
#include <string>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <bitset>

typedef enum STATE
{
    Normal,
    Subnormal,
    Zero,
    Inf,
    Nan
} State;

int findFirstOneBit(uint64_t bin_value)
{
    if (bin_value == 0)
        return 0;
    return 63 - __builtin_clzll(bin_value);
}

template <int exponent, int mantissa>
class FloatingPoint
{
private:
    bool sign;
    uint64_t E_value;
    int E_length = exponent;
    uint64_t E_mask = (1ULL << exponent) - 1;

    uint64_t M_value;
    int M_length = mantissa;
    uint64_t M_mask = (1ULL << mantissa) - 1;

    State state;

    void update_state()
    {
        if (E_value == 0 && M_value == 0)
        {
            state = Zero;
        }
        else if (E_value == 0 && M_value != 0)
        {
            state = Subnormal;
        }
        else if (E_value == E_mask && M_value == 0)
        {
            state = Inf;
        }
        else if (E_value == E_mask && M_value != 0)
        {
            state = Nan;
        }
        else
        {
            state = Normal;
        }
    }

public:
    bool get_sign() const { return sign; }
    uint64_t get_E_value() const { return E_value; }
    uint64_t get_M_value() const { return M_value; }
    int get_E_length() const { return E_length; }
    int get_M_length() const { return M_length; }
    uint64_t get_E_mask() const { return E_mask; }
    uint64_t get_M_mask() const { return M_mask; }

    // change the floatingpoint into binary form
    template <int other_exponent, int other_mantissa>
    uint64_t Bin(FloatingPoint<other_exponent, other_mantissa> value)
    {
        uint64_t result = (value.get_sign() << (value.get_E_length() + value.get_M_length())) |
                          ((value.get_E_value()) << value.get_M_length()) |
                          (value.get_M_value());
        return result;
    }

    // Default: Initialize to Floating Point 0
    FloatingPoint() : sign(false), E_value(0), M_value(0), state(Zero) {}

    // Initialize with binary value
    FloatingPoint(const uint64_t val_binary)
    {
        sign = (val_binary >> (exponent + mantissa)) & 1;
        E_value = (val_binary >> mantissa) & (E_mask);
        M_value = val_binary & (M_mask);
        update_state();
    }

    // Initialize with class type
    FloatingPoint(bool sign, uint64_t E_value, uint64_t M_value) : sign(sign), E_value(E_value), M_value(M_value)
    {
        update_state();
    }

    // Initialize with int value
    FloatingPoint(int value)
    {
        int F1 = findFirstOneBit(value);
        sign = (value < 0);
        E_value = F1 + (E_mask >> 1);
        M_value = (value & ((1ULL << F1) - 1)) << (mantissa - F1);
        update_state();
    }

    // Initialize with float value
    FloatingPoint(float value)
    {
        uint32_t bin_value = *reinterpret_cast<uint32_t *>(&value);
        // std::cout << std::bitset<32>(bin_value) << std::endl;
        FloatingPoint temp = std::move(Trans(8, 23, bin_value));
        sign = temp.get_sign();
        E_value = temp.get_E_value();
        M_value = temp.get_M_value();
        update_state();
    }

    // Initialize with double value
    FloatingPoint(double value)
    {
        uint64_t bin_value = *reinterpret_cast<uint64_t *>(&value);
        // std::cout << std::bitset<64>(bin_value) << std::endl;
        FloatingPoint temp = std::move(Trans(11, 52, bin_value));
        sign = temp.get_sign();
        E_value = temp.get_E_value();
        M_value = temp.get_M_value();
        update_state();
    }

    // Initialize with other floatingpoint value
    template <int other_exponent, int other_mantissa>
    FloatingPoint(const FloatingPoint<other_exponent, other_mantissa> &value)
    {
        uint64_t bin_value = Bin(value);
        // std::cout << std::bitset<64>(bin_value) << std::endl;
        FloatingPoint temp = std::move(Trans(other_exponent, other_mantissa, bin_value));
        sign = temp.get_sign();
        E_value = temp.get_E_value();
        M_value = temp.get_M_value();
        update_state();
    }

    // Initialize with other floatingpoint value
    template <int other_exponent, int other_mantissa>
    FloatingPoint(const FloatingPoint<other_exponent, other_mantissa> &&value)
    {
        uint64_t bin_value = Bin(value);
        // std::cout << std::bitset<64>(bin_value) << std::endl;
        FloatingPoint temp = std::move(Trans(other_exponent, other_mantissa, bin_value));
        sign = temp.get_sign();
        E_value = temp.get_E_value();
        M_value = temp.get_M_value();
        update_state();
    }

    template <int other_exponent, int other_mantissa>
    FloatingPoint& operator=(const FloatingPoint<other_exponent, other_mantissa> &value)
    {
        uint64_t bin_value = Bin(value);
        // std::cout << std::bitset<64>(bin_value) << std::endl;
        FloatingPoint temp = std::move(Trans(other_exponent, other_mantissa, bin_value));
        sign = temp.get_sign();
        E_value = temp.get_E_value();
        M_value = temp.get_M_value();
        update_state();

        return *this;
    }

    template <int other_exponent, int other_mantissa>
    FloatingPoint& operator=(const FloatingPoint<other_exponent, other_mantissa> &&value)
    {
        uint64_t bin_value = Bin(value);
        // std::cout << std::bitset<64>(bin_value) << std::endl;
        FloatingPoint temp = std::move(Trans(other_exponent, other_mantissa, bin_value));
        sign = temp.get_sign();
        E_value = temp.get_E_value();
        M_value = temp.get_M_value();
        update_state();
        
        return *this;
    }

    FloatingPoint& operator=(int value) {
        return FloatingPoint(value);
    }

    FloatingPoint& operator=(float value) {
        return FloatingPoint(value);
    }

    FloatingPoint& operator=(double value) {
        return FloatingPoint(value);
    }

    FloatingPoint& operator=(uint64_t value) {
        return FloatingPoint(value);
    }

    FloatingPoint Trans(int L_E, int L_M, uint64_t bin_value)
    {
        int L_S = 1;
        uint64_t S_other_mask = (0b1ULL << L_S) - 1;
        uint64_t E_other_mask = (0b1ULL << L_E) - 1;
        uint64_t M_other_mask = (0b1ULL << L_M) - 1;
        bool new_sign = (bin_value >> (L_E + L_M)) & S_other_mask;
        uint64_t E_other = (bin_value >> L_M) & E_other_mask;
        uint64_t M_other = bin_value & M_other_mask;
        uint64_t new_E_value, new_M_value;

        if (exponent > L_E && mantissa >= L_M)
        {
            if (E_other == 0 && M_other == 0)
            { // zero
                new_E_value = 0;
                new_M_value = 0;
            }
            else if (E_other == 0 && M_other != 0)
            { // subnormal
                new_E_value = 0;
                new_M_value = M_other << (mantissa - L_M);
            }
            else if (E_other == (E_other_mask) && M_other == 0)
            { // infinity
                new_E_value = E_mask;
                new_M_value = 0;
            }
            else if (E_other == (E_other_mask) && M_other != 0)
            { // NAN
                new_E_value = E_mask;
                new_M_value = M_other << (mantissa - L_M);
            }
            else
            { // normal
                new_E_value = E_other + (E_mask >> 1) - (E_other_mask >> 1);
                new_M_value = M_other << (mantissa - L_M);
            }
        }
        else if (exponent > L_E && mantissa < L_M)
        {
            if (E_other == 0 && M_other == 0)
            { // zero
                new_E_value = 0;
                new_M_value = 0;
            }
            else if (E_other == 0 && M_other != 0)
            { // subnormal
                new_E_value = 0;
                new_M_value = M_other >> (L_M - mantissa);
            }
            else if (E_other == (E_other_mask) && M_other == 0)
            { // infinity
                new_E_value = E_mask;
                new_M_value = 0;
            }
            else if (E_other == (E_other_mask) && M_other != 0)
            { // NAN
                new_E_value = E_mask;
                new_M_value = M_other >> (L_M - mantissa);
            }
            else
            { // normal
                new_E_value = E_other + (E_mask >> 1) - (E_other_mask >> 1);
                if ((M_other & ((0b1ULL << (L_M - mantissa)) - 1)) != 0)
                {
                    new_M_value = (M_other >> (L_M - mantissa)) + 1;
                    if (new_M_value > M_mask)
                    {
                        new_E_value += 1;
                        new_M_value -= (M_mask + 1);
                    }
                }
                else
                {
                    new_M_value = M_other >> (L_M - mantissa);
                }
            }
        }
        else if (exponent < L_E && mantissa >= L_M)
        {
            if (E_other == 0 && M_other == 0)
            { // zero
                new_E_value = 0;
                new_M_value = 0;
            }
            else if (E_other == 0 && M_other != 0)
            { // subnormal
                new_E_value = 0;
                new_M_value = M_other << (mantissa - L_M);
            }
            else if (E_other == (E_other_mask) && M_other == 0)
            { // infinity
                new_E_value = E_mask;
                new_M_value = 0;
            }
            else if (E_other == (E_other_mask) && M_other != 0)
            { // NAN
                new_E_value = E_mask;
                new_M_value = M_other << (mantissa - L_M);
            }
            else
            { // normal
                if (E_other > (E_mask >> 1) + (E_other_mask >> 1))
                { // infinity
                    new_E_value = E_mask;
                    new_M_value = 0;
                    std::cout << "n_inf" << std::endl;
                }
                else if (E_other + (E_mask >> 1) < 0b1 + (E_other_mask >> 1))
                {
                    new_E_value = 0;
                    if ((E_other_mask >> 1) - E_other >= mantissa + (E_mask >> 1))
                    { // subnormal
                        new_M_value = 0b1;
                    }
                    else
                    { // normal
                        M_other <<= (mantissa - L_M);
                        uint64_t temp = (M_other >> 1) | ((M_mask >> 1) + 1);
                        M_other = temp >> ((E_other_mask >> 1) - E_other - (E_mask >> 1));
                        if (temp > (M_other << ((E_other_mask >> 1) - E_other - (E_mask >> 1))))
                        {
                            new_M_value = M_other + 1;
                            if (new_M_value > M_mask)
                            {
                                new_E_value += 1;
                                new_M_value -= (M_mask + 1);
                            }
                            std::cout << "n_n_1" << std::endl;
                        }
                        else
                        {
                            new_M_value = M_other;
                            std::cout << "n_n_0" << std::endl;
                        }
                    }
                }
                else
                {
                    new_E_value = E_other + (E_mask >> 1) - (E_other_mask >> 1);
                    new_M_value = M_other << (mantissa - L_M);
                    std::cout << "n_0" << std::endl;
                }
            }
        }
        else if (L_E == exponent && L_M == mantissa)
        {
            new_E_value = E_other;
            new_M_value = M_other;
        }
        else
        {
            if (E_other == 0 && M_other == 0)
            { // zero
                new_E_value = 0;
                new_M_value = 0;
                // std::cout << "zero" << std::endl;
            }
            else if (E_other == 0 && M_other != 0)
            { // subnormal
                new_E_value = 0;
                new_M_value = 0b1;
                // std::cout << "sub" << std::endl;
            }
            else if (E_other == (E_other_mask) && M_other == 0)
            { // infinity
                new_E_value = E_mask;
                new_M_value = 0;
                // std::cout << "inf" << std::endl;
            }
            else if (E_other == (E_other_mask) && M_other != 0)
            { // NAN
                new_E_value = E_mask;
                new_M_value = M_other >> (L_M - mantissa);
                // std::cout << "nan" << std::endl;
            }
            else
            { // normal
                if (E_other > (E_mask >> 1) + (E_other_mask >> 1))
                { // infinity
                    new_E_value = E_mask;
                    new_M_value = 0;
                    // std::cout << "n_inf" << std::endl;
                }
                else if (E_other + (E_mask >> 1) < 0b1 + (E_other_mask >> 1))
                {
                    new_E_value = 0;
                    if ((E_other_mask >> 1) - E_other >= mantissa + (E_mask >> 1))
                    { // subnormal
                        new_M_value = 0b1;
                    }
                    else
                    { // normal
                        uint64_t temp = (M_other >> 1) | ((M_other_mask >> 1) + 1);
                        M_other = temp >> ((E_other_mask >> 1) - E_other - (E_mask >> 1));
                        if (((M_other & ((0b1ULL << (L_M - mantissa)) - 1)) != 0) || (temp > (M_other << ((E_other_mask >> 1) - E_other - (E_mask >> 1)))))
                        {
                            new_M_value = (M_other >> (L_M - mantissa)) + 1;
                            if (new_M_value > M_mask)
                            {
                                new_E_value += 1;
                                new_M_value -= (M_mask + 1);
                            }
                            // std::cout << "n_n_1" << std::endl;
                        }
                        else
                        {
                            new_M_value = M_other >> (L_M - mantissa);
                            // std::cout << "n_n_0" << std::endl;
                        }
                    }
                }
                else
                {
                    new_E_value = E_other + (E_mask >> 1) - (E_other_mask >> 1);
                    if ((M_other & ((0b1ULL << (L_M - mantissa)) - 1)) != 0)
                    {
                        new_M_value = (M_other >> (L_M - mantissa)) + 1;
                        if (new_M_value > M_mask)
                        {
                            new_E_value += 1;
                            new_M_value -= (M_mask + 1);
                            // std::cout << "n_1_0" << std::endl;
                        }
                        // std::cout << "n_1" << std::endl;
                    }
                    else
                    {
                        new_M_value = M_other >> (L_M - mantissa);
                        // std::cout << "n_0" << std::endl;
                    }
                }
            }
        }
        return FloatingPoint(new_sign, new_E_value, new_M_value);
    }

    /* Unary Operation */
    FloatingPoint neg() const {
        return FloatingPoint(!sign, E_value, M_value);
    }

    FloatingPoint operator-() const {
        return neg();
    }

    friend FloatingPoint neg(const FloatingPoint& value) {
        return value.neg();
    }

    FloatingPoint abs() const {
        return FloatingPoint(false, E_value, M_value);
    }

    friend FloatingPoint abs(const FloatingPoint& value) {
        return value.abs();
    }

    // Print the state of the floating point number
    void print_state() const
    {
        std::cout << "Sign: " << sign << "\n";
        std::cout << "Exponent: 0b" << std::bitset<exponent>(E_value) << "\n";
        std::cout << "Mantissa: 0b" << std::bitset<mantissa>(M_value) << "\n";
        std::cout << "State: ";
        switch (state)
        {
        case Normal:
            std::cout << "Normal\n";
            break;
        case Subnormal:
            std::cout << "Subnormal\n";
            break;
        case Zero:
            std::cout << "Zero\n";
            break;
        case Inf:
            std::cout << "Inf\n";
            break;
        case Nan:
            std::cout << "NaN\n";
            break;
        }
    }

    
};

#endif // FLOATINGPOINT_HPP_