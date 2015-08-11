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
#include "wlwindow.h"
#ifdef GST_WLSINK_ENHANCEMENT
#include "gstwaylandsink.h"
#define SWAP(a, b) { (a) ^= (b) ^= (a) ^= (b); }
#endif

GST_DEBUG_CATEGORY_EXTERN (gstwayland_debug);
#define GST_CAT_DEFAULT gstwayland_debug

G_DEFINE_TYPE (GstWlWindow, gst_wl_window, G_TYPE_OBJECT);

static void gst_wl_window_finalize (GObject * gobject);

static void
handle_ping (void *data, struct wl_shell_surface *shell_surface,
    uint32_t serial)
{
  FUNCTION_ENTER ();

  wl_shell_surface_pong (shell_surface, serial);
}

static void
handle_configure (void *data, struct wl_shell_surface *shell_surface,
    uint32_t edges, int32_t width, int32_t height)
{
  FUNCTION_ENTER ();

}

static void
handle_popup_done (void *data, struct wl_shell_surface *shell_surface)
{
  FUNCTION_ENTER ();

}

static const struct wl_shell_surface_listener shell_surface_listener = {
  handle_ping,
  handle_configure,
  handle_popup_done
};

static void
gst_wl_window_class_init (GstWlWindowClass * klass)
{
  FUNCTION_ENTER ();

  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = gst_wl_window_finalize;
}

static void
gst_wl_window_init (GstWlWindow * self)
{
  FUNCTION_ENTER ();

}

static void
gst_wl_window_finalize (GObject * gobject)
{
  FUNCTION_ENTER ();

  GstWlWindow *self = GST_WL_WINDOW (gobject);

  if (self->shell_surface) {
    wl_shell_surface_destroy (self->shell_surface);
  }

  if (self->subsurface) {
    wl_subsurface_destroy (self->subsurface);
  }

  wl_viewport_destroy (self->viewport);
  wl_surface_destroy (self->surface);

  g_clear_object (&self->display);

  G_OBJECT_CLASS (gst_wl_window_parent_class)->finalize (gobject);
}

static GstWlWindow *
gst_wl_window_new_internal (GstWlDisplay * display, struct wl_surface *surface)
{
  FUNCTION_ENTER ();

  GstWlWindow *window;
  struct wl_region *region;

  g_return_val_if_fail (surface != NULL, NULL);

  window = g_object_new (GST_TYPE_WL_WINDOW, NULL);
  window->display = g_object_ref (display);
  window->surface = surface;

  /* make sure the surface runs on our local queue */
  wl_proxy_set_queue ((struct wl_proxy *) surface, display->queue);

  window->viewport = wl_scaler_get_viewport (display->scaler, window->surface);

  /* do not accept input */
  region = wl_compositor_create_region (display->compositor);
  wl_surface_set_input_region (surface, region);
  wl_region_destroy (region);

  return window;
}

GstWlWindow *
gst_wl_window_new_toplevel (GstWlDisplay * display, GstVideoInfo * video_info)
{
  FUNCTION_ENTER ();

  GstWlWindow *window;

  window = gst_wl_window_new_internal (display,
      wl_compositor_create_surface (display->compositor));

  gst_wl_window_set_video_info (window, video_info);
  gst_wl_window_set_render_rectangle (window, 0, 0, window->video_width,
      window->video_height);

  window->shell_surface = wl_shell_get_shell_surface (display->shell,
      window->surface);

  if (window->shell_surface) {
    wl_shell_surface_add_listener (window->shell_surface,
        &shell_surface_listener, window);
    wl_shell_surface_set_toplevel (window->shell_surface);
  } else {
    GST_ERROR ("Unable to get wl_shell_surface");

    g_object_unref (window);
    return NULL;
  }

  return window;
}

GstWlWindow *
gst_wl_window_new_in_surface (GstWlDisplay * display,
    struct wl_surface * parent)
{
  FUNCTION_ENTER ();

  GstWlWindow *window;

  window = gst_wl_window_new_internal (display,
      wl_compositor_create_surface (display->compositor));

  window->subsurface = wl_subcompositor_get_subsurface (display->subcompositor,
      window->surface, parent);
  wl_subsurface_set_desync (window->subsurface);
#ifdef GST_WLSINK_ENHANCEMENT
  if (display->tizen_subsurface)
    tizen_subsurface_place_below_parent (display->tizen_subsurface,
        window->subsurface);

  wl_surface_commit (parent);
#endif
  return window;
}

GstWlDisplay *
gst_wl_window_get_display (GstWlWindow * window)
{
  FUNCTION_ENTER ();

  g_return_val_if_fail (window != NULL, NULL);

  return g_object_ref (window->display);
}

struct wl_surface *
gst_wl_window_get_wl_surface (GstWlWindow * window)
{
  FUNCTION_ENTER ();

  g_return_val_if_fail (window != NULL, NULL);

  return window->surface;
}

