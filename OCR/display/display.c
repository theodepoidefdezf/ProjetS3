#include "display.h"
#include "pipeline_run.h"
#include <cairo.h>
#include <math.h>
#include <string.h>

#define FABS(x) ((x) < 0 ? -(x) : (x))
#define CEIL(x) ((int)((x) == (int)(x) ? (x) : ((int)(x) + 1)))

AppWidgets* app_widgets_new(void){
    AppWidgets *aw = g_new0(AppWidgets, 1);
    aw->orig_pixbuf = NULL;
    aw->display_pixbuf = NULL;
    aw->rotation_angle = 0.0;
    aw->pipeline_launched = FALSE;
    aw->current_image_path[0] = '\0';
    return aw;
}

void app_widgets_free(AppWidgets* aw){
    if(!aw) return;
    if(aw->orig_pixbuf) g_object_unref(aw->orig_pixbuf);
    if(aw->display_pixbuf) g_object_unref(aw->display_pixbuf);
    g_free(aw);
}

static GdkPixbuf* rotate_pixbuf(GdkPixbuf *src, double degrees){
    if(!src) return NULL;
    int w = gdk_pixbuf_get_width(src);
    int h = gdk_pixbuf_get_height(src);
    double rad = degrees * G_PI / 180.0;
    double ca = fabs(cos(rad));
    double sa = fabs(sin(rad));
    int nw = (int)ceil(w * ca + h * sa);
    int nh = (int)ceil(w * sa + h * ca);
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nw, nh);
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgba(cr, 0,0,0,0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_translate(cr, nw/2.0, nh/2.0);
    cairo_rotate(cr, rad);
    cairo_translate(cr, -w/2.0, -h/2.0);
    gdk_cairo_set_source_pixbuf(cr, src, 0, 0);
    cairo_paint(cr);
    GdkPixbuf *res = gdk_pixbuf_get_from_surface(surface, 0, 0, nw, nh);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return res;
}

static void update_display_pixbuf(AppWidgets *aw){
    if(aw->display_pixbuf){ g_object_unref(aw->display_pixbuf); aw->display_pixbuf = NULL; }
    if(!aw->orig_pixbuf) return;
    GdkPixbuf *rot = rotate_pixbuf(aw->orig_pixbuf, aw->rotation_angle);
    if(!rot) return;
    GtkAllocation alloc;
    gtk_widget_get_allocation(aw->image_area, &alloc);
    int area_w = alloc.width > 10 ? alloc.width : 600;
    int area_h = alloc.height > 10 ? alloc.height : 600;
    int w = gdk_pixbuf_get_width(rot);
    int h = gdk_pixbuf_get_height(rot);
    double ratio = fmin((double)area_w / w, (double)area_h / h);
    if(ratio > 1.0) ratio = 1.0;
    int new_w = (int)(w * ratio);
    int new_h = (int)(h * ratio);
    aw->display_pixbuf = gdk_pixbuf_scale_simple(rot, new_w, new_h, GDK_INTERP_BILINEAR);
    g_object_unref(rot);
    gtk_widget_queue_draw(aw->image_area);
}

static gboolean on_draw_image(GtkWidget *widget, cairo_t *cr, gpointer user_data){
    AppWidgets *aw = (AppWidgets*)user_data;
    if(aw->display_pixbuf){
        int pw = gdk_pixbuf_get_width(aw->display_pixbuf);
        int ph = gdk_pixbuf_get_height(aw->display_pixbuf);
        GtkAllocation alloc;
        gtk_widget_get_allocation(widget, &alloc);
        int x = (alloc.width - pw)/2;
        int y = (alloc.height - ph)/2;
        gdk_cairo_set_source_pixbuf(cr, aw->display_pixbuf, x, y);
        cairo_paint(cr);
    } else {
        GtkAllocation alloc;
        gtk_widget_get_allocation(widget, &alloc);
        cairo_set_source_rgb(cr, 0.98,0.98,0.98);
        cairo_paint(cr);
        cairo_set_source_rgb(cr, 0.45,0.45,0.45);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 14.0);
        cairo_move_to(cr, 10, 20);
        cairo_show_text(cr, "Aucune image chargee");
    }
    return FALSE;
}

