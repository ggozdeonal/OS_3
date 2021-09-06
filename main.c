#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TASK_FILE_NAME  "tasks.txt"
#define MAX_FRAME_SIZE  2048
#define MAX_FRAME_COUNT 500
#define MAX_TASK_FILE_LINE_SIZE 1000
#define MAX_TASK_NAME   256
#define MAX_TASK_ACCESS_COUNT   1000
#define MAX_TASK_COUNT   100
#define ACCESS_COUNT_EVERY_CYCLE 10

#define DEBUG_PRINT__    // TODO: remove this

typedef struct processPageTable
{
    int aiFrame[MAX_FRAME_COUNT];
    int iFrameCount;
} processPageTableStruct;

typedef struct taskList
{
    char strTaskName[MAX_TASK_NAME];
    int iAccessList[MAX_TASK_ACCESS_COUNT];
    int iAccessCount;
    int iTaskFileSize;
    processPageTableStruct taskPageTable;
} taskListStruct;

static char **s_strMemory;
static taskListStruct s_taskListArray[MAX_TASK_COUNT];
static int s_iTaskCount;
static int s_iFrameCount;
static int s_aiUsedFrames[MAX_FRAME_COUNT];

static int isAllFrameUsed()
{
    int iFrameNumber;
    int iRet = 1;

    for (iFrameNumber = 0; iFrameNumber < MAX_FRAME_COUNT; iFrameNumber++)
    {
        if (s_aiUsedFrames[iFrameNumber] == 0)
        {
            iRet = 0;
            break;
        }
    }

    return iRet;
}

static int getRandomEmptyFrame()
{
    int iRandomFrameNumber = -1;

    if (isAllFrameUsed() == 0)
    {
        iRandomFrameNumber = rand() % MAX_FRAME_COUNT;
        while (s_aiUsedFrames[iRandomFrameNumber] != 0)
        {
            iRandomFrameNumber = rand() % MAX_FRAME_COUNT;
        }

        s_aiUsedFrames[iRandomFrameNumber] = 1;
    }

    return iRandomFrameNumber;
}

static int getMaxAccessCount()
{
    int iMax = 0;
    int iTaskIndex;

    for (iTaskIndex = 0; iTaskIndex < s_iTaskCount; iTaskIndex++)
    {
        // find max access count between accepted tasks
        if (s_taskListArray[iTaskIndex].taskPageTable.iFrameCount > 0)
        {
            if (s_taskListArray[iTaskIndex].iAccessCount > iMax)
            {
                iMax = s_taskListArray[iTaskIndex].iAccessCount;
            }
        }
    }

    return iMax;
}

static void allocateMemory()
{
    int iPageIndex;

    s_strMemory = (char **)malloc(MAX_FRAME_COUNT * sizeof(char *));
    for (iPageIndex = 0; iPageIndex < MAX_FRAME_COUNT; iPageIndex++)
        s_strMemory[iPageIndex] = (char *)malloc(MAX_FRAME_SIZE * sizeof(char));
}

static void freeMemory()
{
    int iPageIndex;

    for (iPageIndex = 0; iPageIndex <  MAX_FRAME_COUNT; iPageIndex++)
        free(s_strMemory[iPageIndex]);
    free(s_strMemory);
}

static void readTaskList()
{
    FILE* pFile = NULL;
    char strLine[MAX_TASK_FILE_LINE_SIZE];
    int iAccessIndex;

    pFile = fopen(TASK_FILE_NAME, "r");
    if (pFile == NULL)
    {
        printf("Task file <%s> cannot be opened!\n", TASK_FILE_NAME);
        return;
    }

    // read file line by line
    while (fgets(strLine, sizeof(strLine), pFile))
    {
        // parse each line in file
        char *token = strtok(strLine, " ");

        strcpy(s_taskListArray[s_iTaskCount].strTaskName, token);
        iAccessIndex = 0;
        while (token != NULL)
        {
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                s_taskListArray[s_iTaskCount].iAccessList[iAccessIndex] = atoi(token);
                iAccessIndex++;
            }
        }

        if (iAccessIndex < MAX_TASK_ACCESS_COUNT)
        {
            s_taskListArray[s_iTaskCount].iAccessCount = iAccessIndex;
        }
        else
        {
            s_taskListArray[s_iTaskCount].iAccessCount = MAX_TASK_ACCESS_COUNT;
            printf("Too many access is requested in input file line. %d of them will be used\n", MAX_TASK_ACCESS_COUNT);
        }

        s_iTaskCount++;
    }

    fclose(pFile);
}

