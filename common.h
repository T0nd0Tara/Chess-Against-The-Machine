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


namespace misc{
    Piece* (*boardCopy(Piece* (*board)[8]))[8];
    std::vector<Piece*> getColor(Piece* board[8][8], Color c);
    std::pair<std::vector<Piece*>, std::vector<Piece*>> getColors(Piece* board[8][8]);
    bool isCheck(Piece* (*board)[8], Color lastTurnBy);
    bool isMate(Piece* (*board)[8], Color lastTurnBy);
    bool illegitimateMove(Piece* (*board)[8], Move& move);
}

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
    virtual std::vector<Move> getMoves(Piece* (*)[8], bool removeCheck = true) = 0;
    virtual inline const int getValue() = 0;
    virtual inline const bool isPawn() { return false; }
    virtual inline const bool isKing() { return false; }
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


namespace misc{
    Piece* (*boardCopy(Piece* (*board)[8]))[8]{
		Piece* (*out)[8] = new Piece*[8][8];

		for (int8_t y=0; y<8; y++)
			for (int8_t x=0; x<8; x++){
                out[y][x] = nullptr;
				if (board[y][x]){
                    out[y][x] = board[y][x]->clone();
				}
			}
		return out;
	}

    std::vector<Piece*> getColor(Piece* board[8][8], Color c){
		std::vector<Piece*> vOut;
		for (int y=0; y<8; y++) for (int x=0; x<8; x++)
				if (board[y][x]) if (board[y][x]->getCol() == c)
						vOut.push_back(board[y][x]);
						
		return vOut;
	}
    // return {whitePieces, blackPieces}
    std::pair<std::vector<Piece*>, std::vector<Piece*>> getColors(Piece* board[8][8]){
        std::pair<std::vector<Piece*>, std::vector<Piece*>> vOut;
		for (int y=0; y<8; y++) for (int x=0; x<8; x++)
                if (board[y][x]){
                    if (board[y][x]->getCol() == Color::WHITE)
                        vOut.first.push_back(board[y][x]);
                    else
                        vOut.second.push_back(board[y][x]);
                }

        return vOut;
    }
    bool isCheck(Piece* (*board)[8], Color checkFor){
        std::vector<Piece*> vWhites = getColor(board,Color::WHITE);
        std::vector<Piece*> vBlacks = getColor(board,Color::BLACK);


        auto printBoard = [board](){
            for (int y = 0; y<8; y++){
                    for (int x = 0; x<8; x++){
                        if (board[y][x]){
                            if (board[y][x]->isKing()) std::cout << 'K';
                            else if (board[y][x]->isPawn()) std::cout << 'P';
                            else std::cout << 'X';
                        }else std::cout << ' ';
                    }
                    std::cout << '\n';
                }
        };

        if (checkFor == Color::WHITE){ 
            // checking if white is 'checked'
            auto wKing = std::find_if(vWhites.begin(), vWhites.end(),
                        [](Piece* p){return p->isKing();});

            if (wKing == vWhites.end()){
                printBoard();
                assert(wKing != vWhites.end() && "ERROR: White King - not found\n");  
            }
            olc::vi2d wPos = (*wKing)->getPos();
            for (auto& p: vBlacks){
                std::vector<Move> vMoves = p->getMoves(board, false);
                if (std::find_if(vMoves.begin(), vMoves.end(),
                    [wPos](Move& m){ return m.vTo == wPos;}) != vMoves.end()){
                        return true;
                    }
            }
        }
        else{
            // checking if black is 'checked'
            auto bKing = std::find_if(vBlacks.begin(), vBlacks.end(),
                        [](Piece* p){return p->isKing();});

            if (bKing == vBlacks.end()){
                printBoard();
                assert(bKing != vBlacks.end() && "ERROR: Black King - not found\n");
            }
            olc::vi2d bPos = (*bKing)->getPos();
            for (auto& p: vWhites){
                std::vector<Move> vMoves = p->getMoves(board, false);
                if (std::find_if(vMoves.begin(), vMoves.end(),
                    [bPos](Move& m){ return m.vTo == bPos;}) != vMoves.end()){
                        return true;
                    }
            }
        }


        return false;

        
    }
    bool isMate(Piece* (*board)[8], Color lastTurnBy){

        //std::vector<Piece*> vBlacks = getColor(board, Color::BLACK);
        //std::vector<Piece*> vWhites = getColor(board, Color::WHITE);

        // no mate if there is no check
        if (!isCheck(board, lastTurnBy)) return false;
      
        // if an allay has a move then, it can get you out of a mate
        std::vector<Piece*> vAllays = getColor(board, (lastTurnBy == Color::WHITE)? Color::BLACK : Color::WHITE);
        if (std::find_if(vAllays.begin(), vAllays.end(),
            [board](Piece* p) {return p->getMoves(board).size() != 0; }) != vAllays.end())
            return false;
        
        return true;
    }
    // make sure move can't lead to self check
    bool illegitimateMove(Piece* (*board)[8], Move& move){
        // creating theoretical board
        Piece* (*theoBoard)[8] = misc::boardCopy(board);
        
        theoBoard[move.vFrom.y][move.vFrom.x]->moveTo(move, theoBoard);

        //Color opCol = (theoBoard[move.vTo.y][move.vTo.x]->getCol() == Color::WHITE)? Color::BLACK : Color::WHITE;
        bool bIsCheck = misc::isCheck(theoBoard, theoBoard[move.vTo.y][move.vTo.x]->getCol());

        // deleting thoreticalBoard
        for (int8_t y=0; y<8; y++){
            for (int8_t x=0; x<8; x++){
				if (theoBoard[y][x]) delete theoBoard[y][x];
			}
        }
        delete[] theoBoard;

        // there is no check
        if (!bIsCheck) return false;
                
        return true;
    }
}
