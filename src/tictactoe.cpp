#include "../include/tictactoe.h"
#include <random>
#include <regex>
#include <sstream>
#include <algorithm>
#include <cmath>

std::random_device rd;

void TicTacToe::fillBoard() {
    board.resize(boardSizeY, std::vector<int>(boardSizeX, 0));
}

void TicTacToe::initializeZobrist() {
    std::mt19937_64 rng(rd());  // Seeded random number generator
    for (int y = 0; y < boardSizeY; ++y) {
        for (int x = 0; x < boardSizeX; ++x) {
            for (int p = 0; p < 3; ++p) {  // 0 = empty, 1 = O, 2 = X
                zobristTable[x][y][p] = rng();
            }
        }
    }
}

std::string TicTacToe::hashBoard(const std::vector<std::vector<int>>& boardCopy) const {
    std::string hash;
    for (const auto& row : boardCopy) {
        for (int cell : row) {
            hash += std::to_string(cell);  // Append cell values as a string
        }
    }
    return hash;
}

std::vector<std::vector<int>> TicTacToe::rotateBoard90(const std::vector<std::vector<int>>& boardCopy) const {
    int N = boardCopy.size();
    std::vector<std::vector<int>> rotatedBoard(N, std::vector<int>(N, 0));

    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            rotatedBoard[x][N - y - 1] = boardCopy[y][x];  // Rotate 90 degrees clockwise
        }
    }

    return rotatedBoard;
}

std::vector<std::vector<int>> TicTacToe::reflectBoardHorizontal(const std::vector<std::vector<int>>& boardCopy) const {
    int N = boardCopy.size();
    std::vector<std::vector<int>> reflectedBoard = boardCopy;

    for (int y = 0; y < N; ++y) {
        reflectedBoard[N - y - 1] = boardCopy[y];  // Swap rows for reflection
    }

    return reflectedBoard;
}

std::string TicTacToe::normalizeBoard() const {
    std::vector<std::string> symmetries;

    // Start with the current board
    auto currentBoard = board;
    symmetries.push_back(hashBoard(currentBoard));

    // Generate all rotations and reflections
    auto rotated = currentBoard;

    for (int i = 0; i < 3; ++i) {
        rotated = rotateBoard90(rotated);  // Rotate 90 degrees clockwise
        symmetries.push_back(hashBoard(rotated));
    }

    // Reflect and generate rotations again
    auto reflected = reflectBoardHorizontal(currentBoard);
    symmetries.push_back(hashBoard(reflected));

    rotated = reflected;
    for (int i = 0; i < 3; ++i) {
        rotated = rotateBoard90(rotated);  // Rotate reflected board
        symmetries.push_back(hashBoard(rotated));
    }

    // Return the smallest hash lexicographically
    return *std::min_element(symmetries.begin(), symmetries.end());
}


void TicTacToe::updateHash(int x, int y, int player) {
    boardHash ^= zobristTable[x - 1][y - 1][player];  // XOR with Zobrist value
}

bool TicTacToe::checkLines(int symbol) {
    for (int i = 0; i < boardSizeY; ++i) {
        int rowMatch = 0;
        int colMatch = 0;
        for (int j = 0; j < boardSizeX; ++j) {
            // Row check
            if (board[i][j] == symbol) {
                if (++rowMatch == matchLength) return true;
            }
            else {
                rowMatch = 0;
            }

            // Column check
            if (board[j][i] == symbol) {
                if (++colMatch == matchLength) return true;
            }
            else {
                colMatch = 0;
            }
        }
    }
    return false;
}

bool TicTacToe::checkDiagonals(int symbol) {
    for (int i = 0; i <= boardSizeY - matchLength; ++i) {
        for (int j = 0; j <= boardSizeX - matchLength; ++j) {
            int mainDiagMatch = 0;
            int antiDiagMatch = 0;

            for (int k = 0; k < matchLength; ++k) {
                if (board[i + k][j + k] == symbol) {
                    if (++mainDiagMatch == matchLength) return true;
                }
                else {
                    mainDiagMatch = 0;
                }

                if (board[i + k][j + matchLength - k - 1] == symbol) {
                    if (++antiDiagMatch == matchLength) return true;
                }
                else {
                    antiDiagMatch = 0;
                }
            }
        }
    }
    return false;
}

