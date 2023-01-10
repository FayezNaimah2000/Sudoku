#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


#define BOARD_SIZE 9

//Integer array representing the sudoku board
int Board[BOARD_SIZE][BOARD_SIZE];
//Boolean array representing the thread determined the board state is valid
int* valid;

pthread_t* threads;

typedef struct {
    int rowStart;
    int rowEnd;
    int colStart;
    int colEnd;
    int tid;
} parameters;
parameters* data;

//For method 1, checks all rows, does not use start row/col
void* CheckRows(void* param)
{
    parameters* location = (parameters*)param;
    
    for(int i = 0; i<BOARD_SIZE; i++)
    {
        int used[10] = {0};
        for(int j = 0; j< BOARD_SIZE; j++)
        {
            if(used[Board[i][j]] == 1)
            {
                valid[location->tid] = 0;
                pthread_exit(0);
            }
            else
                used[Board[i][j]] = 1;
        }
    }
    valid[location->tid] = 1;
    pthread_exit(0);
}

//For method 1, checks all columns, does not use start row/col
void* CheckCols(void* param)
{
    parameters* location = (parameters*)param;
    for(int i = 0; i<BOARD_SIZE; i++)
    {
        int used[10] = {0};
        
        for(int j = 0; j< BOARD_SIZE; j++)
        {
            if(used[Board[j][i]] == 1)
            {
                valid[location->tid] = 0;
                pthread_exit(0);
            }
            else
            {
                used[Board[j][i]] = 1;
            }
        }
        
    }
    valid[location->tid] = 1;
    pthread_exit(0);
}

/*Checks the area passed by rowStart/rowEnd and colStart/colEnd*/
void* CheckValidity(void* param)
{
    parameters* location = (parameters*)param;
    int used[10] = {0};
    for(int i = location->rowStart; i<=location->rowEnd; i++)
        for(int j = location->colStart; j<=location->colEnd; j++)
        {
            if(used[Board[i][j]] == 1)
            {
                valid[location->tid] = 0;
                pthread_exit(0);
            }
            else
                used[Board[i][j]] = 1;
        }
    valid[location->tid] = 1;
    pthread_exit(0);
}

void ParseInput()
{
    char BoardString[128];
    FILE* inputF = fopen("input.txt","r");
    printf("Board state of input.txt:\n");
    
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        fgets(BoardString,128,inputF);
        printf("%s",BoardString);
        char* split = strtok(BoardString," \n");
        if(split == NULL)
            break;
        for(int j = 0; j<BOARD_SIZE;j++)
        {
            Board[i][j] = atoi(split);
            split = strtok(NULL," \n");
        }
    }
    printf("\n");
}
/*Method 1 creates 1 thread to check all rows, 1 thread to check
all columns, and 1 thread for each 3x3 square, for a total of 11 threads*/
void method1()
{
    clock_t start,end;
    start = clock();
    ParseInput();
    int tid = 0;
    threads = (pthread_t*)malloc(sizeof(pthread_t) * 11);
    valid = (int*)malloc(sizeof(int)*11);
    data = (parameters*)malloc(sizeof(parameters) * 11);
    
    data[0].tid = tid;
    threads[tid] = pthread_create(&threads[tid],NULL,CheckRows,&data[tid]);
    tid++;
    data[1].tid = tid;
    threads[tid] = pthread_create(&threads[tid],NULL,CheckCols,&data[tid]);
    tid++;

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            data[tid].tid = tid;
            data[tid].rowStart = i * 3;
            data[tid].rowEnd = i * 3 + 2;
            data[tid].colStart = j * 3;
            data[tid].colEnd = j * 3 + 2; 
            pthread_create(&threads[tid],NULL,CheckValidity,&data[tid]);
            tid++;
            
        }
    }
    for(int i = 0; i< 11; i++)
    {
        pthread_join(threads[i],NULL);
    }

    for(int i = 0; i < 11; i++)
    {
        if(valid[i] != 1)
        {
            end = clock();
            printf("SOLUTION: NO (%f MILLISECONDS)\n",(double)((end-start)*1000.0/CLOCKS_PER_SEC));
            free(valid);
            free(threads);
            free(data);
            return;
        }
    }
    end = clock();
    printf("SOLUTION: YES (%f MILLISECONDS)\n",(double)((end-start)*1000.0/CLOCKS_PER_SEC));
    free(valid);
    free(threads);
    free(data);
    return;
}

