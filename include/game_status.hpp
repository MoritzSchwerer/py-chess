#pragma once

#include <cstdint>

class GameStatus {
public:
    bool isWhite;
    bool wKingC;
    bool wQueenC;
    bool bKingC;
    bool bQueenC;
    bool enpassant;

    constexpr GameStatus() : isWhite(true), wKingC(true), wQueenC(true), bKingC(true), bQueenC(true), enpassant(false) {}
    constexpr GameStatus(uint8_t pattern) {
        isWhite   = (pattern & 0b000001) != 0;
        wKingC    = (pattern & 0b000010) != 0;
        wQueenC   = (pattern & 0b000100) != 0;
        bKingC    = (pattern & 0b001000) != 0;
        bQueenC   = (pattern & 0b010000) != 0;
        enpassant = (pattern & 0b100000) != 0;
    }
    GameStatus(bool isWhite, bool wKingC, bool wQueenC, bool bKingC, bool bQueenC, bool enpassant)
        : isWhite(isWhite), wKingC(wKingC), wQueenC(wQueenC), bKingC(bKingC), bQueenC(bQueenC), enpassant(enpassant) {}

    template<bool isWhite>
    void removeCastlingRightsLeft() {
        if constexpr (isWhite) wQueenC = false;
        else bQueenC = false;
    }

    template<bool isWhite>
    void removeCastlingRightsRight() {
        if constexpr (isWhite) wKingC = false;
        else bKingC = false;
    }

    template<bool isWhite>
    void removeCastlingRights() {
        removeCastlingRightsLeft<isWhite>();
        removeCastlingRightsRight<isWhite>();
    }

    uint8_t getStatusPattern() const {
        return isWhite | wKingC << 1 | wQueenC << 2 | bKingC << 3 | bQueenC << 4 | enpassant << 5;
    }

    void nextPlayer() {
        isWhite = !isWhite;
    }

    void wCastle() {
        wKingC = false;
        wQueenC = false;
    }

    void bCastle() {
        bKingC = false;
        bQueenC = false;
    }

    void wMoveRookLeft() {
        wQueenC = false;
    }

    void wMoveRookRight() {
        wKingC = false;
    }

    void bMoveRookLeft() {
        bQueenC = false;
    }

    void bMoveRookRight() {
        bKingC = false;
    }

    void enableEnpassant() {
        enpassant = true;
    }

    void disableEnpassant() {
        enpassant = false;
    }
};
