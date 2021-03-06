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

#ifndef __GST_WL_DISPLAY_H__
#define __GST_WL_DISPLAY_H__

#include <gst/gst.h>
#include <wayland-client.h>
#include "scaler-client-protocol.h"
#ifdef GST_WLSINK_ENHANCEMENT
#include <tbm_bufmgr.h>
#include <wayland-tbm-client.h>
#include <tizen-extension-client-protocol.h>
#define NV_BUF_PLANE_NUM    2   /*SN12 or ST12 has 2 plane */
#define USE_WL_FLUSH_BUFFER
#endif

G_BEGIN_DECLS
#define GST_TYPE_WL_DISPLAY                  (gst_wl_display_get_type ())
#define GST_WL_DISPLAY(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_WL_DISPLAY, GstWlDisplay))
#define GST_IS_WL_DISPLAY(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_WL_DISPLAY))
#define GST_WL_DISPLAY_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_WL_DISPLAY, GstWlDisplayClass))
#define GST_IS_WL_DISPLAY_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_WL_DISPLAY))
#define GST_WL_DISPLAY_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_WL_DISPLAY, GstWlDisplayClass))
#define FUNCTION GST_INFO ("<ENTER>")
typedef struct _GstWlDisplay GstWlDisplay;
typedef struct _GstWlDisplayClass GstWlDisplayClass;

#ifdef USE_WL_FLUSH_BUFFER
typedef struct {
  void *bo[NV_BUF_PLANE_NUM];
}GstWlFlushBuffer;
#endif

#define TBM_BO_NUM 20

struct _GstWlDisplay
{
  GObject parent_instance;

  /* public objects */
  struct wl_display *display;
  struct wl_event_queue *queue;

  /* globals */
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_subcompositor *subcompositor;
  struct wl_shell *shell;
  struct wl_shm *shm;
  struct wl_scaler *scaler;
  GArray *formats;
#ifdef GST_WLSINK_ENHANCEMENT
  GArray *tbm_formats;
#endif
  /* private */
  gboolean own_display;
  GThread *thread;
  GstPoll *wl_fd_poll;

  GMutex buffers_mutex;
  GHashTable *buffers;
  gboolean shutting_down;

#ifdef GST_WLSINK_ENHANCEMENT
  /*video output layer */
  struct tizen_policy *tizen_policy;
  struct tizen_video *tizen_video;

  struct wayland_tbm_client *tbm_client;
  tbm_bufmgr tbm_bufmgr;
  tbm_bo tbm_bo[TBM_BO_NUM];
  int tbm_bo_idx;
  tbm_surface_h tsurface;
  gboolean USE_TBM;

#ifdef USE_WL_FLUSH_BUFFER
  GstWlFlushBuffer *flush_buffer;
  tbm_bufmgr flush_tbm_bufmgr;
  int flush_request;
#endif

  gboolean is_native_format;    /*SN12, ST12 */
  void *bo[NV_BUF_PLANE_NUM];
  int plane_size[NV_BUF_PLANE_NUM];
  int stride_width[NV_BUF_PLANE_NUM];
  int stride_height[NV_BUF_PLANE_NUM];
  int native_video_size;
  unsigned int wl_surface_id;
#endif

#if 1
  gboolean need_shell_surface;
  gboolean use_parent_wl_surface;
#endif
};

struct _GstWlDisplayClass
{
  GObjectClass parent_class;
};

GType gst_wl_display_get_type (void);

GstWlDisplay *gst_wl_display_new (const gchar * name, GError ** error);
GstWlDisplay *gst_wl_display_new_existing (struct wl_display *display,
    gboolean take_ownership, GError ** error);

/* see wlbuffer.c for explanation */
void gst_wl_display_register_buffer (GstWlDisplay * self, gpointer buf);
void gst_wl_display_unregister_buffer (GstWlDisplay * self, gpointer buf);

G_END_DECLS
#endif /* __GST_WL_DISPLAY_H__ */
