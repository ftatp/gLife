#include <iostream>
#include <fstream>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define LIVE 1
#define DEAD 0

# define timersub(a, b, result)                 \
	do {                        \
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec;           \
		(result)->tv_usec = (a)->tv_usec - (b)->tv_usec;            \
		if ((result)->tv_usec < 0) {                \
			--(result)->tv_sec;                 \
			(result)->tv_usec += 1000000;               \
		}                       \
	} while (0)

using namespace std;

// function prototype
void* workerThread(void *);
void para_range(int, int, int, int, int*, int*);

typedef struct{
	int id;
	int start;
	int end;
}range;


//void init_next_Grid(int** next_Grid, int** next_Temp, int m_Rows, int m_Cols);

class GameOfLifeGrid {

public:
	GameOfLifeGrid(int rows, int cols, int gen);

	void next();
	void next(const int from, const int to);
	
	int isLive(int rows, int cols) { return (m_Grid[rows][cols] ? LIVE: DEAD); }
	int getNumOfNeighbors(int rows, int cols);
	
	void dead(int rows, int cols) { m_Temp[rows][cols] = 0; }
	void live(int rows, int cols) { m_Temp[rows][cols] = 1; }

	int decGen() { return m_Generations--; }
	void setGen(int n) { m_Generations = n; }
	void setCell(int rows, int cols) { m_Grid[rows][cols] = true; }
	
	void dump();
	void dumpCoordinate();
	int* getRowAddr(int row) { return m_Grid[row]; }

	int getRows() { return m_Rows; }
	int getCols() { return m_Cols; }
	int getGens() { return m_Generations; }


private:
	int** m_Grid;
	int** m_Temp;
	int m_Rows;
	int m_Cols;
	int m_Generations;

};

GameOfLifeGrid* g_GameOfLifeGrid;
int nprocs;

// Entry point
int main(int argc, char* argv[])
{
	int rows, cols, gen;
	ifstream inputFile;
	int x, y;
	struct timeval start_time, end_time, result_time;
	pthread_t* threadID;
	int status;

	int n_Cells;
	int n_Cells_per_Thread;

	int quot, remain, gap;

	range* work_ranges;

	if (argc != 6) {
		cout <<"Usage: " << argv[0] << " <input file> <nprocs> <# of generations> <width> <heigh>" << endl;
		return 1;
	}

	inputFile.open(argv[1], ifstream::in);

	if (inputFile.is_open() == false) {
		cout << "The "<< argv[1] << " file can not be opend" << endl;
		return 1;
	}
	
	nprocs = atoi(argv[2]);
	gen = atoi(argv[3]);
	cols = atoi(argv[4]);
	rows = atoi(argv[5]);

	g_GameOfLifeGrid = new GameOfLifeGrid(rows, cols, gen);

	while (inputFile.good()) {
		inputFile >> x >> y;
		g_GameOfLifeGrid->setCell(x, y);
		//g_GameOfLifeGrid->dump();
	}

	gettimeofday(&start_time, NULL);

    // HINT: YOU MAY NEED TO WRITE PTHREAD INVOKING CODES HERE

	g_GameOfLifeGrid->dump();

	for(int i = 0; i < gen; i++)
		g_GameOfLifeGrid->next();

//	n_Cells = rows * cols;
//	
//	quot = n_Cells / nprocs;
//	x = n_Cells - quot*nprocs;
//	y = nprocs - x;
//	
////	threadID = (pthread_t*)malloc(nprocs * sizeof(pthread_t));
//	work_ranges = (range*)malloc(nprocs * sizeof(range));
////
//	int thread_start_idx = 0;
//	int thread_end_idx = 0;
//	for(int tid = 0; tid < nprocs; tid++){
//		thread_start_idx = thread_end_idx;
//		if(tid < x)
//			gap = quot + 1;
//		else
//			gap = quot;
//		
//		thread_end_idx += gap;
//
//		work_ranges[tid].id = tid;
//		work_ranges[tid].start = thread_start_idx;
//		work_ranges[tid].end = thread_end_idx;
//
//		//pthread_create(&threadID[tid], NULL, workerThread, &work_ranges[tid]);
//		g_GameOfLifeGrid(thread_start_idx, thread_end_idx);
//
//
//		//printf("id: %d, start: %d, end: %d\n", tid, thread_start_idx, thread_end_idx);
//	}
//
//	for(int tid; tid < nprocs; tid++)
//		pthread_join(threadID[tid], NULL);
//
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &result_time);
	
	cout << "Execution Time: " << result_time.tv_sec <<  "s" << endl;
	//g_GameOfLifeGrid->dumpCoordinate();

	inputFile.close();

	cout << "Program end... " << endl;
	return 0;
}

// HINT: YOU MAY NEED TO FILL OUT BELOW FUNCTIONS
void* workerThread(void *arg)
{
	range* p_range;
	
	p_range = (range*) arg;
	printf("id: %d, start: %d, end: %d\n", p_range->id, p_range->start, p_range->end);

	g_GameOfLifeGrid->next(p_range->start, p_range->end);
}

//void init_next_Grid(int** next_Grid, int** next_Temp, int m_Rows, int m_Cols){


//}

