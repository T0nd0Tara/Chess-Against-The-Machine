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

public:
    explicit Piece(const Piece& p)  = default;
    explicit Piece(Piece&& p)  = default;
    explicit Piece(olc::vi2d pos={0,0}, Color c=Color::WHITE): m_pos(pos), m_col(c){}
    static inline bool empty(Piece* board[8][8], olc::vi2d cell){ return !board[cell.y][cell.x]; }
	Color getCol() const { return m_col;}
    olc::vi2d getPos() const {return m_pos;}
    virtual std::vector<Move> getMoves(Piece* (*)[8]) = 0;
    virtual inline const int getValue() = 0;
    virtual inline const bool isPawn() { return false; }
    virtual inline Piece* clone() const = 0;
	virtual bool moveTo(Move move, Piece* board[8][8]) {
		// Invalid starting point for move
		if (move.vFrom != m_pos)
        {
            std::cerr << "invalid move\n";
            return false;
        }
        if (move.eaten) delete board[move.vTo.y][move.vTo.x];
        board[move.vTo.y][move.vTo.x] = board[move.vFrom.y][move.vFrom.x];
        m_pos = move.vTo;
        board[move.vFrom.y][move.vFrom.x] = nullptr;

        return true;

	};
	void inline draw(olc::PixelGameEngine* pge, olc::Decal* decal, olc::vi2d cellSize){
        pge->DrawPartialDecal(cellSize * m_pos, cellSize, decal, m_sprite_cords * DECAL_PIECE_SIZE ,DECAL_PIECE_SIZE);
    }
};
