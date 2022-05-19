#pragma once
#include <vector>
#include "common.h"

class Computer;

struct Node{
	Piece* (*board)[8];
	Move* move;
	std::vector<Node> vChildren;

	Node(Piece* (*board_)[8], Move* m):board(board_), move(m), vChildren(std::vector<Node>()) {}
	Node() = delete;
	Node(const Node&) = delete;
	Node(Node&& rvalue) = default;
	~Node(){
		for (int8_t y=0; y<8; y++)
			for (int8_t x=0; x<8; x++){
				if (board[y][x]) delete board[y][x];
			}

		if (move) delete move;
	}
};


class Computer{
	Piece* (*m_board)[8];

	std::vector<Piece*> vBlacks, vWhites;

	static std::vector<Piece*> getColor(Piece* board[8][8], Color c){
		std::vector<Piece*> vOut;
		for (int y=0; y<8; y++) for (int x=0; x<8; x++)
				if (board[y][x]) 
					if (board[y][x]->getCol() == c)
						vOut.push_back(board[y][x]);
		return vOut;
	}
public:
	Computer(Piece* (*board)[8])
	:m_board(board)
	{
		//m_board = board;

		vBlacks = getColor(board, Color::BLACK);
		vWhites = getColor(board, Color::WHITE);
	}

	static auto boardCopy(Piece* (*board)[8]){
		Piece* (*out)[8] = new Piece*[8][8];
		const size_t size = sizeof(Piece);
		for (int8_t y=0; y<8; y++)
			for (int8_t x=0; x<8; x++){
				if (board[y][x]){
					memcpy(&out[y][x], &board[y][x],size);
				}
			}

		return out;
	}
// function negamax(node, depth, color) is
// if depth = 0 or node is a terminal node then
//     return color × the heuristic value of node
// value := −∞
// for each child of node do
//     value := max(value, −negamax(child, depth − 1, −color))
// return value
	static std::pair<Move, int> negamax(const Node& node, int nDepth, Color c){
		if (nDepth == 0 /*|| isCheckMate(node.board)*/)
			return std::make_pair<Move,int>(Move(*node.move), evaluateBoard(node.board));
		int val = INT_MIN;
		//auto max = [](int a, int b) { return (a>b)? a : b; };
		Color opCol = (c == Color::WHITE)? Color::BLACK : Color::WHITE;
		Move* outMove = nullptr;
		for (auto& n: node.vChildren){
			auto [childMove, childVal] = negamax(n, nDepth-1, opCol);
			childVal = -childVal;
			if (val < childVal){
				val = childVal;
				if (outMove) delete outMove;
				outMove = new Move(childMove);
			}
		}
		return std::make_pair<Move,int>(Move(*outMove), (int)val);
	}

	static void initTree(Node& node, int nDepth, Color moveBy){
		if (nDepth == 0) return;
		Color opCol = (moveBy == Color::WHITE)? Color::BLACK : Color::WHITE;
		std::vector<Piece*> vAvailPieces = getColor(node.board, opCol);
		for (auto& p : vAvailPieces)
			for (auto& m : p->getMoves(node.board)){
				olc::vi2d pos = p->getPos();
				node.vChildren.push_back(Node(boardCopy(node.board), new Move(m)));
				node.vChildren.back().board[pos.y][pos.x]->moveTo(m, node.vChildren.back().board);
				initTree(node.vChildren.back(), nDepth-1, opCol);
			}
	}


// evaluateBoard(board) =
// 	  20000(K-K')
// 	+ 90(Q-Q')
// 	+ 50(R-R')
// 	+ 30(B-B' + N-N')
// 	+ 10(P-P')
// 	- 5(D-D' + S-S' + I-I')
// 	+ 1(M-M')

// KQRBNP = number of kings, queens, rooks, bishops, knights and pawns
// D,S,I = doubled, blocked and isolated pawns
// M = Mobility (the number of legal moves)
	static int evaluateBoard(Piece* (*board)[8]){
		int out = 0;
		std::vector<Piece*>
			vBoardBlacks = getColor(board, Color::BLACK),
			vBoardWhites = getColor(board, Color::WHITE);

		for (auto& p: vBoardBlacks){
			out += p->getValue() // value
				+ (int)p->getMoves(board).size(); // mobility
		}
			
		for (auto& p: vBoardWhites){
			out -= p->getValue() // value
				+ (int)p->getMoves(board).size(); // mobility
		}
			
		int total = 0;
		for (int8_t x=0; x<8; x++){
			int nWPawns = 0, nBPawns = 0,
				nWBlocked = 0, nBBlocked = 0,
				nWIsolate = 0, nBIsolate = 0;

			for (int8_t y=0; y<8; y++){
				if (Piece::empty(board, olc::vi2d{x,y})) continue;
				if (!board[y][x]->isPawn()) continue;

				Color c = board[y][x]->getCol();

				bool bIsolated = true;
				for(int8_t scanY = -1; scanY<2; scanY++)
					for(int8_t scanX = -1; scanX<2; scanX++){
						if (scanX == 0 && scanY == 0) continue;
						olc::vi2d pos{x + scanX, y+scanY};
						if (pos.x < 0 || pos.x > 7 ||
							pos.y < 0 || pos.y > 7) continue;
						if (Piece::empty(board, pos)) continue;
						
						if (board[pos.y][pos.x]->isPawn() && board[pos.y][pos.x]->getCol() == c){
							bIsolated = false;
							goto NOT_ISOLATED;
						}
					}
			NOT_ISOLATED:

				if (c == Color::BLACK){
					nBPawns++;
					if (y < 7)
						if (!Piece::empty(board, olc::vi2d{x, y+1}))
							nBBlocked++;
					if (bIsolated) nBIsolate++;
				}
				else{
					nWPawns++;
					if (y > 0)
						if (!Piece::empty(board, olc::vi2d{x, y-1}))
							nWBlocked++;
					if (bIsolated) nWIsolate++;
				}
			}

			total += (nBPawns >> 1) - (nWPawns >> 1) // doubled pawns
					+ nBBlocked - nWBlocked // blocked pawns
					+ nBIsolate - nWIsolate; // isolated pawns
		
		}
		out -= 5 * total;
		return out;
			
	}
	void play(){
		vBlacks = getColor(m_board, Color::BLACK);
		vWhites = getColor(m_board, Color::WHITE);
	}
};