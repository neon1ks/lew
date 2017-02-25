#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "lew_json.h"




static GtkWidget   *window = NULL;

static GtkToolItem *saveItem = NULL;
static GtkToolItem *openItem = NULL;

int createDictFile( const char *dictFile );

int rand_range_correct( int range );
char* read_line_from_stdin( char* buf );
int translate_cmp(const char *translate, const char *russian);
size_t stripWhiteSpace(char** s);
int word_cmp (char* word1, char* word2);

static void lew_add_columns (GtkTreeView *treeview);
void lew_open_file_dialog (GtkToolButton *toolbutton, gpointer user_data);

static gboolean
treeview_onButtonPressed (GtkWidget *widget, GdkEvent *event, gpointer user_data);

static void
lew_edit_item (GtkWidget *button, gpointer data);

GtkTreeViewColumn *lew_create_column_index   (void);
GtkTreeViewColumn *lew_create_column_english (void);
GtkTreeViewColumn *lew_create_column_russian (void);

int main (int argc, char **argv)
{
	// Объявляем виджеты
	GtkWidget   *sw;
	GtkWidget   *toolbar;
	GtkWidget   *vbox;
	GtkWidget   *hbox;
	GtkWidget   *button;
	//GtkToolItem *item;

	// Инициализируем GTK+
	gtk_init (&argc, &argv);

	// Создаем главное окно
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Здравствуй, мир!");
	gtk_widget_set_size_request (window, 900, 200);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(window), vbox);


	// создаем контенер с прокруткой
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
	                                     GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	GtkListStore *model = gtk_list_store_new (NUM_COLUMNS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));

	lew_add_columns (GTK_TREE_VIEW (treeview));
	g_object_set(G_OBJECT(treeview), "enable-grid-lines", GTK_TREE_VIEW_GRID_LINES_BOTH, NULL);
	gtk_container_add (GTK_CONTAINER (sw), treeview);

	g_signal_connect (treeview, "button-press-event", G_CALLBACK (treeview_onButtonPressed), NULL);

	toolbar = gtk_toolbar_new ();

	openItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (openItem), "document-open");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (openItem), "Open");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (openItem), TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), openItem, -1);
	g_signal_connect(G_OBJECT(openItem), "clicked", G_CALLBACK(lew_open_file_dialog), (gpointer)model);

	saveItem = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (saveItem), "document-save");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (saveItem), "Save");
	gtk_tool_item_set_is_important (GTK_TOOL_ITEM (saveItem), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (saveItem), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), saveItem, -1);


	gtk_box_pack_start (GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), sw, TRUE, TRUE, 0);

      /* some buttons */
      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
      gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

      button = gtk_button_new_with_label ("Add item");
      //g_signal_connect (button, "clicked",
      //                  G_CALLBACK (add_item), treeview);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      button = gtk_button_new_with_label ("Edit item");
      g_signal_connect (button, "clicked", G_CALLBACK (lew_edit_item), treeview);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      button = gtk_button_new_with_label ("Remove item");
      //g_signal_connect (button, "clicked",
      //                  G_CALLBACK (remove_item), treeview);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

	// Показываем окно вместе с виджетами
	gtk_widget_show_all(window);

	// Соединяем сигнал завершения с выходом из программы
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), (gpointer)window);


	//char *envhome = getenv("HOME");
	//char *dictFile = NULL;
	//dictFile = calloc(strlen(envhome) + strlen(".dictionary.json") + 3, 1);
	//sprintf(dictFile,"%s/.dictionary.json",envhome);
	//printf("%s\n", dictFile);
	//createDictFile( dictFile );


	// Приложение переходит в вечный цикл ожидания действий пользователя
	gtk_main();


	return EXIT_SUCCESS;
}


