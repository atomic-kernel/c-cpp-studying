#!/bin/bash

echo "线性递减法，10维："
for ((i = 1; i <= 10; ++i))
do
    echo 10 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_1
done

echo "线性递减法，30维："
for ((i = 1; i <= 10; ++i))
do
    echo 30 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_1
done

echo "随机惯量权重法，10维："
for ((i = 1; i <= 10; ++i))
do
    echo 10 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_2
done

echo "随机惯量权重法，30维："
for ((i = 1; i <= 10; ++i))
do
    echo 30 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_2
done

echo "惯量权重凹函数递减法，10维："
for ((i = 1; i <= 10; ++i))
do
    echo 10 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_3
done

echo "惯量权重凹函数递减法，30维："
for ((i = 1; i <= 10; ++i))
do
    echo 30 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_3
done

echo "惯量权重凸函数递减法，10维："
for ((i = 1; i <= 10; ++i))
do
    echo 10 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_4
done

echo "惯量权重凸函数递减法，30维："
for ((i = 1; i <= 10; ++i))
do
    echo 30 | LD_LIBRARY_PATH='.' LD_PRELOAD="f${i}.so" ./pos_4
done
