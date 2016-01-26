/* GStreamer Wayland video sink
 *
 * Copyright (C) 2011 Intel Corporation
 * Copyright (C) 2011 Sreerenj Balachandran <sreerenj.balachandran@intel.com>
 * Copyright (C) 2012 Wim Taymans <wim.taymans@gmail.com>
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

/**
 * SECTION:element-waylandsink
 *
 *  The waylandsink is creating its own window and render the decoded video frames to that.
 *  Setup the Wayland environment as described in
 *  <ulink url="http://wayland.freedesktop.org/building.html">Wayland</ulink> home page.
 *  The current implementaion is based on weston compositor.
 *
 * <refsect2>
 * <title>Example pipelines</title>
 * |[
 * gst-launch -v videotestsrc ! waylandsink
 * ]| test the video rendering in wayland
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gstwaylandsink.h"
#ifdef GST_WLSINK_ENHANCEMENT
#include <mm_types.h>
#endif
#include "wlvideoformat.h"
#include "wlbuffer.h"
#include "wlshmallocator.h"

#include <gst/wayland/wayland.h>
#include <gst/video/videooverlay.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

//#include "libsec-video-set-display.h"


//#define DUMP_BUFFER
#ifdef GST_WLSINK_ENHANCEMENT
#define GST_TYPE_WAYLANDSINK_DISPLAY_GEOMETRY_METHOD (gst_waylandsink_display_geometry_method_get_type())
#define GST_TYPE_WAYLANDSINK_ROTATE_ANGLE (gst_waylandsink_rotate_angle_get_type())
#define GST_TYPE_WAYLANDSINK_FLIP (gst_waylandsink_flip_get_type())


typedef struct
{
//  void *v4l2_ptr;
  int src_input_width;
  int src_input_height;
  int src_input_x;
  int src_input_y;
  int result_width;
  int result_height;
  int result_x;
  int result_y;
  int stereoscopic_info;
  int sw_hres;
  int sw_vres;
  int max_hres;
  int max_vres;
} wl_meta;

typedef struct _Set_plane
{
  void *v4l2_ptr;
  int src_input_width;
  int src_input_height;
  int src_input_x;
  int src_input_y;
  int result_width;
  int result_height;
  int result_x;
  int result_y;
  int stereoscopic_info;
  int sw_hres;
  int sw_vres;
  int max_hres;
  int max_vres;
} Set_plane;


static GType
gst_waylandsink_rotate_angle_get_type (void)
{
  static GType waylandsink_rotate_angle_type = 0;
  static const GEnumValue rotate_angle_type[] = {
    {0, "No rotate", "DEGREE_0"},
    {1, "Rotate 90 degree", "DEGREE_90"},
    {2, "Rotate 180 degree", "DEGREE_180"},
    {3, "Rotate 270 degree", "DEGREE_270"},
    {4, NULL, NULL},
  };

  if (!waylandsink_rotate_angle_type) {
    waylandsink_rotate_angle_type =
        g_enum_register_static ("GstWaylandSinkRotateAngleType",
        rotate_angle_type);
  }

  return waylandsink_rotate_angle_type;
}


static GType
gst_waylandsink_display_geometry_method_get_type (void)
{
  static GType waylandsink_display_geometry_method_type = 0;
  static const GEnumValue display_geometry_method_type[] = {
    {0, "Letter box", "LETTER_BOX"},
    {1, "Origin size", "ORIGIN_SIZE"},
    {2, "Full-screen", "FULL_SCREEN"},
    {3, "Cropped full-screen", "CROPPED_FULL_SCREEN"},
    {4, "Origin size(if screen size is larger than video size(width/height)) or Letter box(if video size(width/height) is larger than screen size)", "ORIGIN_SIZE_OR_LETTER_BOX"},
    {5, NULL, NULL},
  };

  if (!waylandsink_display_geometry_method_type) {
    waylandsink_display_geometry_method_type =
        g_enum_register_static ("GstWaylandSinkDisplayGeometryMethodType",
        display_geometry_method_type);
  }
  return waylandsink_display_geometry_method_type;
}

static GType
gst_waylandsink_flip_get_type (void)
{
  static GType waylandsink_flip_type = 0;
  static const GEnumValue flip_type[] = {
    {FLIP_NONE, "Flip NONE", "FLIP_NONE"},
    {FLIP_HORIZONTAL, "Flip HORIZONTAL", "FLIP_HORIZONTAL"},
    {FLIP_VERTICAL, "Flip VERTICAL", "FLIP_VERTICAL"},
    {FLIP_BOTH, "Flip BOTH", "FLIP_BOTH"},
    {FLIP_NUM, NULL, NULL},
  };

  if (!waylandsink_flip_type) {
    waylandsink_flip_type =
        g_enum_register_static ("GstWaylandSinkFlipType", flip_type);
  }

  return waylandsink_flip_type;
}

#endif

/* signals */
enum
{
  SIGNAL_0,
  LAST_SIGNAL
};

/* Properties */
enum
{
  PROP_0,
  PROP_DISPLAY,
#ifdef GST_WLSINK_ENHANCEMENT
  PROP_USE_TBM,
  PROP_ROTATE_ANGLE,
  PROP_DISPLAY_GEOMETRY_METHOD,
  PROP_ORIENTATION,
  PROP_FLIP
#endif
};
int dump__cnt = 0;

GST_DEBUG_CATEGORY (gstwayland_debug);
#define GST_CAT_DEFAULT gstwayland_debug

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE
        ("{ BGRx, BGRA, RGBx, xBGR, xRGB, RGBA, ABGR, ARGB, RGB, BGR, "
            "RGB16, BGR16, YUY2, YVYU, UYVY, AYUV, NV12, NV21, NV16, "
#ifdef GST_WLSINK_ENHANCEMENT
            "SN12, ST12, "
#endif
            "YUV9, YVU9, Y41B, I420, YV12, Y42B, v308 }"))
    );

static void gst_wayland_sink_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_wayland_sink_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_wayland_sink_finalize (GObject * object);

static GstStateChangeReturn gst_wayland_sink_change_state (GstElement * element,
    GstStateChange transition);
static void gst_wayland_sink_set_context (GstElement * element,
    GstContext * context);

static GstCaps *gst_wayland_sink_get_caps (GstBaseSink * bsink,
    GstCaps * filter);
static gboolean gst_wayland_sink_set_caps (GstBaseSink * bsink, GstCaps * caps);
static gboolean gst_wayland_sink_preroll (GstBaseSink * bsink,
    GstBuffer * buffer);
