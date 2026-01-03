#include <gtk/gtk.h>
#include <stdlib.h>
#include "../include/editor.h"

EditorApp *app;

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Load CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    app = g_malloc(sizeof(EditorApp));

    editor_window_init(app);

    gtk_widget_show_all(app->window);

    gtk_main();

    g_free(app);
    return 0;
}
