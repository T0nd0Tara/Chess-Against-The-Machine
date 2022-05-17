#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>
#include <unistd.h>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "common.h"

#include "Pawn.h"
#include "Rook.h"
#include "Knight.h"
#include "Bishop.h"
#include "Queen.h"
#include "King.h"

#include "Computer.h"

#define MAX_FPS 30.0

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
		if(dPieces) delete dPieces;
		if (com) delete com;
	}
private:
	olc::vi2d vCellSize;
	Piece* board[8][8];
	olc::Decal* dPieces;
	Piece* selectedPiece = nullptr;
	
	Color turn;

	Computer* com;

	void drawPieces(){
		for (int y = 0; y<8; y++)
			for (int x = 0; x<8; x++){
				if (board[y][x]){
					board[y][x]->draw(this, dPieces, vCellSize);
				}
			}
	}
public:


	bool OnUserCreate() override
	{
		vCellSize = olc::vi2d{ScreenWidth() >> 3, ScreenHeight() >> 3};

		for (int y=0; y<8; y++)
			for (int x=0; x<8; x++)
				board[y][x] = nullptr;


		dPieces = new olc::Decal(new olc::Sprite("Chess_Pieces_Sprite.png"));

		turn = Color::WHITE;

		com = new Computer(board);

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
		auto clock_start = std::chrono::high_resolution_clock::now();
		if (GetKey(olc::ESCAPE).bPressed) return false;

		if (turn == Color::BLACK){
			com->play();
			turn = Color::WHITE;
		}

		Clear(olc::BLACK);
		// drawing BG
		for (int y = 0; y<8; y++)
			for (int x = 0; x<8; x++){
				FillRect(vCellSize * olc::vi2d{x,y}, vCellSize, ((x+y)%2 == 0) ? olc::WHITE : olc::DARK_CYAN);
			}


		olc::vi2d vMousePos = GetMousePos() / vCellSize;
		if (GetMouse(olc::Mouse::LEFT).bPressed){
			bool bAssaignSelected = true;
			if (selectedPiece){
				std::vector<Move> vMoves = selectedPiece->getMoves(board);
				std::vector<Move>::iterator move;
				// if we click again on the selected piece
				if (selectedPiece->getPos() == vMousePos){
					selectedPiece = nullptr;
					// don't reassaign after we de-assaign selectedPiece
					bAssaignSelected = false;

				}
				// if we clicked on a move
				else if (move = std::find_if(vMoves.begin(), vMoves.end(),
											[vMousePos](Move m){return m.vTo == vMousePos;})
						,
						move != vMoves.end()){
					olc::vi2d pos1 = selectedPiece->getPos();
					olc::vi2d pos2 = move->vTo;
					selectedPiece->moveTo(*move, board);
					if (move->eaten) delete move->eaten;
					board[pos2.y][pos2.x] = board[pos1.y][pos1.x];
					board[pos1.y][pos1.x] = nullptr;
					selectedPiece = nullptr;
					turn = Color::BLACK;
					bAssaignSelected = false;
				}
			}
				
					
			if (bAssaignSelected)
				selectedPiece = board[vMousePos.y][vMousePos.x];
		}
			
		
		// highlighting selected piece
		bool bHighlightOnHover = true;
		if (selectedPiece){
			bHighlightOnHover = selectedPiece->getPos() != vMousePos;
			FillRectDecal(selectedPiece->getPos()*vCellSize,vCellSize, olc::GREEN);
			bool bMouseOnMove = false;
			for (auto& move : selectedPiece->getMoves(board)){
				if (vMousePos == move.vTo) bMouseOnMove = true;
				olc::Pixel transRed = olc::Pixel(255,0,0,128),
					transYellow = olc::Pixel(255,255,0,128);
				FillRectDecal(vCellSize * move.vTo, vCellSize, (move.eaten)? transRed : transYellow);
			}
			if (bMouseOnMove){
				DrawRect(vMousePos * vCellSize, vCellSize - olc::vi2d{1,1}, olc::GREEN);
				bHighlightOnHover = false;
			}

		}
		// highlighting available moves
		if (board[vMousePos.y][vMousePos.x] && bHighlightOnHover){    
			for (auto& move : board[vMousePos.y][vMousePos.x]->getMoves(board)){
				FillRectDecal(vCellSize * move.vTo, vCellSize, olc::Pixel(255,255,0,64));
				DrawRect(vCellSize * move.vTo, vCellSize - olc::vi2d{1,1}, (move.eaten)? olc::RED : olc::YELLOW);
			}
		}

		// drawing pieces
		drawPieces();

		auto clock_end = std::chrono::high_resolution_clock::now();
		double nTime = std::chrono::duration_cast<std::chrono::microseconds>(clock_end - clock_start).count();
		nTime = 1000000.0/MAX_FPS - nTime;
		if (nTime > 0) usleep(nTime);
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