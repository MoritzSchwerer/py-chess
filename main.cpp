#include <vector>
#include <iostream>


#include "src/moves.hpp"
#include "src/utils.hpp"

int main() {

    Bitboard res = broadcastBit(10ull);
    printBinary(res);
    res = broadcastBit(11ull);
    printBinary(res);

    return 0;
}
