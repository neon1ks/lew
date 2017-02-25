#ifndef LEW_JSON_H
#define LEW_JSON_H

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

enum
{
	COLUMN_NUM,
	COLUMN_ENG,
	COLUMN_RUS,
	NUM_COLUMNS
};

int lew_read_json_file (gchar *filename, GtkListStore *model);

JsonNode * lew_create_new_translation ( const gchar *english, const gchar *russian );


void lew_form_translation_edit (GtkWidget    *window,
                                GtkTreeView   *treeview,
                                gboolean       isNewTranslation);



#endif
