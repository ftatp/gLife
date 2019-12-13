#include <iostream>
#include <fstream>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

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

	
int** nextCompleteGrid;
int** nextCompleteTemp;
int colSizes[100000000] = {0};
//int* thread_join_check_arr;

//void init_next_Grid(int** next_Grid, int** next_Temp, int m_Rows, int m_Cols);

class GameOfLifeGrid {

public:
	GameOfLifeGrid(int cols, int rows, int gen);

	void next();
	void next(const int id, const int from, const int to);
	
	int isLive(int cols, int rows) { return (m_Grid[cols][rows] ? LIVE: DEAD); }
	int getNumOfNeighbors(int cols, int rows);
	
	void dead(int cols, int rows) { m_Temp[cols][rows] = 0; }
	void live(int cols, int rows) { m_Temp[cols][rows] = 1; }

	int decGen() { return m_Generations--; }
	void setGen(int n) { m_Generations = n; }
	void setCell(int cols, int rows) { m_Grid[cols][rows] = true; }
	
	void dump();
	void dumpCoordinate();
	int* getRowAddr(int row) { return m_Grid[row]; }

	int getRows() { return m_Rows; }
	int getCols() { return m_Cols; }
	int getGens() { return m_Generations; }

	void Update(int nprocs);

private:
	int** m_Grid;
	int** m_Temp;
	int m_Cols;
	int m_Rows;
	int m_Generations;

};

GameOfLifeGrid* g_GameOfLifeGrid;
int nprocs;

// Entry point
int main(int argc, char* argv[])
{
	int cols, rows, gen;
	ifstream inputFile;
	int x, y;
	struct timeval start_time, end_time, result_time;
	pthread_t* threadID;
	int status;

	int n_Cells;
	int n_Cells_per_Thread;

	int quot, remain, gap;
	int idx;
	range* work_ranges;
	//int join_check_flag;

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

	g_GameOfLifeGrid = new GameOfLifeGrid(cols, rows, gen);

	while (inputFile.good()) {
		inputFile >> x >> y;
		g_GameOfLifeGrid->setCell(x, y);
		//g_GameOfLifeGrid->dump();
	}

	gettimeofday(&start_time, NULL);

    // HINT: YOU MAY NEED TO WRITE PTHREAD INVOKING CODES HERE

	//g_GameOfLifeGrid->dump();

//	for(int i = 0; i < gen; i++)
//		g_GameOfLifeGrid->next();


	// 1. Calculate job number for each thread
	n_Cells = cols * rows;
	
	quot = n_Cells / nprocs;
	x = n_Cells - quot*nprocs;
	y = nprocs - x;
	
	// 2. Make threads' variables   
	threadID = (pthread_t*)malloc(nprocs * sizeof(pthread_t));
	work_ranges = (range*)malloc(nprocs * sizeof(range));
	//thread_join_check_arr = (int*)malloc(nprocs * sizeof(int));
	//memset(thread_join_check_arr, 0, nprocs * sizeof(int));

	// 3. Repeat gen times
	for(int i = 0; i < gen; i++){
		int thread_start_idx = 0;
		int thread_end_idx = 0;

		nextCompleteGrid = (int**)malloc(nprocs * sizeof(int*));
		nextCompleteTemp = (int**)malloc(nprocs * sizeof(int*));

		// 4. Make threads 
		for(int tid = 0; tid < nprocs; tid++){
			thread_start_idx = thread_end_idx;
			if(tid < x)
				gap = quot + 1;
			else
				gap = quot;
			
			thread_end_idx += gap;

			work_ranges[tid].id = tid;
			work_ranges[tid].start = thread_start_idx;
			work_ranges[tid].end = thread_end_idx;

			colSizes[tid] = thread_end_idx - thread_start_idx;
			
//			thread_join_check_arr[tid] = 1;
			pthread_create(&threadID[tid], NULL, workerThread, &work_ranges[tid]);
			
			//g_GameOfLifeGrid->next(tid, thread_start_idx, thread_end_idx);
			//printf("id: %d, start: %d, end: %d\n", tid, thread_start_idx, thread_end_idx);
		}

		// 5. Synchronize threads
//		while(1){
//			for(int tid = 0; tid < nprocs; tid++)
//				join_check_flag = join_check_flag || thread_join_check_arr[tid];
//
//			if(join_check_flag)
//				join_check_flag = 0;
//			else
//				break;
//		}
		
		for(int tid = 0; tid < nprocs; tid++)
			pthread_join(threadID[tid], NULL);

		// 6. Update m_Grid, m_Temp with nextCompleteGrid, nextCompleteTemp
		g_GameOfLifeGrid->Update(nprocs);

		// 7. free
		for(int tid = 0; tid < nprocs; tid++){
			free(nextCompleteGrid[tid]);
			free(nextCompleteTemp[tid]);
		}
		free(nextCompleteGrid);
		free(nextCompleteTemp);
	}

	// 8. print
	g_GameOfLifeGrid->dump();

	free(threadID);
	free(work_ranges);
		
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &result_time);
	
	cout << "Execution Time: " << result_time.tv_sec << "." << result_time.tv_usec << "s" << endl;

	inputFile.close();

	cout << "Program end... " << endl;
	return 0;
}

