#ifndef LEW_JSON_H
#define LEW_JSON_H

#include <string.h>


#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

enum
{
	COLUMN_NUM,
	COLUMN_ENG,
	COLUMN_RUS,
	COLUMN_STATUS,
	NUM_COLUMNS
};

void lew_create_json_root (void);

guint lew_get_dict_len (void);

gboolean lew_read_json_file  (gchar *filename, GtkListStore *model);
gboolean lew_write_json_file (const gchar *filename);

JsonNode * lew_create_new_translation (const gchar *english, const gchar *russian);

gboolean lew_form_translation_edit (GtkWidget     *window,
                                    GtkTreeView   *treeview,
                                    gboolean       isNewTranslation);

#endif
