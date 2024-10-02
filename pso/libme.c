#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void init_my_random();
long double my_random(long double min, long double max);
long double average(const long double x[], size_t n);
long double std_dev(const long double x[], size_t n, long double avg);

void init_my_random()
{
    srand((unsigned int)time(NULL));
}

long double my_random(const long double min, const long double max)
{
    if (min > max) {
        fputs("error: get random: min > max\n", stderr);
        return nanl("");
    }
    const long double ret = (long double)rand() / (long double)RAND_MAX * (max - min) + min;
    if (ret < min)
        return min;
    if (ret > max)
        return max;
    return ret;
}

long double average(const long double x[], const size_t n)
{
    if (n == 0) {
        fputs("error: average(): n == 0!\n", stderr);
        return nanl("");
    }
    long double ret = x[0];
    for (size_t i = 1; i < n; ++i)
        ret += x[i];
    return ret / n;
}

long double std_dev(const long double x[], const size_t n, const long double avg)
{
    if (n == 0) {
        fputs("error: average(): n == 0!\n", stderr);
        return nanl("");
    }
    long double ret = 0;
    for (size_t i = 0; i < n; ++i) {
        const long double temp = x[i] - avg;
        ret += temp * temp;
    }
    return sqrtl(ret);
}
