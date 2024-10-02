#include <stddef.h>
#include <math.h>
#include <stdio.h>

const long double x_min = -10;
const long double x_max = 10;
const long double _fmin = 0;

long double f(const long double x[], const size_t n)
{
    if (n == 0) {
        fputs("错误：f：维度为0\n", stderr);
        return nanl("");
    }
    long double ret = fabsl(x[0]), temp = ret;
    for (size_t i = 1; i != n; ++i) {
        const long double tt = fabsl(x[i]);
        ret += tt;
        temp *= tt;
    }
    return ret + temp;
}
