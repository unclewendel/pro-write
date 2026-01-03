#ifndef EDITOR_H
#define EDITOR_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *statusbar;
    guint statusbar_context_id;
} EditorApp;

// Global app pointer (simplifies callbacks for this example)
extern EditorApp *app;

// Function prototypes
void editor_window_init(EditorApp *app);
void editor_new_file(GtkWidget *widget, gpointer data);
void editor_open_file(GtkWidget *widget, gpointer data);
void editor_save_file(GtkWidget *widget, gpointer data);
void editor_save_as_file(GtkWidget *widget, gpointer data);
void editor_quit(GtkWidget *widget, gpointer data);

void editor_update_statusbar(GtkTextBuffer *buffer, GtkStatusbar *statusbar);
void editor_text_changed_callback(GtkTextBuffer *buffer, gpointer data);
void editor_cursor_moved_callback(GtkTextBuffer *buffer, GtkTextIter *location, GtkTextMark *mark, gpointer data);

GtkWidget *create_tab_label(const gchar *title, GtkWidget *notebook, GtkWidget *child);

#endif
