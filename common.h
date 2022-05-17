#pragma once
#include <olcPixelGameEngine.h>
#define DECAL_PIECE_SIZE olc::vi2d{64,64}
enum class Color: uint8_t{
	WHITE,
	BLACK
};

class Piece;
struct Move {
	olc::vi2d vFrom;
	olc::vi2d vTo;
	Piece* eaten;
    Move(olc::vi2d from, olc::vi2d to, Piece* eaten_ = nullptr):
        vFrom(from), vTo(to), eaten(eaten_){}
};

class Piece{
protected:
	olc::vi2d m_pos;
	Color m_col;
    olc::vi2d m_sprite_cords;

    static inline bool empty(Piece* board[8][8], olc::vi2d cell){
        return !board[cell.y][cell.x];
    }
public:
    explicit Piece(const Piece& p){
        m_pos = p.m_pos;
        m_col = p.m_col;
        m_sprite_cords = p.m_sprite_cords;
    }
    explicit Piece(olc::vi2d pos={0,0}, Color c=Color::WHITE): m_pos(pos), m_col(c){}
	Color getCol() const { return m_col;}
    virtual std::vector<Move> getMoves(Piece* (*)[8]) = 0;
	bool moveTo(Move move, Piece* board[8][8]) {
		// Invalid starting point for move
		if (move.vFrom != m_pos)
			return false;

		for (auto m: getMoves(board)){
			if (move.vTo == m.vTo){
				m_pos = move.vTo;
				return true;
			}
		}
		// Invalid move
		return false;
	};
	void inline draw(olc::PixelGameEngine* pge, olc::Decal* decal, olc::vi2d cellSize){
        pge->DrawPartialDecal(cellSize * m_pos, cellSize, decal, m_sprite_cords * DECAL_PIECE_SIZE ,DECAL_PIECE_SIZE);

    }
};
