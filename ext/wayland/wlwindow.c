/* GStreamer Wayland video sink
 *
 * Copyright (C) 2011 Intel Corporation
 * Copyright (C) 2011 Sreerenj Balachandran <sreerenj.balachandran@intel.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef GST_WLSINK_ENHANCEMENT
#include "gstwaylandsink.h"
#else
#include "wlwindow.h"
#endif
#include "wlshmallocator.h"
#include "wlbuffer.h"

enum
{
  ROTATE_0_FLIP_NONE,
  ROTATE_0_FLIP_HORIZONTAL,
  ROTATE_0_FLIP_VERTICAL,
  ROTATE_0_FLIP_BOTH,
  ROTATE_90_FLIP_NONE = 10,
  ROTATE_90_FLIP_HORIZONTAL,
  ROTATE_90_FLIP_VERTICAL,
  ROTATE_90_FLIP_BOTH,
  ROTATE_180_FLIP_NONE = 20,
  ROTATE_180_FLIP_HORIZONTAL,
  ROTATE_180_FLIP_VERTICAL,
  ROTATE_180_FLIP_BOTH,
  ROTATE_270_FLIP_NONE = 30,
  ROTATE_270_FLIP_HORIZONTAL,
  ROTATE_270_FLIP_VERTICAL,
  ROTATE_270_FLIP_BOTH,
  ROTATE_NUM,
};

GST_DEBUG_CATEGORY_EXTERN (gstwayland_debug);
#define GST_CAT_DEFAULT gstwayland_debug

G_DEFINE_TYPE (GstWlWindow, gst_wl_window, G_TYPE_OBJECT);

static void gst_wl_window_finalize (GObject * gobject);

static void
handle_ping (void *data, struct wl_shell_surface *shell_surface,
    uint32_t serial)
{
  wl_shell_surface_pong (shell_surface, serial);
}

static void
handle_configure (void *data, struct wl_shell_surface *shell_surface,
    uint32_t edges, int32_t width, int32_t height)
{
}

static void
handle_popup_done (void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
  handle_ping,
  handle_configure,
  handle_popup_done
};

static void
gst_wl_window_class_init (GstWlWindowClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  FUNCTION;
  gobject_class->finalize = gst_wl_window_finalize;
}

static void
gst_wl_window_init (GstWlWindow * self)
{
}

static void
gst_wl_window_finalize (GObject * gobject)
{
  GstWlWindow *self = GST_WL_WINDOW (gobject);
  FUNCTION;

#ifdef GST_WLSINK_ENHANCEMENT
  if (self->video_object)
    tizen_video_object_destroy (self->video_object);
#endif

  if (self->shell_surface) {
    wl_shell_surface_destroy (self->shell_surface);
  }

  wl_viewport_destroy (self->video_viewport);
  wl_subsurface_destroy (self->video_subsurface);
  wl_surface_destroy (self->video_surface);

  if (self->area_subsurface) {
    wl_subsurface_destroy (self->area_subsurface);
  }
  wl_viewport_destroy (self->area_viewport);
  wl_surface_destroy (self->area_surface);

  g_clear_object (&self->display);

  G_OBJECT_CLASS (gst_wl_window_parent_class)->finalize (gobject);
}

static GstWlWindow *
#ifdef GST_WLSINK_ENHANCEMENT
gst_wl_window_new_internal (GstWlDisplay * display, struct wl_surface *parent)
#else
gst_wl_window_new_internal (GstWlDisplay * display)
#endif
{
  GstWlWindow *window;
  GstVideoInfo info;
#ifndef GST_WLSINK_ENHANCEMENT
  GstBuffer *buf;
  GstMapInfo mapinfo;
  struct wl_buffer *wlbuf;
  GstWlBuffer *gwlbuf;
#endif
  struct wl_region *region;
  FUNCTION;

  window = g_object_new (GST_TYPE_WL_WINDOW, NULL);
  window->display = g_object_ref (display);

  window->area_surface = wl_compositor_create_surface (display->compositor);
  window->video_surface = wl_compositor_create_surface (display->compositor);

  wl_proxy_set_queue ((struct wl_proxy *) window->area_surface, display->queue);
  wl_proxy_set_queue ((struct wl_proxy *) window->video_surface,
      display->queue);

#if 1                           /* create shell_surface here for enlightenment */
  /* go toplevel */
  if (display->need_shell_surface) {
    window->shell_surface = wl_shell_get_shell_surface (display->shell,
        window->area_surface);
  } else if (display->use_parent_wl_surface) {
#ifdef GST_WLSINK_ENHANCEMENT
    GST_INFO ("call tizen_policy_get_subsurface");
    if (display->wl_surface_id && parent == NULL) {
      window->area_subsurface =
          tizen_policy_get_subsurface (display->tizen_policy,
          window->area_surface, display->wl_surface_id);
      wl_subsurface_set_desync (window->area_subsurface);
      wl_surface_commit (window->area_surface);
    } else {
      GST_INFO (" wl_surface %p", parent);
      window->area_subsurface =
          wl_subcompositor_get_subsurface (display->subcompositor,
          window->area_surface, parent);
      wl_subsurface_set_desync (window->area_subsurface);
    }
#else
    window->area_subsurface =
        wl_subcompositor_get_subsurface (display->subcompositor,
        window->area_surface, parent);
    wl_subsurface_set_desync (window->area_subsurface);
#endif
  }
