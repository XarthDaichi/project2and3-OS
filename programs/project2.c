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
#include <string.h>

// mutex for each element in the solution_array for entering
static pthread_mutex_t solution_mutex[256];

// solution and aux for array sorting to maintain right byte
static unsigned long solution_array[256] = {0};
static int solution_aux[256];

// amount of readers that are going to be used
static int readers_num;

// original file info
static char *filepath;
static long filelen;

// new files info
static char *filename;

// methods for sorting array (radixsort)
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


// method for reading file
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


// things for tree creation
struct Node {
    int byte;
    unsigned long amount_of_byte;
    int path;
    struct Node* left;
    struct Node* right;
};

struct Node* create_node(int inserting_byte, unsigned long amount) {
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
    new_node->byte = inserting_byte;
    new_node->amount_of_byte = amount;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

struct Node* create_node_not_byte(int amount) {
    return create_node(-1, amount);
}

int amount_of_zeros() {
    int counter = -1;
    for (int i = 0; i < 256; i++) {
        if (solution_array[i] != 0) break;
        counter++;
    }
    return counter;
}

struct Node* create_tree() {
    int amount_of_bytes_zero = amount_of_zeros();

    struct Node* non_byte_nodes[255] = { NULL };
    int nb_total = 0, last_nb_add = 0;

    non_byte_nodes[nb_total] = create_node_not_byte(0);

    non_byte_nodes[nb_total]->left = create_node(solution_aux[amount_of_bytes_zero + 1], solution_array[amount_of_bytes_zero + 1]);
    non_byte_nodes[nb_total]->right = create_node(solution_aux[amount_of_bytes_zero + 2], solution_array[amount_of_bytes_zero + 2]);
    non_byte_nodes[nb_total]->amount_of_byte = solution_array[amount_of_bytes_zero + 1] + solution_array[amount_of_bytes_zero + 2];

    int all_loaded = 0;
    int i = amount_of_bytes_zero + 3;

    while (i < 256 || !all_loaded) {
        if (i < 255) {
            if (non_byte_nodes[last_nb_add]->amount_of_byte < solution_array[i]) {
                if (last_nb_add > 0 && non_byte_nodes[last_nb_add + 1]->amount_of_byte < solution_array[i]) {
                    non_byte_nodes[++nb_total] = create_node_not_byte(non_byte_nodes[last_nb_add + 1]->amount_of_byte + non_byte_nodes[last_nb_add]->amount_of_byte);
                    non_byte_nodes[nb_total]->left = non_byte_nodes[last_nb_add];
                    non_byte_nodes[nb_total]->right = non_byte_nodes[last_nb_add + 1];
                    last_nb_add += 2;
                } else {
                    non_byte_nodes[++nb_total] = create_node_not_byte(non_byte_nodes[last_nb_add]->amount_of_byte + solution_array[i]);
                    non_byte_nodes[nb_total]->left = non_byte_nodes[last_nb_add];
                    non_byte_nodes[nb_total]->right = create_node(solution_aux[i], solution_array[i]);
                    last_nb_add++;
                    i++;
                }
            } else {
                if (non_byte_nodes[last_nb_add]->amount_of_byte < solution_array[i + 1]) {
                    non_byte_nodes[++nb_total] = create_node_not_byte(non_byte_nodes[last_nb_add]->amount_of_byte + solution_array[i]);
                    non_byte_nodes[nb_total]->left = create_node(solution_aux[i], solution_array[i]);
                    non_byte_nodes[nb_total]->right = non_byte_nodes[last_nb_add];
                    last_nb_add++;
                    i++;
                } else {
                    non_byte_nodes[++nb_total] = create_node_not_byte(solution_array[i] + solution_array[i + 1]);
                    non_byte_nodes[nb_total]->left = create_node(solution_aux[i], solution_array[i]);
                    non_byte_nodes[nb_total]->right = create_node(solution_aux[i + 1], solution_array[i + 1]);
                    i += 2;
                }
            }
        }
        else if (i == 255) {
            if (non_byte_nodes[last_nb_add]->amount_of_byte < solution_array[i]) {
                non_byte_nodes[++nb_total] = create_node_not_byte(non_byte_nodes[last_nb_add]->amount_of_byte + solution_array[i]);
                non_byte_nodes[nb_total]->left = non_byte_nodes[last_nb_add];
                non_byte_nodes[nb_total]->right = create_node(solution_aux[i], solution_array[i]);
                last_nb_add++;
                i++;
            } else {
                non_byte_nodes[++nb_total] = create_node_not_byte(non_byte_nodes[last_nb_add]->amount_of_byte + solution_array[i]);
                non_byte_nodes[nb_total]->left = create_node(solution_aux[i], solution_array[i]);
                non_byte_nodes[nb_total]->right = non_byte_nodes[last_nb_add];
                last_nb_add++;
                i++;
            }
        }
        else {
            non_byte_nodes[++nb_total] = create_node_not_byte(non_byte_nodes[last_nb_add + 1]->amount_of_byte + non_byte_nodes[last_nb_add]->amount_of_byte);
            non_byte_nodes[nb_total]->left = non_byte_nodes[last_nb_add];
            non_byte_nodes[nb_total]->right = non_byte_nodes[last_nb_add + 1];
            last_nb_add += 2;
        }
        if (last_nb_add == nb_total) {
            break;
        }
    }

    return non_byte_nodes[nb_total];
}

void print_tree(struct Node* root, char* tab) {
    if (root->byte != -1) {
        printf("%s|_>%c:%lu\n",tab, root->byte, root->amount_of_byte);
    } else {
        char* new_tab = malloc(strlen(tab)+1+1);
        strcpy(new_tab, tab);
        strcat(new_tab, "\t");
        if (tab != "") printf("%s|_>%lu\n",tab, root->amount_of_byte);
        else printf("%s\t%lu\n",tab, root->amount_of_byte);
        print_tree(root->left, new_tab);
        print_tree(root->right, new_tab);
        free(new_tab);
    }
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
    printf("La cantidad de caracteres es: %lu\n", filelen);
    long total = 0;
    for (int i = 0; i < 256; i++) {
        if (solution_array[i] != 0) {
            printf("%c aparece %d veces\n", solution_aux[i], solution_array[i]);
            total += solution_array[i];
        }
    }
    printf("La cantidad que leyo es: %lu\n", total);
    for (int i = 0; i < 256; i++) {
        pthread_mutex_destroy(&solution_mutex[i]);
    }

    if (filelen != 0) {
        struct Node* tree_root = create_tree();
        print_tree(tree_root, "");
    }



    return 0;
}