#include <stddef.h>
#include <math.h>
#include <stdio.h>

const long double x_min = -32;
const long double x_max = 32;
const long double _fmin = 0;

long double f(const long double x[], const size_t n)
{
    if (n == 0) {
        fputs("错误：f：维度为0\n", stderr);
        return nanl("");
    }
    long double temp1 = x[0] * x[0];
    long double temp2 = cosl(2 * M_PI * x[0]);
    for (size_t i = 1; i < n; ++i) {
        temp1 += x[i] * x[i];
        temp2 += cosl((long double)2 * M_PI * x[i]);
    }
    return -20 * expl(sqrtl(temp1 / n) / -5) - expl(temp2 / n) + 20 + M_E;
}
