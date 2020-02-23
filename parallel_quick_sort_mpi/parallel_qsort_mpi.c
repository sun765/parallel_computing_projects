

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#define MASTER		0
#define MAX_VALUE   100
#define PART_SIZE_TAG 1
#define PART_STARTP_TAG 2
#define SMALL_COUNT_TAG 3
#define LARGE_COUNT_TAG 4
#define SMALL_THAN_PIVOT_TAG 5
#define LARGE_THAN_PIVOT_TAG 6

/***** helper functions *****/
// generate an array with random numbers
int* generateRandomArray(int arr_size);

// print all element in the array (for testing)
void printArr(int* arr_p, int arr_size);

// compare function for sorting
int compare(const void* a, const void* b);

// return a deep copy of array
int* copyArr(int* arr, int size);

// get the median of a sorted array
int get_median(const int* arr_p, int arr_size);

// get the median of an unsorted array
int get_unsorted_median(int* arr_p, int arr_size);

// binary search on sorted array, return the index i where the pivot should be inserted
int binary_search(const int* arr_p ,int arr_size, int pivot);

// use two pointer technique to merge two sorted array
void merge(const int* arr1_p, int arr1_size, const int* arr2_p, int arr2_size, int* buffer_p);

// compute the total levels of the pairing process. (eg, 16 processes will return 4)
int compute_level(int local_task_nums);

// compute the offsets for each process
int* compute_offsets(const int* counts, int task_num);

// compute the time spent in mili sec
long  get_time_milisec(const struct timeval* t1, const struct timeval* t2);

// compare two array number by number
void compareArr(int* correct_arr, int* your_arr, int size);

