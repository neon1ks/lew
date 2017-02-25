#include "lew_json.h"

static JsonParser *parser    = NULL;
static JsonNode   *root      = NULL;
static JsonArray  *dictArray = NULL;


static guint dictLen = 0;

int lew_read_json_file (gchar *filename, GtkListStore *model)
{

	g_print ("%s\n", filename);

	GtkTreeIter iter;
	GError *error;

	parser = json_parser_new ();

	//gboolean immutable;
	//g_object_get (G_OBJECT(parser), "immutable", &immutable, NULL);
	//if (immutable == FALSE) {
	//	g_print("%s\n", "immutable == FALSE");
	//}
	//if (immutable == TRUE) {
	//	g_print("%s\n", "immutable == TRUE");
	//}

	error = NULL;
	json_parser_load_from_file (parser, filename, &error);
	if (error) {
		g_print ("Unable to parse `%s': %s\n", filename, error->message);
		g_error_free (error);
		g_object_unref (parser);
		return 1;
	}
	root = json_parser_get_root (parser);

	if ( JSON_NODE_HOLDS_OBJECT(root) ) {
		printf("The node contains a JsonObject\n");
	}

	dictArray = json_object_get_array_member (json_node_get_object (root), "dictionary");

	guint len = json_array_get_length(dictArray);
	dictLen = len;

	JsonObject *objWord;
	const gchar *english = NULL;
	const gchar *russian = NULL;

	for (int i = 0; i < len; ++i) {

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
	return 0;
}


JsonNode * lew_create_new_translation (const gchar *english, const gchar *russian)
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


void lew_form_translation_edit (GtkWidget     *window,
                                GtkTreeModel  *model,
                                GtkTreeIter   *iter,
                                gint          index)
{

	JsonObject  *objWord      = NULL;
	const gchar *english      = NULL;
	const gchar *russian      = NULL;

	GtkWidget   *dialog       = NULL;
	GtkWidget   *content_area = NULL;
	GtkWidget   *grid         = NULL;
	GtkWidget   *local_entry1 = NULL;
	GtkWidget   *local_entry2 = NULL;
	GtkWidget   *label        = NULL;

	gint response;

	dialog = gtk_dialog_new_with_buttons ("Interactive Dialog",
	                                      GTK_WINDOW (window),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      "OK", GTK_RESPONSE_OK,
	                                      "Cancel", GTK_RESPONSE_CANCEL,
	                                      NULL);

	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_set_border_width (GTK_CONTAINER (content_area), 10);

	grid = gtk_grid_new ();

	gtk_grid_set_row_spacing (GTK_GRID (grid), 5);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 5);
	gtk_box_pack_start (GTK_BOX (content_area), grid, TRUE, TRUE, 0);

	label = gtk_label_new_with_mnemonic ("english");
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
	local_entry1 = gtk_entry_new ();

	gtk_grid_attach (GTK_GRID (grid), local_entry1, 1, 0, 1, 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), local_entry1);

	label = gtk_label_new_with_mnemonic ("russian");
	gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);

	local_entry2 = gtk_entry_new ();

	gtk_grid_attach (GTK_GRID (grid), local_entry2, 1, 1, 1, 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), local_entry2);

	g_print ("index = %d\n", index);
	if (index >= 0) {
		objWord = json_array_get_object_element (dictArray, index);
		english = json_object_get_string_member (objWord, "english");
		russian = json_object_get_string_member (objWord, "russian");
		gtk_entry_set_text (GTK_ENTRY (local_entry1), english);
		gtk_entry_set_text (GTK_ENTRY (local_entry2), russian);
	}


	gtk_widget_show_all (content_area);

	response = gtk_dialog_run (GTK_DIALOG (dialog));

	if (response == GTK_RESPONSE_OK) {

		if (index == -1) {
			JsonNode *nodeWord = NULL;
			nodeWord = lew_create_new_translation (gtk_entry_get_text (GTK_ENTRY (local_entry1)),
			                                       gtk_entry_get_text (GTK_ENTRY (local_entry2)));
			json_array_add_element (dictArray, nodeWord);
			objWord = json_node_get_object (nodeWord);
			index = dictLen;
			dictLen++;
		} else {
			json_object_set_string_member(objWord, "english", gtk_entry_get_text (GTK_ENTRY (local_entry1)));
			json_object_set_string_member(objWord, "russian", gtk_entry_get_text (GTK_ENTRY (local_entry2)));
		}

		english = json_object_get_string_member (objWord, "english");
		russian = json_object_get_string_member (objWord, "russian");

		gtk_list_store_set (GTK_LIST_STORE(model), iter,
		                    COLUMN_NUM, index,
		                    COLUMN_ENG, english,
		                    COLUMN_RUS, russian,
		                    -1);
	}
	gtk_widget_destroy (dialog);

}

