#include "../include/tictactoe.h"

void TicTacToe::fillBoard() {
    board.resize(boardSizeY, std::vector<int>(boardSizeX, 0));
}

bool TicTacToe::checkLines(int symbol) {
    for (int i = 0; i < boardSizeY; ++i) {
        int rowMatch = 0;
        int colMatch = 0;
        for (int j = 0; j < boardSizeX; ++j) {
            if (board[i][j] == symbol) {
                if (++rowMatch == matchLength) return true;
            }
            else {
                rowMatch = 0;
            }
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
            emptyCells -= static_cast<int>(std::count(row.begin(), row.end(), 0));
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

    makeMove(x, y, isXTurn ? 2 : 1);
    isXTurn = !isXTurn;
    isOTurn = !isOTurn;
    previousMoves.push_back({ x, y });
    if (isXTurn) moveNumber++;
    updateHash(x, y, isXTurn ? 1 : 2);
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