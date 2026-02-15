#pragma once
#include "Game.h"

//
// the classic game of tic tac toe
//

//
// the main game class
//
class Connect : public Game
{
public:
    Connect();
    ~Connect();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override { return "000000000000000000000000000000000000000000"; };
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override { return false; };
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override { return false; };
    void        stopGame() override;
	  void        updateAI() override;
    bool        gameHasAI() override { return true; }
    Grid*       getGrid() override { return _grid; }
    BitHolder*  lowestEmptyInColumn(int column);
private:
    Bit *       PieceForPlayer(const int playerNumber);
    int         negamax(std::string &state, int depth, int alpha, int beta, int color); 

    Grid*       _grid;
};