void TicTacToe::checkGameState() {
    if (checkLines(2) || checkDiagonals(2)) {
        isGameOver = true;
        winner = "X";
    }
    else if (checkLines(1) || checkDiagonals(1)) {
        isGameOver = true;
        winner = "O";
    }
    else {
        int emptyCells = boardSizeX * boardSizeY;
        for (const auto& row : board) {
            emptyCells -= std::count(row.begin(), row.end(), 0);
        }
        if (emptyCells == 0) {
            isDraw = true;
            isGameOver = true;
        }
    }
}

void TicTacToe::reset() {
    fillBoard();
    isGameOver = (false);
    isXTurn = (true);
    isOTurn = (false);
    isPositionInvalid = (false);
    isDraw = (false);
    previousMoves = std::vector<std::pair<int, int>>();
    checkGameState();
}

bool TicTacToe::move(int x, int y) {
    if (isGameOver) {
        std::cerr << "Game is already over\n";
        return false;
    }
    if (x < 1 || x > boardSizeX || y < 1 || y > boardSizeY || board[y - 1][x - 1] != 0) {
        std::cerr << "Invalid move\n";
        return false;
    }

    board[y - 1][x - 1] = isXTurn ? 2 : 1;
    isXTurn = !isXTurn;
    isOTurn = !isOTurn;
    previousMoves.push_back({ x, y });
    checkGameState();
    return true;
}

std::string TicTacToe::ascii() const {
    std::string asciiBoard = "";
    for (size_t i = 0; i < board.size(); ++i) {
        asciiBoard += "|";
        for (size_t j = 0; j < board[i].size(); ++j) {
            asciiBoard += (board[i][j] == 0 ? ' ' : (board[i][j] == 1 ? 'O' : 'X'));
            if (j < board[i].size() - 1) asciiBoard += "|";
        }
        if (i < board.size() - 1) {
            asciiBoard += "|\n" + std::string(board[i].size() * 2 - 1, '-') + "\n";
        }
    }
    return asciiBoard;
}

bool TicTacToe::isOver() const { return isGameOver; }
std::string TicTacToe::getWinner() const { return winner; }
bool TicTacToe::isDrawGame() const { return isDraw; }

std::vector<std::pair<int, int>> TicTacToe::getAvailableMoves() {
    std::vector<std::pair<int, int>> moves;
    for (int y = 0; y < boardSizeY; ++y) {
        for (int x = 0; x < boardSizeX; ++x) {
            if (board[y][x] == 0) {
                moves.emplace_back(x + 1, y + 1);  // Store 1-based coordinates
            }
        }
    }
    return moves;
}

// Helper to evaluate position
int TicTacToe::evaluatePosition(bool isMaximizing) {
    // Example: Return a basic heuristic (this should be customized further)
    if (checkLines(isMaximizing ? 2 : 1) || checkDiagonals(isMaximizing ? 2 : 1)) {
        return isMaximizing ? 1000 : -1000;
    }
    return 0;  // Neutral position
}

// Minimax function
int TicTacToe::minimax(int depth, bool isMaximizing, int alpha, int beta) {
    if (depth == 0 || isGameOver) {
        return evaluatePosition(isMaximizing);
    }

    // Normalize and hash the board
    unsigned long long currentHash = boardHash;

    // Check the transposition table
    if (transpositionTable.find(currentHash) != transpositionTable.end()) {
        const TTEntry& entry = transpositionTable[currentHash];
        if (entry.depth >= depth) {
            if (entry.flag == TTEntry::EXACT ||
                (entry.flag == TTEntry::LOWER && entry.score >= beta) ||
                (entry.flag == TTEntry::UPPER && entry.score <= alpha)) {
                return entry.score;
            }
        }
    }

    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    int player = isMaximizing ? 2 : 1;

    auto moves = getAvailableMoves();
    for (const auto& move : moves) {
        makeMove(move.first, move.second, player);
        int score = minimax(depth - 1, !isMaximizing, alpha, beta);
        undoMove(move.first, move.second);

        if (isMaximizing) {
            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, bestScore);
        }
        else {
            bestScore = std::min(bestScore, score);
            beta = std::min(beta, bestScore);
        }

        if (beta <= alpha) break;  // Alpha-beta pruning
    }

    // Store result in the transposition table
    TTEntry::BoundType bound = (bestScore <= alpha) ? TTEntry::UPPER
        : (bestScore >= beta) ? TTEntry::LOWER
        : TTEntry::EXACT;
    transpositionTable[currentHash] = { bestScore, depth, bound };

    return bestScore;
}
void TicTacToe::makeMove(int x, int y, int player) {
    board[y - 1][x - 1] = player;
    updateHash(x, y, player);  // Update the hash
}

