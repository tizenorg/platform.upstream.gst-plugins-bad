/*
 * GStreamer Wayland video sink
 * Copyright (C) 2011 Intel Corporation
 * Copyright (C) 2011 Sreerenj Balachandran <sreerenj.balachandran@intel.com>
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

#ifndef __GST_WAYLAND_VIDEO_SINK_H__
#define __GST_WAYLAND_VIDEO_SINK_H__

#include <gst/gst.h>
#include <gst/video/video.h>

#include <wayland-client.h>

#include "wldisplay.h"
#include "wlwindow.h"

G_BEGIN_DECLS
#define GST_TYPE_WAYLAND_SINK \
	    (gst_wayland_sink_get_type())
#define GST_WAYLAND_SINK(obj) \
	    (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_WAYLAND_SINK,GstWaylandSink))
#define GST_WAYLAND_SINK_CLASS(klass) \
	    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_WAYLAND_SINK,GstWaylandSinkClass))
#define GST_IS_WAYLAND_SINK(obj) \
	    (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_WAYLAND_SINK))
#define GST_IS_WAYLAND_SINK_CLASS(klass) \
	    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_WAYLAND_SINK))
#define GST_WAYLAND_SINK_GET_CLASS(inst) \
        (G_TYPE_INSTANCE_GET_CLASS ((inst), GST_TYPE_WAYLAND_SINK, GstWaylandSinkClass))
#ifdef GST_WLSINK_ENHANCEMENT
enum
{
  DISP_GEO_METHOD_LETTER_BOX = 0,
  DISP_GEO_METHOD_ORIGIN_SIZE,
  DISP_GEO_METHOD_FULL_SCREEN,
  DISP_GEO_METHOD_CROPPED_FULL_SCREEN,
  DISP_GEO_METHOD_ORIGIN_SIZE_OR_LETTER_BOX,
  DISP_GEO_METHOD_NUM,
};

enum
{
  DEGREE_0,
  DEGREE_90,
  DEGREE_180,
  DEGREE_270,
  DEGREE_NUM,
};

enum
{
  FLIP_NONE = 0,
  FLIP_HORIZONTAL,
  FLIP_VERTICAL,
  FLIP_BOTH,
  FLIP_NUM,
};

#define DEF_DISPLAY_FLIP            FLIP_NONE
#define DEF_DISPLAY_GEOMETRY_METHOD         DISP_GEO_METHOD_LETTER_BOX
#endif

typedef struct _GstWaylandSink GstWaylandSink;
typedef struct _GstWaylandSinkClass GstWaylandSinkClass;

struct _GstWaylandSink
{
  GstVideoSink parent;

  GMutex display_lock;
  GstWlDisplay *display;
  GstWlWindow *window;
  GstBufferPool *pool;

  gboolean video_info_changed;
  GstVideoInfo video_info;

  gchar *display_name;

  gboolean redraw_pending;
  GMutex render_lock;
  GstBuffer *last_buffer;
#ifdef GST_WLSINK_ENHANCEMENT
  gboolean use_gapless;
  gboolean keep_camera_preview;
  gboolean got_eos_event;
  gboolean USE_TBM;
  guint rotate_angle;
  guint display_geometry_method;
  guint orientation;
  guint flip;
  gboolean visible;

  GMutex render_flush_buffer_lock;
  GCond render_flush_buffer_cond;
#endif
};

struct _GstWaylandSinkClass
{
  GstVideoSinkClass parent;
};

GType
gst_wayland_sink_get_type (void)
    G_GNUC_CONST;

G_END_DECLS
#endif /* __GST_WAYLAND_VIDEO_SINK_H__ */
