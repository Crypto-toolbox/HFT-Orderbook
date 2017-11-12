cmake_minimum_required(VERSION 2.8)
project(HFT_Orderbook)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/hftlob.h
        src/datastructs.c
        src/limits.c
        src/orders.c
        src/bst.c
        src/utils.c
        src/main.c
        src/CuTest.h
        src/CuTest.c
        src/testCases.c
        LICENSE
        lob.py
        orderbook_tests.py
        README.md)

add_executable(HFT_Orderbook ${SOURCE_FILES})

set(CMAKE_BUILD_TYPE Debug)