gboolean
gst_wl_window_is_toplevel (GstWlWindow * window)
{
  FUNCTION_ENTER ();

  g_return_val_if_fail (window != NULL, FALSE);

  return (window->shell_surface != NULL);
}

static void
gst_wl_window_resize_internal (GstWlWindow * window, gboolean commit)
{
  FUNCTION_ENTER ();

  GstVideoRectangle src = { 0, };
  GstVideoRectangle res; //dst

  src.w = window->video_width;
  src.h = window->video_height;
#ifdef GST_WLSINK_ENHANCEMENT // need to change ifndef to ifdef
  GstVideoRectangle src_origin = { 0, 0, 0, 0};
  GstVideoRectangle src_input = {0, 0, 0, 0};
  GstVideoRectangle dst = {0, 0, 0, 0};

  gint res_rotate_angle = 0;
  gint rotate = 0;

  res_rotate_angle = window->rotate_angle;
  src.x = src.y = 0;
  src_input.w = src_origin.w = window->video_width;
  src_input.h = src_origin.h = window->video_height;
  
  if (window->rotate_angle == DEGREE_0 ||
      window->rotate_angle == DEGREE_180) {
    src.w = window->video_width; //video_width
    src.h = window->video_height; //video_height
  } else {
    src.w = window->video_height;
    src.h = window->video_width;
  }

  /*default res.w and res.h*/
  dst.w = window->render_rectangle.w; 
  dst.h = window->render_rectangle.h;

  switch (window->disp_geo_method) {
  	case DISP_GEO_METHOD_LETTER_BOX:
	  gst_video_sink_center_rect (src, dst, &res, TRUE);
	  res.x += window->render_rectangle.x;
	  res.y += window->render_rectangle.y;
	  break;
	case DISP_GEO_METHOD_ORIGIN_SIZE_OR_LETTER_BOX:
	  GST_WARNING("not supported API, set ORIGIN_SIZE mode");
	case DISP_GEO_METHOD_ORIGIN_SIZE:
	  gst_video_sink_center_rect (src, dst, &res, FALSE);
	  gst_video_sink_center_rect (dst, src, &src_input, FALSE);

	  if (window->rotate_angle == DEGREE_90 || window->rotate_angle == DEGREE_270) {
	  	SWAP(src_input.x, src_input.y);
	  	SWAP(src_input.w, src_input.h);
	  }
	  break;
	case DISP_GEO_METHOD_FULL_SCREEN:
	  res.x = res.y = 0;
	  res.w = window->render_rectangle.w;
	  res.h = window->render_rectangle.h;
	  break;
    case DISP_GEO_METHOD_CROPPED_FULL_SCREEN:
	  gst_video_sink_center_rect (dst, src, &src_input, TRUE);
      res.x = res.y = 0;
      res.w = dst.w;
      res.h = dst.h;
	  
      if (window->rotate_angle == DEGREE_90 || window->rotate_angle == DEGREE_270) {
		SWAP(src_input.x, src_input.y);
		SWAP(src_input.w, src_input.h);
      }
      break;
	case DISP_GEO_METHOD_CUSTOM_DST_ROI:
	  {
	    GstVideoRectangle dst_roi_cmpns;
		dst_roi_cmpns.w = window->dst_roi.w;
		dst_roi_cmpns.h = window->dst_roi.h;
		dst_roi_cmpns.x = window->dst_roi.x;
		dst_roi_cmpns.y = window->dst_roi.y;

		/*setting for DST ROI mode*/
		switch (window->dst_roi_mode) {
		  case ROI_DISP_GEO_METHOD_FULL_SCREEN:
			break;
		  case ROI_DISP_GEO_METHOD_LETTER_BOX:
		  {
			GstVideoRectangle roi_result;
			if (window->orientation == DEGREE_0 ||
				window->orientation == DEGREE_180) {
			  src.w = src_origin.w;
			  src.h = src_origin.h;
			} else {
			  src.w = src_origin.h;
			  src.h = src_origin.w;
			}
			dst.w = window->dst_roi.w;
			dst.h = window->dst_roi.h;
		
			gst_video_sink_center_rect (src, dst, &roi_result, TRUE);
			dst_roi_cmpns.w = roi_result.w;
			dst_roi_cmpns.h = roi_result.h;
			dst_roi_cmpns.x = window->dst_roi.x + roi_result.x;
			dst_roi_cmpns.y = window->dst_roi.y + roi_result.y;
		  }
			break;
		  default:
			break;
		}

        switch (window->rotate_angle) {
          case DEGREE_90:
            res.w = dst_roi_cmpns.h;
            res.h = dst_roi_cmpns.w;
            res.x = dst_roi_cmpns.y;
            res.y = window->render_rectangle.h - dst_roi_cmpns.x - dst_roi_cmpns.w;
            break;
          case DEGREE_180:
            res.w = dst_roi_cmpns.w;
            res.h = dst_roi_cmpns.h;
            res.x = window->render_rectangle.w - res.w - dst_roi_cmpns.x;
            res.y = window->render_rectangle.h - res.h - dst_roi_cmpns.y;
            break;
          case DEGREE_270:
            res.w = dst_roi_cmpns.h;
            res.h = dst_roi_cmpns.w;
            res.x = window->render_rectangle.w - dst_roi_cmpns.y - dst_roi_cmpns.h;
            res.y = dst_roi_cmpns.x;
            break;
          default:
           res.x = dst_roi_cmpns.x;
           res.y = dst_roi_cmpns.y;
           res.w = dst_roi_cmpns.w;
           res.h = dst_roi_cmpns.h;
           break;
	    }
        /* orientation setting for auto rotation in DST ROI */
        if (window->orientation) {
          res_rotate_angle = window->rotate_angle - window->orientation;
          if (res_rotate_angle < 0) {
            res_rotate_angle += DEGREE_NUM;
          }
          GST_ERROR ("changing rotation value internally by ROI orientation[%d] : rotate[%d->%d]",
            window->orientation, window->rotate_angle,res_rotate_angle);
        }
		GST_ERROR ("rotate[%d], dst ROI: orientation[%d], mode[%d], input[%d,%d,%dx%d]->result[%d,%d,%dx%d]",
			  window->rotate_angle, window->orientation,
			  window->dst_roi_mode, window->dst_roi.x,
			  window->dst_roi.y, window->dst_roi.w,
			  window->dst_roi.h, res.x, res.y, res.w, res.h);
		  break;
		}
	default:
	  break;

  }

  switch (res_rotate_angle) {
    case DEGREE_0:
      break;
    case DEGREE_90:
      rotate = 270;
      break;
    case DEGREE_180:
      rotate = 180;
      break;
    case DEGREE_270:
      rotate = 90;
      break;
    default:
      GST_ERROR("Unsupported rotation [%d]... set DEGREE 0.", res_rotate_angle);
      break;
  }
  GST_ERROR (
	  "window[%dx%d],method[%d],rotate[%d],zoom[%lf],dp_mode[%d],src[%d,%d,%dx%d],dst[%d,%d,%dx%d],input[%d,%d,%dx%d],result[%d,%d,%dx%d]",
	  window->render_rectangle.w, window->render_rectangle.h, window->disp_geo_method,
	  rotate, 0, window->disp_geo_method, src_origin.x, src_origin.y, src_origin.w, src_origin.h, dst.x, dst.y,
	  dst.w, dst.h, src_input.x, src_input.y, src_input.w, src_input.h,res.x, res.y, res.w, res.h);


  
 // gst_video_sink_center_rect (src, window->render_rectangle, &res, TRUE);
  if (window->subsurface)
    wl_subsurface_set_position (window->subsurface,
        window->render_rectangle.x + res.x, window->render_rectangle.y + res.y);
    wl_viewport_set_destination (window->viewport, res.w, res.h);
    wl_viewport_set_source(window->viewport, src.x, src.y, src.w, src.h);
//    wl_viewport_set (window->viewport, src.x, src.y, src.w, src.h, res.w, res.h);
  if (commit) {
    wl_surface_damage (window->surface, 0, 0, res.w, res.h);
    wl_surface_commit (window->surface);
  }

  /* this is saved for use in wl_surface_damage */
  window->surface_width = res.w;
  window->surface_height = res.h;

#else
  gst_video_sink_center_rect (src, window->render_rectangle, &res, TRUE);
  if (window->subsurface)
    wl_subsurface_set_position (window->subsurface,
        window->render_rectangle.x + res.x, window->render_rectangle.y + res.y);
  wl_viewport_set_destination (window->viewport, res.w, res.h);

  if (commit) {
    wl_surface_damage (window->surface, 0, 0, res.w, res.h);
    wl_surface_commit (window->surface);
  }

  /* this is saved for use in wl_surface_damage */
  window->surface_width = res.w;
  window->surface_height = res.h;
#endif
}

void
gst_wl_window_set_video_info (GstWlWindow * window, GstVideoInfo * info)
{
  FUNCTION_ENTER ();

  g_return_if_fail (window != NULL);

  window->video_width =
      gst_util_uint64_scale_int_round (info->width, info->par_n, info->par_d);
  window->video_height = info->height;

  if (window->render_rectangle.w != 0)
    gst_wl_window_resize_internal (window, FALSE);
}

void
gst_wl_window_set_render_rectangle (GstWlWindow * window, gint x, gint y,
    gint w, gint h)
{
  FUNCTION_ENTER ();

  g_return_if_fail (window != NULL);

  window->render_rectangle.x = x;
  window->render_rectangle.y = y;
  window->render_rectangle.w = w;
  window->render_rectangle.h = h;

  if (window->video_width != 0)
    gst_wl_window_resize_internal (window, TRUE);
}
