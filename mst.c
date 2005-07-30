#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <dirent.h>

#include <time.h>

#include <gtk/gtk.h>
#include <glib/gfileutils.h>

#include "http.h"
#include "screenshot.h"

#define MYDATAPATH  "~/MyDocs/.images"
#define BUFSIZE     20480

struct fileType {
  char *ext, *mimetype;
} knownTypes[] = {
  { ".png", "image/png" },
  { ".jpg", "image/jpeg" },
  { ".gif", "image/gif" },
  { 0, 0 }
};

void mst_init() {
  if (!gtk_init_check(NULL, NULL)) {
    fprintf(stderr, "Unable to initialize gtk+\n");

    exit(2);
  }
}

char *expand_home(const char *fname) {
  char *result;

  if (fname != 0 && fname[0] != '\0' && fname[1] != '\0' && fname[0] == '~' && fname[1] == G_DIR_SEPARATOR) {
    result = g_build_path(G_DIR_SEPARATOR_S, g_get_home_dir(), fname + 2, NULL);
  } else {
    result = g_strdup(fname);
  }

  return result;
}

int myAlphaSort(const void *a, const void *b) {
  return strcmp((const char *)a, (const char *)b);
}

const char *getMType(const char *fname) {
  const char *type = 0;
  const char *ext = strrchr(fname, '.');

  if (ext != 0) {
    int i;

    for (i = 0 ; knownTypes[i].ext != 0 ; ++i) {
      if (strcmp(ext, knownTypes[i].ext) == 0) {
        type = knownTypes[i].mimetype;
        break;
      }
    }
  }

  return type;
}

int filter_images(const struct dirent *entry) {
  int result = 0;

  if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
    result = getMType(entry->d_name) != 0;
  }

  return result;
}

int myScanDir(const char *dname, char ***entries) {
  DIR *dir;
  int result = 0;

  dir = opendir (dname);

  if (dir != 0) {
    struct dirent *current;

    while ((current = readdir(dir)) != 0) {
      if (filter_images(current)) {
        char *item = strdup(current->d_name);

        *entries = (char **)realloc(*entries, (result + 1) * sizeof(char *));

        (*entries)[result] = item;

        ++result;
      }
    }

    if (result != 0) {
      qsort(*entries, result, sizeof(struct dirent *), myAlphaSort);
    }
  }

  return result;
}

gboolean mst_http_write_callback(const gchar *buf, gsize count, GError **error, gpointer data) {
  http_response_write((HttpResponse *)data, buf, count);

  return TRUE;
}

gboolean mst_file_write_callback(const gchar *buf, gsize count, GError **error, gpointer data) {
  return fwrite(buf, 1, count, (FILE *)data) == count;
}

void mst_process_root(HttpRequest *req, HttpResponse *res, const char *command) {
  http_response_set_status(res, 200, "OK");

  if (strcmp(command, "get") == 0) {
    GError *error = 0;

    http_response_set_content_type(res, "image/png");

    get_screenshot(mst_http_write_callback, res, &error);
  } else {
    gchar *datadir;

    http_response_set_content_type(res, "text/html");
    http_response_printf(res, "<html><body><a href=\"/?save\">shot and store</a> | <a href=\"/?get\">shot and get</a><br/><hr/>");

    datadir = expand_home(MYDATAPATH);

    if (strcmp(command, "save") == 0) {
      time_t now = time(NULL);
      char fname[32];
      gchar *fullname;
      struct tm *lnow = localtime(&now);
      FILE *f;
      GError *error = 0;

      strftime(fname, 32, "%Y-%m-%d-%H-%M-%S.png", lnow);

      fullname = g_build_path(G_DIR_SEPARATOR_S, datadir, fname, NULL);

      f = fopen(fullname, "wb");

      if (!get_screenshot(mst_file_write_callback, f, &error) || error != 0) {
        const char *message = error == 0 ? "Unable to save screenshot" : error->message;

        http_response_printf(res, "<p><b>Error</b>: %s\n", message);
        http_response_printf(res, "<p>File was not created<hr/>", message);

        fclose(f);

        unlink(fullname);
      } else {
        fclose(f);
      }
    }

    {
      char **files = 0;
      int fileno = myScanDir (datadir, &files);
      int i;

      http_response_printf(res, "<ul>\n");

      for (i = 0 ; i < fileno ; ++i) {
        http_response_printf(res, "<li><a href=\"%s\">%s</a></li>\n", files[i], files[i]);
      }

      http_response_printf(res, "</ul>\n");
    }

    g_free(datadir);
  }

  http_response_printf(res, "<hr/></body></html>\n");
}

void mst_handle_http_request(HttpRequest *req) {
  HttpResponse *res = http_response_new(req);

  if (strcmp(http_request_get_method(req), "GET") != 0) {
    http_response_set_status(res, 405, "Method Not Allowed");
    http_response_printf(res, "<html><body>405 Method Not Allowed</body></html>\n");
  } else {
    const char *page = http_request_get_path_info(req); 
    const char *command  = http_request_get_query_string(req); 

    if (strcmp(page, "/") == 0) {
      mst_process_root(req, res, command);
    } else {
      char *datadir = expand_home(MYDATAPATH);
      size_t flen = strlen(datadir) + strlen(page) + 1;
      char *fname = malloc(flen), *tempo;
      const char *type;
      FILE *f = 0;

      strcpy(fname, datadir);
      strcat(fname, page);

      g_free(datadir);

      type = getMType(page);

      tempo = strstr(page, "../");      /* simplistic check if they try to get below the data dir */

      if (tempo == 0 && type != 0 && (f = fopen(fname, "rb")) != 0) {
        size_t written;

        http_response_set_status(res, 200, "OK");
        http_response_set_content_type(res, type);

        tempo = malloc(BUFSIZE);

        while((written = fread(tempo, 1, BUFSIZE, f)) > 0) {
          http_response_write(res, tempo, written);
        }

        fclose(f);

        free(tempo);
      } else {
        http_response_set_status(res, 404, "Not Found");
        http_response_printf(res, "<html><body>404 Not Found</body></html>\n");
      }
    }
  }

  http_response_send(res);
}

int main(int argc, char **argv) {
  HttpRequest  *req  = NULL;
  int           port = 10101;

  mst_init();

  if(argc > 1 && strcmp(argv[1],"debug") == 0)
    req = http_request_new(); 	/* reads request from stdin */
  else
    req = http_server("MST", port);    /* forks here */

  mst_handle_http_request(req);

  return 0;
}
