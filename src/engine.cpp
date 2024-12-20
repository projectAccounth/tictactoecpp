#include "../include/tictactoe.h"
#include <future>
#include <thread>

std::random_device rd;

void TicTacToe::initializeZobrist() {
    std::mt19937_64 rng(rd());
    for (int y = 0; y < boardSizeY; ++y) {
        for (int x = 0; x < boardSizeX; ++x) {
            for (int p = 0; p < 3; ++p) {
                zobristTable[x][y][p] = rng();
            }
        }
    }
}

std::string TicTacToe::hashBoard(const std::vector<std::vector<int>>& boardCopy) const {
    std::string hash;
    for (const auto& row : boardCopy) {
        for (int cell : row) {
            hash += std::to_string(cell);
        }
    }
    return hash;
}

std::vector<std::vector<int>> TicTacToe::rotateBoard90(const std::vector<std::vector<int>>& boardCopy) const {
    int N = static_cast<int>(boardCopy.size());
    std::vector<std::vector<int>> rotatedBoard(N, std::vector<int>(N, 0));

    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            rotatedBoard[x][N - y - 1] = boardCopy[y][x];
        }
    }

    return rotatedBoard;
}

std::vector<std::vector<int>> TicTacToe::reflectBoardHorizontal(const std::vector<std::vector<int>>& boardCopy) const {
    int N = static_cast<int>(boardCopy.size());
    std::vector<std::vector<int>> reflectedBoard = boardCopy;

    for (int y = 0; y < N; ++y) {
        reflectedBoard[N - y - 1] = boardCopy[y];
    }

    return reflectedBoard;
}

std::string TicTacToe::normalizeBoard() const {
    std::vector<std::string> symmetries;

    auto currentBoard = board;
    symmetries.push_back(hashBoard(currentBoard));

    auto rotated = currentBoard;

    for (int i = 0; i < 3; ++i) {
        rotated = rotateBoard90(rotated);
        symmetries.push_back(hashBoard(rotated));
    }

    auto reflected = reflectBoardHorizontal(currentBoard);
    symmetries.push_back(hashBoard(reflected));

    rotated = reflected;
    for (int i = 0; i < 3; ++i) {
        rotated = rotateBoard90(rotated);
        symmetries.push_back(hashBoard(rotated));
    }

    return *std::min_element(symmetries.begin(), symmetries.end());
}

void TicTacToe::updateHash(int x, int y, int player) {
    boardHash ^= zobristTable[x - 1][y - 1][player];
}

std::pair<int, int> TicTacToe::getBestMove(int maxDepth, bool isMaximizing) {
    resetNodeCounter();
    std::pair<int, int> bestMove = { -1, -1 };
    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    int player = isMaximizing ? 2 : 1;
    int opponent = 3 - player;

    int dDepth = calculateDepth(maxDepth);
    std::cout << "Maximum depth: " + std::to_string(dDepth) + '\n';

    time_t tick1 = clock();

    for (int currentDepth = 1; currentDepth <= dDepth; ++currentDepth) {
        auto moves = getOrderedMoves(player);
        for (const auto& move : moves) {
            makeMove(move.first, move.second, player);

            if (checkLines(player) || checkDiagonals(player)) {
                undoMove(move.first, move.second);
                std::cout << "Best move: (" + std::to_string(move.first) + ", " + std::to_string(move.second) + "), reason: Immediate win\n";
                return move;
            }
            makeMove(move.first, move.second, opponent);
            if (checkLines(opponent) || checkDiagonals(opponent)) {
                undoMove(move.first, move.second);
                undoMove(move.first, move.second);
                std::cout << "Best move: (" + std::to_string(move.first) + ", " + std::to_string(move.second) + "), reason: Forced move\n";
                return move;
            }
            undoMove(move.first, move.second);

            int score = minimax(currentDepth - 1, !isMaximizing,
                std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max());
            undoMove(move.first, move.second);

            printDebugInfo(currentDepth, moves, player);
            // std::cout << "Move: (" + std::to_string(move.first) + ", " + std::to_string(move.second) + "), Position score: " + std::to_string(score) + '\n';

            if ((isMaximizing && score > bestScore) || (!isMaximizing && score < bestScore)) {
                bestScore = score;
                bestMove = move;
            }
        }

        time_t tick2 = clock();
        if (bestMove != std::make_pair(-1, -1))
            std::cout << "Depth: " << currentDepth << ", Best move: ("
                << bestMove.first << ", " << bestMove.second << "), Position score: "
                << bestScore << ", " << " Move score: " + std::to_string(scoreMove(bestMove, player)) + ", " <<
                "Time elasped : " + std::to_string(static_cast<double>(tick2 - tick1) / 1000.0) + '\n';
    }

    return bestMove;
}

