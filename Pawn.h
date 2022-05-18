#pragma once
#include "common.h"

class Pawn: public Piece{
    bool bFirstMove;
public:
    explicit Pawn(olc::vi2d pos, Color c): Piece(pos, c){
        m_sprite_cords = olc::vi2d{5, (int)c};
        bFirstMove = true;
    }
    size_t getValue() override { return 10U; }
    std::vector<Move> getMoves(Piece* board[8][8]) override{
        std::vector<Move> out;
        olc::vi2d end_pos;
        int nMoveDir = (m_col == Color::WHITE)? -1 : 1;

        end_pos = m_pos + olc::vi2d{0, nMoveDir};
        if (empty(board, end_pos)){
            out.push_back(Move(m_pos, end_pos));
        }
        end_pos = m_pos + 2*olc::vi2d{0, nMoveDir};
        if (bFirstMove && empty(board, end_pos) && empty(board, m_pos + olc::vi2d{0, nMoveDir})){
            out.push_back(Move(m_pos, end_pos));
        }

        end_pos = m_pos + olc::vi2d{1, nMoveDir};
        if (!empty(board, end_pos)){
            // only eat if he's not in your team
            if (board[end_pos.y][end_pos.x]->getCol() != m_col)
                out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
        }

        end_pos = m_pos + olc::vi2d{-1, nMoveDir};
        if (!empty(board, end_pos)){
            // only eat if he's not in your team
            if (board[end_pos.y][end_pos.x]->getCol() != m_col)
                out.push_back(Move(m_pos, end_pos, board[end_pos.y][end_pos.x]));
        }
        return out;
    }
    bool moveTo(Move move, Piece* board[8][8]) override {
        bool hasMoved = Piece::moveTo(move, board);
        // TODO: fix bug - bFirstMove doesn't change after first move
        if (hasMoved)
            bFirstMove = false;
        return hasMoved;
    }
};