int createDictFile( const char *dictFile ) {


	GError *error = NULL;
	JsonGenerator * dictGenerator;
	dictGenerator = json_generator_new ();

	JsonObject * rootObject = NULL;
	rootObject = json_object_new ();

	JsonNode *root = NULL;
	root = json_node_alloc();
	root = json_node_init_object (root, rootObject);

	JsonNode *node1 = NULL;
	node1 = json_node_alloc();
	node1 = json_node_init_string (node1, "utf-8");
	json_object_set_member (rootObject, "charset", node1);

	JsonNode *node2 = NULL;
	node2 = json_node_alloc();
	node2 = json_node_init_string (node2, "Maksim Demyanov");
	json_object_set_member (rootObject, "autor", node2);


	JsonArray *dictArray = NULL;
	dictArray = json_array_new ();
	JsonNode *node3 = NULL;
	node3 = json_node_alloc();
	node3 = json_node_init_array (node3, dictArray);
	json_object_set_member (rootObject, "dictionary", node3);

	JsonNode * nodeWord = NULL;

	nodeWord = lew_create_new_translation ("home", "дом");
	json_array_add_element (dictArray, nodeWord);

	nodeWord = lew_create_new_translation ("garden", "сад");
	json_array_add_element (dictArray, nodeWord);

	json_generator_set_root (dictGenerator, root);
	json_generator_set_pretty (dictGenerator, TRUE);


	json_generator_to_file( dictGenerator, dictFile, &error );
	if (error)
	{
		g_print ("Unable to write `%s': %s\n", dictFile, error->message);
		g_error_free (error);
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}



int rand_range_correct(int range)
{
	return (int)( (float)rand()/RAND_MAX * range );
}

char* read_line_from_stdin( char* buf )
{

	unsigned int N = 256, delta=256, i = 0;
	buf = (char*) malloc (sizeof(char)*N);
	buf[i] = getchar();
	while ( buf[i] != EOF && buf[i] != '\n' ) {
		++i;
		if (i >= N) {
			N += delta;
			buf = (char*) realloc (buf, sizeof(char)*N);
		}
		buf[i] = getchar();
	}
	buf[i] = '\0';
	return buf;
}

int translate_cmp(const char *translate, const char *russian)
{


	char *s1 = strdup(translate);
	char *s1_d = s1;
	char *s2;
	char *s2_d;

	int check = 0;
	int found = 0;
	int counter1 = 0;
	int counter2 = 0;
	char *scratch1, *txt1, *delimiter = ",;";
	char *scratch2, *txt2;
	while ( ( txt1 = strtok_r(!counter1 ? s1 : NULL, delimiter, &scratch1) ) )
	{
		counter1++;
		s2 = strdup(russian);
		s2_d = s2;
		counter2 = 0;
		found = 0;
		while ( ( txt2 = strtok_r(!counter2 ? s2 : NULL, delimiter, &scratch2) ) )
		{
			counter2++;
			if ( !word_cmp(txt1, txt2) ) {
				found++;
			}
		}
		if (found > 0) {
			check++;
		}
		free(s2_d);
	}
	free(s1_d);

	if ( counter1 == check) {
		return 0;
	}

	return 1;
}


size_t stripWhiteSpace(char** s)
{
	// индекс последнего не NULL символа
	int lastChar = strlen(*s) - 1;

	// Если последний символ CR, LF, пробел или табуляция
	while ( (lastChar >= 0) &
		(((*s)[lastChar] == 10 ) || ((*s)[lastChar] == 13  ) ||
		 ((*s)[lastChar] == ' ') || ((*s)[lastChar] == '\t')) )
	{
		(*s)[lastChar] = '\0';
		lastChar--;
	}

	// Пропускаем пробельные символы, кроеме \n, в начале строки
	while ((*s)[0]==' ' || (*s)[0]=='\t') {
		++(*s);
	}

	return strlen(*s);
}


int word_cmp (char* word1, char* word2)
{
	int i_cmp = -1;

	char *s1_orig = strdup(word1);
	char *s2_orig = strdup(word2);
	char *s1 = s1_orig;
	char *s2 = s2_orig;

	int i;
	int i_cut = 0;
	for (i=0;  i<strlen(s1); i++) {
		if ( s1[i] == '(' ) {
			i_cut = 1;
		}
		if ( i_cut == 1 ) {
			s1[i] = ' ';
		}
		if ( s1[i] == ')' ) {
			i_cut = 0;
		}
	}

	i_cut = 0;
	for (i=0;  i<strlen(s2); i++) {
		if ( s2[i] == '(' ) {
			i_cut = 1;
		}
		if ( i_cut == 1 ) {
			s2[i] = ' ';
		}
		if ( s2[i] == ')' ) {
			i_cut = 0;
		}
	}

	size_t i1 = stripWhiteSpace(&s1);
	size_t i2 = stripWhiteSpace(&s2);

	if (i1 > 0 && i2 > 0) {
		i_cmp = strcmp(s1, s2);
	}

	free(s1_orig);
	free(s2_orig);

	return i_cmp;
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


void
lew_open_file_dialog (GtkToolButton *toolbutton,
                      gpointer       user_data)
{

	GtkListStore *model = (GtkListStore *)user_data;
	gchar *home_dir = (gchar*)g_get_home_dir ();

	GtkFileFilter *filter = gtk_file_filter_new ();        // Создание фильтра

	gtk_file_filter_add_pattern (filter, "*.json");

	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open image",
	                                                 GTK_WINDOW (window),
	                                                 GTK_FILE_CHOOSER_ACTION_OPEN,
	                                                 "_OK", GTK_RESPONSE_ACCEPT,
	                                                 "_Cancel", GTK_RESPONSE_CANCEL,
	                                                 NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), home_dir);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	// запуск диалога выбора файла
	switch (gtk_dialog_run (GTK_DIALOG (dialog)))
	{
		case GTK_RESPONSE_ACCEPT:   // если нажали клавишу 'Open'
			{
				// Узнаём имя файла
				gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
				if ( !lew_read_json_file (filename, model) ) {
					gtk_widget_set_sensitive (GTK_WIDGET (saveItem), TRUE);
					gtk_widget_set_sensitive (GTK_WIDGET (openItem), FALSE);
				}
			}
			break;
		default:
			break;
	}
	gtk_widget_destroy (dialog);

}




