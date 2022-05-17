#pragma once
#include <vector>
#include "common.h"

class Computer{
	Piece* m_board[8][8];

	std::vector<Piece*> vBlacks, vWhites;

	void updateBlacks(){
		vBlacks.clear();
		for (int y=0; y<8; y++) for (int x=0; x<8; x++)
				if (m_board[y][x]) 
					if (m_board[y][x]->getCol() == Color::BLACK)
						vBlacks.push_back(m_board[y][x]);
	}
	void updateWhites(){
		vWhites.clear();
		for (int y=0; y<8; y++) for (int x=0; x<8; x++)
				if (m_board[y][x]) 
					if (m_board[y][x]->getCol() == Color::WHITE)
						vWhites.push_back(m_board[y][x]);
	}
public:
	Computer(Piece* board[8][8]): m_board(board){
		updateBlacks();
		updateWhites();

	}
	void play(){
		updateBlacks();
		updateWhites();
	}
}