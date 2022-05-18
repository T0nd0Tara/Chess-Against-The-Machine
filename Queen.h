#pragma once
#include "common.h"
#include "Rook.h"
#include "Bishop.h"

class Queen: public Piece{
public:

	explicit Queen(olc::vi2d pos, Color c): Piece(pos, c){
		m_sprite_cords = olc::vi2d{1, (int)c};
	}
    inline const int getValue() override { return 90; }
	std::vector<Move> getMoves(Piece* board[8][8]) override{
        // we check where it could go if it was a Knight and if it was a Bishop
        // and merge the results
        Rook k(m_pos, m_col);
        Bishop b(m_pos, m_col);
		std::vector<Move> out = k.getMoves(board);
        for (auto& m: b.getMoves(board)){
            out.push_back(m);
        }
		return out;
	}
	
};