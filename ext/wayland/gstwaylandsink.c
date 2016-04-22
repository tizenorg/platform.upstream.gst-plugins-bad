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
#include "tizen-wlvideoformat.h"
#endif
#include "wlvideoformat.h"
#include "wlbuffer.h"
#include "wlshmallocator.h"

#include <gst/wayland/wayland.h>
#include <gst/video/videooverlay.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef GST_WLSINK_ENHANCEMENT
#define GST_TYPE_WAYLANDSINK_DISPLAY_GEOMETRY_METHOD (gst_waylandsink_display_geometry_method_get_type())
#define GST_TYPE_WAYLANDSINK_ROTATE_ANGLE (gst_waylandsink_rotate_angle_get_type())
#define GST_TYPE_WAYLANDSINK_FLIP (gst_waylandsink_flip_get_type())

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
  PROP_USE_GAPLESS,
  PROP_USE_TBM,
  PROP_ROTATE_ANGLE,
  PROP_DISPLAY_GEOMETRY_METHOD,
  PROP_ORIENTATION,
  PROP_FLIP,
  PROP_VISIBLE
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
static void
gst_wayland_sink_set_wl_window_wl_surface_id (GstVideoOverlay * overlay,
    guintptr wl_surface_id);
static void gst_wayland_sink_set_render_rectangle (GstVideoOverlay * overlay,
    gint x, gint y, gint w, gint h);
static void gst_wayland_sink_expose (GstVideoOverlay * overlay);

/* WaylandVideo interface */
static void gst_wayland_sink_waylandvideo_init (GstWaylandVideoInterface *
    iface);
static void gst_wayland_sink_begin_geometry_change (GstWaylandVideo * video);
static void gst_wayland_sink_end_geometry_change (GstWaylandVideo * video);
#ifdef GST_WLSINK_ENHANCEMENT
static gboolean gst_wayland_sink_event (GstBaseSink * bsink, GstEvent * event);
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
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;
  FUNCTION;

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
#ifdef GST_WLSINK_ENHANCEMENT
  gstbasesink_class->event = GST_DEBUG_FUNCPTR (gst_wayland_sink_event);
