#pragma once
#include <vector>
#include "common.h"

class Computer;

struct Node{
	std::vector<Node> vChildren;
	uint8_t nDepth : 7;
	Color bMoveMadeBy : 1;

	Piece* board[8][8];
};


class Computer{
	Piece* (&m_board)[8][8];

	std::vector<Piece*> vBlacks, vWhites;

	static std::vector<Piece*> getColor(Piece* board[8][8], Color c){
		std::vector<Piece*> vOut;
		for (int y=0; y<8; y++) for (int x=0; x<8; x++)
				if (board[y][x]) 
					if (board[y][x]->getCol() == c)
						vOut.push_back(board[y][x]);
		return vOut;
	}
public:
	Computer(Piece* (&board)[8][8])
	:m_board(board)
	{
		//m_board = board;

		vBlacks = getColor(board, Color::BLACK);
		vWhites = getColor(board, Color::WHITE);
	}

	static int evaluateBoard(Piece* board[8][8]){
		int out = 0;
		std::vector<Piece*>
			vBoardBlacks = getColor(board, Color::BLACK),
			vBoardWhites = getColor(board, Color::WHITE);

		for (auto& p: vBoardBlacks)
			out += p->getValue();
		for (auto& p: vBoardWhites)
			out -= p->getValue();

		// doubled, blocked and isolated pawns
		int total = 0;
		for (int8_t x=0; x<8; x++){
			int nWPawns = 0, nBPawns = 0,
				nWBlocked = 0, nBBlocked = 0,
				nWIsolate = 0, nBIsolate = 0;

			for (int8_t y=0; y<8; y++){
				if (Piece::empty(board, olc::vi2d{x,y})) continue;
				if (!board[y][x]->isPawn()) continue;

				Color c = board[y][x]->getCol();

				bool bIsolated = true;
				for(int8_t scanY = -1; scanY<2; scanY++)
					for(int8_t scanX = -1; scanX<2; scanX++){
						if (scanX == 0 && scanY == 0) continue;
						olc::vi2d pos{x + scanX, y+scanY};
						if (pos.x < 0 || pos.x > 7 ||
							pos.y < 0 || pos.y > 7) continue;
						
						if (Piece::empty(board, pos)) continue;
						if (board[pos.y][pos.x]->isPawn() && board[pos.y][pos.x]->getCol() == c){
							bIsolated = false;
							goto NOT_ISOLATED;
						}
					}
			NOT_ISOLATED:

				if (c == Color::BLACK){
					nBPawns++;
					if (y < 7)
						if (!Piece::empty(board, olc::vi2d{x, y+1}))
							nBBlocked++;
					if (bIsolated) nBIsolate++;
				}
				else{
					nWPawns++;
					if (y < 7)
						if (!Piece::empty(board, olc::vi2d{x, y+1}))
							nWBlocked++;
					if (bIsolated) nWIsolate++;
				}
			}

			total += (nBPawns >> 1) - (nWPawns >> 1) // doubled pawns
					+ nBBlocked - nWBlocked
					+ nBIsolate - nWIsolate;
		
		}
		out -= 5 * total;

		// mobility
		for (auto& p: vBoardBlacks)
			out += (int)p->getMoves(board).size();
		for (auto& p: vBoardWhites)
			out -= (int)p->getMoves(board).size();
		
		return out;
			
	}
// f(p) = 200(K-K')
// 	+ 9(Q-Q')
// 	+ 5(R-R')
// 	+ 3(B-B' + N-N')
// 	+ 1(P-P')
// 	- 0.5(D-D' + S-S' + I-I')
// 	+ 0.1(M-M') + ...

// KQRBNP = number of kings, queens, rooks, bishops, knights and pawns
// D,S,I = doubled, blocked and isolated pawns
// M = Mobility (the number of legal moves)
	void play(){
		vBlacks = getColor(m_board, Color::BLACK);
		vWhites = getColor(m_board, Color::WHITE);
	}
};