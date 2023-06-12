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
static char *filename_with_ext_data;
static char *filename_with_ext_table;
static char *filename_with_ext_comp;

// path table for decompression
static int path_table[256][2];

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

    for (int pos_dig = 1; max / pos_dig > 0; pos_dig *= 10) count_sort(pos_dig);
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
    // int path;
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
    if (amount_of_bytes_zero < 254) {
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
            else if (nb_total > 0) {
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
    } else if (amount_of_bytes_zero == 254){
        return create_node(solution_aux[amount_of_bytes_zero + 1], solution_array[amount_of_bytes_zero + 1]);
    } else {
        return NULL;
    }
}


// methods for tree data file creation

int is_leaf(struct Node* root) {
    if (root->left == NULL && root->right == NULL) return 1;
    return 0;
}

void print_tree(struct Node* root, char* tab) {
    FILE * out = fopen(filename_with_ext_data, "a");
    if (is_leaf(root)) {
        fprintf(out, "%s|_>%c:%lu\n",tab, root->byte, root->amount_of_byte);
        fclose(out);
    } else {
        char* new_tab = malloc(strlen(tab)+1+1);
        strcpy(new_tab, tab);
        strcat(new_tab, "\t");
        if (tab != "") {
            fprintf(out, "%s|_>%lu\n",tab, root->amount_of_byte);
        } else {
            fprintf(out, "%s\t%lu\n",tab, root->amount_of_byte);
        }
        fclose(out);
        print_tree(root->left, new_tab);
        print_tree(root->right, new_tab);
        free(new_tab);
    }
}

int get_tree_height(struct Node* root, int height) {
    if (is_leaf(root)) return height;
    int height_left = get_tree_height(root->left, height + 1);
    int height_right = get_tree_height(root->right, height + 1);
    if (height_left < height_right) return height_right;
    return height_left;
}

void get_tree_widths(struct Node* root, int height, int widths[]) {
    if (root != NULL) {
        widths[height]++;
        get_tree_widths(root->left, height + 1, widths);
        get_tree_widths(root->right, height + 1, widths);
    }
}

int get_max1(int arr[], int n) {
    int max = arr[0];
    for (int i = 1; i < n; i++) {
        if (max < arr[i]) max = arr[i];
    }
    return max;
}

void get_tree_frequencies(struct Node* root, int node_total) {
    struct Node* nodes[node_total];

    int counter = 0, last = 0;
    nodes[counter++] = root;
    FILE * out = fopen(filename_with_ext_data, "a");
    while (last < node_total) {
        struct Node* temp = nodes[last];
        if (temp->left != NULL) nodes[counter++] = temp->left;
        if (temp->right != NULL) nodes[counter++] = temp->right;
        if (is_leaf(temp)) fprintf(out, "Frecuencia de %c : %lu\n", temp->byte, temp->amount_of_byte);
        last++;
    }
    fclose(out);
}

int get_node_amount(struct Node* root) {
    if (is_leaf(root)) return 1;
    return get_node_amount(root->left) + get_node_amount(root->right) + 1;
}

void write_tree_data(struct Node* root) {
    filename_with_ext_data = malloc(strlen(filename)+1+4);
    strcpy(filename_with_ext_data, filename);
    strcat(filename_with_ext_data, ".edy");

    int height = get_tree_height(root, 0);

    int * widths = (int*)calloc(sizeof(int), height);

    get_tree_widths(root, 0, widths);

    int max_width = get_max1(widths, height);

    int node_total = get_node_amount(root);

    FILE * out;

    out = fopen(filename_with_ext_data, "w");
    fprintf(out, "Altura: %d\n", height); 
    fprintf(out, "Anchura: %d\n", max_width);
    fprintf(out, "Total de frecuencia: %lu\n", root->amount_of_byte);
    fclose(out);

    get_tree_frequencies(root, node_total);

    out = fopen(filename_with_ext_data, "a");
    fprintf(out, "Total de nodos: %d\n", node_total);
    fclose(out);

    out = fopen(filename_with_ext_data, "a");
    fprintf(out, "Arbol: \n");
    fclose(out);

    print_tree(root, "");
}


