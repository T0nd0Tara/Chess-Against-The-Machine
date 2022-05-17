#include <iostream>
#include <vector>
#include <algorithm>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "common.h"

#include "Pawn.h"
#include "Rook.h"
#include "Knight.h"
#include "Bishop.h"
#include "Queen.h"
#include "King.h"

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


        dPieces = new olc::Decal(new olc::Sprite("Chess_Pieces_Sprite.png"));

#pragma region Setup Pieces
        // Pawns
        for (int i=0; i<8; i++){
            board[1][i] = new Pawn(olc::vi2d{i, 1}, Color::BLACK);
            board[6][i] = new Pawn(olc::vi2d{i, 6}, Color::WHITE);
        }
        // Rooks
        board[0][0] = new Rook(olc::vi2d{0,0}, Color::BLACK);
        board[0][7] = new Rook(olc::vi2d{7,0}, Color::BLACK);

        board[7][0] = new Rook(olc::vi2d{0,7}, Color::WHITE);
        board[7][7] = new Rook(olc::vi2d{7,7}, Color::WHITE);

        // Knights
        board[0][1] = new Knight(olc::vi2d{1,0}, Color::BLACK);
        board[0][6] = new Knight(olc::vi2d{6,0}, Color::BLACK);

        board[7][1] = new Knight(olc::vi2d{1,7}, Color::WHITE);
        board[7][6] = new Knight(olc::vi2d{6,7}, Color::WHITE);

        // Bishops
        board[0][2] = new Bishop(olc::vi2d{2,0}, Color::BLACK);
        board[0][5] = new Bishop(olc::vi2d{5,0}, Color::BLACK);

        board[7][2] = new Bishop(olc::vi2d{2,7}, Color::WHITE);
        board[7][5] = new Bishop(olc::vi2d{5,7}, Color::WHITE);

        // Queens
        board[0][3] = new Queen(olc::vi2d{3,0}, Color::BLACK);
        board[7][3] = new Queen(olc::vi2d{3,7}, Color::WHITE);

        // Kings
        board[0][4] = new King(olc::vi2d{4,0}, Color::BLACK);
        board[7][4] = new King(olc::vi2d{4,7}, Color::WHITE);
#pragma endregion

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
                DrawRect(vCellSize * move.vTo, vCellSize - olc::vi2d{1,1}, (move.eaten)? olc::RED : olc::YELLOW);
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