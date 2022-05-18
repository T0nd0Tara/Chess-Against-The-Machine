#pragma once
#include "common.h"

class King: public Piece{
public:

	explicit King(olc::vi2d pos, Color c): Piece(pos, c){
		m_sprite_cords = olc::vi2d{0, (int)c};
	}
    inline const int getValue() override { return 2000; }
	std::vector<Move> getMoves(Piece* board[8][8]) override{
		std::vector<Move> out;
        for (int y =-1; y<2; y++)
            for (int x =-1; x<2; x++){
                if (y==0 && x == 0) continue;
                olc::vi2d end_pos = m_pos + olc::vi2d{x,y};
                if (end_pos.y < 0 || end_pos.y > 7) continue;
                if (end_pos.x < 0 || end_pos.x > 7) continue;

                if (empty(board, end_pos)){
                    out.push_back(Move(m_pos, end_pos));
                }else if (board[end_pos.y][end_pos.x]->getCol() != m_col){
                    out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
                }
            }

		return out;
	}
	
};