static gboolean
treeview_onButtonPressed (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GtkTreeView *treeview = GTK_TREE_VIEW (widget);

	if (event->type == GDK_DOUBLE_BUTTON_PRESS)
	{
		lew_edit_json_item (window, treeview);
	}

	return FALSE;
}

static void
lew_edit_item (GtkWidget *button, gpointer data)
{
	GtkTreeView *treeview = (GtkTreeView *)data;
	lew_edit_json_item (window, treeview);
}


/*
static void
add_item (GtkWidget *button, gpointer data)
{
  //Item foo;
  //GtkTreeIter current, iter;
  //GtkTreePath *path;

  GtkTreeModel *model;
  //GtkTreeViewColumn *column;


  GtkTreeView *treeview = (GtkTreeView *)data;

  //g_return_if_fail (articles != NULL);

  //foo.number = 0;
  //foo.product = g_strdup ("Description here");
  //foo.yummy = 50;
  //g_array_append_vals (articles, &foo, 1);

  // Insert a new row below the current one
  //gtk_tree_view_get_cursor (treeview, &path, NULL);
	model = gtk_tree_view_get_model (treeview);

	gtk_list_store_insert (GTK_LIST_STORE (model), &iter, -1);

	gtk_list_store_insert (model, &iter, -1);
	//gtk_list_store_append (model, &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
	                    COLUMN_ENG, english,
	                    COLUMN_RUS, russian,
	                    -1);


  // Set the data for the new row
  //gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COLUMN_ITEM_NUMBER, foo.number,
                      COLUMN_ITEM_PRODUCT, foo.product,
                      COLUMN_ITEM_YUMMY, foo.yummy,
                      -1);

  // Move focus to the new row
  path = gtk_tree_model_get_path (model, &iter);
  column = gtk_tree_view_get_column (treeview, 0);
  gtk_tree_view_set_cursor (treeview, path, column, FALSE);

  gtk_tree_path_free (path);
}*/


GtkTreeViewColumn *
lew_create_column_index (void)
{
	GtkTreeViewColumn *col      = NULL;
	GtkCellRenderer   *renderer = NULL;

	col = gtk_tree_view_column_new ();

	// Настраиваем столбец
	gtk_tree_view_column_set_title(col, "Index");              // Заголовок столбца
	gtk_tree_view_column_set_resizable (col, TRUE);            // Включение изменяемости ширины столбца
	gtk_tree_view_column_set_sort_column_id (col, COLUMN_NUM); // Сортировка

	// Создаем и настраиваем обработчик ячеек
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_NUM);

	return col;
}

GtkTreeViewColumn *
lew_create_column_english (void)
{
	GtkTreeViewColumn *col      = NULL;
	GtkCellRenderer   *renderer = NULL;

	col = gtk_tree_view_column_new ();

	// Настраиваем столбец
	gtk_tree_view_column_set_title(col, "English");            // Заголовок
	gtk_tree_view_column_set_min_width (col, 395);             // Минимальная ширина
	gtk_tree_view_column_set_resizable (col, TRUE);            // Изменяемость ширины
	gtk_tree_view_column_set_sort_column_id (col, COLUMN_ENG); // Сортировка

	// Создаем и настраиваем обработчик ячеек
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer), "width-chars", 49, "wrap-mode", PANGO_WRAP_WORD, "wrap-width", 49, NULL);
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_ENG);

	return col;
}


GtkTreeViewColumn *
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
	g_object_set(G_OBJECT(renderer), "wrap-mode", PANGO_WRAP_WORD, "wrap-width", 60, NULL);
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_RUS);

	return col;
}
