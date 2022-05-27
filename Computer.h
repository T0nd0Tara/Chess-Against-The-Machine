#pragma once
#include <vector>
#include <mutex>
#include <syncstream>
#include <condition_variable>

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

struct WorkingThread{
    std::thread th;
    std::function<void()> func;
    std::condition_variable cvStart, *cvFinishd;
    std::mutex mux;
    std::atomic<int>* nAvailThreads = nullptr;
    int nId;

    bool bAlive = true, bWorking = false;

    void start(std::function<void()> f){
        func.swap(f);

        std::unique_lock<std::mutex> lm(mux);
        cvStart.notify_one();
    }

    void loop(){

        while (bAlive){
        
            std::unique_lock<std::mutex> lm(mux);
            cvStart.wait(lm);
        
            std::osyncstream(std::cout) << "Thread #" << nId << ", Started\n";
            //(*nAvailThreads)--;
            //bWorking = true;

            func();

            std::osyncstream(std::cout) << "Thread #" << nId << ", Finished\n";
            (*nAvailThreads)++;
            bWorking = false;

            //std::unique_lock<std::mutex> lm(mux);
            //cvFinishd->notify_one();
        }
    }

};

class Computer{
	Piece* (*m_board)[8];

    //std::condition_variable m_cvThrFinished;

    WorkingThread t[MAX_THREADS];

    std::atomic<int> m_nAvailThreads;

    bool bBaseThreadsStarted;

private:
    std::mutex mux;
    std::vector<int> vAvailThreads;
    // insist means the function will not exit
    // if it didn't a free thread right away
    // return if thread start was successful
    bool startThread(std::function<void()> f, bool insist = true){
        std::lock_guard<std::mutex> lm(mux);
        //std::cout << "startThread called\n";

        if (m_nAvailThreads == 0 && !insist) return false;
        while (m_nAvailThreads == 0){}

        if (vAvailThreads.size() == 0)
            for (int i=0; i<MAX_THREADS; i++){
                if (!t[i].bWorking)
                    vAvailThreads.push_back(i);
            }

        std::cout << "vAvailThreads: ";
        for (auto& n: vAvailThreads){
            std::cout << n << " ";
        }
        std::cout << '\n';
        int id = vAvailThreads.back();
        vAvailThreads.pop_back();
        t[id].bWorking = true;
        m_nAvailThreads--;
        std::osyncstream(std::cout) << "Starting thread #" << id << "\n";
        t[id].start(f);

        return true;
    } 
public:
	Computer(Piece* (*board)[8]): m_board(board){
        for (int i=0; i<MAX_THREADS; i++){
            t[i].bAlive = true;
            t[i].bWorking = false;
            t[i].nId = i;
            //t[i].cvFinishd = &m_cvThrFinished;
            t[i].nAvailThreads = &m_nAvailThreads;
            t[i].th = std::thread(&WorkingThread::loop, &t[i]);
        }
        m_nAvailThreads = MAX_THREADS;
    }

    ~Computer(){
        for (int i=0; i<MAX_THREADS; i++){
            t[i].bAlive = false;
            t[i].start([](){});
        }
    }


// function negamax(node, depth, color) is
// if depth = 0 or node is a terminal node then
//     return color × the heuristic value of node
// value := −∞
// for each child of node do
//     value := max(value, −negamax(child, depth − 1, −color))
// return value
	int negamax(const Node* node, int nDepth, Color c){
		Color opCol = (c == Color::WHITE)? Color::BLACK : Color::WHITE;
		if (nDepth == 0 || misc::isMate(node->board, opCol)){
            //auto start = std::chrono::high_resolution_clock::now();
            int out = evaluateBoard(node->board, c);
            //auto end = std::chrono::high_resolution_clock::now();
            //double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            //std::osyncstream(std::cout) << "evaluateBoard time: " << time << "\n";
            return out; 
        }
			
        size_t nChildren = node->vChildren.size();
		int vals[nChildren];
        
		for (int i=0; i<nChildren; i++){
            vals[i] = INT_MIN;
            auto func = [=, this, &vals, &node](){
                vals[i] = -negamax(node->vChildren[i], nDepth-1, opCol);
            };
            
            if (bBaseThreadsStarted)
                if (startThread(func, false))
                    continue;
            func();
		}

        // waiting for all the threads we are using in here to finish
        std::osyncstream(std::cout) << "Waiting for threads to end\n";
        while(std::find(vals, vals + nChildren, INT_MIN) != vals + nChildren){}
        std::osyncstream(std::cout) << "Threads to ended\n";

        int max_val = *std::max_element(vals, vals + nChildren);
        //delete[] vals;
		return max_val;
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
		auto [vBoardWhites, vBoardBlacks] = misc::getColors(board);
        // std::vector<Piece*>
        //     vBoardWhites = misc::getColor(board, Color::WHITE),
        //     vBoardBlacks = misc::getColor(board, Color::BLACK);

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
            ". from: " << start << ", to: " << end << "\n";
        }
    }

	void play(){

		Node currState(misc::boardCopy(m_board), nullptr);

		// We're assuming that the last move been done by White
		initTree(&currState, DEPTH_SEARCH, Color::WHITE);
        int nChildren = currState.vChildren.size();
		std::cout << "Moves Avail: " << nChildren << '\n';
        
        int *values = new int[nChildren];

        
        
        auto start = std::chrono::high_resolution_clock::now();
        int nThreads = std::min(MAX_THREADS , nChildren);
        int len = nChildren / nThreads;
        bBaseThreadsStarted = false;
        assert(len > 0);
		for (int i=0; i<nThreads - 1; i++){
			startThread([=, this, &currState, &values](){evalNegamax(i*len, (i+1)*len, values, currState.vChildren);});
		}
        startThread([=, this, &currState, &values](){evalNegamax((nThreads - 1) * len , nChildren, values, currState.vChildren);});
        bBaseThreadsStarted = true;
        std::osyncstream(std::cout) << "started base threads\n";
        while (m_nAvailThreads < MAX_THREADS){}

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