static gboolean
gst_wayland_sink_propose_allocation (GstBaseSink * bsink, GstQuery * query);
static gboolean gst_wayland_sink_render (GstBaseSink * bsink,
    GstBuffer * buffer);

/* VideoOverlay interface */
static void gst_wayland_sink_videooverlay_init (GstVideoOverlayInterface *
    iface);
static void gst_wayland_sink_set_window_handle (GstVideoOverlay * overlay,
    guintptr handle);
static void gst_wayland_sink_set_render_rectangle (GstVideoOverlay * overlay,
    gint x, gint y, gint w, gint h);
static void gst_wayland_sink_expose (GstVideoOverlay * overlay);

/* WaylandVideo interface */
static void gst_wayland_sink_waylandvideo_init (GstWaylandVideoInterface *
    iface);
static void gst_wayland_sink_begin_geometry_change (GstWaylandVideo * video);
static void gst_wayland_sink_end_geometry_change (GstWaylandVideo * video);
#ifdef GST_WLSINK_ENHANCEMENT
static void gst_wayland_sink_update_window_geometry (GstWaylandSink * sink);
static void render_last_buffer (GstWaylandSink * sink);
#endif
#define gst_wayland_sink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstWaylandSink, gst_wayland_sink, GST_TYPE_VIDEO_SINK,
    G_IMPLEMENT_INTERFACE (GST_TYPE_VIDEO_OVERLAY,
        gst_wayland_sink_videooverlay_init)
    G_IMPLEMENT_INTERFACE (GST_TYPE_WAYLAND_VIDEO,
        gst_wayland_sink_waylandvideo_init));

