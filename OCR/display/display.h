// display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *image_area;         // drawing area for image
    GtkWidget *level1_btn;
    GtkWidget *level2_btn;
    GtkWidget *level3_btn;
    GtkWidget *run_btn;
    GtkWidget *load_btn;
    GtkWidget *save_btn;
    GtkWidget *rotation_scale;

    GtkWidget *preproc_btn;        // bouton Prétraitement
    GtkWidget *decoupage_btn;      // bouton Découpage
    GtkWidget *detection_btn;      // bouton Détection
    GtkWidget *solver_btn;         // bouton Solver

    GtkWidget *preproc_check;      // reste éventuellement pour autre usage
    GtkWidget *info_label;

    GdkPixbuf *orig_pixbuf;        // original loaded image
    GdkPixbuf *display_pixbuf;     // transformed pixbuf shown
    double rotation_angle;         // degrees
    gboolean pipeline_launched;
} AppWidgets;

AppWidgets* app_widgets_new(void);
void app_widgets_free(AppWidgets* aw);
int init_gui(int argc, char **argv, AppWidgets **out_widgets);

#endif // DISPLAY_H