// HINT: YOU MAY NEED TO FILL OUT BELOW FUNCTIONS
void* workerThread(void *arg)
{
	range* p_range;
	
	p_range = (range*) arg;
	//printf("id: %d, start: %d, end: %d\n", p_range->id, p_range->start, p_range->end);

	g_GameOfLifeGrid->next(p_range->id, p_range->start, p_range->end);

	//printf("id: %d exit\n\n", p_range->id);
	
	//thread_join_check_arr[p_range->id] = 0;
	pthread_exit(NULL);
	//return (void*) &p_range->id;
}

//void init_next_Grid(int** next_Grid, int** next_Temp, int m_Rows, int m_Cols){


//}

void GameOfLifeGrid::Update(int nprocs){
	int colSize;
	int idx = 0;

	for(int i = 0; i < nprocs ; i++){
		colSize = colSizes[i];//sizeof(nextCompleteGrid[i]) / sizeof(int*);
		for(int j = 0; j < colSize; j++){
			*(*(m_Grid) + idx) = nextCompleteGrid[i][j];
			*(*(m_Temp) + idx) = nextCompleteTemp[i][j];
			idx++;
		}
	}	
}

void GameOfLifeGrid::next(const int id, const int from, const int to){
	int* next_Grid;
	int* next_Temp;

	int i, j;

	next_Grid = (int*) malloc(sizeof(int) * (to - from));
	if (next_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	next_Temp = (int*) malloc(sizeof(int) * (to - from));
	if (next_Temp == NULL) 
		cout << "1 Memory allocation error " << endl;


	for(int idx = from; idx < to; idx++){
		i = idx / m_Rows;
		j = idx % m_Rows;
		next_Temp[idx - from] = getNumOfNeighbors(i, j);

		//next Grid
		if(m_Grid[i][j] == LIVE){
			if(next_Temp[idx - from] == 2 || next_Temp[idx - from] == 3){
				next_Grid[idx - from] = LIVE;
			}
			else{
				next_Grid[idx - from] = DEAD;
			}
		}
		else {
			if(next_Temp[idx - from] == 3){
				next_Grid[idx - from] = LIVE;
			}
			else{
				next_Grid[idx - from] = DEAD;
			}
		}
	}

	nextCompleteGrid[id] = next_Grid;
	nextCompleteTemp[id] = next_Temp;
}

void GameOfLifeGrid::next()
{
	int** next_Grid;
	int** next_Temp;

	// 1. Initialize Next Grid, Next Temp
	next_Grid = (int**)malloc(sizeof(int*) * m_Cols);
	if (next_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	next_Temp = (int**)malloc(sizeof(int*) * m_Cols);
	if (next_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	next_Grid[0] = (int*)malloc(sizeof(int) * (m_Cols * m_Rows));
	if (next_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	next_Temp[0] = (int*)malloc(sizeof(int) * (m_Cols * m_Rows));
	if (next_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< m_Cols; i++) {
		next_Grid[i] = next_Grid[i-1] + m_Rows;
		next_Temp[i] = next_Temp[i-1] + m_Rows;
	}

	for (int i=0; i < m_Cols; i++) {
		for (int j=0; j < m_Rows; j++) {
			next_Grid[i][j] = next_Temp[i][j] = 0;
		}
	}


	// 2. Fill next_Grid, next_Temp
	for(int i = 0; i < m_Cols; i++){
		for(int j = 0; j < m_Rows; j++){
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
	for(int i = 1; i < m_Cols; i++){
		m_Grid[i] = m_Grid[i-1] + m_Rows;
		m_Temp[i] = m_Temp[i-1] + m_Rows;
	}

	// 5. print
	dump();
}

int GameOfLifeGrid::getNumOfNeighbors(int cols, int rows)
{
	int numOfNeighbors = 0;

	for(int i = cols - 1; i < cols + 2; i++){
		if(i >= 0 && i < m_Cols){
			for(int j = rows - 1; j < rows + 2; j++){
				if(j >= 0 && j < m_Rows){
					numOfNeighbors += m_Grid[i][j];
				}
			}
		}
	}	
	numOfNeighbors -= m_Grid[cols][rows]; 
	
	return numOfNeighbors;
}

// HINT: YOU CAN MODIFY BELOW CODES IF YOU WANT
void GameOfLifeGrid::dump() 
{
	cout << "===============================" << endl;

	for (int i=0; i < m_Rows; i++) {
		
		cout << "[" << i << "] ";
		
		for (int j=0; j < m_Cols; j++) {
			cout << m_Grid[j][i] << ' ';
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


GameOfLifeGrid::GameOfLifeGrid(int cols, int rows, int gen)
{
	m_Generations = gen;
	m_Cols = cols;
	m_Rows = rows;

	m_Grid = (int**)malloc(sizeof(int*) * cols);
	if (m_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	m_Temp = (int**)malloc(sizeof(int*) * cols);
	if (m_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	m_Grid[0] = (int*)malloc(sizeof(int) * (cols * rows));
	if (m_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	m_Temp[0] = (int*)malloc(sizeof(int) * (cols * rows));	
	if (m_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< cols; i++) {
		m_Grid[i] = m_Grid[i-1] + rows;
		m_Temp[i] = m_Temp[i-1] + rows;
	}

	for (int i=0; i < cols; i++) {
		for (int j=0; j < rows; j++) {
			m_Grid[i][j] = m_Temp[i][j] = 0;
		}
	}

}

