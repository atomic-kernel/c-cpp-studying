#include <stddef.h>
#include <math.h>
#include <stdio.h>

const long double x_min = -5.12;
const long double x_max = 5.12;
const long double _fmin = 0;

long double f(const long double x[], const size_t n)
{
    if (n == 0) {
        fputs("错误：f：维度为0\n", stderr);
        return nanl("");
    }
    long double ret = 0;
    for (size_t i = 0; i < n; ++i) {
        ret += x[i] * x[i] - 10 * cosl((long double)2 * M_PI * x[i]);
    }
    return ret + 10 * n;
}
