cmake_minimum_required(VERSION 3.5)

project(main_gcc-10 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwidget.cpp
        mainwidget.h
        mainwidget.ui
        hash.h
        init.h
        my_data_struct.cpp
        my_data_struct.h
        search.h
        split.h
        d_search_essay.cpp
        d_search_essay.h
        d_search_essay.ui
        d_essay_list.cpp
        d_essay_list.ui
        d_essay_list.h
        my_error.h
        d_essay.cpp
        d_essay.h
        d_essay.ui
        type_to_str.h
        d_author.cpp
        d_author.h
        d_author.ui
        d_search_author.cpp
        d_search_author.h
        d_search_author.ui
        d_author_list.cpp
        d_author_list.h
        d_author_list.ui
        qobject_init.h
        d_init.cpp
        d_init.h
        d_init.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(main_gcc-10
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(main_gcc-10 SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(main_gcc-10
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(main_gcc-10 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets crypto)

set(CMAKE_CXX_FLAGS "-g0 -O3 -Wall -Wextra -pedantic -v -static -mwindows")