static void
gst_wayland_sink_class_init (GstWaylandSinkClass * klass)
{
  FUNCTION;
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;

  gobject_class->set_property = gst_wayland_sink_set_property;
  gobject_class->get_property = gst_wayland_sink_get_property;
  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_wayland_sink_finalize);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  gst_element_class_set_static_metadata (gstelement_class,
      "wayland video sink", "Sink/Video",
      "Output to wayland surface",
      "Sreerenj Balachandran <sreerenj.balachandran@intel.com>, "
      "George Kiagiadakis <george.kiagiadakis@collabora.com>");

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_wayland_sink_change_state);
  gstelement_class->set_context =
      GST_DEBUG_FUNCPTR (gst_wayland_sink_set_context);

  gstbasesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_wayland_sink_get_caps);
  gstbasesink_class->set_caps = GST_DEBUG_FUNCPTR (gst_wayland_sink_set_caps);
  gstbasesink_class->preroll = GST_DEBUG_FUNCPTR (gst_wayland_sink_preroll);
  gstbasesink_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_wayland_sink_propose_allocation);
  gstbasesink_class->render = GST_DEBUG_FUNCPTR (gst_wayland_sink_render);

  g_object_class_install_property (gobject_class, PROP_DISPLAY,
      g_param_spec_string ("display", "Wayland Display name", "Wayland "
          "display name to connect to, if not supplied via the GstContext",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#ifdef GST_WLSINK_ENHANCEMENT
  g_object_class_install_property (gobject_class, PROP_USE_TBM,
      g_param_spec_boolean ("use-tbm",
          "Use Tizen Buffer Memory insted of Shared memory",
          "When enabled, Memory is alloced by TBM insted of SHM ", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ROTATE_ANGLE,
      g_param_spec_enum ("rotate", "Rotate angle",
          "Rotate angle of display output",
          GST_TYPE_WAYLANDSINK_ROTATE_ANGLE, DEGREE_0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DISPLAY_GEOMETRY_METHOD,
      g_param_spec_enum ("display-geometry-method", "Display geometry method",
          "Geometrical method for display",
          GST_TYPE_WAYLANDSINK_DISPLAY_GEOMETRY_METHOD,
          DEF_DISPLAY_GEOMETRY_METHOD,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ORIENTATION,
      g_param_spec_enum ("orientation",
          "Orientation information used for ROI/ZOOM",
          "Orientation information for display",
          GST_TYPE_WAYLANDSINK_ROTATE_ANGLE, DEGREE_0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_FLIP,
      g_param_spec_enum ("flip", "Display flip",
          "Flip for display",
          GST_TYPE_WAYLANDSINK_FLIP, DEF_DISPLAY_FLIP,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

#endif
}

#ifdef DUMP_BUFFER
int
__write_rawdata (const char *file, const void *data, unsigned int size)
{
  FILE *fp;

  fp = fopen (file, "wb");
  if (fp == NULL)
    return -1;

  fwrite ((char *) data, sizeof (char), size, fp);
  fclose (fp);

  return 0;
}
#endif
static void
gst_wayland_sink_init (GstWaylandSink * sink)
{
  FUNCTION;
#ifdef GST_WLSINK_ENHANCEMENT
  sink->USE_TBM = TRUE;
  sink->display_geometry_method = DEF_DISPLAY_GEOMETRY_METHOD;
  sink->flip = DEF_DISPLAY_FLIP;
  sink->rotate_angle = DEGREE_0;
  sink->orientation = DEGREE_0;
#endif
  g_mutex_init (&sink->display_lock);
  g_mutex_init (&sink->render_lock);
}

static void
gst_wayland_sink_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (object);

  switch (prop_id) {
    case PROP_DISPLAY:
      GST_OBJECT_LOCK (sink);
      g_value_set_string (value, sink->display_name);
      GST_OBJECT_UNLOCK (sink);
      break;
#ifdef GST_WLSINK_ENHANCEMENT
    case PROP_USE_TBM:
      g_value_set_boolean (value, sink->USE_TBM);
      break;
    case PROP_ROTATE_ANGLE:
      g_value_set_enum (value, sink->rotate_angle);
      break;
    case PROP_DISPLAY_GEOMETRY_METHOD:
      g_value_set_enum (value, sink->display_geometry_method);
      break;
    case PROP_ORIENTATION:
      g_value_set_enum (value, sink->orientation);
      break;
    case PROP_FLIP:
      g_value_set_enum (value, sink->flip);
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_wayland_sink_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (object);

  switch (prop_id) {
    case PROP_DISPLAY:
      GST_OBJECT_LOCK (sink);
      sink->display_name = g_value_dup_string (value);
      GST_OBJECT_UNLOCK (sink);
      break;
#ifdef GST_WLSINK_ENHANCEMENT
    case PROP_USE_TBM:
      sink->USE_TBM = g_value_get_boolean (value);
      GST_LOG ("1:USE TBM 0: USE SHM set(%d)", sink->USE_TBM);
      break;
    case PROP_ROTATE_ANGLE:
      sink->rotate_angle = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "Rotate angle is set (%d)", sink->rotate_angle);
      if (sink->window) {
        gst_wl_window_set_rotate_angle (sink->window, sink->rotate_angle);
      }
      sink->video_info_changed = TRUE;
      if (GST_STATE (sink) == GST_STATE_PAUSED) {
        /*need to render last buffer */
        g_mutex_lock (&sink->render_lock);
        render_last_buffer (sink);
        g_mutex_unlock (&sink->render_lock);
      }
      break;
    case PROP_DISPLAY_GEOMETRY_METHOD:
      sink->display_geometry_method = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "Display geometry method is set (%d)",
          sink->display_geometry_method);
      if (sink->window) {
        gst_wl_window_set_disp_geo_method (sink->window,
            sink->display_geometry_method);
      }
      sink->video_info_changed = TRUE;
      if (GST_STATE (sink) == GST_STATE_PAUSED) {
        /*need to render last buffer */
        g_mutex_lock (&sink->render_lock);
        render_last_buffer (sink);
        g_mutex_unlock (&sink->render_lock);
      }
      break;
    case PROP_ORIENTATION:
      sink->orientation = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "Orientation is set (%d)", sink->orientation);
      if (sink->window) {
        gst_wl_window_set_orientation (sink->window, sink->orientation);
      }
      sink->video_info_changed = TRUE;
      if (GST_STATE (sink) == GST_STATE_PAUSED) {
        /*need to render last buffer */
        g_mutex_lock (&sink->render_lock);
        render_last_buffer (sink);
        g_mutex_unlock (&sink->render_lock);
      }
      break;
    case PROP_FLIP:
      sink->flip = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "flip is set (%d)", sink->flip);
      if (sink->flip) {
        gst_wl_window_set_flip (sink->window, sink->flip);
      }
      sink->video_info_changed = TRUE;
      if (GST_STATE (sink) == GST_STATE_PAUSED) {
        /*need to render last buffer */
        g_mutex_lock (&sink->render_lock);
        render_last_buffer (sink);
        g_mutex_unlock (&sink->render_lock);
      }
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_wayland_sink_finalize (GObject * object)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (object);

  GST_DEBUG_OBJECT (sink, "Finalizing the sink..");

  if (sink->last_buffer)
    gst_buffer_unref (sink->last_buffer);
  if (sink->display)
    g_object_unref (sink->display);
  if (sink->window)
    g_object_unref (sink->window);
  if (sink->pool)
    gst_object_unref (sink->pool);

  if (sink->display_name)
    g_free (sink->display_name);

  g_mutex_clear (&sink->display_lock);
  g_mutex_clear (&sink->render_lock);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* must be called with the display_lock */
static void
gst_wayland_sink_set_display_from_context (GstWaylandSink * sink,
    GstContext * context)
{
  FUNCTION;
  struct wl_display *display;
  GError *error = NULL;

  display = gst_wayland_display_handle_context_get_handle (context);
  sink->display = gst_wl_display_new_existing (display, FALSE, &error);

  if (error) {
    GST_ELEMENT_WARNING (sink, RESOURCE, OPEN_READ_WRITE,
        ("Could not set display handle"),
        ("Failed to use the external wayland display: '%s'", error->message));
    g_error_free (error);
  }
#ifdef GST_WLSINK_ENHANCEMENT
  sink->display->USE_TBM = sink->USE_TBM;
#endif
}

static gboolean
gst_wayland_sink_find_display (GstWaylandSink * sink)
{
  FUNCTION;
  GstQuery *query;
  GstMessage *msg;
  GstContext *context = NULL;
  GError *error = NULL;
  gboolean ret = TRUE;

  g_mutex_lock (&sink->display_lock);

  if (!sink->display) {
    /* first query upstream for the needed display handle */
    query = gst_query_new_context (GST_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE);
    if (gst_pad_peer_query (GST_VIDEO_SINK_PAD (sink), query)) {
      gst_query_parse_context (query, &context);
      gst_wayland_sink_set_display_from_context (sink, context);
    }
    gst_query_unref (query);

    if (G_LIKELY (!sink->display)) {
      /* now ask the application to set the display handle */
      msg = gst_message_new_need_context (GST_OBJECT_CAST (sink),
          GST_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE);

      g_mutex_unlock (&sink->display_lock);
      gst_element_post_message (GST_ELEMENT_CAST (sink), msg);
      /* at this point we expect gst_wayland_sink_set_context
       * to get called and fill sink->display */
      g_mutex_lock (&sink->display_lock);

      if (!sink->display) {
        /* if the application didn't set a display, let's create it ourselves */
        GST_OBJECT_LOCK (sink);
        sink->display = gst_wl_display_new (sink->display_name, &error);
        GST_OBJECT_UNLOCK (sink);

        if (error) {
          GST_ELEMENT_WARNING (sink, RESOURCE, OPEN_READ_WRITE,
              ("Could not initialise Wayland output"),
              ("Failed to create GstWlDisplay: '%s'", error->message));
          g_error_free (error);
          ret = FALSE;
        }
#ifdef GST_WLSINK_ENHANCEMENT
        sink->display->USE_TBM = sink->USE_TBM;
#endif
      }
    }
  }

  g_mutex_unlock (&sink->display_lock);

  return ret;
}

static GstStateChangeReturn
gst_wayland_sink_change_state (GstElement * element, GstStateChange transition)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_wayland_sink_find_display (sink))
        return GST_STATE_CHANGE_FAILURE;
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_buffer_replace (&sink->last_buffer, NULL);
      if (sink->window) {
        if (gst_wl_window_is_toplevel (sink->window)) {
          g_clear_object (&sink->window);
        } else {
          /* remove buffer from surface, show nothing */
          gst_wl_window_render (sink->window, NULL, NULL);
        }
      }
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      g_mutex_lock (&sink->display_lock);
      /* If we had a toplevel window, we most likely have our own connection
       * to the display too, and it is a good idea to disconnect and allow
       * potentially the application to embed us with GstVideoOverlay
       * (which requires to re-use the same display connection as the parent
       * surface). If we didn't have a toplevel window, then the display
       * connection that we have is definitely shared with the application
       * and it's better to keep it around (together with the window handle)
       * to avoid requesting them again from the application if/when we are
       * restarted (GstVideoOverlay behaves like that in other sinks)
       */
      if (sink->display && !sink->window) {     /* -> the window was toplevel */
        g_clear_object (&sink->display);
      }
      g_mutex_unlock (&sink->display_lock);
      g_clear_object (&sink->pool);
      break;
    default:
      break;
  }

  return ret;
}

static void
gst_wayland_sink_set_context (GstElement * element, GstContext * context)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (element);

  if (gst_context_has_context_type (context,
          GST_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE)) {
    g_mutex_lock (&sink->display_lock);
    if (G_LIKELY (!sink->display))
      gst_wayland_sink_set_display_from_context (sink, context);
    else {
      GST_WARNING_OBJECT (element, "changing display handle is not supported");
#ifdef GST_WLSINK_ENHANCEMENT
      g_mutex_unlock (&sink->display_lock);
      return;
#endif
    }
    g_mutex_unlock (&sink->display_lock);
  }

  if (GST_ELEMENT_CLASS (parent_class)->set_context)
    GST_ELEMENT_CLASS (parent_class)->set_context (element, context);
}

static GstCaps *
gst_wayland_sink_get_caps (GstBaseSink * bsink, GstCaps * filter)
{
  FUNCTION;
  GstWaylandSink *sink;
  GstCaps *caps;

  sink = GST_WAYLAND_SINK (bsink);

  caps = gst_pad_get_pad_template_caps (GST_VIDEO_SINK_PAD (sink));

  g_mutex_lock (&sink->display_lock);

  if (sink->display) {
    GValue list = G_VALUE_INIT;
    GValue value = G_VALUE_INIT;
    GArray *formats;
    gint i;
#ifdef GST_WLSINK_ENHANCEMENT
    uint32_t tbm_fmt;
#endif
    enum wl_shm_format fmt;

    g_value_init (&list, GST_TYPE_LIST);
    g_value_init (&value, G_TYPE_STRING);
#ifdef GST_WLSINK_ENHANCEMENT
    if (sink->display->USE_TBM)
      formats = sink->display->tbm_formats;
    else                        /* SHM */
#endif
      formats = sink->display->formats;

    for (i = 0; i < formats->len; i++) {
#ifdef GST_WLSINK_ENHANCEMENT
      if (sink->USE_TBM) {
        tbm_fmt = g_array_index (formats, uint32_t, i);
        g_value_set_string (&value, gst_wl_tbm_format_to_string (tbm_fmt));
        gst_value_list_append_value (&list, &value);
        /* TBM doesn't support SN12. So we add SN12 manually as supported format.
         * SN12 is exactly same with NV12.
         */
        if (tbm_fmt == TBM_FORMAT_NV12) {
          g_value_set_string (&value,
              gst_video_format_to_string (GST_VIDEO_FORMAT_SN12));
          gst_value_list_append_value (&list, &value);
        }
      } else {                  /* USE SHM */
        fmt = g_array_index (formats, uint32_t, i);
        g_value_set_string (&value, gst_wl_shm_format_to_string (fmt));
        gst_value_list_append_value (&list, &value);
      }
#else /* open source */
      fmt = g_array_index (formats, uint32_t, i);
      g_value_set_string (&value, gst_wl_shm_format_to_string (fmt));
      gst_value_list_append_value (&list, &value);
#endif
    }

    caps = gst_caps_make_writable (caps);
    gst_structure_set_value (gst_caps_get_structure (caps, 0), "format", &list);

    GST_DEBUG_OBJECT (sink, "display caps: %" GST_PTR_FORMAT, caps);
  }

  g_mutex_unlock (&sink->display_lock);

  if (filter) {
    GstCaps *intersection;

    intersection =
        gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
    caps = intersection;
  }

  return caps;
}

static gboolean
gst_wayland_sink_set_caps (GstBaseSink * bsink, GstCaps * caps)
{
  FUNCTION;
  GstWaylandSink *sink;
  GstBufferPool *newpool;
  GstVideoInfo info;
#ifdef GST_WLSINK_ENHANCEMENT
  uint32_t tbm_format;
#endif
  enum wl_shm_format format;

  GArray *formats;
  gint i;
  GstStructure *structure;

  sink = GST_WAYLAND_SINK (bsink);

  GST_DEBUG_OBJECT (sink, "set caps %" GST_PTR_FORMAT, caps);

  /* extract info from caps */
  if (!gst_video_info_from_caps (&info, caps))
    goto invalid_format;
#ifdef GST_WLSINK_ENHANCEMENT
  sink->caps = gst_caps_copy (caps);
  if (sink->USE_TBM) {
    tbm_format =
        gst_video_format_to_wl_tbm_format (GST_VIDEO_INFO_FORMAT (&info));
    if ((gint) tbm_format == -1)
      goto invalid_format;
  } else {
    format = gst_video_format_to_wl_shm_format (GST_VIDEO_INFO_FORMAT (&info));
    if ((gint) format == -1)
      goto invalid_format;
  }
#else /* open source */
  format = gst_video_format_to_wl_shm_format (GST_VIDEO_INFO_FORMAT (&info));

  if ((gint) format == -1)
    goto invalid_format;
#endif

  /* verify we support the requested format */
#ifdef GST_WLSINK_ENHANCEMENT
  if (sink->display->USE_TBM) {
    GST_ERROR ("USE TBM FORMAT");
    formats = sink->display->tbm_formats;
    for (i = 0; i < formats->len; i++) {
      if (g_array_index (formats, uint32_t, i) == tbm_format)
        break;
    }
  } else {                      /* USE SHM */
    GST_ERROR ("USE SHM FORMAT");
    formats = sink->display->formats;
    for (i = 0; i < formats->len; i++) {
      if (g_array_index (formats, uint32_t, i) == format)
        break;
    }
  }
#else /* open source */

  formats = sink->display->formats;
  for (i = 0; i < formats->len; i++) {
    if (g_array_index (formats, uint32_t, i) == format)
      break;
  }
#endif
  if (i >= formats->len)
    goto unsupported_format;

#ifdef GST_WLSINK_ENHANCEMENT
  if (sink->USE_TBM) {
    if (GST_VIDEO_INFO_FORMAT (&info) == GST_VIDEO_FORMAT_SN12 ||
        GST_VIDEO_INFO_FORMAT (&info) == GST_VIDEO_FORMAT_ST12) {
      sink->display->is_native_format = TRUE;

      /* store the video info */
      sink->video_info = info;
      sink->video_info_changed = TRUE;
    } else {
      sink->display->is_native_format = FALSE;
      GstWlShmAllocator *self =
          GST_WL_SHM_ALLOCATOR (gst_wl_shm_allocator_get ());
      self->display = sink->display;
      /* create a new pool for the new configuration */
      newpool = gst_video_buffer_pool_new ();
      if (!newpool)
        goto pool_failed;

      structure = gst_buffer_pool_get_config (newpool);
      gst_buffer_pool_config_set_params (structure, caps, info.size, 6, 0);
      gst_buffer_pool_config_set_allocator (structure,
          gst_wl_shm_allocator_get (), NULL);
      if (!gst_buffer_pool_set_config (newpool, structure))
        goto config_failed;

      /* store the video info */
      sink->video_info = info;
      sink->video_info_changed = TRUE;

      gst_object_replace ((GstObject **) & sink->pool, (GstObject *) newpool);
      gst_object_unref (newpool);
    }
  } else {                      /* USE SHM */

    GstWlShmAllocator *self =
        GST_WL_SHM_ALLOCATOR (gst_wl_shm_allocator_get ());
    self->display = sink->display;

    /* create a new pool for the new configuration */
    newpool = gst_video_buffer_pool_new ();
    if (!newpool)
      goto pool_failed;

    structure = gst_buffer_pool_get_config (newpool);
    gst_buffer_pool_config_set_params (structure, caps, info.size, 6, 0);
    gst_buffer_pool_config_set_allocator (structure,
        gst_wl_shm_allocator_get (), NULL);
    if (!gst_buffer_pool_set_config (newpool, structure))
      goto config_failed;

    /* store the video info */
    sink->video_info = info;
    sink->video_info_changed = TRUE;

    gst_object_replace ((GstObject **) & sink->pool, (GstObject *) newpool);
    gst_object_unref (newpool);
  }
#else /*open source */
  /* create a new pool for the new configuration */
  newpool = gst_video_buffer_pool_new ();
  if (!newpool)
    goto pool_failed;

  structure = gst_buffer_pool_get_config (newpool);
  gst_buffer_pool_config_set_params (structure, caps, info.size, 6, 0);
  gst_buffer_pool_config_set_allocator (structure,
      gst_wl_shm_allocator_get (), NULL);
  if (!gst_buffer_pool_set_config (newpool, structure))
    goto config_failed;

  /* store the video info */
  sink->video_info = info;
  sink->video_info_changed = TRUE;

  gst_object_replace ((GstObject **) & sink->pool, (GstObject *) newpool);
  gst_object_unref (newpool);
#endif

  return TRUE;

invalid_format:
  {
    GST_DEBUG_OBJECT (sink,
        "Could not locate image format from caps %" GST_PTR_FORMAT, caps);
    return FALSE;
  }
unsupported_format:
  {
#ifdef GST_WLSINK_ENHANCEMENT
    if (sink->USE_TBM)
      GST_DEBUG_OBJECT (sink, "Format %s is not available on the display",
          gst_wl_tbm_format_to_string (tbm_format));
    else                        /*USE SHM */
      GST_DEBUG_OBJECT (sink, "Format %s is not available on the display",
          gst_wl_shm_format_to_string (format));
#else /*open source */
    GST_DEBUG_OBJECT (sink, "Format %s is not available on the display",
        gst_wl_shm_format_to_string (format));
#endif
    return FALSE;
  }
pool_failed:
  {
    GST_DEBUG_OBJECT (sink, "Failed to create new pool");
    return FALSE;
  }
config_failed:
  {
    GST_DEBUG_OBJECT (bsink, "failed setting config");
    gst_object_unref (newpool);
    return FALSE;
  }
}

static gboolean
gst_wayland_sink_propose_allocation (GstBaseSink * bsink, GstQuery * query)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (bsink);
  GstStructure *config;
  guint size, min_bufs, max_bufs;
#ifdef GST_WLSINK_ENHANCEMENT
  gboolean need_pool;
  GstCaps *caps;
  if (sink->USE_TBM) {
    if (sink->display->is_native_format == TRUE)
      return TRUE;

    gst_query_parse_allocation (query, &caps, &need_pool);

    if (caps == NULL) {
      GST_DEBUG_OBJECT (bsink, "no caps specified");
      return FALSE;
    }
  }
#endif
  config = gst_buffer_pool_get_config (sink->pool);
  gst_buffer_pool_config_get_params (config, NULL, &size, &min_bufs, &max_bufs);

  /* we do have a pool for sure (created in set_caps),
   * so let's propose it anyway, but also propose the allocator on its own */
  gst_query_add_allocation_pool (query, sink->pool, size, min_bufs, max_bufs);
  gst_query_add_allocation_param (query, gst_wl_shm_allocator_get (), NULL);

  gst_structure_free (config);

  return TRUE;
}