#endif

  /* embed video_surface in area_surface */
  window->video_subsurface =
      wl_subcompositor_get_subsurface (display->subcompositor,
      window->video_surface, window->area_surface);
  wl_subsurface_set_desync (window->video_subsurface);
  wl_surface_commit (window->video_surface);

  window->area_viewport = wl_scaler_get_viewport (display->scaler,
      window->area_surface);
  window->video_viewport = wl_scaler_get_viewport (display->scaler,
      window->video_surface);

  /* draw the area_subsurface */
  gst_video_info_set_format (&info,
      /* we want WL_SHM_FORMAT_XRGB8888 */
#if G_BYTE_ORDER == G_BIG_ENDIAN
      GST_VIDEO_FORMAT_xRGB,
#else
      GST_VIDEO_FORMAT_BGRx,
#endif
      1, 1);

#ifdef GST_WLSINK_ENHANCEMENT
  if (window->display->USE_TBM) {
    /* Inform enlightenment of surface which render video */
    window->video_object =
        tizen_video_get_object (display->tizen_video, window->video_surface);
  }
#else /* open source */
  buf = gst_buffer_new_allocate (gst_wl_shm_allocator_get (), info.size, NULL);
  gst_buffer_map (buf, &mapinfo, GST_MAP_WRITE);
  *((guint32 *) mapinfo.data) = 0;      /* paint it black */
  gst_buffer_unmap (buf, &mapinfo);
  wlbuf =
      gst_wl_shm_memory_construct_wl_buffer (gst_buffer_peek_memory (buf, 0),
      display, &info);
  gwlbuf = gst_buffer_add_wl_buffer (buf, wlbuf, display);
  gst_wl_buffer_attach (gwlbuf, window->area_surface);

  /* at this point, the GstWlBuffer keeps the buffer
   * alive and will free it on wl_buffer::release */
  gst_buffer_unref (buf);
#endif

  /* do not accept input */
  region = wl_compositor_create_region (display->compositor);
  wl_surface_set_input_region (window->area_surface, region);
  wl_region_destroy (region);

  region = wl_compositor_create_region (display->compositor);
  wl_surface_set_input_region (window->video_surface, region);
  wl_region_destroy (region);

  return window;
}

GstWlWindow *
gst_wl_window_new_toplevel (GstWlDisplay * display, const GstVideoInfo * info)
{
  GstWlWindow *window;
  gint width;
  FUNCTION;

/* not create shell_surface here for enlightenment */
#ifdef GST_WLSINK_ENHANCEMENT
  display->need_shell_surface = TRUE;

  window = gst_wl_window_new_internal (display, NULL);

#if 0                           //GST_WLSINK_ENHANCEMENT
  /* go toplevel */
  window->shell_surface = wl_shell_get_shell_surface (display->shell,
      window->area_surface);
#endif
#endif
  if (window->shell_surface) {
    wl_shell_surface_add_listener (window->shell_surface,
        &shell_surface_listener, window);
    wl_shell_surface_set_toplevel (window->shell_surface);
  } else {
    GST_ERROR ("Unable to get wl_shell_surface");

    g_object_unref (window);
    return NULL;
  }

  /* set the initial size to be the same as the reported video size */
  width =
      gst_util_uint64_scale_int_round (info->width, info->par_n, info->par_d);
  gst_wl_window_set_render_rectangle (window, 0, 0, width, info->height);

  return window;
}