bool TicTacToe::isLineBlocked(int x, int y, int player) {
    int opponent = (player == 1) ? 2 : 1;
    std::vector<std::pair<int, int>> directions = {
        {0, 1}, {1, 0}, {1, 1}, {1, -1}
    };

    for (const auto& dir : directions) {
        int dx = dir.first, dy = dir.second;
        int countPlayer = 0, countEmpty = 0, countOpponent = 0;

        for (int step = -matchLength + 1; step < matchLength; ++step) {
            int nx = x + step * dx;
            int ny = y + step * dy;

            if (nx < 0 || ny < 0 || nx >= boardSizeX || ny >= boardSizeY) continue;

            if (board[ny][nx] == player) countPlayer++;
            else if (board[ny][nx] == 0) countEmpty++;
            else if (board[ny][nx] == opponent) countOpponent++;
        }

        // If the line is blocked on both sides, it's a blocked opportunity
        if (countOpponent > 0 && countPlayer + countEmpty >= matchLength) {
            return true;
        }
    }

    return false;
}

int TicTacToe::scoreMove(const std::pair<int, int>& move, int player) {
    int x = move.first - 1;
    int y = move.second - 1;

    int opponent = 3 - player;
    int score = 0;

    makeMove(move.first, move.second, player);

    if (checkLines(player) || checkDiagonals(player)) {
        undoMove(move.first, move.second);
        return 4000;  // Winning move
    }

    // Check for blocking opponent's winning move
    makeMove(move.first, move.second, opponent);
    if (checkLines(opponent) || checkDiagonals(opponent)) {
        undoMove(move.first, move.second);
        undoMove(move.first, move.second);
        return 3750;  // Defensive (opponent blocking) move
    }
    undoMove(move.first, move.second);
    makeMove(move.first, move.second, player);

    int maxPoint = 1000;

    // Calculate scores based on matchLength logic
    score += countLines(player, matchLength - 1) * maxPoint / (matchLength * 2);  // Nearly winning
    score += countLines(player, matchLength - 2) * maxPoint / (matchLength * 4);  // Strong positioning
    if (matchLength > 3)
        score += countLines(player, matchLength - 3) * maxPoint / (matchLength * 6);  // Weak positioning

    // Penalize blocked opportunities: Check if the move completes a line blocked by an opponent
    int blockedPenalty = 100;
    if (isLineBlocked(x, y, player)) {
        score -= blockedPenalty;
    }

    // Penalize blocking potential for the opponent
    score += countLines(opponent, matchLength - 1) * maxPoint / (matchLength * 4);  // Block opponent's nearly winning
    if (matchLength > 3)
        score += countLines(opponent, matchLength - 2) * maxPoint / (matchLength * 6);

    int centerX = boardSizeX / 2;
    int centerY = boardSizeY / 2;
    if (boardSizeX > 3 || boardSizeY > 3) {
        int distanceFromCenter = std::abs(x - centerX) + std::abs(y - centerY);
        int proximityWeight = std::max(75 - (distanceFromCenter * 10 * boardSizeX), 0);
        score += proximityWeight;

        if ((x == 0 && y == 0) || (x == 0 && y == boardSizeY - 1) ||
            (x == boardSizeX - 1 && y == 0) || (x == boardSizeX - 1 && y == boardSizeY - 1)) {
            score += 15;
        }
    }
    else {
        if (x == centerX && y == centerY) {
            score += 50;
        }
        if ((x == 0 && y == 0) || (x == 0 && y == boardSizeY - 1) ||
            (x == boardSizeX - 1 && y == 0) || (x == boardSizeX - 1 && y == boardSizeY - 1)) {
            score += 15;
        }
    }
    if ((x == 0 || x == boardSizeX - 1) || (y == 0 || y == boardSizeY - 1)
        && (!(x == 0 && y == 0) || (x == 0 && y == boardSizeY - 1) ||
        (x == boardSizeX - 1 && y == 0) || (x == boardSizeX - 1 && y == boardSizeY - 1))) {
        score += 5;
    }

    undoMove(move.first, move.second);

    return std::min(score, 3749);
}