static gboolean load_image_from_file(AppWidgets *aw, const char *path){
    GError *err = NULL;
    GdkPixbuf *pix = gdk_pixbuf_new_from_file(path, &err);
    if(!pix){
        g_warning("Erreur chargement image: %s", err ? err->message : "?");
        if(err) g_error_free(err);
        return FALSE;
    }
    if(aw->orig_pixbuf) g_object_unref(aw->orig_pixbuf);
    aw->orig_pixbuf = pix;
    aw->rotation_angle = 0.0;
    strncpy(aw->current_image_path, path, sizeof(aw->current_image_path) - 1);
    aw->current_image_path[sizeof(aw->current_image_path) - 1] = '\0';
    update_display_pixbuf(aw);
    char info[256];
    snprintf(info, sizeof(info), "Size: %dx%d px\nRotation: %.1f deg\nFormat: PNG/JPEG",
             gdk_pixbuf_get_width(pix), gdk_pixbuf_get_height(pix), aw->rotation_angle);
    gtk_label_set_text(GTK_LABEL(aw->info_label), info);
    return TRUE;
}

static void update_status(AppWidgets *aw, const char *message){
    if(aw->status_label){
        gtk_label_set_text(GTK_LABEL(aw->status_label), message);
    }
    while(gtk_events_pending()) gtk_main_iteration();
}

static void show_info_dialog(GtkWindow *parent, const char *message){
    GtkWidget *dlg = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

static void open_folder(AppWidgets *aw, const char *folder){
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Ouvrir un fichier", GTK_WINDOW(aw->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Annuler", GTK_RESPONSE_CANCEL,
                                                    "_Ouvrir", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), folder);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void on_level_button_clicked(GtkButton *btn, gpointer user_data){
    AppWidgets *aw = (AppWidgets*)user_data;
    const char *level = gtk_button_get_label(btn);
    char folder[512];
    if(g_strcmp0(level, "Niveau 1") == 0) snprintf(folder, sizeof(folder), "../Images/Niveau1");
    else if(g_strcmp0(level, "Niveau 2") == 0) snprintf(folder, sizeof(folder), "../Images/Niveau2");
    else snprintf(folder, sizeof(folder), "../Images/Niveau3");
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choisir une image", GTK_WINDOW(aw->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Annuler", GTK_RESPONSE_CANCEL,
                                                    "_Ouvrir", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), folder);
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        load_image_from_file(aw, filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_load_button_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choisir une image", GTK_WINDOW(aw->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Annuler", GTK_RESPONSE_CANCEL,
                                                    "_Ouvrir", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        load_image_from_file(aw, filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_save_button_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    if(!aw->display_pixbuf){
        show_info_dialog(GTK_WINDOW(aw->window), "Aucune image affichee a sauvegarder.");
        return;
    }
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Sauvegarder l'image", GTK_WINDOW(aw->window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Annuler", GTK_RESPONSE_CANCEL,
                                                    "_Sauvegarder", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "image.png");
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gdk_pixbuf_save(aw->display_pixbuf, filename, "png", NULL, NULL);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_clean_button_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    update_status(aw, "Nettoyage du dossier output...");
    int ret = clean_output();
    if(ret == 0){
        update_status(aw, "Nettoyage termine !");
        show_info_dialog(GTK_WINDOW(aw->window), "Le dossier output a ete nettoye avec succes.");
    } else {
        update_status(aw, "Erreur lors du nettoyage.");
        show_info_dialog(GTK_WINDOW(aw->window), "Erreur lors du nettoyage du dossier output.");
    }
}

static void on_run_button_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    if(strlen(aw->current_image_path) == 0){
        show_info_dialog(GTK_WINDOW(aw->window), "Veuillez d'abord charger une image.");
        return;
    }
    gtk_widget_set_sensitive(aw->run_btn, FALSE);
    gtk_widget_set_sensitive(aw->clean_btn, FALSE);
    update_status(aw, "Lancement du pipeline OCR complet...");
    int result = run_pipeline_full(aw->current_image_path, aw->rotation_angle);
    gtk_widget_set_sensitive(aw->run_btn, TRUE);
    gtk_widget_set_sensitive(aw->clean_btn, TRUE);
    if(result == 0){
        update_status(aw, "Pipeline termine avec succes !");
        show_info_dialog(GTK_WINDOW(aw->window), 
            "Pipeline OCR termine avec succes !\n\n"
            "Resultats disponibles dans:\n"
            "- ../output/ (images pretraitees)\n"
            "- ../output/auto_run/ (decoupage)\n"
            "- ../Solver/coordonnees (solution)");
    } else {
        update_status(aw, "Erreur dans le pipeline.");
        show_info_dialog(GTK_WINDOW(aw->window), 
            "Une erreur s'est produite dans le pipeline.\n"
            "Verifiez la console pour plus de details.");
    }
}