GstWlWindow *
gst_wl_window_new_in_surface (GstWlDisplay * display,
    struct wl_surface * parent)
{
  GstWlWindow *window;
  FUNCTION;

  display->use_parent_wl_surface = TRUE;
#ifdef GST_WLSINK_ENHANCEMENT
  if (parent) {                 /*use wl_surface */
    window = gst_wl_window_new_internal (display, parent);
  } else {                      /* use wl_surface id */
    window = gst_wl_window_new_internal (display, NULL);
  }
#else
  window = gst_wl_window_new_internal (display, parent);
#endif

#if 0                           /*for enlightment */
  /* embed in parent */
  window->area_subsurface =
      wl_subcompositor_get_subsurface (display->subcompositor,
      window->area_surface, parent);
  wl_subsurface_set_desync (window->area_subsurface);
#endif

#ifdef GST_WLSINK_ENHANCEMENT
  /*Area surface from App need to be under parent surface */
  if (display->tizen_policy) {
    GST_INFO (" call tizen_policy_place_subsurface_below_parent ");
    tizen_policy_place_subsurface_below_parent (display->tizen_policy,
        window->area_subsurface);
    tizen_policy_place_subsurface_below_parent (display->tizen_policy,
        window->video_subsurface);
  }
#else
  wl_surface_commit (parent);
#endif
  return window;
}

GstWlDisplay *
gst_wl_window_get_display (GstWlWindow * window)
{
  FUNCTION;
  g_return_val_if_fail (window != NULL, NULL);

  return g_object_ref (window->display);
}

struct wl_surface *
gst_wl_window_get_wl_surface (GstWlWindow * window)
{
  FUNCTION;
  g_return_val_if_fail (window != NULL, NULL);

  return window->video_surface;
}

gboolean
gst_wl_window_is_toplevel (GstWlWindow * window)
{
  FUNCTION;
  g_return_val_if_fail (window != NULL, FALSE);

  return (window->shell_surface != NULL);
}

#ifdef GST_WLSINK_ENHANCEMENT
static gint
gst_wl_window_find_transform (guint rotate_angle, guint flip)
{
  gint transform;
  guint combine = rotate_angle * 10 + flip;
  FUNCTION;
  GST_DEBUG ("rotate %d, flip %d, combine %d", rotate_angle, flip, combine);
  switch (combine) {
    case ROTATE_0_FLIP_NONE:
      transform = WL_OUTPUT_TRANSFORM_NORMAL;
      break;
    case ROTATE_0_FLIP_HORIZONTAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED;
      break;
    case ROTATE_0_FLIP_VERTICAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED_180;
      break;
    case ROTATE_0_FLIP_BOTH:
      transform = WL_OUTPUT_TRANSFORM_180;
      break;
    case ROTATE_90_FLIP_NONE:
      transform = WL_OUTPUT_TRANSFORM_90;
      break;
    case ROTATE_90_FLIP_HORIZONTAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED_90;
      break;
    case ROTATE_90_FLIP_VERTICAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED_270;
      break;
    case ROTATE_90_FLIP_BOTH:
      transform = WL_OUTPUT_TRANSFORM_270;
      break;
    case ROTATE_180_FLIP_NONE:
      transform = WL_OUTPUT_TRANSFORM_180;
      break;
    case ROTATE_180_FLIP_HORIZONTAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED_180;
      break;
    case ROTATE_180_FLIP_VERTICAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED;
      break;
    case ROTATE_180_FLIP_BOTH:
      transform = WL_OUTPUT_TRANSFORM_NORMAL;
      break;
    case ROTATE_270_FLIP_NONE:
      transform = WL_OUTPUT_TRANSFORM_270;
      break;
    case ROTATE_270_FLIP_HORIZONTAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED_270;
      break;
    case ROTATE_270_FLIP_VERTICAL:
      transform = WL_OUTPUT_TRANSFORM_FLIPPED_90;
      break;
    case ROTATE_270_FLIP_BOTH:
      transform = WL_OUTPUT_TRANSFORM_90;
      break;
  }

  return transform;
}

