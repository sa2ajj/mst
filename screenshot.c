#include <stdio.h>

#include <gtk/gtk.h>

#include "screenshot.h"

gboolean get_screenshot(GdkPixbufSaveFunc save_func, gpointer data, GError **error) {
	gboolean ret = FALSE;

	GdkDrawable *root_window;
	int width, height;
	GdkPixbuf *pixbuf;

	root_window = gdk_get_default_root_window();

	if (root_window != 0) {
		gdk_drawable_get_size(root_window, &width, &height);

		pixbuf = gdk_pixbuf_get_from_drawable(NULL,
							root_window,
							gdk_drawable_get_colormap(root_window),
							0, 0,
							0, 0,
							width, height);

		if (pixbuf != 0) {
			ret = gdk_pixbuf_save_to_callback(pixbuf, save_func, data, "png", error, NULL);

			g_object_unref (pixbuf);
		} else {
			fprintf(stderr, "Unable to get pixbuf\n");
		}
	} else {
		fprintf(stderr, "Unable to get root window\n");
	}

	return ret;
}