static void on_rotation_changed(GtkRange *range, gpointer user_data){
    AppWidgets *aw = (AppWidgets*)user_data;
    aw->rotation_angle = gtk_range_get_value(range);
    update_display_pixbuf(aw);
    if(aw->orig_pixbuf){
        char info[256];
        snprintf(info, sizeof(info), "Size: %dx%d px\nRotation: %.1f deg\nFormat: PNG/JPEG",
                 gdk_pixbuf_get_width(aw->orig_pixbuf), gdk_pixbuf_get_height(aw->orig_pixbuf), aw->rotation_angle);
        gtk_label_set_text(GTK_LABEL(aw->info_label), info);
    }
}

static void on_reset_rotation_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    gtk_range_set_value(GTK_RANGE(aw->rotation_scale), 0.0);
    aw->rotation_angle = 0.0;
    update_display_pixbuf(aw);
}

static void on_preproc_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    open_folder(aw, "../output");
}

static void on_decoupage_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    open_folder(aw, "../output/auto_run");
}

static void on_detection_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    open_folder(aw, "../ocr");
}

static void on_solver_clicked(GtkButton *btn, gpointer user_data){
    (void)btn;
    AppWidgets *aw = (AppWidgets*)user_data;
    open_folder(aw, "../Solver");
}

