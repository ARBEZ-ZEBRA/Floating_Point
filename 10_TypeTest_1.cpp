#include "../Utils/Ftype.hpp"
#include <bitset>

int main()
{
    double f = 1;
    double d = 2;
    double p = 3;
    double s = 1;

    // 使用 CA25 类型
    CA25 customValue(f);
    customValue.print_state();

    // 使用 Double 类型
    Double doubleValue(d);
    doubleValue.print_state();

    // 使用 Float 类型
    Float floatValue(d);
    floatValue.print_state();

    // 使用 Half 类型
    Half halfValue(d);
    halfValue.print_state();

    Double test(f);
    test.print_state();
    test += doubleValue;
    test.print_state();

    Double halfValue1 = p;
    test += halfValue1;
    test.print_state();

    test *= s;
    test.print_state();

    // Float t = test.exp();
    Double t = test.exp();
    t.print_state();

    std::cout << std::exp(6);
    return 0;
}