int main(int argc, char *argv[]){

    int arr_size = atoi(argv[1]);
    int local_task_num, global_task_num, local_rank, global_rank;
    int* arr_p =NULL;
    int* arr_copy_p = NULL;
    struct timeval start_time, computing_start_time, end_time;
    long single_qsort_time_milisec;
    MPI_Status status;
    MPI_Comm cur_comm = MPI_COMM_WORLD;

    /***** mpi initialization *****/
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &global_task_num);
    local_task_num = global_task_num;
    MPI_Comm_rank(MPI_COMM_WORLD,&global_rank);
    local_rank = global_rank;
    //printf ("MPI task %d has started...  \n", local_rank);

    /* array stores partition information to scatter*/
    int* send_count = malloc(sizeof(int)*local_task_num);
    int* offset = malloc(sizeof(int)*local_task_num);
    int  data_count = arr_size/local_task_num;
    int* data = malloc(sizeof(int)*arr_size);
    int* buffer = malloc(sizeof(int)*arr_size);                // buffer will be used in merging . buffer and arr will be used in turn.
    int level = compute_level(local_task_num);

    /***** Master Process *****/
    if(global_rank==MASTER){
        printf("I'm master process and I'm initalizing the array, please wait...\n");
        // 1. generate a random array
        arr_p = generateRandomArray(arr_size);
        arr_copy_p = copyArr(arr_p, arr_size);

        printf("Initialization done. Start sorting using single process\n");
        struct timeval single_start_time,  single_end_time;
        gettimeofday(&single_start_time,NULL);
        qsort(arr_copy_p,(size_t)arr_size, sizeof(int),compare);
        gettimeofday(&single_end_time,NULL);
        single_qsort_time_milisec = get_time_milisec(&single_start_time, &single_end_time);
        printf("Sorting the array using one process uses %ld mili seconds\n", single_qsort_time_milisec);

        // 2. get start time
        gettimeofday(&start_time,NULL);
        printf("Start sorting the array of size %d with %d processes\n", arr_size,global_task_num);

        // 3. compute scatter information
        int remain = arr_size % local_task_num;
        int avg_size = arr_size/ local_task_num;
        for(int i =0;i< local_task_num; i++) {
            send_count[i] = remain > 0 ? 1 + avg_size : avg_size;
            offset[i] = i == 0 ? 0 : offset[i - 1] + send_count[i - 1];
            if (remain > 0)
                remain--;
        }
    }

    /***** Worker Process *****/
    // scatter the array into pieces and send to all processes
    MPI_Scatterv(arr_p, send_count, offset, MPI_INT, data, data_count, MPI_INT, MASTER, MPI_COMM_WORLD);

    // used to calc time that doesn't include scatter process
    if(global_rank==MASTER)
        gettimeofday(&computing_start_time,NULL);

    //1. sort part of array.
    qsort(data,(size_t)data_count, sizeof(int),compare);

    //2. keep pairing processes until all the array is sorted
    while(level>0) {

        //a. compute global pivot (compute local pivot first and then allgather to all processes)
        int *pivot_buffer = malloc(sizeof(int) * local_task_num);
        int local_pivot = get_median(data, data_count);
        int global_pivot;
        //printf("process %d's local_pivot is %d\n", local_rank, local_pivot);

        MPI_Allgather(&local_pivot, 1, MPI_INT, pivot_buffer, 1, MPI_INT, cur_comm);
        global_pivot = get_unsorted_median(pivot_buffer, local_task_num);
        //printf("process %d's global_pivot is %d\n", local_rank, global_pivot);

        //b. binary search on local array to find the pivot index
        int global_pivot_index = binary_search(data, data_count, global_pivot);
        //printf("process %d's pivot index is %d\n", local_rank, global_pivot_index);


        //c. send , receive , and merge
        if (local_rank < local_task_num / 2) {
            // this process will send data(larger than global pivot) to partner first and then receive(lower than pivot)
            int partner = local_rank % (local_task_num / 2) + local_task_num / 2;
            int large_count = data_count - global_pivot_index;

            // send count and data of the part larger part to partner
            MPI_Send(&large_count, 1, MPI_INT, partner, LARGE_COUNT_TAG, cur_comm);
            //printf("tastk %d send count %d to partner %d \n", local_rank, large_count, partner);
            MPI_Send(&data[global_pivot_index], large_count, MPI_INT, partner, LARGE_THAN_PIVOT_TAG, cur_comm);

            // receive count and data of the larger part from partner
            int small_count;
            MPI_Recv(&small_count, 1, MPI_INT, partner, SMALL_COUNT_TAG, cur_comm, &status);
            //printf("tastk %d receive count %d from partner %d \n", local_rank, small_count, partner);
            int *receive_buffer = malloc(sizeof(int) * small_count);
            MPI_Recv(receive_buffer, small_count, MPI_INT, partner, SMALL_THAN_PIVOT_TAG, cur_comm, &status);

            // merge, update count ,and scroll buffer
            merge(data, data_count - large_count, receive_buffer, small_count, buffer);
            data_count = data_count - large_count + small_count;
            int *temp = data;
            data = buffer;
            buffer = temp;

            free(receive_buffer);
            /*
            for(int i =0;i< data_count; i++){
                printf("task %d  %d th num: %d           ", local_rank, i, data[i]);
            }
             */
        }
        else {
            // these processes will receive data(larger than global pivot) from partner first and then send (lower than global pivot)
            int partner = local_rank % (local_task_num / 2);
            int large_count;

            // receive count and data of the larger part from partner
            MPI_Recv(&large_count, 1, MPI_INT, partner, LARGE_COUNT_TAG, cur_comm, &status);
            //printf("task %d receive count %d from partner %d \n", local_rank, large_count,partner);
            int *receive_buffer = malloc(sizeof(int) * large_count);
            MPI_Recv(receive_buffer, large_count, MPI_INT, partner, LARGE_THAN_PIVOT_TAG, cur_comm, &status);


            // send count and data of the smaller part to partner
            int small_count = global_pivot_index;
            MPI_Send(&small_count, 1, MPI_INT, partner, SMALL_COUNT_TAG, cur_comm);
            //printf("tastk %d send count %d to partner %d \n", local_rank, small_count, partner);
            MPI_Send(data, small_count, MPI_INT, partner, SMALL_THAN_PIVOT_TAG, cur_comm);

            // merge, update count ,and scroll buffer
            int *data_start = data + global_pivot_index;
            merge(data_start, data_count - small_count, receive_buffer, large_count, buffer);
            data_count = data_count - small_count + large_count;
            int *temp = data;
            data = buffer;
            buffer = temp;

            free(receive_buffer);
        }
        level --;

        //d. split the comunicator, update local task num, local rank
        int color = local_rank / (local_task_num/2);
        MPI_Comm temp;
        MPI_Comm_split( cur_comm, color, local_rank, &temp);
        cur_comm = temp;
        MPI_Comm_rank(cur_comm, &local_rank);
        MPI_Comm_size(cur_comm, &local_task_num);
        //printf("my local id : %d, my world id : %d, and my comm has %d tasks\n", local_rank, global_rank, local_task_num);
        //printf("global id: %d, I have %d num \n", global_rank, data_count);
    }

    //3. gather data counts and data into the master's buffer task

    int* global_counts = malloc(sizeof(int)*global_task_num);
    MPI_Gather(&data_count, 1, MPI_INT, global_counts,1, MPI_INT, MASTER,MPI_COMM_WORLD);
    int* offsets = NULL;
    int* receive_counts = NULL;

    if(global_rank ==MASTER){
        receive_counts = malloc(sizeof(int)*global_task_num);
        offsets = compute_offsets(global_counts, global_task_num);
    }

    MPI_Gatherv(data, data_count, MPI_INT, buffer, global_counts, offsets, MPI_INT, MASTER, MPI_COMM_WORLD);

    //4. report time spent
    if(global_rank ==MASTER){
        gettimeofday(&end_time,NULL);
        long time_milisec = get_time_milisec(&start_time, &end_time);
        long compute_time_milisec = get_time_milisec(&computing_start_time, &end_time);
        /*
        printf("start time sed :%ld, micro sec: %ld\n", start_time.tv_sec, start_time.tv_usec);
        printf("compute start time sed :%ld, micro sec: %ld\n", computing_start_time.tv_sec, computing_start_time.tv_usec);
        printf("end time sed :%ld, micro sec: %ld\n", end_time.tv_sec, end_time.tv_usec);
         */
        printf("used %ld mili seconds to qsort %d numbers using %d processes\n", time_milisec, arr_size,global_task_num);
        printf("the performance of your parallel qsort using %d processes is %f times compared with single process\n",global_task_num,(float)single_qsort_time_milisec/(float)time_milisec);
        printf("used %ld mili seconds to qsort %d numbers using %d processes, if scatter is not included\n", compute_time_milisec, arr_size,global_task_num);
        printf("the performance of your parallel qsort using %d processes is %f times compared with single process, if scatter is not included\n",global_task_num,(float)single_qsort_time_milisec/(float)compute_time_milisec);
        //printf("using %d processes to qsort is %d times faster compared with using only 1 process! (scatter included)\n", )

        compareArr(arr_copy_p, buffer, arr_size);
    }

    /***** end of each process *****/
    //5. free dynamically allocated memory
    free(data);
    free(buffer);


    if(global_rank==MASTER){
        free(offsets);
        free(arr_p);
        free(arr_copy_p);
    }

    // finish process
    MPI_Finalize();
}

