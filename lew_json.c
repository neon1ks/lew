#include "lew_json.h"

static JsonParser *parser    = NULL;
static JsonNode   *root      = NULL;
static JsonArray  *dictArray = NULL;

int lew_read_json_file (gchar *filename, GtkListStore *model)
{

	g_print ("%s\n", filename);

	GtkTreeIter iter;
	GError *error;

	parser = json_parser_new ();

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

	JsonObject *objWord;
	const gchar *english = NULL;
	const gchar *russian = NULL;

	for (int i = 0; i < len; ++i) {

		objWord = json_array_get_object_element (dictArray, i);
		english = json_object_get_string_member (objWord, "english");
		russian = json_object_get_string_member (objWord, "russian");

		gtk_list_store_insert (model, &iter, -1);
		gtk_list_store_set (model, &iter,
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