static GstFlowReturn
gst_wayland_sink_preroll (GstBaseSink * bsink, GstBuffer * buffer)
{
  FUNCTION;
  GST_DEBUG_OBJECT (bsink, "preroll buffer %p", buffer);
  return gst_wayland_sink_render (bsink, buffer);
}

static void
frame_redraw_callback (void *data, struct wl_callback *callback, uint32_t time)
{
  FUNCTION;
  GstWaylandSink *sink = data;

  GST_LOG ("frame_redraw_cb");

  g_atomic_int_set (&sink->redraw_pending, FALSE);
  wl_callback_destroy (callback);
}

static const struct wl_callback_listener frame_callback_listener = {
  frame_redraw_callback
};

#ifdef GST_WLSINK_ENHANCEMENT
static void
gst_wayland_sink_update_window_geometry (GstWaylandSink * sink)
{
  FUNCTION;
  g_return_if_fail (sink != NULL);
  g_return_if_fail (sink->window != NULL);

  gst_wl_window_set_rotate_angle (sink->window, sink->rotate_angle);
  gst_wl_window_set_disp_geo_method (sink->window,
      sink->display_geometry_method);
  gst_wl_window_set_orientation (sink->window, sink->orientation);
  gst_wl_window_set_flip (sink->window, sink->flip);
}
#endif
/* must be called with the render lock */
static void
render_last_buffer (GstWaylandSink * sink)
{
  FUNCTION;
  GstWlBuffer *wlbuffer;
  const GstVideoInfo *info = NULL;
  struct wl_surface *surface;
  struct wl_callback *callback;

  wlbuffer = gst_buffer_get_wl_buffer (sink->last_buffer);
  surface = gst_wl_window_get_wl_surface (sink->window);

  g_atomic_int_set (&sink->redraw_pending, TRUE);
  callback = wl_surface_frame (surface);
  /* frame_callback_listener is called when wayland-client finish rendering the wl_buffer */
  wl_callback_add_listener (callback, &frame_callback_listener, sink);

  if (G_UNLIKELY (sink->video_info_changed)) {
    info = &sink->video_info;
    sink->video_info_changed = FALSE;
  }
#ifdef GST_WLSINK_ENHANCEMENT
  if (sink->last_buffer)
    gst_wl_window_render (sink->window, wlbuffer, info);
  else {
    if (G_UNLIKELY (info)) {
      gst_wl_window_set_video_info (sink->window, &info);
    }
  }
#else
  gst_wl_window_render (sink->window, wlbuffer, info);
#endif
}

