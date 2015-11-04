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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "tizen-wlvideoformat.h"
#ifdef GST_WLSINK_ENHANCEMENT

GST_DEBUG_CATEGORY_EXTERN (gstwayland_debug);
#define GST_CAT_DEFAULT gstwayland_debug

typedef struct
{
  enum tizen_buffer_pool_format wl_format;
  GstVideoFormat gst_format;
} wl_VideoFormat;

static const wl_VideoFormat formats[] = {
#if G_BYTE_ORDER == G_BIG_ENDIAN
  {TIZEN_BUFFER_POOL_FORMAT_XRGB8888, GST_VIDEO_FORMAT_xRGB},
  {TIZEN_BUFFER_POOL_FORMAT_XBGR8888, GST_VIDEO_FORMAT_xBGR},
  {TIZEN_BUFFER_POOL_FORMAT_RGBX8888, GST_VIDEO_FORMAT_RGBx},
  {TIZEN_BUFFER_POOL_FORMAT_BGRX8888, GST_VIDEO_FORMAT_BGRx},
  {TIZEN_BUFFER_POOL_FORMAT_ARGB8888, GST_VIDEO_FORMAT_ARGB},
  {TIZEN_BUFFER_POOL_FORMAT_ABGR8888, GST_VIDEO_FORMAT_RGBA},
  {TIZEN_BUFFER_POOL_FORMAT_RGBA8888, GST_VIDEO_FORMAT_RGBA},
  {TIZEN_BUFFER_POOL_FORMAT_BGRA8888, GST_VIDEO_FORMAT_BGRA},
#else
  {TIZEN_BUFFER_POOL_FORMAT_XRGB8888, GST_VIDEO_FORMAT_BGRx},
  {TIZEN_BUFFER_POOL_FORMAT_XBGR8888, GST_VIDEO_FORMAT_RGBx},
  {TIZEN_BUFFER_POOL_FORMAT_RGBX8888, GST_VIDEO_FORMAT_xBGR},
  {TIZEN_BUFFER_POOL_FORMAT_BGRX8888, GST_VIDEO_FORMAT_xRGB},
  {TIZEN_BUFFER_POOL_FORMAT_ARGB8888, GST_VIDEO_FORMAT_BGRA},
  {TIZEN_BUFFER_POOL_FORMAT_ABGR8888, GST_VIDEO_FORMAT_RGBA},
  {TIZEN_BUFFER_POOL_FORMAT_RGBA8888, GST_VIDEO_FORMAT_ABGR},
  {TIZEN_BUFFER_POOL_FORMAT_BGRA8888, GST_VIDEO_FORMAT_ARGB},
#endif
  {TIZEN_BUFFER_POOL_FORMAT_RGB565, GST_VIDEO_FORMAT_RGB16},
  {TIZEN_BUFFER_POOL_FORMAT_BGR565, GST_VIDEO_FORMAT_BGR16},
  {TIZEN_BUFFER_POOL_FORMAT_RGB888, GST_VIDEO_FORMAT_RGB},
  {TIZEN_BUFFER_POOL_FORMAT_BGR888, GST_VIDEO_FORMAT_BGR},
  {TIZEN_BUFFER_POOL_FORMAT_YUYV, GST_VIDEO_FORMAT_YUY2},
  {TIZEN_BUFFER_POOL_FORMAT_YVYU, GST_VIDEO_FORMAT_YVYU},
  {TIZEN_BUFFER_POOL_FORMAT_UYVY, GST_VIDEO_FORMAT_UYVY},
  {TIZEN_BUFFER_POOL_FORMAT_AYUV, GST_VIDEO_FORMAT_AYUV},
  {TIZEN_BUFFER_POOL_FORMAT_NV12, GST_VIDEO_FORMAT_NV12},
  {TIZEN_BUFFER_POOL_FORMAT_NV21, GST_VIDEO_FORMAT_NV21},
  {TIZEN_BUFFER_POOL_FORMAT_NV16, GST_VIDEO_FORMAT_NV16},
  {TIZEN_BUFFER_POOL_FORMAT_YUV410, GST_VIDEO_FORMAT_YUV9},
  {TIZEN_BUFFER_POOL_FORMAT_YVU410, GST_VIDEO_FORMAT_YVU9},
  {TIZEN_BUFFER_POOL_FORMAT_YUV411, GST_VIDEO_FORMAT_Y41B},
  {TIZEN_BUFFER_POOL_FORMAT_YUV420, GST_VIDEO_FORMAT_I420},
  {TIZEN_BUFFER_POOL_FORMAT_YVU420, GST_VIDEO_FORMAT_YV12},
  {TIZEN_BUFFER_POOL_FORMAT_YUV422, GST_VIDEO_FORMAT_Y42B},
  {TIZEN_BUFFER_POOL_FORMAT_YUV444, GST_VIDEO_FORMAT_v308},
  {TIZEN_BUFFER_POOL_FORMAT_ST12, GST_VIDEO_FORMAT_ST12},
  {TIZEN_BUFFER_POOL_FORMAT_SN12, GST_VIDEO_FORMAT_SN12}
};

enum tizen_buffer_pool_format
gst_video_format_to_wayland_format (GstVideoFormat format)
{
  guint i;

  for (i = 0; i < G_N_ELEMENTS (formats); i++)
    if (formats[i].gst_format == format)
      return formats[i].wl_format;

  GST_WARNING ("wayland video format not found");
  return -1;
}

GstVideoFormat
gst_wayland_format_to_video_format (enum tizen_buffer_pool_format wl_format)
{
  guint i;

  for (i = 0; i < G_N_ELEMENTS (formats); i++)
    if (formats[i].wl_format == wl_format)
      return formats[i].gst_format;

  GST_WARNING ("gst video format not found");
  return GST_VIDEO_FORMAT_UNKNOWN;
}

const gchar *
gst_wayland_format_to_string (enum tizen_buffer_pool_format wl_format)
{
  return gst_video_format_to_string
      (gst_wayland_format_to_video_format (wl_format));
}
#endif
