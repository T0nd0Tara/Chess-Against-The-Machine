#pragma once
#include "common.h"

class Knight: public Piece{
public:
    explicit Knight(olc::vi2d pos, Color c): Piece(pos, c){
        m_sprite_cords = olc::vi2d{3, (int)c};
    }
    inline const int getValue() override { return 30; }
    inline Piece* clone() const override { return new Knight(*this); }
    std::vector<Move> getMoves(Piece* board[8][8]) override{
        std::vector<Move> out;
        std::vector<olc::vi2d> vDest = {
            {-2, -1}, {-2, +1},
            {-1, +2}, {+1, +2},
            {+2, -1}, {+2, +1},
            {+1, -2}, {-1, -2}
        };
        for (auto& dest : vDest){
            olc::vi2d end_pos = m_pos + dest;
            
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