#ifdef GST_WLSINK_ENHANCEMENT
void
_add_meta_data (GstWlWindow * window, wl_meta * meta)
{
  FUNCTION;
  GstVideoRectangle src = { 0, };
  GstVideoRectangle res;

  /* center the video_subsurface inside area_subsurface */
  src.w = window->video_width;
  src.h = window->video_height;

  GstVideoRectangle src_origin = { 0, 0, 0, 0 };
  GstVideoRectangle src_input = { 0, 0, 0, 0 };
  GstVideoRectangle dst = { 0, 0, 0, 0 };

  gint transform = WL_OUTPUT_TRANSFORM_NORMAL;

  src.x = src.y = 0;
  src_input.w = src_origin.w = window->video_width;
  src_input.h = src_origin.h = window->video_height;
  GST_INFO ("video (%d x %d)", window->video_width, window->video_height);
  GST_INFO ("src_input(%d, %d, %d x %d)", src_input.x, src_input.y, src_input.w,
      src_input.h);
  GST_INFO ("src_origin(%d, %d, %d x %d)", src_origin.x, src_origin.y,
      src_origin.w, src_origin.h);

  if (window->rotate_angle == DEGREE_0 || window->rotate_angle == DEGREE_180) {
    src.w = window->video_width;        //video_width
    src.h = window->video_height;       //video_height
  } else {
    src.w = window->video_height;
    src.h = window->video_width;
  }
  GST_INFO ("src(%d, %d, %d x %d)", src.x, src.y, src.w, src.h);

  /*default res.w and res.h */
  dst.w = window->render_rectangle.w;
  dst.h = window->render_rectangle.h;
  GST_INFO ("dst(%d,%d,%d x %d)", dst.x, dst.y, dst.w, dst.h);
  GST_INFO ("window->render_rectangle(%d,%d,%d x %d)",
      window->render_rectangle.x, window->render_rectangle.y,
      window->render_rectangle.w, window->render_rectangle.h);
  switch (window->disp_geo_method) {
    case DISP_GEO_METHOD_LETTER_BOX:
      GST_INFO ("DISP_GEO_METHOD_LETTER_BOX");
      gst_video_sink_center_rect (src, dst, &res, TRUE);
      res.x += window->render_rectangle.x;
      res.y += window->render_rectangle.y;
      break;
    case DISP_GEO_METHOD_ORIGIN_SIZE_OR_LETTER_BOX:
      if (src.w > dst.w || src.h > dst.h) {
        /*LETTER BOX */
        GST_INFO
            ("DISP_GEO_METHOD_ORIGIN_SIZE_OR_LETTER_BOX -> set LETTER BOX");
        gst_video_sink_center_rect (src, dst, &res, TRUE);
        res.x += window->render_rectangle.x;
        res.y += window->render_rectangle.y;
      } else {
        /*ORIGIN SIZE */
        GST_INFO ("DISP_GEO_METHOD_ORIGIN_SIZE");
        gst_video_sink_center_rect (src, dst, &res, FALSE);
        gst_video_sink_center_rect (dst, src, &src_input, FALSE);
      }
      break;
    case DISP_GEO_METHOD_ORIGIN_SIZE:  //is working
      GST_INFO ("DISP_GEO_METHOD_ORIGIN_SIZE");
      gst_video_sink_center_rect (src, dst, &res, FALSE);
      gst_video_sink_center_rect (dst, src, &src_input, FALSE);
      break;
    case DISP_GEO_METHOD_FULL_SCREEN:  //is working
      GST_INFO ("DISP_GEO_METHOD_FULL_SCREEN");
      res.x = res.y = 0;
      res.w = window->render_rectangle.w;
      res.h = window->render_rectangle.h;
      break;
    case DISP_GEO_METHOD_CROPPED_FULL_SCREEN:
      GST_INFO ("DISP_GEO_METHOD_CROPPED_FULL_SCREEN");
      gst_video_sink_center_rect (src, dst, &res, FALSE);
      gst_video_sink_center_rect (dst, src, &src_input, FALSE);
      res.x = res.y = 0;
      res.w = dst.w;
      res.h = dst.h;
      break;
    default:
      break;
  }

  GST_INFO
      ("window[%d x %d] src[%d,%d,%d x %d],dst[%d,%d,%d x %d],input[%d,%d,%d x %d],result[%d,%d,%d x %d]",
      window->render_rectangle.w, window->render_rectangle.h,
      src.x, src.y, src.w, src.h,
      dst.x, dst.y, dst.w, dst.h,
      src_input.x, src_input.y, src_input.w, src_input.h,
      res.x, res.y, res.w, res.h);

  meta->src_input_width = src_input.w;
  meta->src_input_height = src_input.h;
  meta->src_input_x = src_input.x;
  meta->src_input_y = src_input.y;
  meta->result_x = res.x;
  meta->result_y = res.y;
  meta->result_width = res.w;
  meta->result_height = res.h;
  /*in case of S/W codec, we need to set sw_hres and sw_vres value.
     in case of H/W codec, these value are not used */
  meta->sw_hres = src.w;
  meta->sw_vres = src.h;
  meta->max_hres = src.w;
  meta->max_vres = src.h;

}


