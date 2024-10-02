// 惯量权重线性递减法
// 收敛到了边缘，可能原因：
// 1. 初始化v
// 2. W_MAX W_MIN

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 设置 w_min w_max
#define W_MAX 0.9
#define W_MIN 0.4

// 设置c1, c2
#define C1 2
#define C2 2

// 设置种群大小
#define POP_SIZE 20

// 设置最大迭代次数
#define TIMES 2000

// 设置运行次数
#define RUNS 100

void init_my_random();
long double my_random(long double min, long double max);
long double average(long double x[], size_t n);
long double std_dev(long double x[], size_t n, long double avg);
long double f(long double x[], size_t n);
extern const long double x_min, x_max, _fmin;

int main()
{
    // 维度
    size_t wei;
    printf("请输入维度：\n");
    if (scanf("%llu", &wei) != 1) {
        fputs("error:输入维度失败！\n", stderr);
        return -1;
    }
    if (wei == 0) {
        fputs("error:维度 == 0!\n", stderr);
        return -1;
    }

    // 个体当前速度 v[pop_size][wei]
    long double (*const v)[wei] = (long double (*)[])malloc(sizeof(*v) * POP_SIZE);
    // 个体当前位置 x[pop_size][wei]
    long double (*const x)[wei] = (long double (*)[])malloc(sizeof(*x) * POP_SIZE);
    // 个体最优位置 pBest[pop_size][wei]
    long double (*const pBest)[wei] = (long double (*)[])malloc(sizeof(*pBest) * POP_SIZE);
    // 个体最优值 pBest_v[pop_size]
    long double *const pBest_v = (long double *)malloc(sizeof(long double) * POP_SIZE);
    // 全局最优的是哪个个体最优
    // 全局最优值：pBest_v[gBest]
    // 全局最优的位置：pBest[gBest]
    size_t gBest = 0;

    // 每次运行计算的结果：result[RUNS]
    long double *const result = (long double *)malloc(sizeof(long double) * RUNS);
    // 运行次数
    size_t runs = 0;

    init_my_random();
    const long double d = ((long double)W_MAX - W_MIN) / TIMES;

    for (size_t run_time = 0; run_time < RUNS; ++run_time) {
        // 初始化
        for (size_t i = 0; i < POP_SIZE; ++i) {
            for (size_t i2 = 0; i2 < wei; ++i2) {
                pBest[i][i2] = x[i][i2] = my_random(x_min, x_max);
                // 初始化速度为范围的10%
                v[i][i2] = 0.05 * my_random(x_min - x_max, x_max - x_min);
                //v[i][i2] = 0;
            }
            pBest_v[i] = f(x[i], wei);
            if (pBest_v[i] < pBest_v[gBest])
                gBest = i;
        }
        long double w = W_MAX;
        // 迭代
        for (size_t time = 0; time < TIMES; ++time) {
            // 先更新速度
            for (size_t i = 0; i < POP_SIZE; ++i) {
                // v[i] = v[i] + c1 * r1 * (pBest[i] - x[i]) + c2 * r2 * (pBest[gBest] - x[i])
                for (size_t i2 = 0; i2 < wei; ++i2) {
                    v[i][i2] = w * v[i][i2] + C1 * my_random(0, 1) * (pBest[i][i2] - x[i][i2]) + C2 * my_random(0, 1) * (pBest[gBest][i2] - x[i][i2]);
                    // 设置速度上限为范围的10%
                    if (v[i][i2] > 0.05 * (x_max - x_min)) {
                        v[i][i2] = 0.05 * (x_max - x_min);
                    }
                    else if (v[i][i2] < 0.05 * (x_min - x_max)) {
                        v[i][i2] = 0.05 * (x_min - x_max);
                    }
                    // 对于一些个体，速度接近0时给一点扰动
                    if (fabsl(v[i][i2]) <= 1e-8 * (x_max - x_min)) {
                        if (i % 6 == 0)
                            v[i][i2] = 0.05 * my_random(x_min - x_max, x_max - x_min);
                    }
                }
            }
            for (size_t i = 0; i < POP_SIZE; ++i) {
                // x[i] = x[i] + v[i]
                for (size_t i2 = 0; i2 < wei; ++i2) {
                    long double temp = x[i][i2] + v[i][i2];
                    // 到达边界时，反弹
                    if (temp < x_min) {
                        //v[i][i2] = my_random(0, 0.1 * (x_max - x_min));
                        v[i][i2] = -v[i][i2];
                        x[i][i2] = x_min;
                    }
                    else if (temp > x_max) {
                        //v[i][i2] = my_random(0.1 * (x_min - x_max), 0);
                        v[i][i2] = -v[i][i2];
                        x[i][i2] = x_max;
                    }
                    else
                        x[i][i2] = temp;
                }
                // 更新pBest,pBest_v,gBest
                const long double temp = f(x[i], wei);
                if (temp < pBest_v[i]) {
                    pBest_v[i] = temp;
                    for (size_t i2 = 0; i2 < wei; ++i2)
                        pBest[i][i2] = x[i][i2];
                    if (pBest_v[i] < pBest_v[gBest])
                        gBest = i;
                }
            }
            w -= d;
            /*
            printf("DEBUG: 计算了%lu次，结果:%lu,%Lf\n", time+1, gBest, pBest_v[gBest]);
            printf("DEBUG:各点位置：\n");
            for (size_t i = 0; i < POP_SIZE; ++i) {
                printf("DEBUG: %lu [ ", i);
                for (size_t i2 = 0; i2 < wei; ++i2) {
                    printf("%Lf, ", x[i][i2]);
                }
                printf("]\n");
            }
            printf("DEBUG:各点速度：\n");
            for (size_t i = 0; i < POP_SIZE; ++i) {
                printf("DEBUG: %lu [ ", i);
                for (size_t i2 = 0; i2 < wei; ++i2) {
                    printf("%Lf, ", v[i][i2]);
                }
                printf("]\n");
            }
            */
        }
        //printf("DEBUG: 保存result[%lu] = %Lf\n", runs,  pBest_v[gBest]);
        result[runs++] = pBest_v[gBest];
    }
    const long double aver = average(result, RUNS);
    printf("平均值：%.6Le , 标准差：%.6Le\n", average(result, RUNS), std_dev(result, RUNS, aver));

    free(result);
    free(pBest);
    free(pBest_v);
    free(v);
    free(x);
    return 0;
}