void TicTacToe::undoMove(int x, int y) {
    int player = board[y - 1][x - 1];
    updateHash(x, y, player);  // Undo the hash update
    board[y - 1][x - 1] = 0;
}

std::string TicTacToe::getResult() {
    if (previousMoves.empty()) return "";
    std::string result;
    std::vector<std::string> tkns;
    for (int i = 0; i < previousMoves.size(); i += 2) {
        if (i + 1 < previousMoves.size()) {
            tkns.push_back(std::to_string(previousMoves[i].first) + "-" + std::to_string(previousMoves[i].second) + " " + std::to_string(previousMoves[i + 1].first) + "-" + std::to_string(previousMoves[i + 1].second));
        }
        else {
            tkns.push_back(std::to_string(previousMoves[i].first) + "-" + std::to_string(previousMoves[i].second));
        }
    }
    for (int i = 0; i < tkns.size(); i++) {
        result += std::to_string(i + 1) + ". " + tkns[i] + '\n';
    }
    if (winner == "X" ) {
        result += " 1-0";
    }
    else if (winner == "O") {
        result += " 0-1";
    }
    else {
        result += " 1/2-1/2";
    }
    return result;
}

void TicTacToe::printCurrentLine(int score) const {
    std::cout << "Evaluating line: ";
    std::stack<std::pair<int, int>> temp = currentLine;
    std::vector<std::pair<int, int>> moves;
    while (!temp.empty()) {
        moves.push_back(temp.top());
        temp.pop();
    }
    std::reverse(moves.begin(), moves.end());
    for (const auto& move : moves) {
        std::cout << "(" << move.first << ", " << move.second << ") ";
    }
    std::cout << "Score: " << score << "\n";
}

std::pair<int, int> TicTacToe::getBestMove(int depth, bool isMaximizing) {
    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    std::pair<int, int> bestMove = { -1, -1 };
    int player = isMaximizing ? 2 : 1;

    auto scoredMoves = getScoredMoves(player);
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    for (const auto& scoredMove : scoredMoves) {
        const auto& move = scoredMove.first;

        makeMove(move.first, move.second, player);
        int score = minimax(depth - 1, !isMaximizing, alpha, beta);
        undoMove(move.first, move.second);

        if (isMaximizing) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
            alpha = std::max(alpha, bestScore);
        }
        else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = move;
            }
            beta = std::min(beta, bestScore);
        }
        if (beta <= alpha) break;
    }

    return bestMove;
}

int TicTacToe::scoreMove(const std::pair<int, int>& move, int player) {
    // Simple heuristic: prioritize center and corners, and block/win
    int x = move.first - 1;
    int y = move.second - 1;

    // Winning or blocking move should have a very high score
    makeMove(move.first, move.second, player);
    bool isWinning = checkLines(player) || checkDiagonals(player);
    undoMove(move.first, move.second);
    if (isWinning) return 1000;

    // Center position (higher value for larger boards)
    if (x == boardSizeX / 2 && y == boardSizeY / 2) return 100;

    // Corners
    if ((x == 0 && y == 0) || (x == 0 && y == boardSizeY - 1) ||
        (x == boardSizeX - 1 && y == 0) || (x == boardSizeX - 1 && y == boardSizeY - 1)) {
        return 50;
    }

    // Default score for other positions
    return 10;
}

std::vector<std::pair<std::pair<int, int>, int>> TicTacToe::getScoredMoves(int player) {
    std::vector<std::pair<std::pair<int, int>, int>> scoredMoves;
    for (int y = 0; y < boardSizeY; ++y) {
        for (int x = 0; x < boardSizeX; ++x) {
            if (board[y][x] == 0) {
                std::pair<int, int> move = { x + 1, y + 1 };
                int score = scoreMove(move, player);
                scoredMoves.emplace_back(move, score);
            }
        }
    }

    // Sort moves by score (descending for maximizing player)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    return scoredMoves;
}