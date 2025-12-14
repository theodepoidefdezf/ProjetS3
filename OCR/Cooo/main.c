#include "cooo.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 4) return 1;

    const char *cells_path = argv[1];
    const char *image_path = argv[2];
    const char *coord_path = argv[3];

    int max_row = find_max_index(cells_path, "line_", "");
    if (max_row < 0) return 1;
    int num_rows = max_row + 1;

    char first_line_path[MAX_PATH];
    concat_path(first_line_path, cells_path, "/line_00");

    int max_col = find_max_index(first_line_path, "cell_", ".pbm");
    if (max_col < 0) return 1;
    int num_cols = max_col + 1;

    PBMImage *img = read_pbm(image_path);
    if (!img) return 1;

    int cell_width_x1000 = (img->width * 1000) / num_cols;
    int cell_height_x1000 = (img->height * 1000) / num_rows;

    Coordinate coords[MAX_COORDS];
    int num_coords = read_coordinates(coord_path, coords, MAX_COORDS);

    for (int i = 0; i < num_coords; i++) {
        int px_start = ((coords[i].col_start * 1000 + 500) * cell_width_x1000) / 1000000;
        int py_start = ((coords[i].row_start * 1000 + 500) * cell_height_x1000) / 1000000;
        int px_end = ((coords[i].col_end * 1000 + 500) * cell_width_x1000) / 1000000;
        int py_end = ((coords[i].row_end * 1000 + 500) * cell_height_x1000) / 1000000;

        draw_cross(img, px_start, py_start, CROSS_SIZE);
        draw_cross(img, px_end, py_end, CROSS_SIZE);
        draw_line(img, px_start, py_start, px_end, py_end);
    }

    write_pbm("output_with_coords.pbm", img);
    return 0;
}