/*Method 2 creates a thread to check each indivdual col and row
and each 3x3 square, for a total of 27 threads*/
void method2()
{
    clock_t start,end;
    start = clock();
    ParseInput();
    int tid = 0;
    threads = (pthread_t*)malloc(sizeof(pthread_t) * 27);
    valid = (int*)malloc(sizeof(int)*27);
    data = (parameters*)malloc(sizeof(parameters) * 27);

    //Create thread for each column
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        data[tid].tid = tid;
        data[tid].rowStart = 0;
        data[tid].rowEnd = 7;
        data[tid].colStart = i;
        data[tid].colEnd = i;
        pthread_create(&threads[tid],NULL,CheckValidity,&data[tid]);
        tid++;
    }

    //Create thread for each row
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        data[tid].tid = tid;
        data[tid].rowStart = i;
        data[tid].rowEnd = i;
        data[tid].colStart = 0;
        data[tid].colEnd = 7;
        pthread_create(&threads[tid],NULL,CheckValidity,&data[tid]);
        tid++;
    }

    //Create thread for each square
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            data[tid].tid = tid;
            data[tid].rowStart = i * 3;
            data[tid].rowEnd = i * 3 + 2;
            data[tid].colStart = j * 3;
            data[tid].colEnd = j * 3 + 2; 
            pthread_create(&threads[tid],NULL,CheckValidity,&data[tid]);
            tid++;
            
        }
    }

    for(int i = 0; i< 27; i++)
    {
        pthread_join(threads[i],NULL);
    }

    for(int i = 0; i < 27; i++)
    {
        if(valid[i] != 1)
        {
            end = clock();
            printf("SOLUTION: NO (%f MILLISECONDS)\n",(double)((end-start)*1000.0/CLOCKS_PER_SEC));
            free(valid);
            free(threads);
            free(data);
            return;
        }
    }
    end = clock();
    printf("SOLUTION: YES (%f MILLISECONDS)\n",(double)((end-start)*1000.0/CLOCKS_PER_SEC));
    free(valid);
    free(threads);
    free(data);
    return;

}

/*Method 3 creates 3 child processes, one to check rows, one for columns
and one for the 9 3x3 squares
Communicates via a shm buffer*/
void method3()
{
    clock_t start,end;
    start = clock();
    ParseInput();
    pid_t pid,pid2,pid3;
    const char* NAME = "shm";
    const int SIZE = 10;
    pid = fork();
    //Child 1
    if(pid == 0)
    {
        int shm_fd;
        void* ptr;
        shm_fd = shm_open(NAME,O_RDWR, 0666);
        ftruncate(shm_fd,SIZE);
        ptr = mmap(0,SIZE,PROT_WRITE,MAP_SHARED, shm_fd,0);
        //Check rows
        for(int i = 0; i<BOARD_SIZE; i++)
        {
            int used[10] = {0};
            for(int j = 0; j< BOARD_SIZE; j++)
            {
                if(used[Board[i][j]] == 1)
                {
                    sprintf(ptr,"0");
                    exit(0);
                }
                else
                    used[Board[i][j]] = 1;
            }
        }
        sprintf(ptr,"1");
        exit(0);
    }
    
    pid2 = fork();

    //Child 2
    if(pid2 == 0)
    {
        int shm_fd;
        void* ptr;
        shm_fd = shm_open(NAME,O_RDWR, 0666);
        ftruncate(shm_fd,SIZE);
        ptr = mmap(0,SIZE,PROT_WRITE,MAP_SHARED, shm_fd,0);
        ptr += strlen("11");
        //Check columns
        for(int i = 0; i<BOARD_SIZE; i++)
        {
            int used[10] = {0};
            for(int j = 0; j< BOARD_SIZE; j++)
            {  
                if(used[Board[j][i]] == 1)
                {
                    sprintf(ptr,"0");
                    exit(0);
                }
                else
                    used[Board[j][i]] = 1;
            }
        }
        sprintf(ptr,"1");
        exit(0);
    }

    pid3 = fork();

    //Child 3
    if(pid3 == 0)
    {
        int shm_fd;
        void* ptr;
        shm_fd = shm_open(NAME,O_RDWR, 0666);
        ftruncate(shm_fd,SIZE);
        ptr = mmap(0,SIZE,PROT_WRITE,MAP_SHARED, shm_fd,0);
        ptr += strlen("1111");

        //Check each square
        int rowStart = 0;
        int colStart = 0;
        for(int i = 0; i < 3; i++)
        {
        
            rowStart = rowStart % 9;
            colStart = colStart % 9;
            for(int j = rowStart;j<rowStart+3;j++)
            {
                int used[10] = {0};
                for(int k = colStart;k < colStart + 3; k++)
                {
                    if(used[Board[k][j]] == 1)
                    {
                        sprintf(ptr,"0");
                        exit(0);
                    }
                    else
                    {
                        used[Board[j][k]] = 1;
                    }
                }
            
            }
            colStart += 3;
            rowStart += 3;
        }
        sprintf(ptr,"1");
        exit(0);
    }

    //Parent
    int shm_fd;
    void* ptr;
    shm_fd = shm_open(NAME,O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd,SIZE);
    ptr = mmap(0,SIZE,PROT_READ,MAP_SHARED, shm_fd,0);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    end = clock();
    if(((char*)ptr)[0] == '1' && ((char*)ptr)[2] == '1' && ((char*)ptr)[4] == '1')
    { 
        printf("SOLUTION: YES (%f MILLISECONDS)\n",(double)((end-start)*1000.0/CLOCKS_PER_SEC));
        shm_unlink(NAME);
        return;
    }
    printf("SOLUTION: NO (%f MILLISECONDS)\n",(double)((end-start)*1000.0/CLOCKS_PER_SEC));
    shm_unlink(NAME);
    return;
    
    
}

int main(int argc, char** argv)
{
    
    if(argc == 1)
    {
        printf("Use: ./main (METHOD NUMBER)\n");
        return 0;
    }
    if(atoi(argv[1]) == 1)
    {
        method1();
    }
    else if(atoi(argv[1]) == 2)
    {
        method2();
    }
    else if(atoi(argv[1]) == 3)
    {
        method3();
    }
    return 0;
}