static int isSpaceAvailable(int iFileSize)
{
    int iFreeSpace;

    iFreeSpace = (MAX_FRAME_COUNT - s_iFrameCount) * MAX_FRAME_SIZE;
    if (iFileSize <= iFreeSpace)
    {
        return 1;
    }

    return 0;
}

static void loadProcesses()
{
    int iTaskIndex;
    int iFileSize;
    int iReadSize;
    int iFrameIndex;
    int iCurrentFrameIndex;
    FILE* pFile = NULL;
    char strLine[MAX_TASK_FILE_LINE_SIZE];

    for (iTaskIndex = 0; iTaskIndex < s_iTaskCount; iTaskIndex++)
    {
        // open task file
        pFile = fopen(s_taskListArray[iTaskIndex].strTaskName, "r");
        if (pFile == NULL)
        {
            printf("Process file <%s> cannot be opened!\n", s_taskListArray[iTaskIndex].strTaskName);
            continue;
        }

        // get file size
        fseek(pFile, 0, SEEK_END);
        iFileSize = ftell(pFile);
        iReadSize = 0;

        s_taskListArray[iTaskIndex].iTaskFileSize = iFileSize;
        s_taskListArray[iTaskIndex].taskPageTable.iFrameCount = 0;

        // read task data chunk by chunk
        if (isSpaceAvailable(iFileSize) == 1)
        {
            rewind(pFile);

            while (iReadSize < iFileSize)
            {
                // get random frame number
                iFrameIndex = getRandomEmptyFrame();
                if (iFrameIndex != -1)
                {
                    iReadSize += fread(s_strMemory[iFrameIndex], sizeof(char), MAX_FRAME_SIZE, pFile);
                    s_iFrameCount++;

                    iCurrentFrameIndex = s_taskListArray[iTaskIndex].taskPageTable.iFrameCount;
                    s_taskListArray[iTaskIndex].taskPageTable.aiFrame[iCurrentFrameIndex] = iFrameIndex;
                    s_taskListArray[iTaskIndex].taskPageTable.iFrameCount++;
                }
                else
                {
                    printf("Page table is full\n");
                    break;
                }
            }
        }
        else
        {
            printf("There is no space left in page table for task <%s>\n", s_taskListArray[iTaskIndex].strTaskName);
        }

        fclose(pFile);
    }
}

static void printTasks()
{
    int iTaskIndex;
    int iAccessIndex;

    printf("----- Task List Begin -----\n");
    for (iTaskIndex = 0; iTaskIndex < s_iTaskCount; iTaskIndex++)
    {
        printf("Task: %s Frame: %d -> ", s_taskListArray[iTaskIndex].strTaskName, s_taskListArray[iTaskIndex].taskPageTable.iFrameCount);
        for (iAccessIndex = 0; iAccessIndex < s_taskListArray[iTaskIndex].iAccessCount; iAccessIndex++)
        {
            printf("%d ", s_taskListArray[iTaskIndex].iAccessList[iAccessIndex]);
        }
        printf("\n");
    }
    printf("----- Task List End -----\n\n");
}

static void printMemory()
{
    int iFrameIndex;

    printf("----- Memory View Begin -----\n");
    for (iFrameIndex = 0; iFrameIndex < MAX_FRAME_COUNT; iFrameIndex++)
    {
        if (s_aiUsedFrames[iFrameIndex] != 0)
        {
            printf("Frame Content: %d %.10s\n", iFrameIndex, s_strMemory[iFrameIndex]);
        }
    }
    printf("----- Memory View End -----\n");
}

static void printPageTable()
{
    int iTaskIndex;
    int iFrameIndex;

    printf("\n----- Page Tables of Processes -----");
    for (iTaskIndex = 0; iTaskIndex < s_iTaskCount; iTaskIndex++)
    {
        printf("\n\nTask: %s (%d bytes)\n", s_taskListArray[iTaskIndex].strTaskName, s_taskListArray[iTaskIndex].iTaskFileSize);
        printf("Frames: ", s_taskListArray[iTaskIndex].strTaskName);
        if (s_taskListArray[iTaskIndex].taskPageTable.iFrameCount == 0)
        {
            printf("-");
        }
        for (iFrameIndex = 0; iFrameIndex < s_taskListArray[iTaskIndex].taskPageTable.iFrameCount; iFrameIndex++)
        {
            printf("%d ", s_taskListArray[iTaskIndex].taskPageTable.aiFrame[iFrameIndex]);
        }
        printf("\nAccepted to Scheduling: %s", s_taskListArray[iTaskIndex].taskPageTable.iFrameCount > 0 ? "Yes" : "No");
    }
    printf("\n\n------------------------------------\n");
}

