#include <gtk/gtk.h>
#include "../include/editor.h"

// Helper to get the current text view
static GtkTextView *get_current_text_view(EditorApp *app) {
    GtkWidget *scrolled_window = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook)));
    if (!scrolled_window) return NULL;
    return GTK_TEXT_VIEW(gtk_bin_get_child(GTK_BIN(scrolled_window)));
}

void editor_update_statusbar(GtkTextBuffer *buffer, GtkStatusbar *statusbar) {
    GtkTextIter iter;
    gint row, col;
    gchar *msg;

    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
    row = gtk_text_iter_get_line(&iter);
    col = gtk_text_iter_get_line_offset(&iter);

    msg = g_strdup_printf("Ln %d, Col %d", row + 1, col + 1);
    gtk_statusbar_push(statusbar, gtk_statusbar_get_context_id(statusbar, "Editor Context"), msg);
    g_free(msg);
}

void editor_cursor_moved_callback(GtkTextBuffer *buffer, GtkTextIter *location, GtkTextMark *mark, gpointer data) {
    editor_update_statusbar(buffer, GTK_STATUSBAR(data));
}

// Actual close implementation needs the child widget
static void on_close_tab_clicked(GtkButton *button, GtkWidget *child) {
    gint page_num = gtk_notebook_page_num(GTK_NOTEBOOK(app->notebook), child);
    if (page_num != -1) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(app->notebook), page_num);
    }
}

GtkWidget *create_tab_label(const gchar *title, GtkWidget *notebook, GtkWidget *child) {
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *image;

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    label = gtk_label_new(title);
    
    // Close button
    button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    // gtk_container_set_border_width(GTK_CONTAINER(button), 0); // Deprecated in GTK3 but works usually, skipping for clean code

    image = gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
    gtk_button_set_image(GTK_BUTTON(button), image);
    
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    
    gtk_widget_show_all(box);

    g_signal_connect(button, "clicked", G_CALLBACK(on_close_tab_clicked), child);

    return box;
}

void editor_new_file(GtkWidget *widget, gpointer data) {
    EditorApp *app = (EditorApp *)data;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkWidget *label;

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    text_view = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    // Connect signals for status bar
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    g_signal_connect(buffer, "mark-set", G_CALLBACK(editor_cursor_moved_callback), app->statusbar);

    label = create_tab_label("Untitled", app->notebook, scrolled_window);
    
    gint index = gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), scrolled_window, label);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), index);
    
    gtk_widget_show_all(app->notebook);
    editor_update_statusbar(buffer, GTK_STATUSBAR(app->statusbar));
}

void editor_open_file(GtkWidget *widget, gpointer data) {
    EditorApp *app = (EditorApp *)data;
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(app->window),
                                         action,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        char *content;
        gsize length;
        GError *error = NULL;

        if (g_file_get_contents(filename, &content, &length, &error)) {
             // Create new tab with content
            GtkWidget *scrolled_window;
            GtkWidget *text_view;
            GtkWidget *label;
            
            scrolled_window = gtk_scrolled_window_new(NULL, NULL);
            text_view = gtk_text_view_new();
            gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
            
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(buffer, content, length);
            g_signal_connect(buffer, "mark-set", G_CALLBACK(editor_cursor_moved_callback), app->statusbar);

            // Set tab label to filename (basename)
            gchar *basename = g_path_get_basename(filename);
            label = create_tab_label(basename, app->notebook, scrolled_window);
            g_free(basename);

            // Store full path in widget data for Save
            g_object_set_data_full(G_OBJECT(text_view), "filename", g_strdup(filename), g_free);

            gint index = gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), scrolled_window, label);
            gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), index);
            
            gtk_widget_show_all(app->notebook);
            g_free(content);
        } else {
            g_printerr("Error opening file: %s\n", error->message);
            g_error_free(error);
        }
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void save_file_internal(EditorApp *app, GtkTextView *text_view, const char *filename) {
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gchar *text;
    GError *error = NULL;

    buffer = gtk_text_view_get_buffer(text_view);
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (!g_file_set_contents(filename, text, -1, &error)) {
        g_printerr("Error saving file: %s\n", error->message);
        g_error_free(error);
    } else {
        // Update tab title
        gchar *basename = g_path_get_basename(filename);
        GtkWidget *scrolled_window = gtk_widget_get_parent(GTK_WIDGET(text_view));
        GtkWidget *label_box = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app->notebook), scrolled_window);
        
        // The label box has [Label, Button]. We need the Label.
        GList *children = gtk_container_get_children(GTK_CONTAINER(label_box));
        if (children) {
            GtkWidget *lbl = GTK_WIDGET(children->data); // First child is label
            if (GTK_IS_LABEL(lbl)) {
                gtk_label_set_text(GTK_LABEL(lbl), basename);
            }
            g_list_free(children);
        }
        g_free(basename);
    }
    g_free(text);
}

void editor_save_file(GtkWidget *widget, gpointer data) {
    EditorApp *app = (EditorApp *)data;
    GtkTextView *text_view = get_current_text_view(app);
    if (!text_view) return;

    char *filename = g_object_get_data(G_OBJECT(text_view), "filename");

    if (filename) {
        save_file_internal(app, text_view, filename);
    } else {
        editor_save_as_file(widget, data);
    }
}

void editor_save_as_file(GtkWidget *widget, gpointer data) {
    EditorApp *app = (EditorApp *)data;
    GtkTextView *text_view = get_current_text_view(app);
    if (!text_view) return;

    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Save File",
                                         GTK_WINDOW(app->window),
                                         action,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Save",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    // If there is already a name, suggest it
    char *current_filename = g_object_get_data(G_OBJECT(text_view), "filename");
    if (current_filename) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), current_filename);
    } else {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "Untitled.txt");
    }

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        save_file_internal(app, text_view, filename);
        
        // Update stored filename
        g_object_set_data_full(G_OBJECT(text_view), "filename", g_strdup(filename), g_free);
        
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void editor_quit(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}
