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

static unsigned long solution_array[256] = {0};
static int solution_aux[256];

static int readers_num;

static char *filepath;
static long filelen;


static char *filename;

unsigned long get_max() {
    unsigned long max = solution_array[0];
    for (int i = 1; i < 256; i++) {
        if (max < solution_array[i]) max = solution_array[i];
    }
    return max;
}

void count_sort(int pos_dig) {
    unsigned long output[256], output_aux[256], count[10] = { 0 };

    for (int i = 0; i < 256; i++) output_aux[i] = solution_aux[i];

    for (int i = 0; i < 256; i++) count[(solution_array[i] / pos_dig) % 10]++;

    for (int i = 1; i < 10; i++) count[i] += count[i - 1];

    for (int i = 255; i >=0; i--) {
        output[count[(solution_array[i] / pos_dig) % 10] - 1] = solution_array[i];
        output_aux[count[(solution_array[i] / pos_dig) % 10] - 1] = solution_aux[i];
        count[(solution_array[i] / pos_dig) % 10]--;
    }

    for (int i = 0; i < 256; i++) {
        solution_array[i] = output[i];
        solution_aux[i] = output_aux[i];
    }

}

void radix_sort() {
    unsigned long max = get_max();

    for (int pos_dig = 1; max / pos_dig; pos_dig *= 10) count_sort(pos_dig);
}

void *reading_file(void* input_num) {
    int reader_num = *((int*) input_num);
    long start= (filelen / readers_num) * reader_num;
    long reading_pos = start;
    long end;
    if (reader_num != readers_num -1) end = (filelen / readers_num) * (reader_num + 1);
    else end = filelen;
    FILE *fileptr;
    fileptr = fopen(filepath, "rb");
    while(!feof(fileptr) && reading_pos < end) {
        fseek(fileptr, reading_pos, SEEK_SET);
        unsigned char read_byte;
        fread(&read_byte, 1, 1, fileptr);
        pthread_mutex_lock(&solution_mutex[read_byte]);
        solution_array[read_byte]++;
        pthread_mutex_unlock(&solution_mutex[read_byte]);
        reading_pos++;
    }
    fclose(fileptr);
    pthread_exit((void*)0);
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < 256; i++) {
        solution_aux[i] = i;
        pthread_mutex_init(&solution_mutex[i], NULL);
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

    radix_sort();
    // printf("La cantidad de caracteres es: %ld\n", filelen);
    long total = 0;
    for (int i = 0; i < 256; i++) {
        if (solution_array[i] != 0) {
            printf("%d aparece %d veces\n", solution_aux[i], solution_array[i]);
            total += solution_array[i];
        }
    }
    // printf("La cantidad que leyo es: %ld\n", total);
    for (int i = 0; i < 256; i++) {
        pthread_mutex_destroy(&solution_mutex[i]);
    }
    return 0;
}