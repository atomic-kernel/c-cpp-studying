#include <stddef.h>
#include <math.h>
#include <stdio.h>

const long double x_min = -1.28;
const long double x_max = 1.28;
const long double _fmin = 0;

long double my_random(long double, long double);

long double f(const long double x[], const size_t n)
{
    if (n == 0) {
        fputs("错误：f：维度为0\n", stderr);
        return nanl("");
    }
    long double ret = 0;
    for (size_t i = 0; i < n;) {
        const long double temp = x[i] * x[i];
        ret += ++i * temp * temp;
    }
    return ret + my_random(0, 1);
}