std::vector<std::pair<std::pair<int, int>, int>> TicTacToe::getScoredMoves(int player, std::pair<int, int> prioritizedMove) {
    std::vector<std::pair<std::pair<int, int>, int>> scoredMoves;

    for (int y = 0; y < boardSizeY; ++y) {
        for (int x = 0; x < boardSizeX; ++x) {
            if (board[y][x] == 0) {
                std::pair<int, int> move = { x + 1, y + 1 };
                int score = (move == prioritizedMove) ? 1000 : scoreMove(move, player);
                scoredMoves.emplace_back(move, score);
            }
        }
    }

    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    return scoredMoves;
}

bool TicTacToe::isStrandedPiece(const std::pair<int, int>& move, int player) {
    int x = move.first - 1;
    int y = move.second - 1;
    int centerX = boardSizeX / 2;
    int centerY = boardSizeY / 2;

    int distanceFromCenter = std::abs(x - centerX) + std::abs(y - centerY);

    // Consider stranded if far from the center and not part of any line
    if (distanceFromCenter > matchLength / 2) {
        int surroundingCount = 0;

        // Check adjacent squares for friendly pieces
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;  // Skip the move itself
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && ny >= 0 && nx < boardSizeX && ny < boardSizeY) {
                    if (board[ny][nx] == player) surroundingCount++;
                }
            }
        }

        // Consider stranded if no adjacent friendly pieces
        return surroundingCount == 0;
    }

    return false;
}

bool TicTacToe::isImmediateThreat(const std::pair<int, int>& move, int opponent) {
    makeMove(move.first, move.second, opponent);
    bool immediateThreat = checkLines(opponent) || checkDiagonals(opponent);
    undoMove(move.first, move.second);
    return immediateThreat;
}

int TicTacToe::countLines(int player, int length) const {
    static const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };

    int count = 0;

    if (length == 0) return 0;

    for (int y = 0; y < boardSizeY; ++y) {
        for (int x = 0; x < boardSizeX; ++x) {
            if (board[y][x] != player) continue;

            for (const auto& p : directions) {
                int dx = p.first;
                int dy = p.second;
                bool valid = true;
                for (int k = 1; k < length; ++k) {
                    int nx = x + k * dx;
                    int ny = y + k * dy;
                    if (nx < 0 || ny < 0 || nx >= boardSizeX || ny >= boardSizeY || board[ny][nx] != player) {
                        valid = false;
                        break;
                    }
                }
                if (valid) count++;
            }
        }
    }

    return count;
}

