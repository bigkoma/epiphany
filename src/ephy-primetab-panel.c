/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "config.h"
#include "ephy-primetab-panel.h"

#if HAVE_PRIMETAB_X11
#include <string.h>

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
                             int       *left,
                             int       *right,
                             int       *height)
{
  GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
  GdkDisplay *gdk_display;
  Display *display;
  Window dock = None;
  unsigned long slot[3];
  unsigned long scale;

  if (!surface || !GDK_IS_X11_SURFACE (surface))
    return FALSE;

  gdk_display = gdk_surface_get_display (surface);
  display = gdk_x11_display_get_xdisplay (gdk_display);
  scale = gtk_widget_get_scale_factor (window);

  if (!read_values (gdk_display, DefaultRootWindow (display),
                    XInternAtom (display, "_PRIMETAB_WINGPANEL_V1", False), XA_WINDOW, &dock, 1) ||
      !dock ||
      !read_values (gdk_display, dock, XInternAtom (display, "_PRIMETAB_BROWSER_SLOT_V2", False),
                    XA_CARDINAL, slot, G_N_ELEMENTS (slot)) ||
      slot[0] > 1024 * scale || slot[1] > 1024 * scale ||
      slot[2] < 24 * scale || slot[2] > 64 * scale ||
      (slot[0] + slot[1]) / scale + 700 > (unsigned long)gtk_widget_get_width (window))
    return FALSE;

  *left = slot[0] / scale;
  *right = slot[1] / scale;
  *height = slot[2] / scale;
  return TRUE;
}

static void
compute_size_cb (GdkToplevel     *surface,
                 GdkToplevelSize *size,
                 GtkWindow       *window)
{
  int height = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (surface), "primetab-panel-height"));

  /* Give Gala a top frame extent for this window alone. The toolbar occupies
   * those pixels while the panel continues to reserve the normal work area. */
  if (height && gtk_window_is_maximized (window) &&
      !gtk_window_is_fullscreen (window))
    gdk_toplevel_size_set_shadow_width (size, 0, 0, height, 0);
}

void
ephy_primetab_panel_set_height (GtkWidget *window,
                                int        height,
                                gboolean   ready)
{
  GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
  int previous_height;
  unsigned long marker, previous_marker;
  Display *display;
  Window xid;
  Atom atom;

  if (!surface || !GDK_IS_X11_SURFACE (surface))
    return;

  previous_height = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (surface), "primetab-panel-height"));
  if (height != previous_height) {
    if (height && !g_object_get_data (G_OBJECT (surface), "primetab-size-hook")) {
      g_signal_connect_object (surface, "compute-size", G_CALLBACK (compute_size_cb),
                               window, G_CONNECT_AFTER);
      g_object_set_data (G_OBJECT (surface), "primetab-size-hook", GINT_TO_POINTER (1));
    }
    g_object_set_data (G_OBJECT (surface), "primetab-panel-height", GINT_TO_POINTER (height));
    gtk_widget_queue_resize (window);
  }

  marker = ready ? height * gtk_widget_get_scale_factor (window) : 0;
  previous_marker = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (surface), "primetab-panel-marker"));
  if (marker == previous_marker)
    return;

  display = gdk_x11_display_get_xdisplay (gdk_surface_get_display (surface));
  xid = gdk_x11_surface_get_xid (surface);
  atom = XInternAtom (display, "_PRIMETAB_BROWSER_BAR_V2", False);
  if (marker)
    XChangeProperty (display, xid, atom, XA_CARDINAL, 32, PropModeReplace,
                     (const unsigned char *)&marker, 1);
  else
    XDeleteProperty (display, xid, atom);
  XFlush (display);
  g_object_set_data (G_OBJECT (surface), "primetab-panel-marker", GUINT_TO_POINTER (marker));
}
#else
gboolean
ephy_primetab_panel_get_slot (GtkWidget *window,
                             int       *left,
                             int       *right,
                             int       *height)
{
  return FALSE;
}

void
ephy_primetab_panel_set_height (GtkWidget *window,
                                int        height,
                                gboolean   ready)
{
}
#endif
