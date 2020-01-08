#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json.h>

#ifdef _WIN32
#include <direct.h>
#include <dir.h>
#define get_cwd _getcwd
#endif
#ifdef linux
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#define get_cwd getcwd
#endif

const char *site_url = "https://moodle.udc.es/webservice/rest/server.php?moodlewsrestformat=json&";
const char *site_info_func = "wsfunction=core_webservice_get_site_info";
const char *course_list_func = "wsfunction=core_enrol_get_users_courses";
const char *course_contents_func = "wsfunction=core_course_get_contents";

/* Moodle structs */
struct moodle_element {
	const char *type;
	const char *filename;
	const char *filepath;
	int64_t filesize;
	const char *fileurl;
	int64_t timecreated;
	int64_t timemodified;
};

struct moodle_module {
	int id;
	const char *name;
	struct moodle_element *elements;
	size_t el_len;
};

struct moodle_category {
	int id;
	const char* name;
	struct moodle_module *modules;
	size_t mod_len;
};

struct moodle_course {
	int id;
	const char *shortname;
	const char *fullname;
	const char *idnumber;
	struct moodle_category *categories;
	size_t cat_len;
};

struct moodle_course_data {
	int userid;
	struct moodle_course *courses;
	size_t len;
};

/* CURL methods */
struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writestr(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

CURL *curl_init() {
	CURL *curl;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
	return curl;
}

void curl_cleanup(CURL *curl) {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

CURLcode http_get(CURL *curl, char *url, struct string *s) {
	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writestr);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

	res = curl_easy_perform(curl);
	if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));
	return res;
}

/* Moodle API functions */

int parse_user_id(char *res) {
	struct json_object *jres, *juserid;
	jres = json_tokener_parse(res);
	juserid = json_object_object_get(jres, "userid");
	return json_object_get_int(juserid);
}

int get_user_id(CURL *curl, char* token) {
	CURLcode res;
	struct string s;
	char urlstr[160]; // The url is 155 chars long
	int userid;

	// Prepare request URL with token
	strcpy(urlstr, site_url);
  	strcat(urlstr, site_info_func);
	strcat(urlstr, "&wstoken=");
	strcat(urlstr, token);

	// Do HTTP request
	init_string(&s);
    res = http_get(curl, urlstr, &s);
    if(res != CURLE_OK)
      fprintf(stderr, "request failed: %s\n", curl_easy_strerror(res));

	// Parse json
	userid = parse_user_id(s.ptr);
    free(s.ptr);
	return userid;
}

size_t parse_user_courses(struct moodle_course_data *courses, char *res) {
	struct json_object *jres, *jcourse;
	struct moodle_course *c;
	int i;

	// Parse json and allocate memory
	jres = json_tokener_parse(res);
	courses->len = json_object_array_length(jres);
	courses->courses = malloc(sizeof(struct moodle_course) * courses->len);

	for (i = 0; i < courses->len; i++) {
		jcourse = json_object_array_get_idx(jres, i);
		c = &courses->courses[i];
		c->id        = json_object_get_int(json_object_object_get(jcourse, "id"));
		c->shortname = json_object_get_string(json_object_object_get(jcourse, "shortname"));
		c->fullname  = json_object_get_string(json_object_object_get(jcourse, "fullname"));
		c->idnumber  = json_object_get_string(json_object_object_get(jcourse, "idnumber"));
	}
	//printf("jobj from str:\n---\n%s\n---\n", json_object_to_json_string_ext(jres, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
	return courses->len;
}

void get_user_courses(CURL *curl, struct moodle_course_data *courses, char* token) {
	CURLcode res;
	struct string s;
	char urlstr[160]; // The url is 155 chars long
	int urlstrlen;

	// Prepare request URL with token
	strcpy(urlstr, site_url);
  	strcat(urlstr, course_list_func);
	urlstrlen = strlen(urlstr);
	snprintf(urlstr+urlstrlen, 16, "&userid=%d", courses->userid);
	strcat(urlstr, "&wstoken=");
	strcat(urlstr, token);

	// Do HTTP request
	init_string(&s);
    res = http_get(curl, urlstr, &s);
    if(res != CURLE_OK)
      fprintf(stderr, "request failed: %s\n", curl_easy_strerror(res));

	// Parse json
	parse_user_courses(courses, s.ptr);
    free(s.ptr);
}

