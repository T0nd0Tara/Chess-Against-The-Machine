#include <iostream>
#include <vector>
#include <algorithm>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "common.h"
#include "Pawn.h"
#include "Rook.h"

class Chess : public olc::PixelGameEngine
{
public:
	Chess()
	{
		sAppName = "Chess";
	}
    
    ~Chess(){
        for (int y=0; y<8; y++)
        {
            for (int x=0; x<8; x++){
                if (board[y][x])
                    delete board[y][x];
            }
            delete[] board[y];
        }

        delete[] board;
        if(dPieces)
            delete dPieces;
    }
private:
	olc::vi2d vCellSize;
	Piece* board[8][8];
    olc::Decal* dPieces;

public:
\

	bool OnUserCreate() override
	{
		vCellSize = olc::vi2d{ScreenWidth() >> 3, ScreenHeight() >> 3};

		for (int y=0; y<8; y++)
			for (int x=0; x<8; x++)
				board[y][x] = nullptr;

		//board[2][2] = std::make_shared<Piece>(new Pawn(olc::vi2d{2,2}, Color::WHITE));
		board[2][2] = new Pawn(olc::vi2d{2,2}, Color::BLACK);
		board[2][0] = new Rook(olc::vi2d{0,2}, Color::WHITE);

        dPieces = new olc::Decal(new olc::Sprite("Chess_Pieces_Sprite.png"));
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        if (GetKey(olc::ESCAPE).bPressed) return false;

		Clear(olc::BLACK);
		// drawing BG
		for (int y = 0; y<8; y++)
			for (int x = 0; x<8; x++){
				FillRect(vCellSize * olc::vi2d{x,y}, vCellSize, ((x+y)%2 == 0) ? olc::WHITE : olc::DARK_CYAN);
			}

		// drawing pieces
		for (int y = 0; y<8; y++)
			for (int x = 0; x<8; x++){
				if (board[y][x]){
					board[y][x]->draw(this, dPieces, vCellSize);
				}
			}

        // highlighting available moves
        olc::vi2d vMousePos = GetMousePos() / vCellSize;
        if (board[vMousePos.y][vMousePos.x]){
            for (auto& move : board[vMousePos.y][vMousePos.x]->getMoves(board)){
                DrawRect(vCellSize * move.vTo, vCellSize, (move.eaten)? olc::RED : olc::YELLOW);
            }
        }
		
		return true;
	}
};


int main()
{
	Chess demo;
	if (demo.Construct(256, 240, 4, 4))
		demo.Start();

	return 0;
}