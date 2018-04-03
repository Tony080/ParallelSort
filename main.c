/* assert */
#include <assert.h>
/* FILE, fopen, fclose, fscanf, rewind */
#include <stdio.h>
/* EXIT_SUCCESS, malloc, free, qsort */
#include <stdlib.h>
/* time, CLOCKS_PER_SEC */
#include <time.h>
#include <mpi.h>

#define MAIN_PROCESS 0
#define MERGE_MESSAGE_TAG 0
static void load(char const *const filename, int *const np, unsigned int **const valsp) {
    int ret;
    int j, n;
    unsigned int dummy;
    FILE *fp;
    unsigned int *vals;

    /* open the file */
    fp = fopen(filename, "r");
    assert(fp);

    /* count the number of values in the file */
    for (n = 0; fscanf(fp, "%u", &dummy) > 0; ++n);
    assert(feof(fp));

    /* allocate memory for values */
    vals = malloc(n * sizeof(*vals));
    assert(vals);

    /* reset file pointer */
    rewind(fp);

    /* read in the values */
    for (j = 0; j < n; ++j) {
        ret = fscanf(fp, "%u", &vals[j]);
        assert(1 == ret);
    }

    /* close file */
    ret = fclose(fp);
    assert(!ret);

    /* record output values */
    *np = n;
    *valsp = vals;
}

static int asc(void const *const a, void const *const b) {
    return (*(unsigned int *) a) < (*(unsigned int *) b) ? -1 : 1;
}

static void sort_local(int const n, unsigned int *const vals) {
    qsort(vals, n, sizeof(*vals), asc);
}

static void print_numbers(char const *const filename, int const n, unsigned int const *const vals) {
    int ret, i;
    FILE *fp;

    /* open file */
    fp = fopen(filename, "w");
    assert(fp);

    /* write list to fout */
    for (i = 0; i < n; ++i) {
        fprintf(fp, "%u\n", vals[i]);
    }

    /* close file */
    ret = fclose(fp);
    assert(!ret);
}

static void print_time(double const seconds) {
    printf("Sort Time: %0.04fs\n", seconds);
}

static void myScatterv(int numberOfElements, int numberOfProcessors, int **localNumOfElements
        , int ** displs, int rank, unsigned int ** localArray, unsigned int * inputArray){

    int local, reminder;
    local = numberOfElements / numberOfProcessors;
    reminder = numberOfElements % numberOfProcessors;

    //calculate numbers to send to each node.
    *localNumOfElements = malloc(numberOfProcessors * sizeof(**localNumOfElements));
    for (int i = 0; i < numberOfProcessors; i++){
        *(*localNumOfElements + i) = local;
    }
    *(*localNumOfElements + numberOfProcessors - 1) += reminder;

    //calculate offset that points from send buffer(vals).
    *displs = malloc(numberOfProcessors * sizeof(**displs));
    for (int i = 0; i < numberOfProcessors; i++){
        *(*displs + i) = 0;
        for (int j = 0; j < i; j++){
            *(*displs + i) += *(*localNumOfElements + j);
        }
    }

    //create receive buffer space
    *localArray = malloc(local * sizeof(**localArray));
    //send & receive
    MPI_Scatterv(inputArray, *localNumOfElements, *displs, MPI_UNSIGNED, *localArray, (*localNumOfElements)[rank], MPI_UNSIGNED, MAIN_PROCESS, MPI_COMM_WORLD);
}

static void mergeHelper(unsigned int * localArray, int localCount, unsigned int * receivedArray, int receivedCount, unsigned int * mergedArray){
    int i = 0;
    int j = 0;
    int k = 0;
    while (i < localCount && j < receivedCount){
        if (localArray[i] < receivedArray[j]){
            mergedArray[k++] = localArray[i++];
        } else {
            mergedArray[k++] = receivedArray[j++];
        }
    }

    while (i < localCount){
        mergedArray[k++] = localArray[i++];
    }
    while (j < receivedCount){
        mergedArray[k++] = receivedArray[j++];
    }
}

static void merge(int rank, int masterRank, int step, int numOfProcessors, unsigned int ** localArray, int * localNumOfElements){
    int slaveRank = masterRank + step;
    unsigned int * mergedArray;

    if (slaveRank >= numOfProcessors){
        return;
    }

    if (rank == masterRank){
        unsigned int * receivedArray;
        receivedArray = malloc(localNumOfElements[slaveRank] * sizeof(*receivedArray));
        mergedArray = malloc((localNumOfElements[masterRank] + localNumOfElements[slaveRank])* sizeof(mergedArray));

        MPI_Recv(receivedArray, localNumOfElements[slaveRank], MPI_UNSIGNED, slaveRank, MERGE_MESSAGE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        mergeHelper(*localArray, localNumOfElements[masterRank], receivedArray, localNumOfElements[slaveRank], mergedArray);

        free(*localArray);
        free(receivedArray);

        *localArray = mergedArray;
    } else if (rank == slaveRank){
        MPI_Send(*localArray, localNumOfElements[rank], MPI_UNSIGNED, masterRank, MERGE_MESSAGE_TAG, MPI_COMM_WORLD);
        free(*localArray);
    }
}

int main(int argc, char **argv) {
    int numberOfElements, rank, numberOfProcessors;
    double s, e;
    unsigned int *inputArray = NULL;
    unsigned int *localArray;
    int *localNumOfElements, *displs;

    assert(argc > 2);
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcessors);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //load data
    if (rank == MAIN_PROCESS) {
        load(argv[1], &numberOfElements, &inputArray);
    }
    MPI_Bcast(&numberOfElements, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    //Send & receive
    myScatterv(numberOfElements, numberOfProcessors, &localNumOfElements, &displs, rank, &localArray, inputArray);

    if (rank == MAIN_PROCESS) {
        free(inputArray);
    }

    //sort
    s = MPI_Wtime();
    sort_local(localNumOfElements[rank], localArray);
    
    //merge. merge odd rank's data into the even one.
    for (int step = 1; step < numberOfProcessors; step <<= 1){
        for (int masterRank = 0; masterRank < numberOfProcessors - step; masterRank += (step << 1)){
            merge(rank, masterRank, step, numberOfProcessors, &localArray, localNumOfElements);
            localNumOfElements[masterRank] += localNumOfElements[masterRank + step];
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    e = MPI_Wtime();

    if (rank == MAIN_PROCESS){
        print_numbers(argv[2], numberOfElements, localArray);
        free(localArray);
        print_time(e - s);
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}