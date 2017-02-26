#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "lew_json.h"



static GtkWidget   *window     = NULL;

static GtkToolItem *newItem    = NULL;
static GtkToolItem *saveItem   = NULL;
static GtkToolItem *openItem   = NULL;
static GtkToolItem *closeItem  = NULL;

static GtkToolItem *addItem    = NULL;
static GtkToolItem *editItem   = NULL;
static GtkToolItem *removeItem = NULL;

static gchar       *filename   = NULL;


static GtkTreeViewColumn *
lew_create_column_index (void)
{
	GtkTreeViewColumn *col      = NULL;
	GtkCellRenderer   *renderer = NULL;

	col = gtk_tree_view_column_new ();

	// Настраиваем столбец
	gtk_tree_view_column_set_title (col, "Index");             // Заголовок столбца
	gtk_tree_view_column_set_resizable (col, TRUE);            // Включение изменяемости ширины столбца
	gtk_tree_view_column_set_sort_column_id (col, COLUMN_NUM); // Сортировка

	// Создаем и настраиваем обработчик ячеек
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_NUM);

	return col;
}



static GtkTreeViewColumn *
lew_create_column_english (void)
{
	GtkTreeViewColumn *col      = NULL;
	GtkCellRenderer   *renderer = NULL;

	col = gtk_tree_view_column_new ();

	// Настраиваем столбец
	gtk_tree_view_column_set_title (col, "English");           // Заголовок
	gtk_tree_view_column_set_min_width (col, 395);             // Минимальная ширина
	gtk_tree_view_column_set_resizable (col, TRUE);            // Изменяемость ширины
	gtk_tree_view_column_set_sort_column_id (col, COLUMN_ENG); // Сортировка

	// Создаем и настраиваем обработчик ячеек
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "width-chars", 49, NULL);
	g_object_set (G_OBJECT (renderer), "wrap-width",  49, NULL);
	g_object_set (G_OBJECT (renderer), "wrap-mode", PANGO_WRAP_WORD, NULL);
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_ENG);

	return col;
}



static GtkTreeViewColumn *
lew_create_column_russian (void)
{
	GtkTreeViewColumn *col      = NULL;
	GtkCellRenderer   *renderer = NULL;

	col = gtk_tree_view_column_new ();

	// Настройка столбца
	gtk_tree_view_column_set_title (col, "Russian");           // Заголовок
	gtk_tree_view_column_set_min_width (col, 395);             // Минимальная ширина
	gtk_tree_view_column_set_resizable (col, TRUE);            // Изменяемость ширины
	gtk_tree_view_column_set_sort_column_id (col, COLUMN_RUS); // Сортировка

	// Создаем и настраиваем обработчик ячеек
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "width-chars", 49, NULL);
	g_object_set (G_OBJECT (renderer), "wrap-width",  49, NULL);
	g_object_set (G_OBJECT (renderer), "wrap-mode", PANGO_WRAP_WORD, NULL);
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_RUS);

	return col;
}



static void
lew_add_columns (GtkTreeView *treeview)
{
	GtkTreeViewColumn *col = NULL;

	// Создаем столбец для индекса
	col = lew_create_column_index ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

	// Создаем столбец для английского текста
	col = lew_create_column_english ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

	// Создаем cтолбец для русского текста
	col = lew_create_column_russian ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);
}



static GtkWidget *
lew_create_toolbar (void)
{
	GtkWidget *toolbar = gtk_toolbar_new ();

	newItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (newItem), "gtk-new");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (newItem), "New");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (newItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (newItem), TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), newItem, -1);

	openItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (openItem), "document-open");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (openItem), "Open");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (openItem), TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), openItem, -1);

	saveItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (saveItem), "document-save");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (saveItem), "Save");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (saveItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (saveItem), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), saveItem, -1);

	closeItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (closeItem), "gtk-close");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (closeItem), "Close");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (closeItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (closeItem), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), closeItem, -1);

	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), -1);

	addItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (addItem), "gtk-add");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (addItem), "Add item");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (addItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (addItem), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), addItem, -1);

	editItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (editItem), "gtk-edit");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (editItem), "Edit item");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (editItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (editItem), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), editItem, -1);

	removeItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (removeItem), "gtk-remove");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (removeItem), "Remove item");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (removeItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (removeItem), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), removeItem, -1);

	return toolbar;
}