struct wl_buffer *
gst_wayland_sink_create_wlbuffer_with_meta (GstWaylandSink * sink)
{
  FUNCTION;

  wl_meta *meta;
  tbm_bo_handle virtual_addr;
  tbm_surface_info_s ts_info;
  struct wl_buffer *wbuffer;
  GstWlDisplay *display;
  GstVideoInfo info;
  int num_bo = 1;

  int idx = 0;
  info = sink->video_info;
  display = sink->display;
  display->tbm_bufmgr = wayland_tbm_client_get_bufmgr (display->tbm_client);
  g_return_if_fail (display->tbm_bufmgr != NULL);
  display->tbm_bo[idx] =
      tbm_bo_alloc (display->tbm_bufmgr, sizeof (wl_meta), TBM_BO_DEFAULT);
  if (!display->tbm_bo[idx]) {
    GST_ERROR ("alloc tbm bo(size:%d) failed: %s", sizeof (wl_meta),
        strerror (errno));
    return NULL;
  }

  virtual_addr.ptr = NULL;
  virtual_addr = tbm_bo_get_handle (display->tbm_bo[idx], TBM_DEVICE_CPU);
  if (!virtual_addr.ptr) {
    GST_ERROR ("get tbm bo handle failed: %s", strerror (errno));
    tbm_bo_unref (display->tbm_bo[idx]);
    display->tbm_bo[idx] = NULL;
    return NULL;
  }
  memset (virtual_addr.ptr, 0, sizeof (wl_meta));
  meta = (wl_meta *) malloc (sizeof (wl_meta));
  g_return_if_fail (meta != NULL);

  /*fill meta */
  _add_meta_data (sink->window, meta);
  memcpy (virtual_addr.ptr, meta, sizeof (meta));

  //create wl_buffer
  ts_info.width = GST_VIDEO_INFO_WIDTH (&info);
  ts_info.height = GST_VIDEO_INFO_HEIGHT (&info);
  ts_info.format = GST_VIDEO_INFO_FORMAT (&info);
  ts_info.bpp = tbm_surface_internal_get_bpp (ts_info.format);
  ts_info.num_planes = tbm_surface_internal_get_num_planes (ts_info.format);
  ts_info.planes[0].stride = GST_VIDEO_INFO_PLANE_STRIDE (&info, 0);
  ts_info.planes[1].stride = GST_VIDEO_INFO_PLANE_STRIDE (&info, 1);
  ts_info.planes[2].stride = GST_VIDEO_INFO_PLANE_STRIDE (&info, 2);
  ts_info.planes[0].offset = GST_VIDEO_INFO_PLANE_OFFSET (&info, 0);
  ts_info.planes[1].offset = GST_VIDEO_INFO_PLANE_OFFSET (&info, 1);
  ts_info.planes[2].offset = GST_VIDEO_INFO_PLANE_OFFSET (&info, 2);
  display->tsurface =
      tbm_surface_internal_create_with_bos (&ts_info, &display->tbm_bo[idx],
      num_bo);
  wbuffer =
      wayland_tbm_client_create_buffer (display->tbm_client, display->tsurface);

  return wbuffer;
}