std::vector<std::pair<int, int>> TicTacToe::getOrderedMoves(int player) {
    auto moves = getAvailableMoves();
    std::vector<std::pair<std::pair<int, int>, int>> scoredMoves;

    // Evaluate and score each move
    for (const auto& move : moves) {
        int score = scoreMove(move, player);
        scoredMoves.emplace_back(move, score);
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::vector<std::pair<int, int>> orderedMoves;
    for (const auto& scoredMove : scoredMoves) {
        orderedMoves.push_back(scoredMove.first);
    }

    return orderedMoves;
}

void TicTacToe::printDebugInfo(int depth, const std::vector<std::pair<int, int>>& moves, int player) {
    std::cout << "\nDepth " + std::to_string(depth) + " analyzation:" << "\n";
    std::cout << "Player: " << player << "\n";

    for (const auto& move : moves) {
        int score = scoreMove(move, player);
        std::cout << "Move: (" << move.first << ", " << move.second << ") Score: " << score << '\n';
    }
}

int TicTacToe::calculateDepth(int maxDepth) {
    int baseDepth = 3;

    if (moveNumber <= 2) {
        // Calculate early-game depth based on board size and match length
        int sizeFactor = std::max(boardSizeX, boardSizeY) / 5;  // +1 depth for every 5 rows/cols
        int matchFactor = (matchLength > 3) ? 1 : 0;  // +1 depth for match lengths > 3
        int earlyGameDepth = baseDepth + sizeFactor + matchFactor;

        return std::min(earlyGameDepth, maxDepth);
    }

    // Adjust depth dynamically for later moves
    int fullMoves = moveNumber;
    int moveAdjustment = fullMoves / 2;  // +1 depth every 2 full moves

    int boardAdjustment = std::max(boardSizeX, boardSizeY) / 5;  // +1 depth for larger boards
    int matchLengthAdjustment = (matchLength > 3) ? 1 : 0;  // +1 depth for longer match lengths

    int dynamicDepth = baseDepth + moveAdjustment + boardAdjustment + matchLengthAdjustment;

    return std::min(dynamicDepth, maxDepth);
}

void TicTacToe::resetNodeCounter() {
    currentNode = 0;
    totalNodes = 0;
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

int TicTacToe::countPotentialWinningLines(int player) {
    int opponent = 3 - player;
    int potentialLines = 0;

    // Check rows
    for (int i = 0; i < boardSizeY; ++i) {
        for (int j = 0; j <= boardSizeX - matchLength; ++j) {
            std::vector<std::pair<int, int>> line;
            for (int k = 0; k < matchLength; ++k) {
                line.emplace_back(i, j + k);
            }
            if (isPotentialWinningLine(line, player)) {
                potentialLines++;
            }
        }
    }

    // Check columns
    for (int j = 0; j < boardSizeY; ++j) {
        for (int i = 0; i <= boardSizeX - matchLength; ++i) {
            std::vector<std::pair<int, int>> line;
            for (int k = 0; k < matchLength; ++k) {
                line.emplace_back(i + k, j);
            }
            if (isPotentialWinningLine(line, player)) {
                potentialLines++;
            }
        }
    }

    // Check diagonals (top-left to bottom-right)
    for (int i = 0; i <= boardSizeY - matchLength; ++i) {
        for (int j = 0; j <= boardSizeX - matchLength; ++j) {
            std::vector<std::pair<int, int>> line;
            for (int k = 0; k < matchLength; ++k) {
                line.emplace_back(i + k, j + k);
            }
            if (isPotentialWinningLine(line, player)) {
                potentialLines++;
            }
        }
    }

    // Check diagonals (top-right to bottom-left)
    for (int i = 0; i <= boardSizeY - matchLength; ++i) {
        for (int j = matchLength - 1; j < boardSizeX; ++j) {
            std::vector<std::pair<int, int>> line;
            for (int k = 0; k < matchLength; ++k) {
                line.emplace_back(i + k, j - k);
            }
            if (isPotentialWinningLine(line, player)) {
                potentialLines++;
            }
        }
    }

    return potentialLines;
}


bool TicTacToe::isPotentialWinningLine(const std::vector<std::pair<int, int>>& line, int player) {
    int opponent = 3 - player;
    bool hasEmptyCell = false;

    for (const auto& cell : line) {
        int row = cell.first;
        int col = cell.second;

        if (board[row][col] == opponent) {
            return false;  // Line is blocked by the opponent
        }

        // Check if the cell is empty
        if (board[row][col] == 0) {
            hasEmptyCell = true;
        }
    }

    // The line is valid if it contains no opponent pieces and has at least one empty cell
    return hasEmptyCell;
}

// Helper function to evaluate position
int TicTacToe::evaluatePosition(bool isMaximizing) {
    int player = isMaximizing ? 2 : 1;
    int opponent = 3 - player;

    // Winning positions
    if (checkLines(player) || checkDiagonals(player)) return 1000;
    if (checkLines(opponent) || checkDiagonals(opponent)) return -1000;

    int score = 0;

    // Add scores for potential lines
    score += countPotentialWinningLines(player) * 50;  // Emphasize potential winning lines for the player
    score -= countPotentialWinningLines(opponent) * 50;  // Penalize opponent's potential lines

    // Add scores for nearly complete lines
    score -= countLines(opponent, matchLength - 1) * 1000 / (matchLength * 2);  // Opponent nearly winning
    score += countLines(player, matchLength - 1) * 1000 / (matchLength * 2);   // Player nearly winning

    score -= countLines(opponent, matchLength - 2) * 400 / (matchLength * 2);  // Opponent strong position
    score += countLines(player, matchLength - 2) * 400 / (matchLength * 2);    // Player strong position

    return score;
}

std::vector<std::pair<int, int>> TicTacToe::getAvailableMoves() {
    std::vector<std::pair<int, int>> moves;
    for (int y = 0; y < boardSizeY; ++y) {
        for (int x = 0; x < boardSizeX; ++x) {
            if (board[y][x] == 0) {
                moves.emplace_back(x + 1, y + 1);
            }
        }
    }
    return moves;
}

// Minimax function
int TicTacToe::minimax(int depth, bool isMaximizing, int alpha, int beta) {
    totalNodes++; // ignore, for debugging

    if (depth == 0 || isGameOver) {
        return evaluatePosition(isMaximizing);
    }

    unsigned long long currentHash = boardHash;

    // transpos-table
    if (transpositionTable.find(currentHash) != transpositionTable.end()) {
        const TTEntry& entry = transpositionTable[currentHash];
        if (entry.depth == depth) {
            if (entry.flag == TTEntry::EXACT ||
                (entry.flag == TTEntry::LOWER && entry.score >= beta) ||
                (entry.flag == TTEntry::UPPER && entry.score <= alpha)) {
                return entry.score;
            }
        }
    }

    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    int player = isMaximizing ? 2 : 1;

    auto moves = getOrderedMoves(player);
    int moveCount = 0;

    for (const auto& move : moves) {
        // ignore, for debugging
        currentNode++;
        moveCount++;

        makeMove(move.first, move.second, player);

        // LMR
        int newDepth = depth - 1;
        if (moveCount > 2 && depth > 2) {
            newDepth--;
        }

        int score = minimax(newDepth, !isMaximizing, alpha, beta);
        undoMove(move.first, move.second);

        if (isMaximizing) {
            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, bestScore);
        }
        else {
            bestScore = std::min(bestScore, score);
            beta = std::min(beta, bestScore);
        }

        if (beta <= alpha) break;
    }

    TTEntry::BoundType bound = (bestScore <= alpha) ? TTEntry::UPPER
        : (bestScore >= beta) ? TTEntry::LOWER
        : TTEntry::EXACT;
    transpositionTable[currentHash] = { bestScore, depth, bound };

    return bestScore;
}

void TicTacToe::makeMove(int x, int y, int player) {
    board[y - 1][x - 1] = player;
    updateHash(x, y, player);
}

void TicTacToe::undoMove(int x, int y) {
    int player = board[y - 1][x - 1];
    board[y - 1][x - 1] = 0;
    updateHash(x, y, player);
}

int TicTacToe::analyzeLastMove() {
    if (previousMoves.empty()) return 0;
    int result = scoreMove(previousMoves.back(), isXTurn ? 1 : 2);
    makeMove(previousMoves.back().first, previousMoves.back().second, isXTurn ? 1 : 2);
    return result;
}

int TicTacToe::countThreatsBlocked(int x, int y, int opponent) {
    int threatsBlocked = 0;

    board[y][x] = opponent;

    // Check all directions for threats blocked
    if (checkLines(opponent)) {
        threatsBlocked++;
    }
    if (checkDiagonals(opponent)) {
        threatsBlocked++;
    }

    board[y][x] = 0;

    return threatsBlocked;
}