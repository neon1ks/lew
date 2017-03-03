#include "lew_json.h"

static JsonNode   *root      = NULL;
static JsonArray  *dictArray = NULL;

static guint dictLen = 0;

/*
static void
lew_object_add_id (JsonObject *object, const guint id)
{
	gint64 value = id;
	JsonNode *nodeIndex = json_node_alloc ();
	nodeIndex = json_node_init_int (nodeIndex, value);
	json_object_set_member (object, "id", nodeIndex);
}*/

gchar * lew_get_text_from_buffer (GtkTextBuffer *buffer)
{

	GtkTextIter iter_prev;
	GtkTextIter iter;

	gint i = 0;
	gchar * line;
	GString * text = g_string_new (NULL);

	gtk_text_buffer_get_start_iter (buffer, &iter);
	iter_prev = iter;

	while ( gtk_text_iter_forward_line (&iter) || gtk_text_iter_is_end (&iter) ) {

		line = gtk_text_buffer_get_text (buffer, &iter_prev, &iter, FALSE);

		line = g_strchug (line);
		line = g_strchomp (line);

		if ( strlen (line) > 0 ) {
			if ( i > 0 ) {
				text = g_string_append_c (text, ' ');
			}
			i++;
			text = g_string_append (text, line);
		}
		iter_prev = iter;

		g_free (line);

		if ( gtk_text_iter_is_end (&iter) ) {
			break;
		}
	}

	return text->str;
}

void lew_create_json_root (void)
{
	JsonObject * rootObject = NULL;
	rootObject = json_object_new ();

	root = json_node_alloc ();
	root = json_node_init_object (root, rootObject);

	JsonNode *node1 = NULL;
	node1 = json_node_alloc ();
	node1 = json_node_init_string (node1, "utf-8");
	json_object_set_member (rootObject, "charset", node1);

	dictArray = json_array_new ();
	JsonNode *node2 = NULL;
	node2 = json_node_alloc ();
	node2 = json_node_init_array (node2, dictArray);
	json_object_set_member (rootObject, "dictionary", node2);

	dictLen = 0;
}

gboolean
lew_read_json_file (gchar *filename, GtkListStore *model)
{

	//~ g_print ("%s\n", filename);

	GtkTreeIter iter;
	GError *error;

	JsonParser *parser = json_parser_new ();

	error = NULL;
	json_parser_load_from_file (parser, filename, &error);
	if (error) {
		g_print ("Unable to parse `%s': %s\n", filename, error->message);
		g_error_free (error);
		g_object_unref (parser);
		return FALSE;
	}
	root = json_parser_get_root (parser);

	//~ if ( JSON_NODE_HOLDS_OBJECT (root) ) {
		//~ printf ("The node contains a JsonObject\n");
	//~ }

	dictArray = json_object_get_array_member (json_node_get_object (root), "dictionary");

	dictLen = json_array_get_length (dictArray);

	JsonObject *objWord;
	const gchar *english = NULL;
	const gchar *russian = NULL;

	for (guint i = 0; i < dictLen; ++i) {

		objWord = json_array_get_object_element (dictArray, i);
		english = json_object_get_string_member (objWord, "english");
		russian = json_object_get_string_member (objWord, "russian");

		gtk_list_store_insert (model, &iter, -1);
		gtk_list_store_set (model, &iter,
		                    COLUMN_NUM, i,
		                    COLUMN_ENG, english,
		                    COLUMN_RUS, russian,
		                    -1);
	}

	return TRUE;
}

gboolean
lew_write_json_file (const gchar *filename)
{
	GError *error = NULL;
	JsonGenerator * dictGenerator;
	dictGenerator = json_generator_new ();

	json_generator_set_root (dictGenerator, root);
	json_generator_set_pretty (dictGenerator, TRUE);

	json_generator_to_file (dictGenerator, filename, &error);
	if (error)
	{
		g_print ("Unable to write `%s': %s\n", filename, error->message);
		g_error_free (error);
		return FALSE;
	}

	return TRUE;
}

JsonNode *
lew_create_new_translation (const gchar *english, const gchar *russian)
{
	JsonNode *nodeWord = json_node_alloc ();
	JsonObject * objectWord = json_object_new ();
	nodeWord = json_node_init_object (nodeWord, objectWord);

	JsonNode *nodeWordEng = json_node_alloc ();
	nodeWordEng = json_node_init_string (nodeWordEng, english);
	json_object_set_member (objectWord, "english", nodeWordEng);

	JsonNode *nodeWordRus = json_node_alloc ();
	nodeWordRus = json_node_init_string (nodeWordRus, russian);
	json_object_set_member (objectWord, "russian", nodeWordRus);

	return nodeWord;
}

