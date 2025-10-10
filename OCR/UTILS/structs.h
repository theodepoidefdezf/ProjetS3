#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

typedef struct {
    int rows;
    int cols;
    char **cells;
} Grid;

typedef struct {
    int count;
    char **words;
} WordList;

typedef struct {
    Grid grid;
    WordList found;
} Result;

typedef struct {
    int count;
    Image *letters;
} Letters;

typedef struct {
    int input_size;
    int hidden_size;
    int output_size;
    float *weights_input_hidden;
    float *weights_hidden_output;
} Network;

#endif