static GtkWidget *
lew_create_treeview (void)
{
	GtkListStore *model = gtk_list_store_new (NUM_COLUMNS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_widget_set_sensitive (GTK_WIDGET (treeview), FALSE);
	lew_add_columns (GTK_TREE_VIEW (treeview));
	g_object_set (G_OBJECT (treeview), "enable-grid-lines", GTK_TREE_VIEW_GRID_LINES_BOTH, NULL);
	return treeview;
}



static void
lew_open_file_dialog (GtkToolButton *toolbutton,
                      gpointer       user_data)
{

	GtkTreeView *treeview = (GtkTreeView *)user_data;
	GtkTreeModel *model = gtk_tree_view_get_model (treeview);

	GtkFileFilter *filter = gtk_file_filter_new (); // Создание фильтра
	gtk_file_filter_add_pattern (filter, "*.json");

	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open image",
	                                                 GTK_WINDOW (window),
	                                                 GTK_FILE_CHOOSER_ACTION_OPEN,
	                                                 "_OK", GTK_RESPONSE_ACCEPT,
	                                                 "_Cancel", GTK_RESPONSE_CANCEL,
	                                                 NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir ());
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	// запуск диалога выбора файла
	switch (gtk_dialog_run (GTK_DIALOG (dialog)))
	{
		case GTK_RESPONSE_ACCEPT:   // если нажали клавишу 'Open'
			{
				// Узнаём имя файла
				filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

				if ( lew_read_json_file (filename, GTK_LIST_STORE (model)) )
				{
					gtk_widget_set_sensitive (GTK_WIDGET (addItem),   TRUE);
					gtk_widget_set_sensitive (GTK_WIDGET (editItem),  TRUE);
					gtk_widget_set_sensitive (GTK_WIDGET (closeItem), TRUE);
					gtk_widget_set_sensitive (GTK_WIDGET (treeview),  TRUE);
					gtk_widget_set_sensitive (GTK_WIDGET (openItem),  FALSE);
					gtk_widget_set_sensitive (GTK_WIDGET (newItem),   FALSE);
				}
			}
			break;
		default:
			break;
	}

	gtk_widget_destroy (dialog);
}



static void
lew_action_iten_save (GtkToolButton *toolbutton,
                      gpointer       user_data)
{

	GtkWidget *dialog;
	GtkFileChooser *chooser;
	gint res;

	dialog = gtk_file_chooser_dialog_new ("Save File",
	                                      GTK_WINDOW (window),
	                                      GTK_FILE_CHOOSER_ACTION_SAVE,
	                                      "Cancel", GTK_RESPONSE_CANCEL,
	                                      "Save", GTK_RESPONSE_ACCEPT,
	                                      NULL);
	chooser = GTK_FILE_CHOOSER (dialog);

	gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

	if (filename == NULL) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir ());
		gtk_file_chooser_set_current_name (chooser, "dict.json");
	}
	else {
		gtk_file_chooser_set_filename (chooser, filename);
	}

	res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		if (filename != NULL) g_free (filename);
		filename = gtk_file_chooser_get_filename (chooser);
		if (lew_write_json_file (filename)) {
			gtk_widget_set_sensitive (GTK_WIDGET (saveItem), FALSE);
		}
	}

	gtk_widget_destroy (dialog);

}



static void
lew_action_iten_add (GtkToolButton *toolbutton,
                     gpointer       user_data)
{
	GtkTreeView *treeview = (GtkTreeView *)user_data;
	if (lew_form_translation_edit (window, treeview, TRUE)) {
		gtk_widget_set_sensitive (GTK_WIDGET (saveItem), TRUE);
	}
}



