#!/bin/bash
if [ "$CC" == "" ]; then
    CC=gcc
fi
$CC libme.c -fPIC -shared -lm -o libme.so "${CFLAGS[@]}"
$CC pos_1.c -z undefs -Wl,--dynamic-list,dyn_list -L. -lme -lm -fPIC -pie -o pos_1 "${CFLAGS[@]}"
$CC pos_2.c -z undefs -Wl,--dynamic-list,dyn_list -L. -lme -lm -fPIC -pie -o pos_2 "${CFLAGS[@]}"
$CC pos_3.c -z undefs -Wl,--dynamic-list,dyn_list -L. -lme -lm -fPIC -pie -o pos_3 "${CFLAGS[@]}"
$CC pos_4.c -z undefs -Wl,--dynamic-list,dyn_list -L. -lme -lm -fPIC -pie -o pos_4 "${CFLAGS[@]}"

$CC f1.c -fPIC -shared -L. -lme -lm -o f1.so "${CFLAGS[@]}"
$CC f2.c -fPIC -shared -L. -lme -lm -o f2.so "${CFLAGS[@]}"
$CC f3.c -fPIC -shared -L. -lme -lm -o f3.so "${CFLAGS[@]}"
$CC f4.c -fPIC -shared -L. -lme -lm -o f4.so "${CFLAGS[@]}"
$CC f5.c -fPIC -shared -L. -lme -lm -o f5.so "${CFLAGS[@]}"
$CC f6.c -fPIC -shared -L. -lme -lm -o f6.so "${CFLAGS[@]}"
$CC f7.c -fPIC -shared -L. -lme -lm -o f7.so "${CFLAGS[@]}"
$CC f8.c -fPIC -shared -L. -lme -lm -o f8.so "${CFLAGS[@]}"
$CC f9.c -fPIC -shared -L. -lme -lm -o f9.so "${CFLAGS[@]}"
$CC f10.c -fPIC -shared -L. -lme -lm -o f10.so "${CFLAGS[@]}"
