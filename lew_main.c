#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

enum
{
	COLUMN_ENG,
	COLUMN_RUS,
	NUM_COLUMNS
};

int createDictFile( const char *dictFile );
JsonNode * create_new_word( const gchar *english, const gchar *russian );
int rand_range_correct( int range );
char* read_line_from_stdin( char* buf );
int translate_cmp(const char *translate, const char *russian);
size_t stripWhiteSpace(char** s);
int word_cmp (char* word1, char* word2);

static void lew_add_columns (GtkTreeView *treeview);

int main (int argc, char **argv)
{
	// Объявляем виджеты
	GtkWidget *window;  // Главное окно
	GtkWidget *sw;

	// Инициализируем GTK+
	gtk_init (&argc, &argv);

	// Создаем главное окно
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Здравствуй, мир!");
	gtk_widget_set_size_request (window, 800, 200);

	// создаем контенер с прокруткой
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
	                                     GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (window), sw);


	GtkListStore *model = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));

	lew_add_columns (GTK_TREE_VIEW (treeview));

	g_object_set(G_OBJECT(treeview), "enable-grid-lines", GTK_TREE_VIEW_GRID_LINES_BOTH, NULL);

	gtk_container_add (GTK_CONTAINER (sw), treeview);

	// Показываем окно вместе с виджетами
	gtk_widget_show_all(window);

	// Соединяем сигнал завершения с выходом из программы
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), (gpointer)window);

	char *envhome = getenv("HOME");
	char *dictFile = NULL;
	dictFile = calloc(strlen(envhome) + strlen(".dictionary.json") + 3, 1);
	sprintf(dictFile,"%s/.dictionary.json",envhome);

	//printf("%s\n", dictFile);
	//createDictFile( dictFile );

	JsonParser *parser;
	JsonNode *root;
	GError *error;

	parser = json_parser_new ();

	error = NULL;
	json_parser_load_from_file (parser, dictFile, &error);
	if (error) {
		g_print ("Unable to parse `%s': %s\n", dictFile, error->message);
		g_error_free (error);
		g_object_unref (parser);
		return EXIT_FAILURE;
	}

	root = json_parser_get_root (parser);

	if ( JSON_NODE_HOLDS_OBJECT(root) ) {
		printf("The node contains a JsonObject\n");
	}

	JsonObject *obj = json_node_get_object(root);
	JsonArray *dictArray = json_object_get_array_member (obj, "dictionary");

	guint len = json_array_get_length(dictArray);

	g_print ("len of array = %u\n", len);

	int index;
	JsonObject *objWord;
	const gchar *english = NULL;
	const gchar *russian = NULL;

	int i_corr = 0;
	int i_err = 0;

	char *translate = NULL;

	for ( int i = 0; i < len; ++i ) {
		index = rand_range_correct(len);

		objWord = json_array_get_object_element (dictArray, index);
		english = json_object_get_string_member (objWord, "english");
		russian = json_object_get_string_member (objWord, "russian");

		printf("%3d) %s\n     ",  i, english);

		translate = read_line_from_stdin(translate);


		if (translate_cmp(translate, russian) == 0) {
			printf("     \x1b[32m%s\x1b[0m\n\n", russian);
			i_corr++;
		} else {
			printf("     \x1b[31m%s\x1b[0m\n\n", russian);
			i_err++;
		}


		free(translate);
	}

	printf("i_corr = %d\n", i_corr);
	printf("i_err = %d\n",  i_err);

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

	nodeWord = create_new_word ("home", "дом");
	json_array_add_element (dictArray, nodeWord);

	nodeWord = create_new_word ("garden", "сад");
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

JsonNode * create_new_word (const gchar *english, const gchar *russian)
{
	JsonNode *nodeWord = json_node_alloc();
	JsonObject * objectWord = json_object_new ();
	nodeWord = json_node_init_object (nodeWord, objectWord);

	JsonNode *nodeWordEng = json_node_alloc();
	nodeWordEng = json_node_init_string (nodeWordEng, english);
	json_object_set_member (objectWord, "english", nodeWordEng);

	JsonNode *nodeWordRus = json_node_alloc();
	nodeWordRus = json_node_init_string (nodeWordRus, russian);
	json_object_set_member (objectWord, "russian", nodeWordRus);

	return nodeWord;
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
	GtkCellRenderer *renderer = NULL;
	GtkTreeViewColumn *col = NULL;

	//==========================================================================

	// Создаем первый столбец для английского текста
	col = gtk_tree_view_column_new ();

	// Настраиваем столбец
	gtk_tree_view_column_set_title(col, "English");            // Заголовок
	gtk_tree_view_column_set_min_width (col, 395);             // Минимальная ширина
	gtk_tree_view_column_set_resizable (col, TRUE);            // Изменяемость ширины
	gtk_tree_view_column_set_sort_column_id (col, COLUMN_ENG); // Сортировка

	// Создаем и настраиваем обработчик ячеек для столбца с английским текстом
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer), "width-chars", 49, "wrap-mode", PANGO_WRAP_WORD, "wrap-width", 49, NULL);
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_ENG);

	// Упаковываем столбец
	gtk_tree_view_append_column (GTK_TREE_VIEW( treeview), col);

	//==========================================================================

	// Создаем второй столбец для руского текста
	col = gtk_tree_view_column_new ();

	// Настройка столбца
	gtk_tree_view_column_set_title (col, "Russian");           // Заголовок
	gtk_tree_view_column_set_min_width (col, 395);             // Минимальная ширина
	gtk_tree_view_column_set_resizable (col, TRUE);            // Изменяемость ширины
	gtk_tree_view_column_set_sort_column_id (col, COLUMN_RUS); // Сортировка

	// Создаем и настраиваем обработчик ячеек для столбца с русским текстом
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer), "wrap-mode", PANGO_WRAP_WORD, "wrap-width", 60, NULL);
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_RUS);

	// Упаковываем столбец
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

	//==========================================================================

}
