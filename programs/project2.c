/*
    Jorge Durán Campos
    Luis Montes de Oca Ruiz
    Diego Quirós Artiñano
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

static pthread_mutex_t solution_mutex[256];

static pthread_cond_t available_condition[256];

static int solution_array[256] = {0};
static int solution_aux[256];

static int readers_num;

static char *filepath;
static long filelen;
static char *filename;

// void merge(int arr1[], int arr2[], int left1[], int left2[], int left_size, int right1[], int right2[], int right_size) {
// 	int i = 0, j = 0, k = 0;
// 	while (i < left_size && j < right_size) {
// 		if (left1[i] >= right1[j]) {
// 			arr1[k] = left1[i];
// 			arr2[k] = left2[i];
// 			i++;
// 		}
// 		else {
// 			arr1[k] = right1[j];
// 			arr2[k] = right2[j];
// 			j++;
// 		}
// 		k++;
// 	}
	
// 	while (i < left_size) {
// 		arr1[k] = left1[i];
// 		arr2[k] = left2[i];
// 		i++;
// 		k++;
// 	}
	
// 	while (j < right_size) {
// 		arr1[k] = right1[j];
// 		arr2[k] = right2[j];
// 		j++;
// 		k++;
// 	}
// }

// void mergesort(int arr1[], int arr2[], int size) {
// 	if (size < 2)
// 		return;
	
// 	int mid = size / 2;
// 	int *left1 = (int*) malloc(mid * sizeof(int));
// 	int *left2 = (int*) malloc(mid * sizeof(int));
// 	int *right1 = (int*) malloc((size - mid) * sizeof(int));
// 	int *right2 = (int*) malloc((size - mid) * sizeof(int));
	
// 	for (int i = 0; i < mid; i++) {
// 		left1[i] = arr1[i];
// 		left2[i] = arr2[i];
// 	}
	
// 	for (int i = mid; i < size; i++) {
// 		right1[i - mid] = arr1[i];
// 		right2[i - mid] = arr2[i];
// 	}
	
// 	mergesort(left1, left2, mid);
// 	mergesort(right1, right2, size - mid);
// 	merge(arr1, arr2, left1, left2, mid, right1, right2, size - mid);
	
// 	free(left1);
// 	free(left2);
// 	free(right1);
// 	free(right2);
// }

void *reading_file(void* input_num) {
    int reader_num = *((int*) input_num);
    long start= (filelen / readers_num) * reader_num;
    long reading_pos = start;
    long end;
    if (reader_num != readers_num -1) end = (filelen / readers_num) * (reader_num + 1);
    else end = filelen;
    // printf("Hilo(%d), leyendo desde %ld - %ld\n", reader_num, start, end);
    FILE *fileptr;
    fileptr = fopen(filepath, "rb");
    while(!feof(fileptr) && reading_pos < end) {
        fseek(fileptr, reading_pos, SEEK_SET);
        unsigned char read_byte;
        fread(&read_byte, 1, 1, fileptr);
        // pthread_cond_wait(&available_condition[read_byte], &solution_mutex[read_byte]);
        pthread_mutex_lock(&solution_mutex[read_byte]);
        solution_array[read_byte]++;
        pthread_mutex_unlock(&solution_mutex[read_byte]);
        // pthread_cond_broadcast(&available_condition[read_byte]);
        reading_pos++;
    }
    fclose(fileptr);
    pthread_exit((void*)0);
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < 256; i++) {
        solution_aux[i] = i;
        pthread_mutex_init(&solution_mutex[i], NULL);
        pthread_cond_init(&available_condition[i], NULL);
    }
    
    if (argc > 1) readers_num = atoi(argv[1]);
    else {
        printf("Falta el numero de lectores");
        return -1;
    }
    int index[readers_num];

    for (int i = 0; i < readers_num; i++) index[i] = i;

    if (argc > 2) filepath = (argv[2]);
    else {
        printf("Falta la dirección del archivo");
        return -1;
    }

    if (argc > 3) filename = (argv[3]);
    else {
        printf("Falta el nombre del archivo final");
        return -1;
    }

    pthread_t readers[readers_num];

    FILE *fileptr;
    fileptr = fopen(filepath, "rb");
    if (fileptr == NULL) {
        printf("Fallo la apertura del documento");
        return -1;
    }
    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);
    fclose(fileptr);
    for (int i = 0; i < readers_num; i++) {
        if (pthread_create(&readers[i], NULL, reading_file, &index[i]) != 0) {
            printf("No se pudo crear el hilo de productores");
            return -1;
        }
    }

    for (int i = 0; i < readers_num; i++) {
        if (pthread_join(readers[i], NULL)) {
            printf("No se pudo unir el hilo de productores");
            return -1;
        }
    }

    // mergesort(solution_array, solution_aux, 256);
    printf("La cantidad de caracteres es: %ld\n", filelen);
    long total = 0;
    for (int i = 0; i < 256; i++) {
        if (solution_array[i] != 0) {
            printf("%d aparece %d veces\n", solution_aux[i], solution_array[i]);
            total += solution_array[i];
        }
    }
    printf("La cantidad que leyo es: %ld\n", total);
    for (int i = 0; i < 256; i++) {
        pthread_mutex_destroy(&solution_mutex[i]);
        pthread_cond_destroy(&available_condition[i]);
    }
    return 0;
}