static void
lew_action_iten_new (GtkToolButton *toolbutton,
                      gpointer       user_data)
{
	GtkTreeView *treeview = (GtkTreeView *)user_data;

	lew_create_json_root ();

	gtk_widget_set_sensitive (GTK_WIDGET (addItem),   TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (editItem),  FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (closeItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (treeview),  TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (openItem),  FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (newItem),   FALSE);
}



static void
lew_action_iten_edit (GtkToolButton *toolbutton,
                      gpointer       user_data)
{
	GtkTreeView *treeview = (GtkTreeView *)user_data;
	if (lew_form_translation_edit (window, treeview, FALSE)) {
		gtk_widget_set_sensitive (GTK_WIDGET (saveItem), TRUE);
	}
}



//~ gboolean
//~ treeview_onButtonPressed (GtkWidget *widget, GdkEvent *event, gpointer user_data)
//~ {
	//~ GtkTreeView *treeview = GTK_TREE_VIEW (widget);
	//~ if (event->type == GDK_DOUBLE_BUTTON_PRESS)
	//~ {
		//~ if (lew_form_translation_edit (window, treeview, FALSE)) {
			//~ gtk_widget_set_sensitive (GTK_WIDGET (saveItem), TRUE);
		//~ }
	//~ }
	//~ return FALSE;
//~ }



static GtkWidget *
lew_create_scrolled_window (void)
{
	GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
	                                     GTK_SHADOW_ETCHED_IN);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (sw), 8);

	return sw;
}



int main (int argc, char **argv)
{
	GtkWidget   *sw         = NULL;
	GtkWidget   *vbox       = NULL;
	GtkWidget   *toolbar    = NULL;
	GtkWidget   *treeview   = NULL;

	// Инициализация GTK+
	gtk_init (&argc, &argv);

	// Создание главного окна
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "LEW Editor");
	gtk_widget_set_size_request (window, 900, 200);
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	// Создание панели инструменов
	toolbar = lew_create_toolbar ();
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);

	// Создание контенера с прокрутками по вертикали и горизонтали
	sw = lew_create_scrolled_window ();
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

	// Создание и настройка treeview
	treeview = lew_create_treeview ();
	gtk_container_add (GTK_CONTAINER (sw), treeview);

	// Показываем окно со всеми виджетами
	gtk_widget_show_all (window);


	g_signal_connect (G_OBJECT (openItem), "clicked", G_CALLBACK (lew_open_file_dialog), (gpointer)treeview);
	g_signal_connect (G_OBJECT (editItem), "clicked", G_CALLBACK (lew_action_iten_edit), (gpointer)treeview);
	g_signal_connect (G_OBJECT (newItem),  "clicked", G_CALLBACK (lew_action_iten_new),  (gpointer)treeview);
	g_signal_connect (G_OBJECT (addItem),  "clicked", G_CALLBACK (lew_action_iten_add),  (gpointer)treeview);
	g_signal_connect (G_OBJECT (saveItem), "clicked", G_CALLBACK (lew_action_iten_save), (gpointer)treeview);

	//~ g_signal_connect (treeview, "button-press-event", G_CALLBACK (treeview_onButtonPressed), NULL);

	// Соединяем сигнал завершения с выходом из программы
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), (gpointer)window);


	//~ char *envhome = getenv("HOME");
	//~ char *dictFile = NULL;
	//~ dictFile = calloc(strlen(envhome) + strlen(".dictionary.json") + 3, 1);
	//~ sprintf(dictFile,"%s/.dictionary.json",envhome);
	//~ printf("%s\n", dictFile);
	//~ createDictFile( dictFile );


	// Приложение переходит в вечный цикл ожидания действий пользователя
	gtk_main ();


	return EXIT_SUCCESS;
}