void parse_course_files(struct moodle_course *course, char *res) {
	struct json_object     *jres, *jcat, *jmodlist, *jmod, *jellist, *jel;
	struct moodle_category *cat;
	struct moodle_module   *mod;
	struct moodle_element  *el;
	int i, j, k;

	// Parse json and allocate memory
	jres = json_tokener_parse(res);

	// Categories
	course->cat_len = json_object_array_length(jres);
	course->categories = malloc(sizeof(struct moodle_category) * course->cat_len);
	for (i = 0; i < course->cat_len; i++) {
		jcat = json_object_array_get_idx(jres, i);
		cat = &course->categories[i];
		cat->id   = json_object_get_int(json_object_object_get(jcat, "id"));
		cat->name = json_object_get_string(json_object_object_get(jcat, "name"));
		
		// Modules
		jmodlist = json_object_object_get(jcat, "modules");
		cat->mod_len = json_object_array_length(jmodlist);
		cat->modules = malloc(sizeof(struct moodle_module) * cat->mod_len);
		for (j = 0; j < cat->mod_len; j++) {
			jmod = json_object_array_get_idx(jmodlist, j);
			mod = &cat->modules[j];
			mod->id   = json_object_get_int(json_object_object_get(jmod, "id"));
			mod->name = json_object_get_string(json_object_object_get(jmod, "name"));

			// Elements
			jellist = json_object_object_get(jmod, "contents");
			if (jellist != NULL) {
				mod->el_len = json_object_array_length(jellist);
				mod->elements = malloc(sizeof(struct moodle_element) * mod->el_len);
				for (k = 0; k < mod->el_len; k++) {
					jel = json_object_array_get_idx(jellist, k);
					el = &mod->elements[k];
					el->type         = json_object_get_string(json_object_object_get(jel, "type"));
					el->filename     = json_object_get_string(json_object_object_get(jel, "filename"));
					el->filepath     = json_object_get_string(json_object_object_get(jel, "filepath"));
					el->filesize     = json_object_get_int64 (json_object_object_get(jel, "filesize"));
					el->fileurl      = json_object_get_string(json_object_object_get(jel, "fileurl"));
					el->timecreated  = json_object_get_int64 (json_object_object_get(jel, "timecreated"));
					el->timemodified = json_object_get_int64 (json_object_object_get(jel, "timemodified"));
				}
			} else {
				mod->el_len = 0;
			}
		}
	}
}

void get_course_files(CURL *curl, struct moodle_course *course, char* token) {
	CURLcode res;
	struct string s;
	char urlstr[180]; // The url is 175 chars long
	int urlstrlen;

	// Prepare request URL with token
	strcpy(urlstr, site_url);
  	strcat(urlstr, course_contents_func);
	urlstrlen = strlen(urlstr);
	snprintf(urlstr+urlstrlen, 16, "&courseid=%d", course->id);
	strcat(urlstr, "&wstoken=");
	strcat(urlstr, token);

	// Do HTTP request
	init_string(&s);
    res = http_get(curl, urlstr, &s);
    if(res != CURLE_OK)
      fprintf(stderr, "request failed: %s\n", curl_easy_strerror(res));

	// Parse json
	parse_course_files(course, s.ptr);
    free(s.ptr);
}

void free_user_courses(struct moodle_course_data *courses) {
	int i, j, k;
	struct moodle_course   *course;
	struct moodle_category *category;
	struct moodle_module   *module;

	// For each course
	for (i = 0; i < courses->len; i++) {
		course = &courses->courses[i];

		// For each category
		for (j = 0; j < course->cat_len; j++) {
			category = &course->categories[j];

			// For each module			
			for (k = 0; k < category->mod_len; k++) {
				module = &category->modules[k];

				// Free elements
				if (module->el_len > 0)
					free(module->elements);
			}
			// Free modules
			free(category->modules);
		}
		// Free category list
		free(course->categories);
	}
	// Free course list
	free(courses->courses);
}

void print_files(struct moodle_course_data *courses) {
	int i, j, k, l;
	struct moodle_course   *course;
	struct moodle_category *category;
	struct moodle_module   *module;
	struct moodle_element  *element;

	// For each course
	for (i = 0; i < courses->len; i++) {
		course = &courses->courses[i];

		// For each category
		for (j = 0; j < course->cat_len; j++) {
			category = &course->categories[j];

			// For each module			
			for (k = 0; k < category->mod_len; k++) {
				module = &category->modules[k];
				for(l = 0; l < module->el_len; l++) {
					element = &module->elements[l];
					if (element != NULL) {
						printf("file: %s\n", element->fileurl);
					}
				}
			}
		}
	}
}

