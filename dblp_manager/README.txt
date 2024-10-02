这是QT的资源文件，打开QT，选择打开文件或项目，选择CMakeLists.txt即可浏览。
不过源码用到了：
1.c++20特性(g++-10版本以上才支持)
2.openssl库(md5哈希算法)
所以默认的qt没法编译。我自己是先安装了qt6，然后安装msys2，在msys2中安装gcc,g++,gdb和openssl库，然后在qt中添加编译器，选择msys2中的gcc。

关于源码的说明：
后端代码包括：
my_data_struct.h
my_data_struct.cpp
init.h
split.h
hash.h
search.h
type_to_str.h
后端文件包括了程序所有的算法，其中my_data_struct.h包含了程序所有的数据结构

xml文件在此处下载：https://dblp.org/xml/