#endif

  g_object_class_install_property (gobject_class, PROP_DISPLAY,
      g_param_spec_string ("display", "Wayland Display name", "Wayland "
          "display name to connect to, if not supplied via the GstContext",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#ifdef GST_WLSINK_ENHANCEMENT
  g_object_class_install_property (gobject_class, PROP_USE_GAPLESS,
      g_param_spec_boolean ("use-gapless", "use gapless",
          "Use gapless playback on GST_STATE_PLAYING state, "
          "Last tbm buffer is copied and returned to codec immediately when enabled",
          TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_USE_TBM,
      g_param_spec_boolean ("use-tbm", "use tbm buffer",
          "Use Tizen Buffer Memory insted of Shared memory, "
          "Memory is alloced by TBM insted of SHM when enabled", TRUE,
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

  g_object_class_install_property (gobject_class, PROP_VISIBLE,
      g_param_spec_boolean ("visible", "Visible",
          "Draws screen or blacks out, true means visible, false blacks out",
          TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
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
  sink->use_gapless = TRUE;
  sink->got_eos_event = FALSE;
  sink->USE_TBM = TRUE;
  sink->display_geometry_method = DEF_DISPLAY_GEOMETRY_METHOD;
  sink->flip = DEF_DISPLAY_FLIP;
  sink->rotate_angle = DEGREE_0;
  sink->orientation = DEGREE_0;
  sink->visible = TRUE;
  g_mutex_init (&sink->gapless_lock);
  g_cond_init (&sink->gapless_cond);
#endif
  g_mutex_init (&sink->display_lock);
  g_mutex_init (&sink->render_lock);
}

#ifdef GST_WLSINK_ENHANCEMENT
static void
gst_wayland_sink_stop_video (GstWaylandSink * sink)
{
  FUNCTION;
  g_return_if_fail (sink != NULL);
  gst_wl_window_render (sink->window, NULL, NULL);
}

static int
gst_wayland_sink_need_to_make_flush_buffer (GstWaylandSink * sink)
{
  g_return_if_fail (sink != NULL);
  g_return_if_fail (sink->display != NULL);

  if ((sink->use_gapless && sink->got_eos_event)
      || sink->display->flush_request) {
    sink->display->flush_request = TRUE;
    return TRUE;
  }
  return FALSE;
}

static void
gst_wayland_sink_update_last_buffer_geometry (GstWaylandSink * sink)
{
  GstWlBuffer *wlbuffer;
  FUNCTION;
  g_return_if_fail (sink != NULL);
  GST_DEBUG ("gstbuffer ref count is %d",
      GST_OBJECT_REFCOUNT_VALUE (sink->last_buffer));
  wlbuffer = gst_buffer_get_wl_buffer (sink->last_buffer);
  wlbuffer->used_by_compositor = FALSE;
  /*need to render last buffer, reuse current GstWlBuffer */
  render_last_buffer (sink);
  /* ref count is incresed in gst_wl_buffer_attach() of render_last_buffer(),
     to call gst_wl_buffer_finalize(), we need to decrease buffer ref count.
     wayland can not release buffer if we attach same buffer,
     if we use no visible, we need to attach null buffer and wayland can release buffer,
     so we don't need to below if() code. */
  if (!sink->visible)
    gst_buffer_unref (wlbuffer->gstbuffer);
}

#ifdef USE_WL_FLUSH_BUFFER
static int
gst_wayland_sink_make_flush_buffer (GstWlDisplay * display,
    MMVideoBuffer * mm_video_buf)
{
  GstWlFlushBuffer *flush_buffer = NULL;
  tbm_bo bo = NULL;
  int bo_size = 0;
  int i;
  FUNCTION;

  g_return_val_if_fail (display != NULL, FALSE);
  g_return_val_if_fail (mm_video_buf != NULL, FALSE);

  flush_buffer = (GstWlFlushBuffer *) malloc (sizeof (GstWlFlushBuffer));
  if (!flush_buffer) {
    GST_ERROR ("GstWlFlushBuffer alloc faile");
    return FALSE;
  }
  memset (flush_buffer, 0x0, sizeof (GstWlFlushBuffer));

  display->flush_tbm_bufmgr =
      wayland_tbm_client_get_bufmgr (display->tbm_client);
  g_return_if_fail (display->flush_tbm_bufmgr != NULL);

  for (i = 0; i < NV_BUF_PLANE_NUM; i++) {
    if (mm_video_buf->handle.bo[i] != NULL) {
      tbm_bo_handle src;
      tbm_bo_handle dst;

      /* get bo size */
      bo_size = tbm_bo_size (mm_video_buf->handle.bo[i]);
      GST_LOG ("tbm bo size: %d", bo_size);
      /* alloc bo */
      bo = tbm_bo_alloc (display->flush_tbm_bufmgr, bo_size, TBM_DEVICE_CPU);
      if (!bo) {
        GST_ERROR ("alloc tbm bo(size:%d) failed: %s", bo_size,
            strerror (errno));
        return FALSE;
      }
      GST_INFO ("flush buffer tbm_bo =(%p)", bo);
      flush_buffer->bo[i] = bo;
      /* get virtual address */
      src.ptr = dst.ptr = NULL;
      /* bo map, we can use tbm_bo_map too. */
      src = tbm_bo_get_handle (mm_video_buf->handle.bo[i], TBM_DEVICE_CPU);
      dst = tbm_bo_get_handle (bo, TBM_DEVICE_CPU);
      if (!src.ptr || !dst.ptr) {
        GST_ERROR ("get tbm bo handle failed src(%p) dst(%p): %s", src.ptr,
            dst.ptr, strerror (errno));
        tbm_bo_unref (mm_video_buf->handle.bo[i]);
        tbm_bo_unref (bo);
        return FALSE;
      }
      /* copy */
      memcpy (dst.ptr, src.ptr, bo_size);
      /* bo unmap */
      tbm_bo_unmap (mm_video_buf->handle.bo[i]);
      tbm_bo_unmap (bo);
    }
  }
  display->flush_buffer = flush_buffer;
  return TRUE;
}

static int
gst_wayland_sink_copy_mm_video_buf_info_to_flush (GstWlDisplay * display,
    MMVideoBuffer * mm_video_buf)
{
  int ret = FALSE;
  g_return_val_if_fail (display != NULL, FALSE);
  g_return_val_if_fail (mm_video_buf != NULL, FALSE);
  FUNCTION;

  ret = gst_wayland_sink_make_flush_buffer (display, mm_video_buf);
  if (ret) {
    int i;
    for (i = 0; i < NV_BUF_PLANE_NUM; i++) {
      if (display->flush_buffer->bo[i] != NULL) {
        display->bo[i] = display->flush_buffer->bo[i];
        GST_LOG ("bo %p", display->bo[i]);
      } else {
        display->bo[i] = 0;
      }
      display->plane_size[i] = mm_video_buf->size[i];
      display->stride_width[i] = mm_video_buf->stride_width[i];
      display->stride_height[i] = mm_video_buf->stride_height[i];
      display->native_video_size += display->plane_size[i];
    }
  }
  return ret;
}
#endif

static void
gst_wayland_sink_add_mm_video_buf_info (GstWlDisplay * display,
    MMVideoBuffer * mm_video_buf)
{
  int i;
  g_return_if_fail (display != NULL);
  g_return_if_fail (mm_video_buf != NULL);
  FUNCTION;

  for (i = 0; i < NV_BUF_PLANE_NUM; i++) {
    if (mm_video_buf->handle.bo[i] != NULL) {
      display->bo[i] = mm_video_buf->handle.bo[i];
    } else {
      display->bo[i] = 0;
    }
    display->plane_size[i] = mm_video_buf->size[i];
    display->stride_width[i] = mm_video_buf->stride_width[i];
    display->stride_height[i] = mm_video_buf->stride_height[i];
    display->native_video_size += display->plane_size[i];
  }
}

static int
gst_wayland_sink_get_mm_video_buf_info (GstWaylandSink * sink,
    GstBuffer * buffer)
{
  GstWlDisplay *display;
  GstMemory *mem;
  GstMapInfo mem_info = GST_MAP_INFO_INIT;
  MMVideoBuffer *mm_video_buf = NULL;

  g_return_val_if_fail (sink != NULL, FALSE);
  g_return_val_if_fail (buffer != NULL, FALSE);

  FUNCTION;
  display = sink->display;
  g_return_val_if_fail (sink->display != NULL, FALSE);

  mem = gst_buffer_peek_memory (buffer, 1);
  gst_memory_map (mem, &mem_info, GST_MAP_READ);
  mm_video_buf = (MMVideoBuffer *) mem_info.data;
  gst_memory_unmap (mem, &mem_info);

  if (mm_video_buf == NULL) {
    GST_WARNING ("mm_video_buf is NULL. Skip rendering");
    return FALSE;
  }
  /* assign mm_video_buf info */
  if (mm_video_buf->type == MM_VIDEO_BUFFER_TYPE_TBM_BO) {
    GST_DEBUG ("TBM bo %p %p %p", mm_video_buf->handle.bo[0],
        mm_video_buf->handle.bo[1], mm_video_buf->handle.bo[2]);
    display->native_video_size = 0;
    display->flush_request = mm_video_buf->flush_request;
    GST_DEBUG ("flush_request value is %d", display->flush_request);
#ifdef USE_WL_FLUSH_BUFFER
    if (gst_wayland_sink_need_to_make_flush_buffer (sink)) {
      if (!gst_wayland_sink_copy_mm_video_buf_info_to_flush (display,
              mm_video_buf)) {
        GST_ERROR ("cat not copy mm_video_buf info to flush");
        return FALSE;
      }
    } else
#endif
      /* normal routine */
      gst_wayland_sink_add_mm_video_buf_info (display, mm_video_buf);
  } else {
    GST_ERROR ("Buffer type is not TBM");
    return FALSE;
  }
  return TRUE;
}

static void
gst_wayland_sink_gapless_render_flush_buffer (GstBaseSink * bsink)
{
  GstWaylandSink *sink;
  GstBuffer *buffer;
  sink = GST_WAYLAND_SINK (bsink);
  FUNCTION;
  g_return_if_fail (sink != NULL);
  g_return_if_fail (sink->last_buffer != NULL);

  buffer = gst_buffer_copy (sink->last_buffer);

  g_mutex_lock (&sink->gapless_lock);
  g_cond_wait (&sink->gapless_cond, &sink->gapless_lock);

  gst_wayland_sink_render (bsink, buffer);
  if (buffer)
    gst_buffer_unref (buffer);
  g_mutex_unlock (&sink->gapless_lock);
}
#endif

static void
gst_wayland_sink_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstWaylandSink *sink = GST_WAYLAND_SINK (object);
  FUNCTION;

  switch (prop_id) {
    case PROP_DISPLAY:
      GST_OBJECT_LOCK (sink);
      g_value_set_string (value, sink->display_name);
      GST_OBJECT_UNLOCK (sink);
      break;
#ifdef GST_WLSINK_ENHANCEMENT
    case PROP_USE_GAPLESS:
      g_value_set_boolean (value, sink->use_gapless);
      break;
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
    case PROP_VISIBLE:
      g_value_set_boolean (value, sink->visible);
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
  GstWaylandSink *sink = GST_WAYLAND_SINK (object);
  FUNCTION;
  g_mutex_lock (&sink->render_lock);

  switch (prop_id) {
    case PROP_DISPLAY:
      GST_OBJECT_LOCK (sink);
      sink->display_name = g_value_dup_string (value);
      GST_OBJECT_UNLOCK (sink);
      break;
#ifdef GST_WLSINK_ENHANCEMENT
    case PROP_USE_GAPLESS:
      sink->use_gapless = g_value_get_boolean (value);
      GST_LOG ("use gapless is (%d)", sink->use_gapless);
      break;
    case PROP_USE_TBM:
      sink->USE_TBM = g_value_get_boolean (value);
      GST_LOG ("1:USE TBM 0: USE SHM set(%d)", sink->USE_TBM);
      break;

    case PROP_ROTATE_ANGLE:
      if (sink->rotate_angle == g_value_get_enum (value))
        break;
      sink->rotate_angle = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "Rotate angle is set (%d)", sink->rotate_angle);
      sink->video_info_changed = TRUE;
      if (sink->window) {
        gst_wl_window_set_rotate_angle (sink->window, sink->rotate_angle);
      }
      break;

    case PROP_DISPLAY_GEOMETRY_METHOD:
      if (sink->display_geometry_method == g_value_get_enum (value))
        break;
      sink->display_geometry_method = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "Display geometry method is set (%d)",
          sink->display_geometry_method);
      sink->video_info_changed = TRUE;
      if (sink->window) {
        gst_wl_window_set_disp_geo_method (sink->window,
            sink->display_geometry_method);
      }
      break;

    case PROP_ORIENTATION:
      if (sink->orientation == g_value_get_enum (value))
        break;
      sink->orientation = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "Orientation is set (%d)", sink->orientation);
      sink->video_info_changed = TRUE;
      if (sink->window) {
        gst_wl_window_set_orientation (sink->window, sink->orientation);
      }
      break;

    case PROP_FLIP:
      if (sink->flip == g_value_get_enum (value))
        break;
      sink->flip = g_value_get_enum (value);
      GST_WARNING_OBJECT (sink, "flip is set (%d)", sink->flip);
      sink->video_info_changed = TRUE;
      if (sink->window) {
        gst_wl_window_set_flip (sink->window, sink->flip);
      }
      break;

    case PROP_VISIBLE:
      if (sink->visible == g_value_get_boolean (value))
        break;
      sink->visible = g_value_get_boolean (value);
      GST_WARNING_OBJECT (sink, "visible is set (%d)", sink->visible);
      if (sink->visible && GST_STATE (sink) == GST_STATE_PAUSED) {
        /* need to attatch last buffer */
        sink->video_info_changed = TRUE;
      } else if (!sink->visible && GST_STATE (sink) >= GST_STATE_PAUSED) {
        /* video stop */
        if (sink->window) {
          gst_wayland_sink_stop_video (sink);
        }
      }
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  if (sink->video_info_changed && sink->window
      && GST_STATE (sink) == GST_STATE_PAUSED) {
    gst_wayland_sink_update_last_buffer_geometry (sink);
  }
  g_mutex_unlock (&sink->render_lock);

}

static void
gst_wayland_sink_finalize (GObject * object)
{
  GstWaylandSink *sink = GST_WAYLAND_SINK (object);
  FUNCTION;
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
#ifdef GST_WLSINK_ENHANCEMENT
  g_mutex_clear (&sink->gapless_lock);
  g_cond_clear (&sink->gapless_cond);
#endif

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

#ifdef GST_WLSINK_ENHANCEMENT
static gboolean
gst_wayland_sink_event (GstBaseSink * bsink, GstEvent * event)
{
  GstWaylandSink *sink;
  sink = GST_WAYLAND_SINK (bsink);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_EOS:
      GST_LOG ("get EOS event..state is %d", GST_STATE (sink));
      if (sink->USE_TBM && sink->display->is_native_format && sink->use_gapless) {
        sink->got_eos_event = TRUE;
        gst_wayland_sink_gapless_render_flush_buffer (bsink);
        sink->got_eos_event = FALSE;
      }
      break;
    default:
      break;
  }
  return GST_BASE_SINK_CLASS (parent_class)->event (bsink, event);
}
#endif

/* must be called with the display_lock */
static void
gst_wayland_sink_set_display_from_context (GstWaylandSink * sink,
    GstContext * context)
{
  struct wl_display *display;
  GError *error = NULL;
  FUNCTION;

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
  GstQuery *query;
  GstMessage *msg;
  GstContext *context = NULL;
  GError *error = NULL;
  gboolean ret = TRUE;
  FUNCTION;

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
        if (sink->display)
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
  GstWaylandSink *sink = GST_WAYLAND_SINK (element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  FUNCTION;

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
#ifdef USE_WL_FLUSH_BUFFER
          sink->display->flush_request = 0;
#endif
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
  GstWaylandSink *sink = GST_WAYLAND_SINK (element);
  FUNCTION;

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
  GstWaylandSink *sink;
  GstCaps *caps;
  FUNCTION;

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
        /* TBM doesn't support SN12 and ST12. So we add SN12 and ST12 manually as supported format.
         * SN12 is same with NV12, ST12 is same with NV12MT
         */
        if (tbm_fmt == TBM_FORMAT_NV12) {
          g_value_set_string (&value,
              gst_video_format_to_string (GST_VIDEO_FORMAT_SN12));
          gst_value_list_append_value (&list, &value);
        } else if (tbm_fmt == TBM_FORMAT_NV12MT) {
          g_value_set_string (&value,
              gst_video_format_to_string (GST_VIDEO_FORMAT_ST12));
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
  GstWlShmAllocator *self = NULL;

  FUNCTION;

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
    GST_INFO ("USE TBM FORMAT");
    formats = sink->display->tbm_formats;
    for (i = 0; i < formats->len; i++) {
      if (g_array_index (formats, uint32_t, i) == tbm_format)
        break;
    }
  } else {                      /* USE SHM */
    GST_INFO ("USE SHM FORMAT");
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
      self = GST_WL_SHM_ALLOCATOR (gst_wl_shm_allocator_get ());
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

    self = GST_WL_SHM_ALLOCATOR (gst_wl_shm_allocator_get ());
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
  GstWaylandSink *sink = GST_WAYLAND_SINK (bsink);
  GstStructure *config;
  guint size, min_bufs, max_bufs;
#ifdef GST_WLSINK_ENHANCEMENT
  gboolean need_pool;
  GstCaps *caps;
  FUNCTION;

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
  GstWaylandSink *sink = data;
  FUNCTION;

  GST_LOG ("frame_redraw_cb");

  g_atomic_int_set (&sink->redraw_pending, FALSE);
  wl_callback_destroy (callback);
#ifdef GST_WLSINK_ENHANCEMENT
  if (sink->got_eos_event && sink->use_gapless && sink->USE_TBM
      && sink->display->is_native_format) {
    g_mutex_lock (&sink->gapless_lock);
    g_cond_signal (&sink->gapless_cond);
    g_mutex_unlock (&sink->gapless_lock);
  }
#endif
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
  GstWlBuffer *wlbuffer;
  const GstVideoInfo *info = NULL;
  struct wl_surface *surface;
  struct wl_callback *callback;
  FUNCTION;

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
      gst_wl_window_set_video_info (sink->window, info);
    }
  }
#else
  gst_wl_window_render (sink->window, wlbuffer, info);
#endif
}

static GstFlowReturn
gst_wayland_sink_render (GstBaseSink * bsink, GstBuffer * buffer)
{
  GstWaylandSink *sink = GST_WAYLAND_SINK (bsink);
  GstBuffer *to_render;
  GstWlBuffer *wlbuffer;
  GstFlowReturn ret = GST_FLOW_OK;
  FUNCTION;

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
#ifdef GST_WLSINK_ENHANCEMENT
  gst_wayland_sink_update_window_geometry (sink);
  sink->video_info_changed = TRUE;
#endif
  /* drop buffers until we get a frame callback */
  if (g_atomic_int_get (&sink->redraw_pending) == TRUE)
    goto done;
  /* make sure that the application has called set_render_rectangle() */
  if (G_UNLIKELY (sink->window->render_rectangle.w == 0))
    goto no_window_size;

#ifdef GST_WLSINK_ENHANCEMENT

  wlbuffer = gst_buffer_get_wl_buffer (buffer);
  if (G_LIKELY (wlbuffer && wlbuffer->display == sink->display)
      && !(sink->use_gapless && sink->got_eos_event)) {
    GST_LOG_OBJECT (sink, "buffer %p has a wl_buffer from our display, " "writing directly", buffer);   //buffer is from our  pool and have wl_buffer
    GST_INFO ("wl_buffer (%p)", wlbuffer->wlbuffer);
    to_render = buffer;
#ifdef DUMP_BUFFER
    GstMemory *mem;
    GstMapInfo mem_info = GST_MAP_INFO_INIT;
    int size = GST_VIDEO_INFO_SIZE (&sink->video_info);
    mem = gst_buffer_peek_memory (to_render, 0);
    gst_memory_map (mem, &mem_info, GST_MAP_READ);
    void *data;
    data = mem_info.data;
    int ret;
    char file_name[128];

    sprintf (file_name, "/home/owner/DUMP/_WLSINK_OUT_DUMP_%2.2d.dump",
        dump__cnt++);
    ret = __write_rawdata (file_name, data, size);
    if (ret) {
      GST_ERROR ("_write_rawdata() failed");
    }
    GST_INFO ("DUMP IMAGE %d, size (%d)", dump__cnt, size);
    gst_memory_unmap (mem, &mem_info);
#endif
  } else {
    GstMemory *mem;
    struct wl_buffer *wbuf = NULL;

    GST_LOG_OBJECT (sink, "buffer %p does not have a wl_buffer from our " "display, creating it", buffer);      //buffer is from our pool but have not wl_buffer
    mem = gst_buffer_peek_memory (buffer, 0);
    if (gst_is_wl_shm_memory (mem)) {
      FUNCTION;
      wbuf = gst_wl_shm_memory_construct_wl_buffer (mem, sink->display,
          &sink->video_info);
      if (wbuf) {
        gst_buffer_add_wl_buffer (buffer, wbuf, sink->display); //careat GstWlBuffer and add  gstbuffer, wlbuffer, display and etc
        to_render = buffer;
      }
    } else {                    //buffer is not from our pool and have not wl_buffer
      GstMapInfo src;
      /* we don't know how to create a wl_buffer directly from the provided
       * memory, so we have to copy the data to a memory that we know how
       * to handle... */

      GST_LOG_OBJECT (sink, "buffer %p is not from our pool", buffer);
      GST_LOG_OBJECT (sink, "buffer %p cannot have a wl_buffer, " "copying",
          buffer);

      if (sink->USE_TBM && sink->display->is_native_format) {
        /* in case of SN12 or ST12 */
        if (!gst_wayland_sink_get_mm_video_buf_info (sink, buffer))
          return GST_FLOW_ERROR;

        wlbuffer = gst_buffer_get_wl_buffer (buffer);
        if (G_UNLIKELY (!wlbuffer) || (sink->use_gapless
                && sink->got_eos_event)) {
          wbuf =
              gst_wl_shm_memory_construct_wl_buffer (mem, sink->display,
              &sink->video_info);
          if (G_UNLIKELY (!wbuf))
            goto no_wl_buffer;
          gst_buffer_add_wl_buffer (buffer, wbuf, sink->display);
        }
      } else if (sink->USE_TBM && !sink->display->is_native_format) {

        /* sink->pool always exists (created in set_caps), but it may not
         * be active if upstream is not using it */
        if (!gst_buffer_pool_is_active (sink->pool)
            && !gst_buffer_pool_set_active (sink->pool, TRUE))
          goto activate_failed;

        ret = gst_buffer_pool_acquire_buffer (sink->pool, &to_render, NULL);
        if (ret != GST_FLOW_OK)
          goto no_buffer;

        //GstMemory *mem;
        //mem = gst_buffer_peek_memory (to_render, 0);
        //if (gst_is_wl_shm_memory (mem)) {
        GST_INFO ("to_render buffer is our buffer");
        //}
        /* the first time we acquire a buffer,
         * we need to attach a wl_buffer on it */
        wlbuffer = gst_buffer_get_wl_buffer (buffer);
        if (G_UNLIKELY (!wlbuffer)) {
          mem = gst_buffer_peek_memory (to_render, 0);
          wbuf = gst_wl_shm_memory_construct_wl_buffer (mem, sink->display,
              &sink->video_info);
          if (G_UNLIKELY (!wbuf))
            goto no_wl_buffer;

          wlbuffer = gst_buffer_add_wl_buffer (to_render, wbuf, sink->display);
        }

        gst_buffer_map (buffer, &src, GST_MAP_READ);
        gst_buffer_fill (to_render, 0, src.data, src.size);
        gst_buffer_unmap (buffer, &src);
      } else {                  /* USE SHM */
        /* sink->pool always exists (created in set_caps), but it may not
         * be active if upstream is not using it */
        if (!gst_buffer_pool_is_active (sink->pool) &&
            !gst_buffer_pool_set_active (sink->pool, TRUE))
          goto activate_failed;
        ret = gst_buffer_pool_acquire_buffer (sink->pool, &to_render, NULL);
        if (ret != GST_FLOW_OK)
          goto no_buffer;
        /* the first time we acquire a buffer,
         * we need to attach a wl_buffer on it */
        wlbuffer = gst_buffer_get_wl_buffer (buffer);
        if (G_UNLIKELY (!wlbuffer)) {
          mem = gst_buffer_peek_memory (to_render, 0);
          wbuf = gst_wl_shm_memory_construct_wl_buffer (mem, sink->display,
              &sink->video_info);
          if (G_UNLIKELY (!wbuf))
            goto no_wl_buffer;

          gst_buffer_add_wl_buffer (to_render, wbuf, sink->display);

        }

        gst_buffer_map (buffer, &src, GST_MAP_READ);
        gst_buffer_fill (to_render, 0, src.data, src.size);
        gst_buffer_unmap (buffer, &src);
      }
    }
  }

  if (sink->USE_TBM && sink->display->is_native_format) {
    if (G_UNLIKELY (buffer == sink->last_buffer) && !(sink->use_gapless
            && sink->got_eos_event)) {
      GST_LOG_OBJECT (sink, "Buffer already being rendered");
      goto done;
    }
    gst_buffer_replace (&sink->last_buffer, buffer);

    if (sink->visible)
      render_last_buffer (sink);

    goto done;

  } else {                      /* USE SHM or normal format */
    /* drop double rendering */
    if (G_UNLIKELY (buffer == sink->last_buffer)) {
      GST_LOG_OBJECT (sink, "Buffer already being rendered");
      goto done;
    }
    gst_buffer_replace (&sink->last_buffer, to_render);

    if (sink->visible)
      render_last_buffer (sink);

    if (buffer != to_render)
      gst_buffer_unref (to_render);

    goto done;
  }

#else /* open source */

  wlbuffer = gst_buffer_get_wl_buffer (buffer);

  if (G_LIKELY (wlbuffer && wlbuffer->display == sink->display)) {
    GST_LOG_OBJECT (sink,
        "buffer %p has a wl_buffer from our display, " "writing directly",
        buffer);
    GST_INFO ("wl_buffer (%p)", wlbuffer->wlbuffer);
    to_render = buffer;

  } else {
    GstMemory *mem;
    struct wl_buffer *wbuf = NULL;

    GST_LOG_OBJECT (sink,
        "buffer %p does not have a wl_buffer from our " "display, creating it",
        buffer);
    mem = gst_buffer_peek_memory (buffer, 0);
    if (gst_is_wl_shm_memory (mem)) {
      FUNCTION;
      wbuf = gst_wl_shm_memory_construct_wl_buffer (mem, sink->display,
          &sink->video_info);
    }
    if (wbuf) {
      gst_buffer_add_wl_buffer (buffer, wbuf, sink->display);
      to_render = buffer;

    } else {
      GstMapInfo src;
      /* we don't know how to create a wl_buffer directly from the provided
       * memory, so we have to copy the data to a memory that we know how
       * to handle... */

      GST_LOG_OBJECT (sink, "buffer %p is not from our pool", buffer);
      GST_LOG_OBJECT (sink, "buffer %p cannot have a wl_buffer, " "copying",
          buffer);
      /* sink->pool always exists (created in set_caps), but it may not
       * be active if upstream is not using it */
      if (!gst_buffer_pool_is_active (sink->pool) &&
          !gst_buffer_pool_set_active (sink->pool, TRUE))
        goto activate_failed;

      ret = gst_buffer_pool_acquire_buffer (sink->pool, &to_render, NULL);
      if (ret != GST_FLOW_OK)
        goto no_buffer;

      /* the first time we acquire a buffer,
       * we need to attach a wl_buffer on it */
      wlbuffer = gst_buffer_get_wl_buffer (buffer);
      if (G_UNLIKELY (!wlbuffer)) {
        mem = gst_buffer_peek_memory (to_render, 0);
        wbuf = gst_wl_shm_memory_construct_wl_buffer (mem, sink->display,
            &sink->video_info);
        if (G_UNLIKELY (!wbuf))
          goto no_wl_buffer;

        gst_buffer_add_wl_buffer (to_render, wbuf, sink->display);
      }

      gst_buffer_map (buffer, &src, GST_MAP_READ);
      gst_buffer_fill (to_render, 0, src.data, src.size);
      gst_buffer_unmap (buffer, &src);
    }
  }
  /* drop double rendering */
  if (G_UNLIKELY (buffer == sink->last_buffer)) {
    GST_LOG_OBJECT (sink, "Buffer already being rendered");
    goto done;
  }

  gst_buffer_replace (&sink->last_buffer, to_render);
  render_last_buffer (sink);

  if (buffer != to_render)
    gst_buffer_unref (to_render);

  goto done;

#endif /* GST_WLSINK_ENHANCEMENT */

no_window_size:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, WRITE,
        ("Window has no size set"),
        ("Make sure you set the size after calling set_window_handle"));
    ret = GST_FLOW_ERROR;
    goto done;
  }
no_buffer:
  {
    GST_WARNING_OBJECT (sink, "could not create buffer");
    goto done;
  }
no_wl_buffer:
  {
    GST_ERROR_OBJECT (sink, "could not create wl_buffer out of wl_shm memory");
    ret = GST_FLOW_ERROR;
    goto done;
  }
activate_failed:
  {
    GST_ERROR_OBJECT (sink, "failed to activate bufferpool.");
    ret = GST_FLOW_ERROR;
    goto done;
  }
done:
  {
    g_mutex_unlock (&sink->render_lock);
    return ret;
  }
}

static void
gst_wayland_sink_videooverlay_init (GstVideoOverlayInterface * iface)
{
  iface->set_window_handle = gst_wayland_sink_set_window_handle;
  iface->set_render_rectangle = gst_wayland_sink_set_render_rectangle;
  iface->expose = gst_wayland_sink_expose;
#ifdef GST_WLSINK_ENHANCEMENT   /* use  unique_id */
  iface->set_wl_window_wl_surface_id =
      gst_wayland_sink_set_wl_window_wl_surface_id;
#endif
}

#ifdef GST_WLSINK_ENHANCEMENT   /* use  unique_id */
static void
gst_wayland_sink_set_wl_window_wl_surface_id (GstVideoOverlay * overlay,
    guintptr wl_surface_id)
{
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);
  FUNCTION;
  g_return_if_fail (sink != NULL);

  if (sink->window != NULL) {
    GST_WARNING_OBJECT (sink, "changing window handle is not supported");
    return;
  }
  g_mutex_lock (&sink->render_lock);
  g_clear_object (&sink->window);

  GST_INFO ("wl_surface_id %d %x", (int) wl_surface_id,
      (guintptr) wl_surface_id);

  if (wl_surface_id) {
    if (G_LIKELY (gst_wayland_sink_find_display (sink))) {
      /* we cannot use our own display with an external window handle */
      if (G_UNLIKELY (sink->display->own_display)) {
        sink->display->wl_surface_id = (int) wl_surface_id;
        sink->window = gst_wl_window_new_in_surface (sink->display, NULL);
      }
    } else {
      GST_ERROR_OBJECT (sink, "Failed to find display handle, "
          "ignoring window handle");
    }
  }
  gst_wayland_sink_update_window_geometry (sink);

  g_mutex_unlock (&sink->render_lock);

}
#endif

static void
gst_wayland_sink_set_window_handle (GstVideoOverlay * overlay, guintptr handle)
{
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);
  struct wl_surface *surface = (struct wl_surface *) handle;
  FUNCTION;

  g_return_if_fail (sink != NULL);

#ifdef GST_WLSINK_ENHANCEMENT   /* use  unique_id */
  if (sink->window != NULL) {
    GST_WARNING_OBJECT (sink, "changing window handle is not supported");
    return;
  }
#endif
  g_mutex_lock (&sink->render_lock);

  GST_DEBUG_OBJECT (sink, "Setting window handle %" GST_PTR_FORMAT,
      (void *) handle);

  g_clear_object (&sink->window);

  if (handle) {
    if (G_LIKELY (gst_wayland_sink_find_display (sink))) {
      /* we cannot use our own display with an external window handle */
      if (G_UNLIKELY (sink->display->own_display)) {
        GST_ELEMENT_WARNING (sink, RESOURCE, OPEN_READ_WRITE,
            ("Application did not provide a wayland display handle"),
            ("Now waylandsink use internal display handle "
                "which is created ourselves. Consider providing a "
                "display handle from your application with GstContext"));
        sink->window = gst_wl_window_new_in_surface (sink->display, surface);
      } else {
        sink->window = gst_wl_window_new_in_surface (sink->display, surface);
      }
    } else {
      GST_ERROR_OBJECT (sink, "Failed to find display handle, "
          "ignoring window handle");
    }
  }
  g_mutex_unlock (&sink->render_lock);

}

static void
gst_wayland_sink_set_render_rectangle (GstVideoOverlay * overlay,
    gint x, gint y, gint w, gint h)
{
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);
  FUNCTION;

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
  GstWaylandSink *sink = GST_WAYLAND_SINK (overlay);
  FUNCTION;

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
  GstWaylandSink *sink = GST_WAYLAND_SINK (video);
  FUNCTION;
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
  GstWaylandSink *sink = GST_WAYLAND_SINK (video);
  FUNCTION;
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
