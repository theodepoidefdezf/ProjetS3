#ifndef DISPLAY_H
#define DISPLAY_H
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *image_area;
    GtkWidget *level1_btn;
    GtkWidget *level2_btn;
    GtkWidget *level3_btn;
    GtkWidget *run_btn;
    GtkWidget *clean_btn;
    GtkWidget *load_btn;
    GtkWidget *save_btn;
    GtkWidget *rotation_scale;
    GtkWidget *preproc_btn;
    GtkWidget *decoupage_btn;
    GtkWidget *detection_btn;
    GtkWidget *solver_btn;
    GtkWidget *preproc_check;
    GtkWidget *info_label;
    GtkWidget *status_label;
    GdkPixbuf *orig_pixbuf;
    GdkPixbuf *display_pixbuf;
    double rotation_angle;
    gboolean pipeline_launched;
    char current_image_path[1024];
} AppWidgets;

AppWidgets* app_widgets_new(void);
void app_widgets_free(AppWidgets* aw);
int init_gui(int argc, char **argv, AppWidgets **out_widgets);

#endif
 