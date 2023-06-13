/*
    Jorge Durán Campos
    Luis Montes de Oca Ruiz
    Diego Quirós Artiñano
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>


static char * filename;
static char * filename_with_ext_data;
static char * filename_with_ext_table;
static char * filename_with_ext_comp;

static char * filename_decomp;
static char * ext_decomp;

static unsigned int path_table[256][2];

struct Node {
    int byte;
    unsigned long frequency;

}


void *get_tree_data() {

}

void decompression(struct Node* root) {
    char * file_decomp = malloc(strlen(filename_decomp)+1+strlen(ext_decomp));
    strcpy(file_decomp, filename_decomp);
    strcat(file_decomp, ext_decomp);

    unsigned long reading_pos = 0;
    unsigned char read_byte = 0;
    int to_left, wrote_counter = 0, temp_bit = 0;

    struct Node* temp = root;

    FILE * in = fopen(filename_with_ext_comp, "rb");
    FILE * out = fopen(file_decomp, "w");

    while(!feof(in) && wrote_counter < root->frequency) {
        fseek(in, reading_pos, SEEK_SET);
        fread(&read_byte, 1, 1, in);
        for (to_left = 0; to_left < 8; to_left++) {
            if (is_leaf(temp)) { 
                fprintf(out,"%c", temp->byte);
                wrote_counter++;
                temp = root;
            }
            temp_bit = (read_byte << to_left)&255;
            temp_bit = temp_bit >> 7;
            if (temp_bit == 0) temp = temp->left;
            else if (temp_bit == 1) temp = temp->right;
            if (is_leaf(temp)) { 
                fprintf(out,"%c", temp->byte);
                wrote_counter++;
                temp = root;
            }
        }
        reading_pos++;
    }
    fclose(out);
    fclose(in);
}

int main(int argc, char *argv[]) {
    if (argc > 1) filename = (argv[1]);
    else {
        printf("Falta el nombre de los archivos externos y comprimido");
        return -1;
    }

    if (argc > 2) filename_decomp = (argv[2]);
    else {
        printf("Falta el nombre del archivo de decompresión");
        return -1;
    }

    if (argc > 3) ext_decomp = (argv[3]);
    else {
        printf("Falta la extensión del documento final");
        return -1;
    }

    
    
    return 0;
}