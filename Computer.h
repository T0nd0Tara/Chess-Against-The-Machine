#pragma once
#include <vector>
#include <mutex>
#include <syncstream>

#include <olcPixelGameEngine.h>

#include "common.h"

#define MAX_THREADS 16
#define DEPTH_SEARCH 3

class Computer;

struct Node{
	Piece* (*board)[8];
	Move* move;
	std::vector<Node*> vChildren;

	Node(Piece* (*board_)[8], Move* m):board(board_), move(m), vChildren(std::vector<Node*>()) {}
	Node() = delete;
	Node(const Node&) = delete;
	Node(Node&&) = default;
	~Node(){
		for (int8_t y=0; y<8; y++){
            for (int8_t x=0; x<8; x++){
				if (board[y][x]) delete board[y][x]; //free(board[y][x]);
			}
        }
        delete[] board;
        for (size_t i=0; i<vChildren.size(); i++)
            if (vChildren[i]) delete vChildren[i];

		if (move) delete move;
	}
};


class Computer{
	Piece* (*m_board)[8];

	std::vector<Piece*> vBlacks, vWhites;


public:
	Computer(Piece* (*board)[8]): m_board(board){}


// function negamax(node, depth, color) is
// if depth = 0 or node is a terminal node then
//     return color × the heuristic value of node
// value := −∞
// for each child of node do
//     value := max(value, −negamax(child, depth − 1, −color))
// return value
	static int negamax(const Node* node, int nDepth, Color c){
		Color opCol = (c == Color::WHITE)? Color::BLACK : Color::WHITE;
		if (nDepth == 0 || misc::isMate(node->board, opCol)){
            auto start = std::chrono::high_resolution_clock::now();
            int out = evaluateBoard(node->board, c);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            //std::cout << "evaluateBoard time: " << time << "\n";
            return out; 
        }
			

		int val = INT_MIN;
		for (auto& n: node->vChildren){
			int childVal = -negamax(n, nDepth-1, opCol);
			if (val < childVal){
				val = childVal;
			}
		}
		return (int)val;
	}

	static void initTree(Node* node, int nDepth, Color moveBy){
		if (nDepth == 0) return;

		Color opCol = (moveBy == Color::WHITE)? Color::BLACK : Color::WHITE;
		std::vector<Piece*> vAvailPieces = misc::getColor(node->board, opCol);
		for (auto& p : vAvailPieces)
			for (auto& m : p->getMoves(node->board)){

				olc::vi2d pos = p->getPos();
                //Node newNode();
				node->vChildren.push_back(new Node(misc::boardCopy(node->board), new Move(m)));
				auto back = node->vChildren.back()->board;
				back[pos.y][pos.x]->moveTo(m, back);
				initTree(node->vChildren.back(), nDepth-1, opCol);
			}
	}

	static int evaluateBoard(Piece* (*board)[8], Color who2Move){
		int out = 0;
		std::vector<Piece*>
			vBoardBlacks = misc::getColor(board, Color::BLACK),
			vBoardWhites = misc::getColor(board, Color::WHITE);

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

        if (who2Move == Color::WHITE)
            out *= -1;
        
        // if there is 
		return out;
			
	}
    //std::osyncstream syncout(std::cout);
    void evalNegamax(int start, int end, int* values, std::vector<Node*>& vChildren){
        for (int i=start; i < end; i++){
            int val = negamax(vChildren[i], DEPTH_SEARCH-1, Color::BLACK);
            values[i] = val;
            std::osyncstream(std::cout) << std::to_string(i) << ": " << std::to_string(val) <<
            ", in vec: " << std::to_string(values[i]) << "\n";
        }
    }

	void play(){
		// vBlacks = getColor(m_board, Color::BLACK);
		// vWhites = getColor(m_board, Color::WHITE);

		Node currState(misc::boardCopy(m_board), nullptr);

		// We're assuming that the last move been done by White
		initTree(&currState, DEPTH_SEARCH, Color::WHITE);
        int nChildren = currState.vChildren.size();
		std::cout << "Moves Avail: " << nChildren << '\n';
        
        int *values = new int[nChildren];

        
        
        auto start = std::chrono::high_resolution_clock::now();
        int nThreads = std::min(MAX_THREADS , nChildren);
        std::thread t[nThreads];
        int len = nChildren / nThreads;
		for (int i=0; i<nThreads - 1; i++){
			t[i] = std::thread(&Computer::evalNegamax,this, i * len, (i + 1) * len, values, std::ref(currState.vChildren));
		}
        t[MAX_THREADS - 1] = std::thread(&Computer::evalNegamax,this, (nThreads - 1) * len , nChildren,  values, std::ref(currState.vChildren));

        for (int i=0; i<nThreads; i++){
            t[i].join();
        }
        auto end = std::chrono::high_resolution_clock::now();
        int time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "evaluation took: " << time << "\n";


        int nMaxNodeIndex = std::max_element(values,values + nChildren) - values;
        delete[] values;

		assert(nMaxNodeIndex < nChildren && "Couldn't find any node for some reason");

        
		Move& move = *currState.vChildren[nMaxNodeIndex]->move;
		m_board[move.vFrom.y][move.vFrom.x]->moveTo(move, m_board);
		
	}
};