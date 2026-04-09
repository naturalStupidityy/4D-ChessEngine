#include "board.h"

// constructor
Board::Board() {
    clear();
    whiteToMove = true;
}

// clear board
void Board::clear() {
    for(int i=0;i<TOTAL_SQUARES;i++)
        squares[i] = EMPTY;
}

// 4D -> 1D index
int Board::index(int x,int y,int z,int t) {
    return x +
           BOARD_SIZE*y +
           BOARD_SIZE*BOARD_SIZE*z +
           BOARD_SIZE*BOARD_SIZE*BOARD_SIZE*t;
}

// decode index -> coordinates
void Board::decode(int idx, int &x, int &y, int &z, int &t) {
    t = idx / 64;
    idx %= 64;

    z = idx / 16;
    idx %= 16;

    y = idx / 4;
    x = idx % 4;
}

// bounds check
bool Board::inBounds(int x,int y,int z,int t) {
    return (x>=0 && x<BOARD_SIZE &&
            y>=0 && y<BOARD_SIZE &&
            z>=0 && z<BOARD_SIZE &&
            t>=0 && t<BOARD_SIZE);
}

// piece helpers
bool Board::isWhite(int piece) {
    return piece >= W_PAWN && piece <= W_KING;
}

bool Board::isBlack(int piece) {
    return piece >= B_PAWN && piece <= B_KING;
}

bool Board::isEmpty(int piece) {
    return piece == EMPTY;
}

// piece to char (for printing)
char Board::pieceToChar(int piece) {
    switch(piece) {
        case W_PAWN: return 'P';
        case W_KNIGHT: return 'N';
        case W_BISHOP: return 'B';
        case W_ROOK: return 'R';
        case W_QUEEN: return 'Q';
        case W_KING: return 'K';

        case B_PAWN: return 'p';
        case B_KNIGHT: return 'n';
        case B_BISHOP: return 'b';
        case B_ROOK: return 'r';
        case B_QUEEN: return 'q';
        case B_KING: return 'k';

        default: return '.';
    }
}

// get piece
int Board::get(int x,int y,int z,int t) const {
    return squares[index(x,y,z,t)];
}

// set piece
void Board::set(int x,int y,int z,int t,int piece) {
    squares[index(x,y,z,t)] = piece;
}

// make move
void Board::makeMove(const Move& move, Undo& undo) {
    undo.captured = squares[move.to];
    undo.whiteToMove = whiteToMove;

    squares[move.to] = squares[move.from];
    squares[move.from] = EMPTY;

    whiteToMove = !whiteToMove;
}

// undo move
void Board::undoMove(const Move& move, const Undo& undo) {
    squares[move.from] = squares[move.to];
    squares[move.to] = undo.captured;

    whiteToMove = undo.whiteToMove;
}

// debug print
void Board::print() const {
    for(int t=0;t<BOARD_SIZE;t++) {
        std::cout << "Time: " << t << "\n";

        for(int z=0;z<BOARD_SIZE;z++) {
            std::cout << " Layer: " << z << "\n";

            for(int y=0;y<BOARD_SIZE;y++) {
                for(int x=0;x<BOARD_SIZE;x++) {
                    std::cout << pieceToChar(
                        squares[index(x,y,z,t)]
                    ) << " ";
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }
        std::cout << "-----------------------\n";
    }
}



