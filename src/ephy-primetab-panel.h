/* SPDX-License-Identifier: GPL-3.0-or-later */

#pragma once

#include <gtk/gtk.h>

gboolean ephy_primetab_panel_get_slot (GtkWidget *window,
                                      int        header_height,
                                      int       *left,
                                      int       *right);