#endif
static void
gst_wl_window_resize_video_surface (GstWlWindow * window, gboolean commit)
{
  GstVideoRectangle src = { 0, };
  GstVideoRectangle res;
#ifdef GST_WLSINK_ENHANCEMENT   // need to change ifndef to ifdef
  GstVideoRectangle src_origin = { 0, 0, 0, 0 };
  GstVideoRectangle src_input = { 0, 0, 0, 0 };
  GstVideoRectangle dst = { 0, 0, 0, 0 };
  int temp = 0;
  gint transform = WL_OUTPUT_TRANSFORM_NORMAL;
#endif
  FUNCTION;

  /* center the video_subsurface inside area_subsurface */
  src.w = window->video_width;
  src.h = window->video_height;

#ifdef GST_WLSINK_ENHANCEMENT   // need to change ifndef to ifdef
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

  transform = gst_wl_window_find_transform (window->rotate_angle, window->flip);

  GST_INFO
      ("window[%d x %d] src[%d,%d,%d x %d],dst[%d,%d,%d x %d],input[%d,%d,%d x %d],result[%d,%d,%d x %d]",
      window->render_rectangle.w, window->render_rectangle.h,
      src.x, src.y, src.w, src.h,
      dst.x, dst.y, dst.w, dst.h,
      src_input.x, src_input.y, src_input.w, src_input.h,
      res.x, res.y, res.w, res.h);

  GST_INFO ("video (%d x %d)", window->video_width, window->video_height);
  GST_INFO ("src_input(%d, %d, %d x %d)", src_input.x, src_input.y, src_input.w,
      src_input.h);
  GST_INFO ("src_origin(%d, %d, %d x %d)", src_origin.x, src_origin.y,
      src_origin.w, src_origin.h);
  GST_INFO ("src(%d, %d, %d x %d)", src.x, src.y, src.w, src.h);
  GST_INFO ("dst(%d,%d,%d x %d)", dst.x, dst.y, dst.w, dst.h);
  GST_INFO ("window->render_rectangle(%d,%d,%d x %d)",
      window->render_rectangle.x, window->render_rectangle.y,
      window->render_rectangle.w, window->render_rectangle.h);
  GST_INFO ("res(%d, %d, %d x %d)", res.x, res.y, res.w, res.h);

  if (window->video_subsurface) {
    GST_INFO ("have window->subsurface");
    wl_subsurface_set_position (window->video_subsurface,
        window->render_rectangle.x + res.x, window->render_rectangle.y + res.y);
    GST_INFO ("wl_subsurface_set_position(%d,%d)",
        window->render_rectangle.x + res.x, window->render_rectangle.y + res.y);
  }
  wl_viewport_set_destination (window->video_viewport, res.w, res.h);
  GST_INFO ("wl_viewport_set_destination(%d,%d)", res.w, res.h);

  /*need to swap */
  if (transform % 2 == 1) {     /*1, 3, 5, 7 */
    temp = src_input.w;
    src_input.w = src_input.h;
    src_input.h = temp;
  }
  wl_viewport_set_source (window->video_viewport,
      wl_fixed_from_int (src_input.x), wl_fixed_from_int (src_input.y),
      wl_fixed_from_int (src_input.w), wl_fixed_from_int (src_input.h));
  GST_INFO ("wl_viewport_set_source(%d,%d, %d x %d)", src_input.x, src_input.y,
      src_input.w, src_input.h);

  wl_surface_set_buffer_transform (window->video_surface, transform);
  GST_INFO ("wl_surface_set_buffer_transform (%d)", transform);
#else
  gst_video_sink_center_rect (src, window->render_rectangle, &res, TRUE);

  wl_subsurface_set_position (window->video_subsurface, res.x, res.y);
  wl_viewport_set_destination (window->video_viewport, res.w, res.h);
#endif
  if (commit) {
    wl_surface_damage (window->video_surface, 0, 0, res.w, res.h);
    wl_surface_commit (window->video_surface);
  }

  if (gst_wl_window_is_toplevel (window)) {
    struct wl_region *region;

    region = wl_compositor_create_region (window->display->compositor);
    wl_region_add (region, 0, 0, window->render_rectangle.w,
        window->render_rectangle.h);
    wl_surface_set_input_region (window->area_surface, region);
    wl_region_destroy (region);
  }

  /* this is saved for use in wl_surface_damage */
  window->surface_width = res.w;
  window->surface_height = res.h;
}

