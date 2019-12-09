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

// 
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
	// 1. next generation

	for(int i = 0; i < nprocs; i++)
		g_GameOfLifeGrid->next();

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
}

void GameOfLifeGrid::next(const int from, const int to)
{
}

void GameOfLifeGrid::next()
{
	
	// 1. Initialize Next Grid, Next Temp
	int** next_Grid;
	int** next_Temp;

	next_Grid = (int**)malloc(sizeof(int*) * (m_Rows + 2));
	if (next_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	next_Temp = (int**)malloc(sizeof(int*) * (m_Rows+ 2));
	if (next_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	next_Grid[0] = (int*)malloc(sizeof(int) * ((m_Rows + 2) * (m_Cols+ 2)));
	if (next_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	next_Temp[0] = (int*)malloc(sizeof(int) * ((m_Rows + 2) * (m_Cols + 2)));
	if (next_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< m_Rows + 2; i++) {
		next_Grid[i] = next_Grid[i-1] + m_Cols + 2;
		next_Temp[i] = next_Temp[i-1] + m_Cols + 2;
	}

	for (int i=0; i < m_Rows + 2; i++) {
		for (int j=0; j < m_Cols + 2; j++) {
			next_Grid[i][j] = next_Temp[i][j] = 0;
		}
	}


	// 2. Fill next_Grid, next_Temp
	for(int i = 1; i <= m_Rows; i++){
		for(int j = 1; j <= m_Cols; j++){
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
	for(int i = 1; i <= m_Rows; i++){
		m_Grid[i] = m_Grid[i-1] + m_Cols + 2;
		m_Temp[i] = m_Temp[i-1] + m_Cols + 2;
	}

	// 5. print
	dump();
}

int GameOfLifeGrid::getNumOfNeighbors(int rows, int cols)
{
	int numOfNeighbors = 0;

	for(int i = rows - 1; i < rows + 2; i++){
		for(int j = cols - 1; j < cols + 2; j++){
			numOfNeighbors += m_Grid[i][j];
		}
	}	
	numOfNeighbors -= m_Grid[rows][cols]; 
	
	return numOfNeighbors;
}

// HINT: YOU CAN MODIFY BELOW CODES IF YOU WANT
void GameOfLifeGrid::dump() 
{
	cout << "===============================" << endl;

	for (int i=1; i <= m_Rows; i++) {
		
		cout << "[" << i << "] ";
		
		for (int j=1; j <= m_Cols; j++) {
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

	m_Grid = (int**)malloc(sizeof(int*) * (rows + 2));
	if (m_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	m_Temp = (int**)malloc(sizeof(int*) * (rows + 2));
	if (m_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	m_Grid[0] = (int*)malloc(sizeof(int) * ((rows+2)*(cols+2)));
	if (m_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	m_Temp[0] = (int*)malloc(sizeof(int) * ((rows+2)*(cols+2)));	
	if (m_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< rows + 2; i++) {
		m_Grid[i] = m_Grid[i-1] + cols + 2;
		m_Temp[i] = m_Temp[i-1] + cols + 2;
	}

	for (int i=0; i < rows + 2; i++) {
		for (int j=0; j < cols + 2; j++) {
			m_Grid[i][j] = m_Temp[i][j] = 0;
		}
	}

}