static GstFlowReturn
gst_wayland_sink_send_meta (GstWaylandSink * sink, GstBuffer * buffer)
{
  FUNCTION;

  GstWlBuffer *wlbuffer;
  wlbuffer = gst_buffer_get_wl_buffer (buffer);
  if (G_LIKELY (wlbuffer && wlbuffer->display == sink->display)) {
    GST_ERROR ("buffer %p has a wl_buffer from our display", buffer);
    GST_ERROR ("We must get codec buffer");
  } else {
    GstMemory *mem;
    struct wl_buffer *wbuf = NULL;
    mem = gst_buffer_peek_memory (buffer, 0);
    if (gst_is_wl_shm_memory (mem)) {
      GST_ERROR ("buffer %p has not a wl_buffer from our display", buffer);
      GST_ERROR ("We must get codec buffer");
    } else {
      /* this buffer is made by codec */

      struct wl_buffer *wbuf = NULL;
      /* create wlbuffer */
      wbuf = gst_wayland_sink_create_wlbuffer_with_meta (sink);
      if (G_UNLIKELY (!wbuf)) {
        GST_ERROR ("could not create wl_buffer out of wl_shm memory");
        return GST_FLOW_ERROR;
      }

      gst_buffer_add_wl_buffer (buffer, wbuf, sink->display);
    }
  }
  if (G_UNLIKELY (buffer == sink->last_buffer)) {
    GST_LOG_OBJECT (sink, "Buffer already being rendered");
    return GST_FLOW_OK;
  }
  gst_buffer_replace (&sink->last_buffer, buffer);
  render_last_buffer (sink);

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_wayland_sink_send_buf (GstWaylandSink * sink, GstBuffer * buffer)
{
  FUNCTION;

  Set_plane set_plane;
  gint buffer_type = 0;
  gint scaler = 0;
  guint data_size;
  guint video_width;
  guint video_height;
  GstStructure *structure;
  void *data;
  guint align = 15;

#if 1
  data_size = sink->video_info.size;
  data = g_malloc (data_size + align);

  gst_buffer_extract (buffer, 0, data, MIN (gst_buffer_get_size (buffer),
          data_size));


  /*if we need meta for set_plane, we can fill meta, don't worry */
  set_plane.v4l2_ptr = data;
  //set_plane.v4l2_ptr = (struct v4l2_drm *) direct_videosink->data; // build error : need to check v412_drm 

  /* send data to libsec */
  //libsec_setplane (&set_plane, buffer_type, scaler);

#else /* we get below code from directvideosink !!!! */
  video_width = GST_VIDEO_INFO_WIDTH (&sink->video_info);
  video_height = GST_VIDEO_INFO_HEIGHT (&sink->video_info);

  /* Get max_hres, max_vres from caps */
  structure = gst_caps_get_structure (caps, 0);
  gst_structure_get_int (structure, "maxwidth", &set_plane.max_hres);
  gst_structure_get_int (structure, "maxheight", &set_plane.max_vres);

  if (set_plane.max_hres < video_width)
    set_plane.max_hres = video_width;
  if (set_plane.max_vres < video_height)
    set_plane.max_vres = video_height;
  GST_DEBUG ("caps: maxwidth = %d, maxheight = %d", set_plane.max_hres,
      set_plane.max_vres);

  data_size = sink->video_info.size;
  data = g_malloc (data_size + align);

  gst_buffer_extract (buffer, 0, data, MIN (gst_buffer_get_size (buffer),
          data_size));

  set_plane.v4l2_ptr = data;
  //set_plane.v4l2_ptr = (struct v4l2_drm *) direct_videosink->data; // build error : need to check v412_drm 
  set_plane.result_height = 0;
  set_plane.result_width = 0;
  set_plane.result_x = 0;
  set_plane.result_y = 0;
  set_plane.src_input_height = video_height;
  set_plane.src_input_width = video_width;
  set_plane.src_input_x = 0;
  set_plane.src_input_y = 0;
  set_plane.max_hres = 0;
  //set_plane.max_vres = 0;
  set_plane.sw_hres = 0;
  set_plane.sw_vres = 0;

  //libsec_setplane (&set_plane, buffer_type, scaler);
#endif

  return GST_FLOW_OK;
}
#endif
static GstFlowReturn
gst_wayland_sink_render (GstBaseSink * bsink, GstBuffer * buffer)
{
  FUNCTION;

  GstWaylandSink *sink = GST_WAYLAND_SINK (bsink);
  GstBuffer *to_render;
  GstWlBuffer *wlbuffer;
  GstFlowReturn ret = GST_FLOW_OK;
  g_mutex_lock (&sink->render_lock);

  GST_LOG_OBJECT (sink, "render buffer %p", buffer);

  if (G_UNLIKELY (!sink->window)) {
    /* ask for window handle. Unlock render_lock while doing that because
     * set_window_handle & friends will lock it in this context */
    g_mutex_unlock (&sink->render_lock);
    gst_video_overlay_prepare_window_handle (GST_VIDEO_OVERLAY (sink));
    g_mutex_lock (&sink->render_lock);

    if (!sink->window) {
      /* if we were not provided a window, create one ourselves */
      sink->window =
          gst_wl_window_new_toplevel (sink->display, &sink->video_info);
    }
  }
  /* make sure that the application has called set_render_rectangle() */
  if (G_UNLIKELY (sink->window->render_rectangle.w == 0))
    return GST_FLOW_ERROR;

  if (sink->video_info_changed == TRUE) {
    /*update geometry values from sink to window */
    gst_wayland_sink_update_window_geometry (sink);
  }

  if (sink->video_info_changed == TRUE) {
    ret = gst_wayland_sink_send_meta (sink, buffer);
    if (ret != GST_FLOW_OK)
      return ret;
  }
  ret = gst_wayland_sink_send_buf (sink, buffer);

  g_mutex_unlock (&sink->render_lock);

  return ret;
}

static void
gst_wayland_sink_videooverlay_init (GstVideoOverlayInterface * iface)
{
  iface->set_window_handle = gst_wayland_sink_set_window_handle;
  iface->set_render_rectangle = gst_wayland_sink_set_render_rectangle;
  iface->expose = gst_wayland_sink_expose;
}

static void
gst_wayland_sink_set_window_handle (GstVideoOverlay * overlay, guintptr handle)
{
  FUNCTION;
#if GST_WLSINK_ENHANCEMENT      /* use  unique_id */
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);
  g_return_if_fail (sink != NULL);

  if (sink->window != NULL) {
    GST_WARNING_OBJECT (sink, "changing window handle is not supported");
    return;
  }
  g_mutex_lock (&sink->render_lock);
  g_clear_object (&sink->window);

  GST_INFO ("parent_id %d %p", (int) handle, handle);

  if (handle) {
    if (G_LIKELY (gst_wayland_sink_find_display (sink))) {
      /* we cannot use our own display with an external window handle */
      if (G_UNLIKELY (sink->display->own_display)) {
        sink->display->parent_id = (int) handle;
        GST_INFO ("parent_id %d", sink->display->parent_id);
        //GST_DEBUG_OBJECT (sink, "Setting parent id %d", handle);
        sink->window = gst_wl_window_new_in_surface (sink->display, NULL);
      }
    } else {
      GST_ERROR_OBJECT (sink, "Failed to find display handle, "
          "ignoring window handle");
    }
  }
  gst_wayland_sink_update_window_geometry (sink);

  g_mutex_unlock (&sink->render_lock);


#else
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);
  struct wl_surface *surface = (struct wl_surface *) handle;

  g_return_if_fail (sink != NULL);

  g_mutex_lock (&sink->render_lock);

  GST_DEBUG_OBJECT (sink, "Setting window handle %" GST_PTR_FORMAT,
      (void *) handle);

  g_clear_object (&sink->window);

  if (handle) {
    if (G_LIKELY (gst_wayland_sink_find_display (sink))) {
      /* we cannot use our own display with an external window handle */
      if (G_UNLIKELY (sink->display->own_display)) {
        GST_ELEMENT_ERROR (sink, RESOURCE, OPEN_READ_WRITE,
            ("waylandsink cannot use an externally-supplied surface without "
                "an externally-supplied display handle. Consider providing a "
                "display handle from your application with GstContext"));
      } else {
        sink->window = gst_wl_window_new_in_surface (sink->display, surface);
      }
    } else {
      GST_ERROR_OBJECT (sink, "Failed to find display handle, "
          "ignoring window handle");
    }
  }
  g_mutex_unlock (&sink->render_lock);