void
gst_wl_window_render (GstWlWindow * window, GstWlBuffer * buffer,
    const GstVideoInfo * info)
{
  FUNCTION;
  if (G_UNLIKELY (info)) {
    window->video_width =
        gst_util_uint64_scale_int_round (info->width, info->par_n, info->par_d);
    window->video_height = info->height;

    wl_subsurface_set_sync (window->video_subsurface);
    gst_wl_window_resize_video_surface (window, FALSE);
  }
  GST_INFO ("GstWlBuffer(%p)", buffer);
  if (G_LIKELY (buffer))
    gst_wl_buffer_attach (buffer, window->video_surface);
  else
    wl_surface_attach (window->video_surface, NULL, 0, 0);

  /*Wayland-compositor will try to render damage area which need  to be updated */
  wl_surface_damage (window->video_surface, 0, 0, window->surface_width,
      window->surface_height);
#ifdef GST_WLSINK_ENHANCEMENT
  GST_LOG ("update area width %d, height %d", window->surface_width,
      window->surface_height);
#endif
  /* wl_surface_commit change surface state, if wl_buffer is not attached newly,  then surface is not changed */
  wl_surface_commit (window->video_surface);

  if (G_UNLIKELY (info)) {
    /* commit also the parent (area_surface) in order to change
     * the position of the video_subsurface */
#ifdef GST_WLSINK_ENHANCEMENT
    GST_DEBUG ("render_rectangle %d*%d", window->render_rectangle.w,
        window->render_rectangle.h);
#endif
    wl_surface_damage (window->area_surface, 0, 0, window->render_rectangle.w,
        window->render_rectangle.h);
    wl_surface_commit (window->area_surface);
    wl_subsurface_set_desync (window->video_subsurface);
  }

  wl_display_flush (window->display->display);
}

void
gst_wl_window_set_render_rectangle (GstWlWindow * window, gint x, gint y,
    gint w, gint h)
{
  FUNCTION;
  g_return_if_fail (window != NULL);
#ifdef GST_WLSINK_ENHANCEMENT
  if (window->render_rectangle.x == x && window->render_rectangle.y == y
      && window->render_rectangle.w == w && window->render_rectangle.h == h) {
    GST_DEBUG ("but the values are same. skip");
    return;
  }
#endif
  window->render_rectangle.x = x;
  window->render_rectangle.y = y;
  window->render_rectangle.w = w;
  window->render_rectangle.h = h;

  /* position the area inside the parent - needs a parent commit to apply */
  if (window->area_subsurface)
    wl_subsurface_set_position (window->area_subsurface, x, y);

  /* change the size of the area */
  wl_viewport_set_destination (window->area_viewport, w, h);

  if (window->video_width != 0) {
    wl_subsurface_set_sync (window->video_subsurface);
    gst_wl_window_resize_video_surface (window, TRUE);
  }

  wl_surface_damage (window->area_surface, 0, 0, w, h);
  wl_surface_commit (window->area_surface);

  if (window->video_width != 0)
    wl_subsurface_set_desync (window->video_subsurface);
}

#ifdef GST_WLSINK_ENHANCEMENT
void
gst_wl_window_set_video_info (GstWlWindow * window, const GstVideoInfo * info)
{
  FUNCTION;
  g_return_if_fail (window != NULL);

  window->video_width =
      gst_util_uint64_scale_int_round (info->width, info->par_n, info->par_d);
  window->video_height = info->height;

  if (window->render_rectangle.w != 0)
    gst_wl_window_resize_video_surface (window, FALSE);
}

void
gst_wl_window_set_rotate_angle (GstWlWindow * window, guint rotate_angle)
{
  FUNCTION;
  g_return_if_fail (window != NULL);
  window->rotate_angle = rotate_angle;
  GST_INFO ("rotate_angle value is (%d)", window->rotate_angle);
}

void
gst_wl_window_set_disp_geo_method (GstWlWindow * window, guint disp_geo_method)
{
  FUNCTION;
  g_return_if_fail (window != NULL);
  window->disp_geo_method = disp_geo_method;
  GST_INFO ("disp_geo_method value is (%d)", window->disp_geo_method);
}

void
gst_wl_window_set_orientation (GstWlWindow * window, guint orientation)
{
  FUNCTION;
  g_return_if_fail (window != NULL);
  window->orientation = orientation;
  GST_INFO ("orientation value is (%d)", window->orientation);
}

void
gst_wl_window_set_flip (GstWlWindow * window, guint flip)
{
  FUNCTION;
  g_return_if_fail (window != NULL);
  window->flip = flip;
  GST_INFO ("flip value is (%d)", window->flip);
}
#endif