static void startScheduling()
{
    int iTaskIndex;
    int iAccessIndex;
    int iCurrentAccessIndex;
    int iTotalAccessCycle;
    int iCurrentAccessCycle;
    int iCalculatedFrameIndex;
    int iCalculatedFrameOffset;
    int iMemoryMappedFrameIndex;

    printf("\nScheduling simulation starting...\n");
    iTotalAccessCycle = ((getMaxAccessCount() + ACCESS_COUNT_EVERY_CYCLE - 1) / ACCESS_COUNT_EVERY_CYCLE);

    // iterate over (n / 10) times where n is total number of access (129 200 14535 8765 3333 3378 9999...)
    for (iCurrentAccessCycle = 0; iCurrentAccessCycle < iTotalAccessCycle; iCurrentAccessCycle++)
    {
        // execute each task
        for (iTaskIndex = 0; iTaskIndex < s_iTaskCount; iTaskIndex++)
        {
            // if it is a valid task
            if (s_taskListArray[iTaskIndex].taskPageTable.iFrameCount > 0)
            {
                printf("Current Task: %s: \n", s_taskListArray[iTaskIndex].strTaskName);
                // access 10 times of bytes
                for (iAccessIndex = 0; iAccessIndex < ACCESS_COUNT_EVERY_CYCLE; iAccessIndex++)
                {
                    iCurrentAccessIndex = (iCurrentAccessCycle * ACCESS_COUNT_EVERY_CYCLE) + iAccessIndex;
                    iCalculatedFrameIndex = s_taskListArray[iTaskIndex].iAccessList[iCurrentAccessIndex] / MAX_FRAME_SIZE;
                    iCalculatedFrameOffset = s_taskListArray[iTaskIndex].iAccessList[iCurrentAccessIndex] % MAX_FRAME_SIZE;

                    if (s_taskListArray[iTaskIndex].iTaskFileSize < s_taskListArray[iTaskIndex].iAccessList[iCurrentAccessIndex])
                    {
                        printf("\tRequested Byte: %d Frame: %d Byte Offset: %d Character: '*** Invalid access. Total file size: %d ***'\n",
                               s_taskListArray[iTaskIndex].iAccessList[iCurrentAccessIndex], iCalculatedFrameIndex,
                               iCalculatedFrameOffset, s_taskListArray[iTaskIndex].iTaskFileSize);
                    }
                    else
                    {
                        if (s_taskListArray[iTaskIndex].iAccessCount <= iCurrentAccessIndex)
                        {
                            printf("\tAll access requests are executed for this task. Requested access: %d | Total access %d\n",
                                   iCurrentAccessIndex, s_taskListArray[iTaskIndex].iAccessCount);
                        }
                        else
                        {
                            iMemoryMappedFrameIndex = s_taskListArray[iTaskIndex].taskPageTable.aiFrame[iCalculatedFrameIndex];
                            printf("\tRequested Byte: %d Frame: %d Byte Offset: %d Character: '%c' \n",
                                   s_taskListArray[iTaskIndex].iAccessList[iCurrentAccessIndex], iCalculatedFrameIndex,
                                   iCalculatedFrameOffset, s_strMemory[iMemoryMappedFrameIndex][iCalculatedFrameOffset]);
                        }
                    }
                }
                printf("\n");
            }
        }
    }
}

int main()
{
    srand(time(NULL));
    memset(s_aiUsedFrames, 0, MAX_FRAME_COUNT);
    s_iTaskCount = 0;
    s_iFrameCount = 0;

    allocateMemory();
    readTaskList();
    loadProcesses();
#ifdef DEBUG_PRINT
    printTasks();
    printMemory();
#endif
    printPageTable();   // part 1 of the homework
    startScheduling();  // part 2 of the homework
    freeMemory();

    return 0;
}
