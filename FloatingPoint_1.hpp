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

// 找到左数第一个一
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

    // Print the state of the floating point number
    void print_state() const
    {
        std::cout << "Decimal: " << To_decimal() << std::endl;
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

    State update_state(FloatingPoint value) const
    {
        if (value.E_value == 0 && value.M_value == 0)
        {
            return Zero;
        }
        else if (value.E_value == 0 && value.M_value != 0)
        {
            return Subnormal;
        }
        else if (value.E_value == value.E_mask && value.M_value == 0)
        {
            return Inf;
        }
        else if (value.E_value == value.E_mask && value.M_value != 0)
        {
            return Nan;
        }
        else
        {
            return Normal;
        }
    }

    double To_decimal() const
    {
        uint64_t bias = E_mask >> 1;

        uint64_t E = static_cast<uint64_t>(E_value & E_mask);
        if (E == 0)
        {
            double base = 0;
            E = 1ULL - bias;
        }
        else
        {
            E -= bias;
        }

        double M = M_value & M_mask;
        if (E != 1ULL - bias)
        {
            M += (M_mask + 1);
        }
        else
        {
            E = 0;
            M = 0;
        }

        double result = std::pow(2.0, E) * M / (M_mask + 1);

        if (sign)
        {
            result = -result;
        }

        return result;
    }

    static FloatingPoint<exponent, mantissa> createInfinity(bool sign)
    {
        FloatingPoint<exponent, mantissa> result;
        result.sign = sign;
        result.E_value = ((1ULL << exponent) - 1);
        result.M_value = 0;
        result.state = Inf;
        return result;
    }

    static FloatingPoint<exponent, mantissa> createZero()
    {
        FloatingPoint<exponent, mantissa> result;
        result.sign = 0;
        result.E_value = 0;
        result.M_value = 0;
        result.state = Zero;
        return result;
    }

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
        sign = (value < 0);
        if (value < 0)
            value = -value;
        int F1 = findFirstOneBit(value);
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
    FloatingPoint &operator=(const FloatingPoint<other_exponent, other_mantissa> &value)
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
    FloatingPoint &operator=(const FloatingPoint<other_exponent, other_mantissa> &&value)
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

    FloatingPoint &operator=(int value)
    {
        return FloatingPoint(value);
    }

    FloatingPoint &operator=(float value)
    {
        return FloatingPoint(value);
    }

    FloatingPoint &operator=(double value)
    {
        return FloatingPoint(value);
    }

    FloatingPoint &operator=(uint64_t value)
    {
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
                        }
                        else
                        {
                            new_M_value = M_other;
                        }
                    }
                }
                else
                {
                    new_E_value = E_other + (E_mask >> 1) - (E_other_mask >> 1);
                    new_M_value = M_other << (mantissa - L_M);
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
            }
            else if (E_other == 0 && M_other != 0)
            { // subnormal
                new_E_value = 0;
                new_M_value = 0b1;
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
                if (E_other > (E_mask >> 1) + (E_other_mask >> 1))
                { // infinity
                    new_E_value = E_mask;
                    new_M_value = 0;
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
                        }
                        else
                        {
                            new_M_value = M_other >> (L_M - mantissa);
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
                        }
                    }
                    else
                    {
                        new_M_value = M_other >> (L_M - mantissa);
                    }
                }
            }
        }
        return FloatingPoint(new_sign, new_E_value, new_M_value);
    }

    /* Unary Operation */
    FloatingPoint neg() const
    {
        return FloatingPoint(!sign, E_value, M_value);
    }

    FloatingPoint operator-() const
    {
        return neg();
    }

    friend FloatingPoint neg(const FloatingPoint &value)
    {
        return value.neg();
    }

    FloatingPoint abs() const
    {
        return FloatingPoint(false, E_value, M_value);
    }

    friend FloatingPoint abs(const FloatingPoint &value)
    {
        return value.abs();
    }

    FloatingPoint sqrt() const
    {
        if (sign == true)
        {
            std::cout << "Negative Input!!!" << std::endl;
        }
        else
        {
        }
    }

    FloatingPoint relu() const
    {
        if (sign)
        {
            return createZero();
        }
        else
        {
            return (*this);
        }
    }
    friend FloatingPoint relu(const FloatingPoint &fp)
    {
        return fp.relu();
    }

    // Addition
    FloatingPoint add(const FloatingPoint &other) const
    {
        if (state == Nan)
        {
            return *this;
        }
        else if (other.state == Nan)
        {
            return other;
        }

        if (state == Inf)
        {
            if (other.state == Inf)
            {
                return (sign == other.sign) ? *this : createZero();
            }
            return *this;
        }

        if (other.state == Inf)
        {
            return other;
        }

        if (state == Zero)
        {
            return other;
        }

        if (other.state == Zero)
        {
            return *this;
        }

        // both normal case
        bool sign1 = sign;
        int64_t exp1 = static_cast<int64_t>(E_value) - (E_mask >> 1);
        uint64_t mantissa1 = (state == Subnormal) ? M_value : M_value | (M_mask + 1);

        bool sign2 = other.sign;
        int64_t exp2 = static_cast<int64_t>(other.E_value) - (E_mask >> 1);
        uint64_t mantissa2 = (other.state == Subnormal) ? other.M_value : other.M_value | (M_mask + 1);

        // Align exponents by shifting the smaller number's mantissa right
        int64_t exp_diff = exp1 - exp2;
        if (exp_diff < 0)
        {
            std::swap(sign1, sign2);
            std::swap(exp1, exp2);
            std::swap(mantissa1, mantissa2);
            exp_diff = -exp_diff;
        }

        if (exp_diff > static_cast<int64_t>(mantissa))
        {
            // The smaller number is too small to affect the larger one
            return *this;
        }

        mantissa1 <<= exp_diff;

        // Perform addition or subtraction based on signs
        uint64_t result_mantissa;
        bool result_sign;
        if (sign1 == sign2)
        {
            result_sign = sign1;
            result_mantissa = mantissa1 + mantissa2;
        }
        else
        {
            if (mantissa1 > mantissa2)
            {
                result_mantissa = mantissa1 - mantissa2;
                result_sign = sign1;
            }
            else
            {
                result_mantissa = mantissa2 - mantissa1;
                result_sign = sign2;
            }
        }

        // Normalize the result
        if (result_mantissa == 0)
        {
            return createZero();
        }

        if ((result_mantissa & ((1ULL << exp_diff) - 1)) > 0)
        {
            result_mantissa >>= exp_diff;
            result_mantissa += 1;
        }
        else
        {
            result_mantissa >>= exp_diff;
        }

        int first = findFirstOneBit(result_mantissa);

        if (first >= mantissa)
        {
            exp1 += (first - mantissa);
            if (exp1 >= E_mask)
            {
                exp1 = E_mask;
                result_mantissa = 0;
            }
            else
            {
                result_mantissa &= ((1ULL << first) - 1);
                if ((result_mantissa & ((1ULL << (first - mantissa)) - 1)) > 0)
                {
                    result_mantissa >>= (first - mantissa);
                    result_mantissa += 1;
                }
                else
                {
                    result_mantissa >>= (first - mantissa);
                }
            }
        }
        else
        {
            if (exp1 + (E_mask >> 1) > (mantissa - first))
            {
                exp1 -= (mantissa - first);
                result_mantissa <<= (mantissa - first);
            }
            else
            {
                result_mantissa <<= (exp1 + (E_mask >> 1));
                exp1 = -(E_mask >> 1);
            }
        }

        // Pack the result
        FloatingPoint<exponent, mantissa> result;
        result.sign = result_sign;
        result.E_value = static_cast<uint64_t>(exp1 + (E_mask >> 1));
        result.M_value = result_mantissa & (M_mask);
        result.state = update_state(result);

        return result;
    }
    friend FloatingPoint add(const FloatingPoint &fp1, const FloatingPoint &fp2)
    {
        return fp1.add(fp2);
    }
    FloatingPoint operator+(const FloatingPoint &other) const
    {
        return add(other);
    };
    friend FloatingPoint operator+(int val, const FloatingPoint &fp)
    {
        return FloatingPoint(val) + fp;
    }
    friend FloatingPoint operator+(const FloatingPoint &fp, int val)
    {
        return fp + FloatingPoint(val);
    }
    friend FloatingPoint operator+(double val, const FloatingPoint &fp)
    {
        return FloatingPoint(val) + fp;
    }
    friend FloatingPoint operator+(const FloatingPoint &fp, double val)
    {
        return fp + FloatingPoint(val);
    }
    FloatingPoint operator+=(const FloatingPoint &other)
    {
        *this = add(other);
        return *this;
    }
    FloatingPoint operator+=(int val)
    {
        *this = add(FloatingPoint(val));
        return *this;
    }
    FloatingPoint operator+=(double val)
    {
        *this = add(FloatingPoint(val));
        return *this;
    }

    // Subtraction
    FloatingPoint sub(const FloatingPoint &other) const
    {
        return add(other.neg());
    }
    friend FloatingPoint sub(const FloatingPoint &fp1, const FloatingPoint &fp2)
    {
        return fp1.sub(fp2);
    }
    FloatingPoint operator-(const FloatingPoint &other) const
    {
        return sub(other);
    };
    friend FloatingPoint operator-(int val, const FloatingPoint &fp)
    {
        return FloatingPoint(val) - fp;
    }
    friend FloatingPoint operator-(const FloatingPoint &fp, int val)
    {
        return fp - FloatingPoint(val);
    }
    friend FloatingPoint operator-(double val, const FloatingPoint &fp)
    {
        return FloatingPoint(val) - fp;
    }
    friend FloatingPoint operator-(const FloatingPoint &fp, double val)
    {
        return fp - FloatingPoint(val);
    }
    FloatingPoint operator-=(const FloatingPoint &other)
    {
        *this = sub(other);
        return *this;
    }
    FloatingPoint operator-=(int val)
    {
        *this = sub(FloatingPoint(val));
        return *this;
    }
    FloatingPoint operator-=(double val)
    {
        *this = sub(FloatingPoint(val));
        return *this;
    }

    // Comparison Operator
    bool operator==(const FloatingPoint &other) const
    {
        if ((state == Nan) || (other.state == Nan))
        {
            std::cout << "The input of compare function can not be NAN !!!" << std::endl;
            return 0;
        }
        return sign == other.sign && E_value == other.E_value && M_value == other.M_value;
    };
    bool operator!=(const FloatingPoint &other) const
    {
        return !(*this == other);
    };
    bool operator<(const FloatingPoint &other) const
    {
        if ((state == Nan) || (other.state == Nan))
        {
            std::cout << "The input of compare function can not be NAN !!!" << std::endl;
            return 0;
        }
        if (sign != other.sign)
        {
            return sign;
        }
        else if (E_value != other.E_value)
        {
            return sign ? E_value > other.E_value : E_value < other.E_value;
        }
        else
        {
            return sign ? M_value > other.M_value : M_value < other.M_value;
        }
    };
    bool operator<=(const FloatingPoint &other) const
    {
        return *this < other || *this == other;
    };
    bool operator>(const FloatingPoint &other) const
    {
        return !(*this <= other);
    };
    bool operator>=(const FloatingPoint &other) const
    {
        return !(*this < other);
    };
};

#endif // FLOATINGPOINT_HPP_