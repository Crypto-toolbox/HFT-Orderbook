cmake_minimum_required(VERSION 3.10)
project(HFT_Orderbook C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(HFTLOB_SANITIZE "Build with AddressSanitizer and UBSan" OFF)
if(HFTLOB_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# The order book itself, as a static library.
add_library(hftlob STATIC
        src/datastructs.c
        src/limits.c
        src/orders.c
        src/bst.c
        src/utils.c)
target_include_directories(hftlob PUBLIC src)
target_compile_options(hftlob PRIVATE -Wall -Wextra)
target_link_libraries(hftlob PUBLIC m)

# Test runner: `./HFT_Orderbook --test`
add_executable(HFT_Orderbook
        src/main.c
        src/CuTest.c
        src/testCases.c)
target_link_libraries(HFT_Orderbook PRIVATE hftlob)

enable_testing()
add_test(NAME cutest COMMAND HFT_Orderbook --test)