int init_gui(int argc, char **argv, AppWidgets **out_widgets){
    gtk_init(&argc, &argv);
    AppWidgets *aw = app_widgets_new();
    aw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(aw->window), "OCR Word Search Solver - EPITA S3");
    gtk_window_set_default_size(GTK_WINDOW(aw->window), 1200, 800);

    GtkCssProvider *css = gtk_css_provider_new();
    const gchar *css_data =
        "#header { background: linear-gradient(#2d5780,#6ea1d6); color: white; padding: 10px; }"
        "#sidebar { background: #f3f1ee; border: 1px solid #ccc; padding: 12px; }"
        "#runbtn { background: #5a7db0; color: white; padding: 10px 20px; border-radius: 6px; }"
        "#cleanbtn { background: #b05a5a; color: white; padding: 8px 16px; border-radius: 6px; }";
    gtk_css_provider_load_from_data(css, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(aw->window), vbox);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_name(header, "header");
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='x-large'><b>OCR Word Search Solver</b></span>");
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_size_request(sidebar, 300, -1);
    gtk_widget_set_name(sidebar, "sidebar");
    gtk_box_pack_start(GTK_BOX(hbox), sidebar, FALSE, FALSE, 0);

    GtkWidget *pipeline_frame = gtk_frame_new("Pipeline du Projet");
    GtkWidget *pipeline_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(pipeline_frame), pipeline_box);
    const char *steps = "1. Choix du niveau\n2. Pretraitement\n3. Decoupage\n4. Reseau de neurones\n5. Recherche (Solver)\n6. Affichage";
    GtkWidget *steps_label = gtk_label_new(steps);
    gtk_label_set_xalign(GTK_LABEL(steps_label), 0.0);
    gtk_box_pack_start(GTK_BOX(pipeline_box), steps_label, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(sidebar), pipeline_frame, FALSE, FALSE, 0);

    aw->level1_btn = gtk_button_new_with_label("Niveau 1");
    aw->level2_btn = gtk_button_new_with_label("Niveau 2");
    aw->level3_btn = gtk_button_new_with_label("Niveau 3");
    gtk_box_pack_start(GTK_BOX(sidebar), aw->level1_btn, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(sidebar), aw->level2_btn, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(sidebar), aw->level3_btn, FALSE, FALSE, 2);

    aw->load_btn = gtk_button_new_with_label("Charger une image");
    gtk_box_pack_start(GTK_BOX(sidebar), aw->load_btn, FALSE, FALSE, 6);

    aw->save_btn = gtk_button_new_with_label("Sauvegarder l'image");
    gtk_box_pack_start(GTK_BOX(sidebar), aw->save_btn, FALSE, FALSE, 8);

    GtkWidget *rot_label = gtk_label_new("Angle de rotation (degres)");
    gtk_box_pack_start(GTK_BOX(sidebar), rot_label, FALSE, FALSE, 2);
    aw->rotation_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -180, 180, 1);
    gtk_scale_set_draw_value(GTK_SCALE(aw->rotation_scale), TRUE);
    gtk_range_set_value(GTK_RANGE(aw->rotation_scale), 0);
    gtk_box_pack_start(GTK_BOX(sidebar), aw->rotation_scale, FALSE, FALSE, 2);

    GtkWidget *reset_rot = gtk_button_new_with_label("Reinitialiser la Rotation");
    gtk_box_pack_start(GTK_BOX(sidebar), reset_rot, FALSE, FALSE, 2);

    aw->info_label = gtk_label_new("Size: -\nRotation: 0 deg\nFormat: -");
    gtk_label_set_xalign(GTK_LABEL(aw->info_label), 0.0);
    gtk_box_pack_end(GTK_BOX(sidebar), aw->info_label, FALSE, FALSE, 4);

    aw->image_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(aw->image_area, 700, 600);
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), aw->image_area);
    gtk_box_pack_start(GTK_BOX(hbox), scrolled, TRUE, TRUE, 0);

    GtkWidget *right_col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_size_request(right_col, 200, -1);
    gtk_box_pack_start(GTK_BOX(hbox), right_col, FALSE, FALSE, 0);

    aw->run_btn = gtk_button_new_with_label("Run");
    gtk_widget_set_name(aw->run_btn, "runbtn");
    gtk_widget_set_size_request(aw->run_btn, 140, 45);
    gtk_box_pack_start(GTK_BOX(right_col), aw->run_btn, FALSE, FALSE, 20);

    aw->clean_btn = gtk_button_new_with_label("Clean Output");
    gtk_widget_set_name(aw->clean_btn, "cleanbtn");
    gtk_widget_set_size_request(aw->clean_btn, 140, 40);
    gtk_box_pack_start(GTK_BOX(right_col), aw->clean_btn, FALSE, FALSE, 4);

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(right_col), sep, FALSE, FALSE, 10);

    GtkWidget *folders_label = gtk_label_new("Ouvrir dossiers:");
    gtk_label_set_xalign(GTK_LABEL(folders_label), 0.0);
    gtk_box_pack_start(GTK_BOX(right_col), folders_label, FALSE, FALSE, 2);

    aw->preproc_btn = gtk_button_new_with_label("Pretraitement");
    aw->decoupage_btn = gtk_button_new_with_label("Decoupage");
    aw->detection_btn = gtk_button_new_with_label("Reseau de neurones");
    aw->solver_btn = gtk_button_new_with_label("Solver");
    gtk_box_pack_start(GTK_BOX(right_col), aw->preproc_btn, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(right_col), aw->decoupage_btn, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(right_col), aw->detection_btn, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(right_col), aw->solver_btn, FALSE, FALSE, 4);

    aw->status_label = gtk_label_new("Pret. Chargez une image pour commencer.");
    gtk_label_set_xalign(GTK_LABEL(aw->status_label), 0.0);
    gtk_box_pack_end(GTK_BOX(right_col), aw->status_label, FALSE, FALSE, 10);

    GtkWidget *status = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(vbox), status, FALSE, TRUE, 0);

    g_signal_connect(aw->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(aw->image_area, "draw", G_CALLBACK(on_draw_image), aw);
    g_signal_connect(aw->load_btn, "clicked", G_CALLBACK(on_load_button_clicked), aw);
    g_signal_connect(aw->save_btn, "clicked", G_CALLBACK(on_save_button_clicked), aw);
    g_signal_connect(aw->rotation_scale, "value-changed", G_CALLBACK(on_rotation_changed), aw);
    g_signal_connect(reset_rot, "clicked", G_CALLBACK(on_reset_rotation_clicked), aw);
    g_signal_connect(aw->run_btn, "clicked", G_CALLBACK(on_run_button_clicked), aw);
    g_signal_connect(aw->clean_btn, "clicked", G_CALLBACK(on_clean_button_clicked), aw);
    g_signal_connect(aw->level1_btn, "clicked", G_CALLBACK(on_level_button_clicked), aw);
    g_signal_connect(aw->level2_btn, "clicked", G_CALLBACK(on_level_button_clicked), aw);
    g_signal_connect(aw->level3_btn, "clicked", G_CALLBACK(on_level_button_clicked), aw);
    g_signal_connect(aw->preproc_btn, "clicked", G_CALLBACK(on_preproc_clicked), aw);
    g_signal_connect(aw->decoupage_btn, "clicked", G_CALLBACK(on_decoupage_clicked), aw);
    g_signal_connect(aw->detection_btn, "clicked", G_CALLBACK(on_detection_clicked), aw);
    g_signal_connect(aw->solver_btn, "clicked", G_CALLBACK(on_solver_clicked), aw);

    gtk_widget_show_all(aw->window);
    *out_widgets = aw;
    return 0;
}
