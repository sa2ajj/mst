#ifndef __SCREENSHOT_H__
#define __SCREENSHOT_H__

#include <gdk/gdkpixbuf.h>
#include <glib/gerror.h>

  gboolean get_screenshot(GdkPixbufSaveFunc save_func, gpointer data, GError **error);
 
#endif
