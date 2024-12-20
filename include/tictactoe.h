#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stack>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <random>

struct TTEntry {
    int score;  // cached score
    int depth;  // depth at which the score was computed
    enum BoundType { EXACT, LOWER, UPPER } flag;  // bounds
};
class TicTacToe {
private:
    std::unordered_map<unsigned long long, TTEntry> transpositionTable;  // transposition table
    unsigned long long zobristTable[100][100][3];  // Zobrist table for N x N (max 100x100)
    unsigned long long boardHash = 0;

    void initializeZobrist();  // Initialize Zobrist table
    void updateHash(int x, int y, int player);  // Update hash for moves

    std::string hashBoard(const std::vector<std::vector<int>>& boardCopy) const;

    std::vector<std::vector<int>> rotateBoard90(const std::vector<std::vector<int>>& boardCopy) const;

    std::vector<std::vector<int>> reflectBoardHorizontal(const std::vector<std::vector<int>>& boardCopy) const;

    std::string normalizeBoard() const;

    int countLines(int player, int length) const;

    std::vector<std::pair<int, int>> getOrderedMoves(int player);


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
    int moveNumber;
    int currentNode, totalNodes;

    std::vector<std::pair<int, int>> previousMoves;

    void resetNodeCounter();

    void fillBoard();

    bool checkLines(int symbol);

    bool checkDiagonals(int symbol);

    void checkGameState();

    void makeMove(int x, int y, int player);

    void undoMove(int x, int y);

    void printCurrentLine(int score) const;

    std::vector<std::pair<std::pair<int, int>, int>> getScoredMoves(int player, std::pair<int, int> prioritizedMove);

    int scoreMove(const std::pair<int, int>& move, int player);

    int calculateDepth(int maxDepth);

    int minimax(int depth, bool isMaximizing, int alpha, int beta);

    int evaluatePosition(bool isMaximizing);

    std::vector<std::pair<int, int>> getAvailableMoves();

    bool isLineBlocked(int x, int y, int player);

    bool isStrandedPiece(const std::pair<int, int>& move, int player);

    bool isImmediateThreat(const std::pair<int, int>& move, int opponent);

    int countThreatsBlocked(int x, int y, int opponent);

    void printDebugInfo(int depth, const std::vector<std::pair<int, int>>& moves, int player);

    bool isPotentialWinningLine(const std::vector<std::pair<int, int>>& line, int player);

    int countPotentialWinningLines(int player);

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
        winner(""),
        moveNumber(0),
        currentNode(0),
        totalNodes(0) {
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

    std::string getResult();

    std::pair<int, int> getBestMove(int depth, bool isMaximizing);

    int analyzeLastMove();
};