int* generateRandomArray(int arr_size){
    int* arr_p = malloc(sizeof(int)*arr_size);

    // set seed with time
    // srand(time(0));

    for(int i =0;i< arr_size;i++){
        *(arr_p+i) = rand() % MAX_VALUE;
    }
    return arr_p;
}

void printArr(int* arr_p, int arr_size){
    for(int i = 0; i< arr_size; i++){
        printf("element %d is %d\n", i, *arr_p);
        arr_p ++;
    }
}

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int get_median(const int* arr_p, int arr_size){
    int median;
    if(arr_size%2==0)
        median = (arr_p[arr_size/2-1] + arr_p[arr_size/2])/2;
    else{
        median = arr_p[arr_size/2];
    }
    return median;
}

int get_unsorted_median(int* arr_p, int arr_size){
    qsort(arr_p,(size_t)arr_size,sizeof(int),compare);
    return get_median(arr_p, arr_size);
}

int binary_search(const int* arr_p ,int arr_size, int pivot){
    if(arr_p[0]>pivot)
        return 0;

    int l = 0, r = arr_size -1;
    int mid;
    while(l<r){
        mid = l + (r-l)/2 +1;
        if(arr_p[mid] > pivot){
            r = mid-1;
        }
        else {
            l = mid ;
        }
    }
    return  r+1;
}

void merge(const int* arr1_p, int arr1_size, const int* arr2_p, int arr2_size, int* buffer_p){
    int i = 0, j = 0, k = 0;
    while(i< arr1_size  && j<arr2_size){
        if(arr1_p[i]< arr2_p[j]){
            buffer_p[k] = arr1_p[i];
            i++;
        }
        else{
            buffer_p[k] = arr2_p[j];
            j++;
        }
        k++;
    }

    while(i< arr1_size ){
        buffer_p[k] = arr1_p[i];
        i++;
        k++;
    }

    while(j< arr2_size){
        buffer_p[k] = arr2_p[j];
        j++;
        k++;
    }
}

int compute_level(int local_task_nums){
    int level = 0;
    while(local_task_nums >1){
        level ++;
        local_task_nums /=2;
    }
    return level;
}

int* compute_offsets(const int* counts, int task_num){
    int sum = 0;
    int* offsets = malloc(sizeof(int)*task_num);
    offsets[0] = 0;
    for(int i = 1;i< task_num;i++){
        sum+= counts[i-1];
        offsets[i] = sum;
    }
    return offsets;
}

long  get_time_milisec(const struct timeval* t_start, const struct timeval* t_end){
    long time_sec = t_end->tv_sec - t_start-> tv_sec;
    long time_milisec = t_end->tv_usec - t_start-> tv_usec;
    return time_sec*1000 + time_milisec/1000;
}

int* copyArr(int* arr, int size)
{
    int* copy = malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++)
        copy[i] = arr[i];
    return copy;
}

void compareArr(int* correct_arr, int* your_arr, int size)
{
    int wrong_count = 0;
    for (int i = 0; i < size; i++) {
        if (correct_arr[i] == your_arr[i]) {
            //printf("element %d, correct num: %d, your num: %d\n", i, correct_arr[i], your_arr[i]);
        }
        else
        {
            wrong_count++;
            //printf("element %d, correct num: %d, but your num is: %d\n", i, correct_arr[i], your_arr[i]);
        }
    }

    if (wrong_count == 0) {
        printf("your result is 100 percent correct!\n");
    }
    else {
        printf("your result has %d wrong nums!", wrong_count);
    }
}