#pragma once
#include "common.h"
#include <iostream>

class Bishop: public Piece{
public:

	explicit Bishop(olc::vi2d pos, Color c): Piece(pos, c){
		m_sprite_cords = olc::vi2d{2, (int)c};
	}
    inline const int getValue() override { return 30; }
    inline Piece* clone() const override { return new Bishop(*this); }
	std::vector<Move> getMoves(Piece* board[8][8]) override{
		std::vector<Move> out;

		// NW
        for (int i=1; i<8; i++){
            olc::vi2d end_pos = m_pos + olc::vi2d{-i,-i};
            if (end_pos.x < 0 || end_pos.y < 0) break;
            if (empty(board, end_pos)) out.push_back(Move(m_pos, end_pos));
            else{
                if (board[end_pos.y][end_pos.x]->getCol() != m_col)
                    out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
                break;
            }
        }

        // NE
        for (int i=1; i<8; i++){
            olc::vi2d end_pos = m_pos + olc::vi2d{i,-i};
            if (end_pos.x > 7 || end_pos.y < 0) break;
            if (empty(board, end_pos)) out.push_back(Move(m_pos, end_pos));
            else{
                if (board[end_pos.y][end_pos.x]->getCol() != m_col)
                    out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
                break;
            }
        }

        // SW
        for (int i=1; i<8; i++){
            olc::vi2d end_pos = m_pos + olc::vi2d{-i,i};
            if (end_pos.x < 0 || end_pos.y > 7) break;
            if (empty(board, end_pos)) out.push_back(Move(m_pos, end_pos));
            else{
                if (board[end_pos.y][end_pos.x]->getCol() != m_col)
                    out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
                break;
            }
        }

        // SE
        for (int i=1; i<8; i++){
            olc::vi2d end_pos = m_pos + olc::vi2d{i,i};
            if (end_pos.x > 7 || end_pos.y > 7) break;
            if (empty(board, end_pos)) out.push_back(Move(m_pos, end_pos));
            else{
                if (board[end_pos.y][end_pos.x]->getCol() != m_col)
                    out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
                break;
            }
        }

		return out;
	}

	
};