#include <gtk/gtk.h>
#include "../include/editor.h"

static void create_menu_bar(EditorApp *app, GtkWidget *vbox) {
    GtkWidget *menubar;
    GtkWidget *fileMenu;
    GtkWidget *fileMi;
    GtkWidget *quitMi;
    GtkWidget *newMi;
    GtkWidget *openMi;
    GtkWidget *saveMi;
    GtkWidget *saveAsMi;

    menubar = gtk_menu_bar_new();
    fileMenu = gtk_menu_new();

    fileMi = gtk_menu_item_new_with_label("File");
    newMi = gtk_menu_item_new_with_label("New");
    openMi = gtk_menu_item_new_with_label("Open");
    saveMi = gtk_menu_item_new_with_label("Save");
    saveAsMi = gtk_menu_item_new_with_label("Save As...");
    quitMi = gtk_menu_item_new_with_label("Quit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), newMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), openMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveAsMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(newMi), "activate", G_CALLBACK(editor_new_file), app);
    g_signal_connect(G_OBJECT(openMi), "activate", G_CALLBACK(editor_open_file), app);
    g_signal_connect(G_OBJECT(saveMi), "activate", G_CALLBACK(editor_save_file), app);
    g_signal_connect(G_OBJECT(saveAsMi), "activate", G_CALLBACK(editor_save_as_file), app);
    g_signal_connect(G_OBJECT(quitMi), "activate", G_CALLBACK(editor_quit), app);
}

void editor_window_init(EditorApp *app) {
    GtkWidget *vbox;

    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(app->window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(app->window), 800, 600);
    gtk_window_set_title(GTK_WINDOW(app->window), "Pro Write");

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);

    create_menu_bar(app, vbox);

    app->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(app->notebook), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), app->notebook, TRUE, TRUE, 0);

    app->statusbar = gtk_statusbar_new();
    app->statusbar_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(app->statusbar), "Editor Context");
    gtk_box_pack_start(GTK_BOX(vbox), app->statusbar, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(app->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create an initial empty tab
    editor_new_file(NULL, app);
}