#endif /* use  unique_id */
}

static void
gst_wayland_sink_set_render_rectangle (GstVideoOverlay * overlay,
    gint x, gint y, gint w, gint h)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);

  g_return_if_fail (sink != NULL);

  g_mutex_lock (&sink->render_lock);
  if (!sink->window) {
    g_mutex_unlock (&sink->render_lock);
    GST_WARNING_OBJECT (sink,
        "set_render_rectangle called without window, ignoring");
    return;
  }

  GST_DEBUG_OBJECT (sink, "window geometry changed to (%d, %d) %d x %d",
      x, y, w, h);
  gst_wl_window_set_render_rectangle (sink->window, x, y, w, h);

  g_mutex_unlock (&sink->render_lock);
}

static void
gst_wayland_sink_expose (GstVideoOverlay * overlay)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);

  g_return_if_fail (sink != NULL);

  GST_DEBUG_OBJECT (sink, "expose");

  g_mutex_lock (&sink->render_lock);
  if (sink->last_buffer && g_atomic_int_get (&sink->redraw_pending) == FALSE) {
    GST_DEBUG_OBJECT (sink, "redrawing last buffer");
    render_last_buffer (sink);
  }
  g_mutex_unlock (&sink->render_lock);
}

static void
gst_wayland_sink_waylandvideo_init (GstWaylandVideoInterface * iface)
{
  iface->begin_geometry_change = gst_wayland_sink_begin_geometry_change;
  iface->end_geometry_change = gst_wayland_sink_end_geometry_change;
}

static void
gst_wayland_sink_begin_geometry_change (GstWaylandVideo * video)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (video);
  g_return_if_fail (sink != NULL);

  g_mutex_lock (&sink->render_lock);
  if (!sink->window || !sink->window->area_subsurface) {
    g_mutex_unlock (&sink->render_lock);
    GST_INFO_OBJECT (sink,
        "begin_geometry_change called without window, ignoring");
    return;
  }

  wl_subsurface_set_sync (sink->window->area_subsurface);
  g_mutex_unlock (&sink->render_lock);
}

static void
gst_wayland_sink_end_geometry_change (GstWaylandVideo * video)
{
  FUNCTION;
  GstWaylandSink *sink = GST_WAYLAND_SINK (video);
  g_return_if_fail (sink != NULL);

  g_mutex_lock (&sink->render_lock);
  if (!sink->window || !sink->window->area_subsurface) {
    g_mutex_unlock (&sink->render_lock);
    GST_INFO_OBJECT (sink,
        "end_geometry_change called without window, ignoring");
    return;
  }

  wl_subsurface_set_desync (sink->window->area_subsurface);
  g_mutex_unlock (&sink->render_lock);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gstwayland_debug, "waylandsink", 0,
      " wayland video sink");

  gst_wl_shm_allocator_register ();

  return gst_element_register (plugin, "waylandsink", GST_RANK_MARGINAL,
      GST_TYPE_WAYLAND_SINK);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    waylandsink,
    "Wayland Video Sink", plugin_init, VERSION, "LGPL", GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN)
