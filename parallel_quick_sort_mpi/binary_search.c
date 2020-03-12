

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>




int binary_search(const int* arr_p ,int arr_size, int pivot);

void printArr(int* arr_p, int arr_size){
    for(int i = 0; i< arr_size; i++){
        printf("element %d is %d\n", i, *arr_p);
        arr_p ++;
    }
}

int main(int argc, char *argv[]){

    int arr_size = atoi(argv[1]);
    int search = atoi(argv[2]);
    int* arr = malloc(sizeof(int)*arr_size);
    for(int i =0;i< arr_size;i++)
        arr[i] = atoi(argv[i+3]);

    int index = binary_search(arr, arr_size, search);
    printArr(arr, arr_size);
    printf("pivot is %d, pivot index is %d\n", search, index);



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

    return r +1;
}