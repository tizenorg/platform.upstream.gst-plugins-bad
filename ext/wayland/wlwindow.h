/* GStreamer Wayland video sink
 *
 * Copyright (C) 2014 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef __GST_WL_WINDOW_H__
#define __GST_WL_WINDOW_H__

#include "wldisplay.h"
#include "wlbuffer.h"
#include <gst/video/video.h>

G_BEGIN_DECLS
#define GST_TYPE_WL_WINDOW                  (gst_wl_window_get_type ())
#define GST_WL_WINDOW(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_WL_WINDOW, GstWlWindow))
#define GST_IS_WL_WINDOW(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_WL_WINDOW))
#define GST_WL_WINDOW_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_WL_WINDOW, GstWlWindowClass))
#define GST_IS_WL_WINDOW_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_WL_WINDOW))
#define GST_WL_WINDOW_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_WL_WINDOW, GstWlWindowClass))
typedef struct _GstWlWindow GstWlWindow;
typedef struct _GstWlWindowClass GstWlWindowClass;

struct _GstWlWindow
{
  GObject parent_instance;

  GstWlDisplay *display;
  struct wl_surface *area_surface;
  struct wl_subsurface *area_subsurface;
  struct wl_surface *video_surface;
  struct wl_subsurface *video_subsurface;
  struct wl_shell_surface *shell_surface;
#ifndef GST_WLSINK_ENHANCEMENT /* no define */
  struct wl_viewport *video_viewport;
  struct wl_viewport *area_viewport;
#else
  struct tizen_video_object *video_object;
  struct tizen_viewport *tizen_area_viewport;
  struct tizen_viewport *tizen_video_viewport;
  guint video_info_changed;
/*Display geometry method */
  guint buffer_width, buffer_height;
  guint buffer_x, buffer_y;
  guint disp_geo_method;
  guint rotate_angle;
  guint orientation;
  guint flip;
  guint mode_ratio_w;
  guint mode_ratio_h;
  guint mode_scale_w;
  guint mode_scale_h;
  guint mode_align_w;
  guint mode_align_h;
#endif

  /* the size and position of the area_(sub)surface */
  GstVideoRectangle render_rectangle;
  /* the size of the video in the buffers */
  gint video_width, video_height;
  /* the size of the video_(sub)surface */
  gint surface_width, surface_height;
};

struct _GstWlWindowClass
{
  GObjectClass parent_class;
};

GType gst_wl_window_get_type (void);

GstWlWindow *gst_wl_window_new_toplevel (GstWlDisplay * display,
    const GstVideoInfo * info);
GstWlWindow *gst_wl_window_new_in_surface (GstWlDisplay * display,
    struct wl_surface *parent);

GstWlDisplay *gst_wl_window_get_display (GstWlWindow * window);
struct wl_surface *gst_wl_window_get_wl_surface (GstWlWindow * window);
gboolean gst_wl_window_is_toplevel (GstWlWindow * window);

void gst_wl_window_render (GstWlWindow * window, GstWlBuffer * buffer,
    const GstVideoInfo * info);
void gst_wl_window_set_render_rectangle (GstWlWindow * window, gint x, gint y,
    gint w, gint h);
#ifdef GST_WLSINK_ENHANCEMENT
void gst_wl_window_set_video_info (GstWlWindow * window,
    const GstVideoInfo * info);
void gst_wl_window_set_rotate_angle (GstWlWindow * window, guint rotate_angle);
void gst_wl_window_set_disp_geo_method (GstWlWindow * window,
    guint disp_geo_method);
void gst_wl_window_set_orientation (GstWlWindow * window, guint orientation);
void gst_wl_window_set_flip (GstWlWindow * window, guint flip);
void gst_wl_window_set_video_info_change (GstWlWindow * window, guint changed);
#endif

G_END_DECLS
#endif /* __GST_WL_WINDOW_H__ */