gboolean
lew_form_translation_edit (GtkWidget     *window,
                           GtkTreeView   *treeview,
                           gboolean       isNewTranslation)
{
	gboolean status = FALSE;
	guint id;
	GtkTreeIter iter;
	GtkTreeSelection *selection = NULL;
	GtkTreeModel *model = gtk_tree_view_get_model (treeview);

	if (!isNewTranslation) {
		selection = gtk_tree_view_get_selection (treeview);
		if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
			gtk_tree_model_get (model, &iter, COLUMN_NUM, &id, -1);
		}
		else {
			return FALSE;
		}
	}

	GtkWidget   *dialog          = NULL;
	GtkWidget   *content_area    = NULL;
	GtkWidget   *scrolled_window = NULL;
	GtkWidget   *vbox            = NULL;
	//GtkWidget   *grid            = NULL;
	//GtkWidget   *local_entry1    = NULL;
	//GtkWidget   *local_entry2    = NULL;
	GtkWidget   *label           = NULL;

	JsonObject  *objWord      = NULL;
	const gchar *english      = NULL;
	const gchar *russian      = NULL;

	GtkTextBuffer *buffer_eng;
	GtkTextBuffer *buffer_rus;

	buffer_eng = gtk_text_buffer_new (NULL);
	buffer_rus = gtk_text_buffer_new (NULL);

	GtkWidget *text_view_eng;
	GtkWidget *text_view_rus;

	text_view_eng = gtk_text_view_new_with_buffer (buffer_eng);
	text_view_rus = gtk_text_view_new_with_buffer (buffer_rus);

	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view_eng), GTK_WRAP_WORD);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view_rus), GTK_WRAP_WORD);

	gtk_widget_set_size_request (text_view_eng, 470, 50);
	gtk_widget_set_size_request (text_view_rus, 470, 50);

	gint response;

	dialog = gtk_dialog_new_with_buttons ("Interactive Dialog",
	                                      GTK_WINDOW (window),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      "OK", GTK_RESPONSE_OK,
	                                      "Cancel", GTK_RESPONSE_CANCEL,
	                                      NULL);

	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_set_border_width (GTK_CONTAINER (content_area), 10);
	gtk_widget_set_size_request (content_area, 500, 220);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (content_area), scrolled_window, TRUE, TRUE, 0);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);

	gtk_container_add (GTK_CONTAINER (scrolled_window), vbox);

	label = gtk_label_new_with_mnemonic ("english");
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), text_view_eng, FALSE, FALSE, 0);


	label = gtk_label_new_with_mnemonic ("russian");
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), text_view_rus, FALSE, FALSE, 0);


	if ( !isNewTranslation ) {
		objWord = json_array_get_object_element (dictArray, id);
		english = json_object_get_string_member (objWord, "english");
		russian = json_object_get_string_member (objWord, "russian");
		gtk_text_buffer_set_text (buffer_eng, english, strlen (english));
		gtk_text_buffer_set_text (buffer_rus, russian, strlen (russian));
	}

	gtk_widget_show_all (content_area);

	response = gtk_dialog_run (GTK_DIALOG (dialog));

	if (response == GTK_RESPONSE_OK) {

		gchar *text_eng = lew_get_text_from_buffer (buffer_eng);
		gchar *text_rus = lew_get_text_from_buffer (buffer_rus);

		if ( isNewTranslation ) {

			JsonNode *nodeWord = NULL;
			nodeWord = lew_create_new_translation (text_eng, text_rus);
			json_array_add_element (dictArray, nodeWord);
			objWord = json_node_get_object (nodeWord);
			id = dictLen;
			dictLen++;
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		} else {
			json_object_set_string_member (objWord, "english", text_eng);
			json_object_set_string_member (objWord, "russian", text_rus);
		}

		english = json_object_get_string_member (objWord, "english");
		russian = json_object_get_string_member (objWord, "russian");

		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		                    COLUMN_NUM, id,
		                    COLUMN_ENG, english,
		                    COLUMN_RUS, russian,
		                    -1);

		// Перемещение фокуса на добавленный перевод
		GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
		GtkTreeViewColumn *column = gtk_tree_view_get_column (treeview, 0);
		gtk_tree_view_set_cursor (treeview, path, column, FALSE);
		gtk_tree_path_free (path);
		status = TRUE;

		g_free (text_eng);
		g_free (text_rus);
	}

	gtk_widget_destroy (dialog);

	return status;
}

guint lew_get_dict_len (void)
{
	return dictLen;
}
