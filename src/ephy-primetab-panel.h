/* SPDX-License-Identifier: GPL-3.0-or-later */

#pragma once

#include <gtk/gtk.h>

gboolean ephy_primetab_panel_get_slot   (GtkWidget *window,
                                         int       *left,
                                         int       *right,
                                         int       *height);
void     ephy_primetab_panel_set_height (GtkWidget *window,
                                        int        height,
                                        gboolean   ready);
