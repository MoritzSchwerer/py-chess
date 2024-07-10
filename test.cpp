#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("My first test") {
    int x = 2;
    int y = 3;
    REQUIRE(x + y == 5);
}