void GameOfLifeGrid::next(const int from, const int to)
{
	int** next_Grid;
	int** next_Temp;

	// 1. Initialize Next Grid, Next Temp
	//init_next_Grid(next_Grid, next_Temp, m_Rows, m_Cols);
	next_Grid = (int**)malloc(sizeof(int*) * (m_Rows));
	if (next_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	next_Temp = (int**)malloc(sizeof(int*) * (m_Rows));
	if (next_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	next_Grid[0] = (int*)malloc(sizeof(int) * ((m_Rows) * (m_Cols)));
	if (next_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	next_Temp[0] = (int*)malloc(sizeof(int) * ((m_Rows) * (m_Cols)));
	if (next_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< m_Rows; i++) {
		next_Grid[i] = next_Grid[i-1] + m_Cols;
		next_Temp[i] = next_Temp[i-1] + m_Cols;
	}

	for (int i=0; i < m_Rows; i++) {
		for (int j=0; j < m_Cols; j++) {
			next_Grid[i][j] = next_Temp[i][j] = 0;
		}
	}


}

void GameOfLifeGrid::next()
{
	int** next_Grid;
	int** next_Temp;

	// 1. Initialize Next Grid, Next Temp
	//init_next_Grid(next_Grid, next_Temp, m_Rows, m_Cols);
	next_Grid = (int**)malloc(sizeof(int*) * (m_Rows));
	if (next_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	next_Temp = (int**)malloc(sizeof(int*) * (m_Rows));
	if (next_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	next_Grid[0] = (int*)malloc(sizeof(int) * ((m_Rows) * (m_Cols)));
	if (next_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	next_Temp[0] = (int*)malloc(sizeof(int) * ((m_Rows) * (m_Cols)));
	if (next_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< m_Rows; i++) {
		next_Grid[i] = next_Grid[i-1] + m_Cols;
		next_Temp[i] = next_Temp[i-1] + m_Cols;
	}

	for (int i=0; i < m_Rows; i++) {
		for (int j=0; j < m_Cols; j++) {
			next_Grid[i][j] = next_Temp[i][j] = 0;
		}
	}


	// 2. Fill next_Grid, next_Temp
	for(int i = 0; i < m_Rows; i++){
		for(int j = 0; j < m_Cols; j++){
			// next Temp
			next_Temp[i][j] = getNumOfNeighbors(i, j);

			//next Grid
			if(m_Grid[i][j] == LIVE){
				if(next_Temp[i][j] == 2 || next_Temp[i][j] == 3){
					next_Grid[i][j] = LIVE;
				}
				else{
					next_Grid[i][j] = DEAD;
				}
			}
			
			else {
				if(next_Temp[i][j] == 3){
					next_Grid[i][j] = LIVE;
				}
				else{
					next_Grid[i][j] = DEAD;
				}
			}
			//printf("%d ", next_Grid[i][j]);
		}
		//printf("\n");
	}
	//printf("\n\n");

	free(m_Grid[0]);
	free(m_Temp[0]);
	free(m_Grid);
	free(m_Temp);

	// 4. Update m_Grid, m_Temp
	m_Grid = next_Grid;
	m_Temp = next_Temp;
	for(int i = 1; i < m_Rows; i++){
		m_Grid[i] = m_Grid[i-1] + m_Cols;
		m_Temp[i] = m_Temp[i-1] + m_Cols;
	}

	// 5. print
	dump();
}

int GameOfLifeGrid::getNumOfNeighbors(int rows, int cols)
{
	int numOfNeighbors = 0;

	for(int i = rows - 1; i < rows + 2; i++){
		if(i >= 0 && i < m_Rows){
			for(int j = cols - 1; j < cols + 2; j++){
				if(j >= 0 && j < m_Cols){
					numOfNeighbors += m_Grid[i][j];
				}
			}
		}
	}	
	numOfNeighbors -= m_Grid[rows][cols]; 
	
	return numOfNeighbors;
}

// HINT: YOU CAN MODIFY BELOW CODES IF YOU WANT
void GameOfLifeGrid::dump() 
{
	cout << "===============================" << endl;

	for (int i=0; i < m_Rows; i++) {
		
		cout << "[" << i << "] ";
		
		for (int j=0; j < m_Cols; j++) {
			cout << m_Grid[i][j] << ' ';
		}
		
		cout << endl;
	}
	
	cout << "===============================\n" << endl;
}

void GameOfLifeGrid::dumpCoordinate()
{
	cout << ":: Dump X-Y coordinate" << endl;

	for (int i=0; i < m_Rows; i++) {

		for (int j=0; j < m_Cols; j++) {

			if (m_Grid[i][j]) cout << i << " " << j << endl;
		}
	}
}


GameOfLifeGrid::GameOfLifeGrid(int rows, int cols, int gen)
{
	m_Generations = gen;
	m_Rows = rows;
	m_Cols = cols;

	m_Grid = (int**)malloc(sizeof(int*) * (rows));
	if (m_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	m_Temp = (int**)malloc(sizeof(int*) * (rows));
	if (m_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	m_Grid[0] = (int*)malloc(sizeof(int) * ((rows)*(cols)));
	if (m_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	m_Temp[0] = (int*)malloc(sizeof(int) * ((rows)*(cols)));	
	if (m_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< rows; i++) {
		m_Grid[i] = m_Grid[i-1] + cols;
		m_Temp[i] = m_Temp[i-1] + cols;
	}

	for (int i=0; i < rows; i++) {
		for (int j=0; j < cols; j++) {
			m_Grid[i][j] = m_Temp[i][j] = 0;
		}
	}

}
