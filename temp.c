int createDictFile( const char *dictFile );

int rand_range_correct( int range );
char* read_line_from_stdin( char* buf );
int translate_cmp(const char *translate, const char *russian);
size_t stripWhiteSpace(char** s);
int word_cmp (char* word1, char* word2);


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
