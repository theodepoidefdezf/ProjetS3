#ifndef COOO_H
#define COOO_H

#define MAX_PATH 1024
#define MAX_COORDS 100
#define CROSS_SIZE 15
#define LINE_THICKNESS 5
#define HEAP_SIZE 10000000

typedef struct {
    int col_start, row_start;
    int col_end, row_end;
} Coordinate;

typedef struct {
    int width;
    int height;
    unsigned char *data;
} PBMImage;

extern char g_heap[HEAP_SIZE];
extern unsigned long g_heap_used;

int my_abs(int x);
void *my_malloc(unsigned long size);
void *my_calloc(unsigned long count, unsigned long size);
int my_strcmp(const char *s1, const char *s2);
void build_path(char *dest, const char *base, const char *prefix, int num, const char *suffix);
int find_max_index(const char *base_path, const char *prefix, const char *suffix);
PBMImage* read_pbm(const char *filename);
int write_pbm(const char *filename, PBMImage *img);
void draw_cross(PBMImage *img, int px, int py, int size);
void draw_thick_pixel(PBMImage *img, int px, int py);
void draw_line(PBMImage *img, int x0, int y0, int x1, int y1);
int read_coordinates(const char *filename, Coordinate *coords, int max_coords);
void concat_path(char *dest, const char *s1, const char *s2);

#endif
