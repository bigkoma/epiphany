/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "config.h"
#include "ephy-primetab-panel.h"

#if HAVE_PRIMETAB_X11
#include <gdk/x11/gdkx.h>
#include <X11/Xatom.h>

static gboolean
read_values (GdkDisplay    *gdk_display,
             Window         window,
             Atom           name,
             Atom           expected_type,
             unsigned long *values,
             unsigned long  length)
{
  Display *display = gdk_x11_display_get_xdisplay (gdk_display);
  Atom type;
  int format;
  unsigned long count, remaining;
  unsigned char *data = NULL;
  gboolean valid;
  int status, xerror;

  gdk_x11_display_error_trap_push (gdk_display);
  status = XGetWindowProperty (display, window, name, 0, length, False,
                               expected_type, &type, &format, &count, &remaining, &data);
  xerror = gdk_x11_display_error_trap_pop (gdk_display);

  valid = !xerror && status == Success && type == expected_type &&
          format == 32 && count == length && remaining == 0 && data;
  if (valid)
    memcpy (values, data, length * sizeof (unsigned long));
  if (data)
    XFree (data);
  return valid;
}

gboolean
ephy_primetab_panel_get_slot (GtkWidget *window,
                             int        header_height,
                             int       *left,
                             int       *right)
{
  GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
  GdkDisplay *gdk_display;
  Display *display;
  Window xid;
  Window dock = None;
  Atom height_atom;
  unsigned long height;
  unsigned long slot[3];
  int scale;

  if (!surface || !GDK_IS_X11_SURFACE (surface))
    return FALSE;

  gdk_display = gdk_surface_get_display (surface);
  display = gdk_x11_display_get_xdisplay (gdk_display);
  xid = gdk_x11_surface_get_xid (surface);
  scale = gtk_widget_get_scale_factor (window);
  height_atom = XInternAtom (display, "_PRIMETAB_BROWSER_BAR_V1", False);

  if (header_height > 0) {
    height = header_height * scale;
    XChangeProperty (display, xid, height_atom, XA_CARDINAL, 32, PropModeReplace,
                     (const unsigned char *)&height, 1);
  } else {
    XDeleteProperty (display, xid, height_atom);
    return FALSE;
  }

  if (!read_values (gdk_display, DefaultRootWindow (display),
                    XInternAtom (display, "_PRIMETAB_WINGPANEL_V1", False), XA_WINDOW, &dock, 1) ||
      !dock ||
      !read_values (gdk_display, dock, XInternAtom (display, "_PRIMETAB_BROWSER_SLOT_V1", False),
                    XA_CARDINAL, slot, G_N_ELEMENTS (slot)) ||
      slot[0] != xid ||
      slot[1] > 1024 * (unsigned long)scale || slot[2] > 1024 * (unsigned long)scale ||
      (slot[1] + slot[2]) / scale + 700 > (unsigned long)gtk_widget_get_width (window))
    return FALSE;

  *left = slot[1] / scale;
  *right = slot[2] / scale;
  return TRUE;
}
#else
gboolean
ephy_primetab_panel_get_slot (GtkWidget *window,
                             int        header_height,
                             int       *left,
                             int       *right)
{
  return FALSE;
}
#endif
