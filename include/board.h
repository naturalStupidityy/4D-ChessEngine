#pragma once
#include <iostream>
#include <vector>

constexpr int BOARD_SIZE = 4;
constexpr int TOTAL_SQUARES = 256;

enum Piece {
    EMPTY = 0,

    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,

    CAT
};

struct Move {
    int from;
    int to;
    int captured;
    int special;

    Move(int f=0,int t=0,int c=0,int s=0)
        : from(f), to(t), captured(c), special(s) {}
};

struct Undo {
    int captured;
    bool whiteToMove;
};

class Board {
public:
    int squares[TOTAL_SQUARES];
    bool whiteToMove;

    Board();

    // indexing
    static int index(int x,int y,int z,int t);
    static bool inBounds(int x,int y,int z,int t);
    static bool isEdge(int x,int y,int z,int t);

    // access
    int get(int x,int y,int z,int t) const;
    void set(int x,int y,int z,int t,int piece);

    // move handling
    void makeMove(const Move& move, Undo& undo);
    void undoMove(const Move& move, const Undo& undo);

    // utilities
    void clear();
    void print() const;
};