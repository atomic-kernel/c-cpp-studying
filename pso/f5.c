#include <stddef.h>
#include <math.h>
#include <stdio.h>

const long double x_min = -30;
const long double x_max = 30;
const long double _fmin = 0;

long double f(const long double x[], const size_t n)
{
    if (n <= 1) {
        fputs("错误：f：维度<=1\n", stderr);
        return nanl("");
    }
    long double ret = 0;
    for (size_t i = 0, max = n - 1; i != max; ++i) {
        const long double temp1 = x[i + 1] - x[i] * x[i];
        const long double temp2 = x[i] - 1;
        ret += 100 * temp1 * temp1 + temp2 * temp2;
    }
    return ret;
}
