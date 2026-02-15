#include "Connect4.h"

#define INF 1000000000

int aiBoardEval(std::string &state);
int max(int a, int b);

Connect::Connect()
{
    _grid = new Grid(7, 6);
}

Connect::~Connect()
{
    delete _grid;
}

Bit* Connect::PieceForPlayer(const int playerNumber)
{
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    bit->LoadTextureFromFile(playerNumber == AI_PLAYER ? "yellow.png" : "red.png");
    bit->setSize(60,60);
    bit->setOwner(getPlayerAt(playerNumber == AI_PLAYER ? 1 : 0));
    return bit;
}

void Connect::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(60, "square.png");

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    startGame();
}

BitHolder* Connect::lowestEmptyInColumn(int column)
{
    for (int i = 5; i >= 0; i--) {
        ChessSquare* square = _grid->getSquare(column, i);
        if (!square->bit()) {
            return square;
        }
    }
    return nullptr; 
}

bool Connect::actionForEmptyHolder(BitHolder &holder)
{
    BitHolder* lowest = lowestEmptyInColumn((holder.getPosition().x - 30)/60);
    if (!lowest) {
        return false;
    }

    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber() == 0 ? HUMAN_PLAYER : AI_PLAYER);
    if (bit) {
        bit->setPosition(lowest->getPosition().x+10, lowest->getPosition().y+10);
        lowest->setBit(bit);
        endTurn();
        return true;
    }   

    return false;
}

void Connect::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Connect::checkForWinner()
{
    // start in bottom right
    // for each square check for up win, left win, diagonal left win, and diagonal right win
    for (int x = 0; x < 7; x++) {
        for (int y = 0; y < 6; y++) {
            ChessSquare* square = _grid->getSquare(x, y); 
            if (square->bit()) {
                Player *player = square->bit()->getOwner();
                // check for all win types
                int x1 = x;
                int y1 = y;
                int y2 = y;
                int left_win = 0;
                int up_win = 0;
                int left_diag_win = 0;
                int right_diag_win = 0;
                for (int i = 0; i < 3; i++) {
                    x1 -= 1;
                    y1 -= 1;
                    y2 += 1;
                    ChessSquare *s1 = nullptr;
                    ChessSquare *s2 = nullptr;
                    ChessSquare *s3 = nullptr;
                    ChessSquare *s4 = nullptr;
                    if (x1 >= 0)
                        s1 = _grid->getSquare(x1, y);  // horizonal win 
                    if (y1 >= 0)
                        s2 = _grid->getSquare(x, y1);  // vertical win
                    if (x1 >= 0 && y1 >= 0)
                        s3 = _grid->getSquare(x1, y1); // diag left win
                    if (x1 >= 0 && y2 <= 5)
                        s4 = _grid->getSquare(x1, y2); // diag right win
                    if (s1 && s1->bit()) {
                        Player *p1 = s1->bit()->getOwner();
                        if (p1 == player) {
                            left_win++;
                        }
                    }
                    if (s2 && s2->bit()) {
                        Player *p2 = s2->bit()->getOwner();
                        if (p2 == player)
                        {
                            up_win++;
                        }
                    }
                    if (s3 && s3->bit()) {
                        Player *p3 = s3->bit()->getOwner();
                        if (p3 == player)
                        {
                            left_diag_win++;
                        } 
                    }
                    if (s4 && s4->bit()) {
                        Player *p4 = s4->bit()->getOwner();
                        if (p4 == player)
                        {
                            right_diag_win++;
                        } 
                    }
                }
                if (left_win == 3 || up_win == 3 || left_diag_win == 3 || right_diag_win == 3) {
                    return player;
                }
            }
        }
    }
    return nullptr;
}

bool Connect::checkForDraw()
{
    bool isDraw = true;
    // check to see if the board is full
    _grid->forEachSquare([&isDraw](ChessSquare* square, int x, int y) {
        if (!square->bit()) {
            isDraw = false;
        }
    });
    return isDraw;
}

std::string Connect::stateString()
{
    std::string s = "000000000000000000000000000000000000000000";
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit *bit = square->bit();
        if (bit) {
            s[y * 7 + x] = std::to_string(bit->getOwner()->playerNumber()+1)[0];
        }
    });
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Connect::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y*7 + x;
        int playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit( PieceForPlayer(playerNumber-1) );
        } else {
            square->setBit( nullptr );
        }
    });
}

