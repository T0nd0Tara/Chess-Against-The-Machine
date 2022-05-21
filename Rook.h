#pragma once
#include "common.h"

class Rook: public Piece{
public:

	explicit Rook(olc::vi2d pos, Color c): Piece(pos, c){
		m_sprite_cords = olc::vi2d{4, (int)c};
	}
    inline const int getValue() override { return 50; }
    inline Piece* clone() const override { return new Rook(*this); }
	std::vector<Move> getMoves(Piece* board[8][8]) override{
		std::vector<Move> out;
		// north
		for (int y = m_pos.y-1; y >= 0; y--){
			olc::vi2d end_pos = olc::vi2d{m_pos.x, y};
			if (empty(board, end_pos)){
				out.push_back(Move(m_pos, end_pos));
			}else{
				if (board[end_pos.y][end_pos.x]->getCol() != m_col)
					out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
				break;
			}
		}
		// south
		for (int y = m_pos.y+1; y < 8; y++){
			olc::vi2d end_pos = olc::vi2d{m_pos.x, y};
			if (empty(board, end_pos)){
				out.push_back(Move(m_pos, end_pos));
			}else{
				if (board[end_pos.y][end_pos.x]->getCol() != m_col)
					out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
				break;
			}
		}

		// west
		for (int x = m_pos.x-1; x >= 0; x--){
			olc::vi2d end_pos = olc::vi2d{x, m_pos.y};
			if (empty(board, end_pos)){
				out.push_back(Move(m_pos, end_pos));
			}else{
				if (board[end_pos.y][end_pos.x]->getCol() != m_col)
					out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
				break;
			}
		}

		// east
		for (int x = m_pos.x+1; x < 8; x++){
			olc::vi2d end_pos = olc::vi2d{x, m_pos.y};
			if (empty(board, end_pos)){
				out.push_back(Move(m_pos, end_pos));
			}else{
				if (board[end_pos.y][end_pos.x]->getCol() != m_col)
					out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
				break;
			}
		}
        out.erase(std::remove_if(out.begin(), out.end(),
                    [board](Move& m){ return misc::illegitimateMove(board, m);}),
                    out.end());
		return out;
	}

	
};