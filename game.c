#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GRID_SIZE 20
#define NUM_THREADS 4
#define GENERATIONS 32
#define ALIVE 1
#define DEAD 0

int grid[GRID_SIZE][GRID_SIZE];
int next_gen[GRID_SIZE][GRID_SIZE];

pthread_barrier_t barrier;

void print_grid() {
    system("clear"); 
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 1) {
                printf("# ");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    usleep(500000); 
}

int check_if_will_be_alive(int i, int j)
{
    int alive_neighbours = 0;
    int start_row = ((i - 1) >= 0) ? (i - 1) : 0;
    int end_row = ((i + 1) < GRID_SIZE) ? (i + 1) : GRID_SIZE - 1;
    int start_col = ((j - 1) >= 0) ? (j - 1) : 0;
    int end_col = ((j + 1) < GRID_SIZE) ? (j + 1) : GRID_SIZE - 1;

    for (int row = start_row; row <= end_row; row++)
    {
        for (int col = start_col; col <= end_col; col++)
        {
            if ((row == i) && (col == j)) continue;

            if (grid[row][col] == ALIVE) alive_neighbours++;                
        }
    }

    if (grid[i][j] == DEAD)
    {
        if (alive_neighbours == 3) return 1; // dead cell with 3 alive neighbours will be alive (birth)

        else  return 0; // dead cell will remain dead
    }

    if (grid[i][j] == ALIVE)
    {
        if ((alive_neighbours == 2) || (alive_neighbours == 3))  return 1;  // alive cell with 2 or 3 alive neighbours will survive

        else return 0; // isolation or overcrowding (death)
    }

    return 0;
}


void *compute_next_gen(void *arg)
{
    
    int *thread_args = (int *)arg;
    int threadNum = thread_args[0];
    int start = thread_args[1];
    int end = thread_args[2];

    for (int i = start; i <= end; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            next_gen[i][j] = check_if_will_be_alive(i, j);
        }
    }
    pthread_barrier_wait(&barrier);

    for (int i = start; i <= end; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            grid[i][j] = next_gen[i][j];
        }
    }

    pthread_barrier_wait(&barrier);

    return NULL;
}

void initialize_grid(int grid[GRID_SIZE][GRID_SIZE])
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            grid[i][j] = 0; // Set every cell to 0 (dead)
        }
    }
}

void initialize_patterns(int grid[GRID_SIZE][GRID_SIZE])
{

    initialize_grid(grid);

    // Initialize Still Life (Square) at top-left
    grid[1][1] = 1;
    grid[1][2] = 1;
    grid[2][1] = 1;
    grid[2][2] = 1;

    // Initialize Oscillator (Blinker) in the middle
    grid[5][6] = 1;
    grid[6][6] = 1;
    grid[7][6] = 1;

    // Initialize Spaceship (Glider) in the bottom-right
    grid[10][10] = 1;
    grid[11][11] = 1;
    grid[12][9] = 1;
    grid[12][10] = 1;
    grid[12][11] = 1;
}


int main()
{
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS][3];

    initialize_patterns(grid);

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    for (int generation = 0; generation < GENERATIONS; generation++)
    {
        // printf("Generation %d\n", generation+1);
        print_grid();
        
        int threadNum;
        for (threadNum = 0; threadNum < NUM_THREADS; threadNum++)
        {
            thread_args[threadNum][0] = threadNum;
            thread_args[threadNum][1] = threadNum * (GRID_SIZE / NUM_THREADS);
            thread_args[threadNum][2] = (threadNum + 1) * (GRID_SIZE / NUM_THREADS) - 1;
            // printf("Creating thread %d\n", threadNum);
            int check = pthread_create(&threads[threadNum],
                                       NULL,
                                       compute_next_gen,
                                       (void *)&thread_args[threadNum]);
            if (check != 0)
            {
                printf("Error creating thread %d\n", threadNum);
                exit(EXIT_FAILURE);
            }
        }
        
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}
