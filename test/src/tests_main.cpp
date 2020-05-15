#include <gtest/gtest.h>

#define FFF_GCC_FUNCTION_ATTRIBUTES __attribute__((weak))

#include "fff.h"
DEFINE_FFF_GLOBALS

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}