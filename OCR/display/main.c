#include <gtk/gtk.h>
#include "display.h"

int main(int argc, char **argv){
    AppWidgets *aw;
    init_gui(argc, argv, &aw);
    gtk_main();
    app_widgets_free(aw);
    return 0;
}