// For table creation
void fill_path(struct Node* root, int level, int mask) {
    if (is_leaf(root)) {
        path_table[root->byte][0] = level;
        path_table[root->byte][1] = mask;
    } else {
        fill_path(root->left, level+1, mask << 1);
        fill_path(root->right, level+1, (mask << 1) + 1);
    }
}

// For table file creation
void write_table_file(struct Node* root) {
    filename_with_ext_table = malloc(strlen(filename)+1+6);
    strcpy(filename_with_ext_table, filename);
    strcat(filename_with_ext_table, ".table");

    FILE * out = fopen(filename_with_ext_table, "w");
    
    if (is_leaf(root)) {
        fprintf(out, "Byte: %c | Level:%d | Mask: %d\n", root->byte, path_table[root->byte][0], path_table[root->byte][1]);
    } else {
        for (int i = 0; i < 256; i++) {
            if (path_table[solution_aux[i]][0] != 0) fprintf(out, "Byte: %c | Level:%d | Mask: %d\n", solution_aux[i], path_table[solution_aux[i]][0], path_table[solution_aux[i]][1]);
        }
    }
    fclose(out);
}

// Compression
void compression() {
    filename_with_ext_comp = malloc(strlen(filename)+1+4);
    strcpy(filename_with_ext_comp, filename);
    strcat(filename_with_ext_comp, ".una");

    unsigned long reading_pos = 0;
    unsigned char read_byte, inputing_byte = 0;
    int bits_left = 8;


    FILE *fileptr = fopen(filepath, "rb");
    FILE *out = fopen(filename_with_ext_comp, "w");
    while(!feof(fileptr)) {
        fseek(fileptr, reading_pos, SEEK_SET);
        fread(&read_byte, 1, 1, fileptr);
        
        if (bits_left < path_table[read_byte][0]) {
            inputing_byte += path_table[read_byte][1] >> (path_table[read_byte][0] - bits_left);
            fprintf(out,"%c", inputing_byte);
            inputing_byte = 0;
            inputing_byte += (path_table[read_byte][1] << (8 - (path_table[read_byte][0] - bits_left)))&255;
            bits_left = 8 - (path_table[read_byte][0] - bits_left);
        } else {
            inputing_byte += path_table[read_byte][1] << (bits_left - path_table[read_byte][0]);
            bits_left -= path_table[read_byte][0];
            if (bits_left == 0) {
                fprintf(out, "%c", inputing_byte);
                inputing_byte = 0;
                bits_left = 8;
            }
        }
        reading_pos++;
    }
    fclose(out);
    fclose(fileptr);
}

void decompression(struct Node* root) {
    char * filename_decomp = malloc(strlen(filename)+1+4);
    strcpy(filename_decomp, filename);
    strcat(filename_decomp, ".txt");

    unsigned long reading_pos = 0;
    unsigned char read_byte = 0;
    int to_left, wrote_counter = 0, temp_bit = 0;

    struct Node* temp = root;

    FILE * in = fopen(filename_with_ext_comp, "rb");
    FILE * out = fopen(filename_decomp, "w");

    while(!feof(in) && wrote_counter < root->amount_of_byte) {
        fseek(in, reading_pos, SEEK_SET);
        fread(&read_byte, 1, 1, in);
        // printf("OG:%c\n", read_byte);
        for (to_left = 0; to_left < 8; to_left++) {
            if (is_leaf(temp)) { 
                fprintf(out,"%c", temp->byte);
                wrote_counter++;
                // printf("N:%c\n", temp->byte);
                temp = root;
            }
            temp_bit = (read_byte << to_left)&255;
            temp_bit = temp_bit >> 7;
            if (temp_bit == 0) temp = temp->left;
            else if (temp_bit == 1) temp = temp->right;
            if (is_leaf(temp)) { 
                fprintf(out,"%c", temp->byte);
                wrote_counter++;
                // printf("N:%c\n", temp->byte);
                temp = root;
            }
        }
        reading_pos++;
    }
    fclose(out);
    fclose(in);
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
    for (int i = 0; i < 256; i++) {
        pthread_mutex_destroy(&solution_mutex[i]);
    }

    struct Node* tree_root = create_tree();

    write_tree_data(tree_root);

    fill_path(tree_root, 0, 0);

    write_table_file(tree_root);

    compression();
    
    decompression(tree_root);

    return 0;
}