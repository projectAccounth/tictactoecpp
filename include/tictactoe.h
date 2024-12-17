#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stack>

struct TTEntry {
    int score;  // Cached score
    int depth;  // Depth at which the score was computed
    enum BoundType { EXACT, LOWER, UPPER } flag;  // Type of bound
};
class TicTacToe {
private:
    std::unordered_map<unsigned long long, TTEntry> transpositionTable;  // Transposition table
    unsigned long long zobristTable[100][100][3];  // Zobrist table for N x N (max 100x100)
    unsigned long long boardHash = 0;

    void initializeZobrist();  // Initialize Zobrist table
    void updateHash(int x, int y, int player);  // Update hash for moves

    std::string hashBoard(const std::vector<std::vector<int>>& boardCopy) const;

    std::vector<std::vector<int>> rotateBoard90(const std::vector<std::vector<int>>& boardCopy) const;

    std::vector<std::vector<int>> reflectBoardHorizontal(const std::vector<std::vector<int>>& boardCopy) const;

    std::string normalizeBoard() const;


    static constexpr int MIN_BOARD_SIZE = 3;
    int boardSizeX;
    int boardSizeY;
    int matchLength;
    bool isGameOver;
    bool isPositionInvalid;
    bool isDraw;
    std::string winner;
    std::vector<std::vector<int>> board;
    std::stack<std::pair<int, int>> currentLine;

    std::vector<std::pair<int, int>> previousMoves;

    void fillBoard();

    bool checkLines(int symbol);

    bool checkDiagonals(int symbol);

    void checkGameState();

    void makeMove(int x, int y, int player);

    void undoMove(int x, int y);

    void printCurrentLine(int score) const;

    std::vector<std::pair<std::pair<int, int>, int>> getScoredMoves(int player);

    int scoreMove(const std::pair<int, int>& move, int player);


public:
    bool isXTurn;
    bool isOTurn;
    TicTacToe(int boardSizeX, int boardSizeY, int matchLength)
        : boardSizeX(std::max(MIN_BOARD_SIZE, boardSizeX)),
        boardSizeY(std::max(MIN_BOARD_SIZE, boardSizeY)),
        matchLength(std::max(3, matchLength)),
        isGameOver(false),
        isXTurn(true),
        isOTurn(false),
        isPositionInvalid(false),
        isDraw(false),
        boardHash(0),
        winner("") {
        if (this->boardSizeX < matchLength || this->boardSizeY < matchLength) {
            throw std::invalid_argument("Invalid match length");
        }
        fillBoard();
        initializeZobrist();
    }

    bool move(int x, int y);

    std::string ascii() const;

    bool isOver() const;
    std::string getWinner() const;
    bool isDrawGame() const;

    void reset();

    int minimax(int depth, bool isMaximizing, int alpha, int beta);
    int evaluatePosition(bool isMaximizing);
    std::vector<std::pair<int, int>> getAvailableMoves();
    std::string getResult();
    std::pair<int, int> getBestMove(int depth, bool isMaximizing);
};