void get_course_data(CURL *curl, struct moodle_course_data *courses, char *token) {
	int i;

	courses->userid = get_user_id(curl, token);
	get_user_courses(curl, courses, token);
	for (i = 0; i < courses->len; i++) {
		get_course_files(curl, &courses->courses[i], token);
	}
}

int create_dir(char *dirpath) {
#ifdef _WIN32
	// TODO
	if (!mkdir(dir)) {
		fprintf(stderr, "Error creating directory %s\n", dir);
		return 1;
	}
	return 0;
#endif
#ifdef linux
	DIR *dir = opendir(dirpath);
	if (dir) {
		closedir(dir);
		return 0;
	} else if (ENOENT == errno) {
		if (mkdir(dirpath, 0755)) {
			fprintf(stderr, "Error creating directory %s: %s\n", dirpath, strerror(errno));
			return 1;
		}
	} else {
		fprintf(stderr, "Error creating directory %s: %s\n", dirpath, strerror(errno));
	}
	return 0;
#endif
}

void sanitize_path(char *path) {
	int i = 0;
	while (path[i] != '\0') {
		if (path[i] == '/')
			path[i] = '_';
		i++;
	}
}

void download_file(CURL *curl, const char *filename, const char *fileurl) {
	FILE *file;
	
	file = fopen(filename, "wb");
	if (file) {
		curl_easy_setopt(curl, CURLOPT_URL, fileurl);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	
		curl_easy_perform(curl);
	
		fclose(file);
	} else {
		fprintf(stderr, "failed to download file: %s", filename);
	}
}

int save_files(CURL *curl, struct moodle_course_data *courses, char *token) {
	int i, j, k, l;
	struct moodle_course   *course;
	struct moodle_category *category;
	struct moodle_module   *module;
	struct moodle_element  *element;
	char tempbuf[FILENAME_MAX];
	char basedir[FILENAME_MAX];
	char coursedir[FILENAME_MAX];
	char categorydir[FILENAME_MAX];
	//char moduledir[FILENAME_MAX];
	char filename[FILENAME_MAX];

	// Create base directory
	get_cwd(basedir, FILENAME_MAX);
	strcat(basedir, "/files");
	if (create_dir(basedir))
		return EXIT_FAILURE;
	printf("Saving files to: %s\n", basedir);

	// For each course
	for (i = 0; i < courses->len; i++) {
		course = &courses->courses[i];
		printf("## %s ##\n", course->fullname);
		strcpy(coursedir, basedir);
		strcat(coursedir, "/");
		strcpy(tempbuf, course->idnumber);
		sanitize_path(tempbuf);
		strcat(coursedir, tempbuf);
		if (create_dir(coursedir))
			return EXIT_FAILURE;

		// For each category
		for (j = 0; j < course->cat_len; j++) {
			category = &course->categories[j];
			strcpy(categorydir, coursedir);
			strcat(categorydir, "/");
			strcpy(tempbuf, category->name);
			sanitize_path(tempbuf);
			strcat(categorydir, tempbuf);
			if (create_dir(categorydir))
				return EXIT_FAILURE;

			// For each module			
			for (k = 0; k < category->mod_len; k++) {
				module = &category->modules[k];
				/*strcpy(moduledir, categorydir);
				strcat(moduledir, "/");
				strcpy(tempbuf, module->name);
				sanitize_path(tempbuf);
				strcat(moduledir, tempbuf);
				if (create_dir(moduledir))
					return EXIT_FAILURE;*/

				// For each element
				for(l = 0; l < module->el_len; l++) {
					element = &module->elements[l];
					if (element != NULL) {
						strcpy(filename, categorydir);
						strcat(filename, "/");
						strcpy(tempbuf, element->filename);
						sanitize_path(tempbuf);
						strcat(filename, tempbuf);
						strcpy(tempbuf, element->fileurl);
						strcat(tempbuf, "&token=");
						strcat(tempbuf, token);
						printf("%s (%s)\n", module->name, element->filename);
						download_file(curl, filename, tempbuf);
					}
				}
			}
		}
	}
	return EXIT_SUCCESS;
}

int download_all(char *token) {
  CURL *curl = curl_init();
  struct moodle_course_data data;
  int res = EXIT_FAILURE;

  if(curl) {
	  get_course_data(curl, &data, token);
	  res = save_files(curl, &data, token);
	  free_user_courses(&data);
  }
  curl_cleanup(curl);
  return res;
}

int main(int argc, char **argv) {
  if (argc != 2) {
	fprintf(stderr, "Wrong arguments. Usage: <program> <token>\n");
	return EXIT_FAILURE;
  }
  return download_all(argv[1]);
}
