#pragma once
#include <algorithm>
// SOURCE
// https://www.geeksforgeeks.org/minimax-algorithm-in-game-theory-set-3-tic-tac-toe-ai-finding-optimal-move/

class Engine
{
	char player;
	char opponent;
public:
	struct Move { int row, col; };
	Engine();
	void SetZnak(char znak);
	bool isMovesLeft(char board[3][3]);
	int evaluate(char b[3][3]);
	int minimax(char board[3][3], int depth, bool isMax);
	Move findBestMove(char board[3][3]);
	char maybe_win(char board[3][3]);
};