void Connect::updateAI() 
{
    std::string currentState = stateString();
    int bestMove = -INF;
    int bestSquare = -1;

    // find available moves
    int available[7];
    for (int i = 0; i < 7; i++)
        available[i] = -1;
    for (int i = 0; i < 42; i++) {
        if (currentState[i] == '0')
            available[i % 7] = i;
    }

    // search moves in middle first
    int moveOrder[7] = {3, 4, 5, 2, 6, 1, 0};
    
    // call negamax on available moves
    for (auto i : moveOrder) {
        if (available[i] == -1)
            continue;
        currentState[available[i]] = '2';
        int newValue = -negamax(currentState, 0, -INF, INF, HUMAN_PLAYER);
        if (newValue > bestMove) {
            bestSquare = i;
            bestMove = newValue;
        }
        std::cout << "move " << i << ": " << newValue << std::endl;
        currentState[available[i]] = '0';
    }

    // play the move that had the highest score
    if (bestSquare != -1) {
        actionForEmptyHolder(*(_grid->getSquareByIndex(bestSquare)));
    } 
}

// I am using a ‘windowing’ analysis function
// I found an example online: https://roboticsproject.readthedocs.io/en/latest/ConnectFourAlgorithm.html

int evalWindow(int depth, int a, int emptyCount) {
    int score = 0;
    if (a == 4) return INF/10 - depth; // win
    if (a == 3 && emptyCount == 1) score += 40; 
    if (a == 2 && emptyCount == 2) score += 10; 
    return score;
}

int aiBoardEval(std::string &state, int depth, int player) {
    int score = 0;
    char pl = (player == 1) ? '1' : '2';
    char pl2 = (player == 1) ? '2' : '1';

    // favor center positions
    for (int i = 0; i < 6; i++) {
        if (state[i * 7 + 3] == pl) score += 1;
    }

    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 4; c++) {
            int a = 0, o = 0, e = 0;
            for (int i = 0; i < 4; i++) {
                char p = state[r * 7 + (c + i)];
                if (p == pl) a++; else if (p == pl2) o++; else e++;
            }
            score += evalWindow(depth, a, e);
        }
    }

    for (int c = 0; c < 7; c++) {
        for (int r = 0; r < 3; r++) {
            int a = 0, o = 0, e = 0;
            for (int i = 0; i < 4; i++) {
                char p = state[(r + i) * 7 + c];
                if (p == pl) a++; else if (p == pl2) o++; else e++;
            }
            score += evalWindow(depth, a, e);
        }
    }

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 4; c++) {
            int a = 0, o = 0, e = 0;
            for (int i = 0; i < 4; i++) {
                char p = state[(r + i) * 7 + (c + i)];
                if (p == pl) a++; else if (p == pl2) o++; else e++;
            }
            score += evalWindow(depth, a, e);
        }
    }

    for (int r = 3; r < 6; r++) {
        for (int c = 0; c < 4; c++) {
            int a = 0, o = 0, e = 0;
            for (int i = 0; i < 4; i++) {
                char p = state[(r - i) * 7 + (c + i)];
                if (p == pl) a++; else if (p == pl2) o++; else e++;
            }
            score += evalWindow(depth, a, e);
        }
    }

    return score;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int Connect::negamax(std::string &state, int depth, int alpha, int beta, int color) {
    if (depth >= 7)
        return -aiBoardEval(state, depth, color); 

    // get the available squares 
    int available[7];
    int num_available = 0;
    for (int i = 0; i < 7; i++)
        available[i] = -1;
    for (int i = 0; i < 42; i++) {
        if (state[i] == '0')
            available[i % 7] = i;
            num_available++;
    }

    // check for full board
    if (num_available == 0)
        return -aiBoardEval(state, depth, color);

    // search moves in middle first
    int moveOrder[7] = {3, 4, 5, 2, 6, 1, 0};

    // call negamax on the available squares
    int bestVal = -INF;
    for (auto i : moveOrder) {
        if (available[i] == -1)
            continue;
        state[available[i]] = (color == HUMAN_PLAYER ? '1' : '2');
        int newVal = -negamax(state, depth+1, -beta, -alpha, -color);
        state[available[i]] = '0';
        bestVal = max(bestVal, newVal);
        // prune
        if(newVal >= beta) return newVal;
        alpha = max(alpha, newVal);
    }
    return bestVal;
}