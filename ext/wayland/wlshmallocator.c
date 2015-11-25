/* GStreamer Wayland video sink
 *
 * Copyright (C) 2012 Intel Corporation
 * Copyright (C) 2012 Sreerenj Balachandran <sreerenj.balachandran@intel.com>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "wlshmallocator.h"
#include "wlvideoformat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#define DUMP_BUFFER
#ifdef DUMP_BUFFER
int dump_cnt = 0;
int _write_rawdata (const char *file, const void *data, unsigned int size);
#endif

GST_DEBUG_CATEGORY_EXTERN (gstwayland_debug);
#define GST_CAT_DEFAULT gstwayland_debug

G_DEFINE_TYPE (GstWlShmAllocator, gst_wl_shm_allocator, GST_TYPE_ALLOCATOR);

static GstMemory *
gst_wl_shm_allocator_alloc (GstAllocator * allocator, gsize size,
    GstAllocationParams * params)
{
  FUNCTION;
  GstWlShmAllocator *self = GST_WL_SHM_ALLOCATOR (allocator);
  char filename[1024];
  static int init = 0;
  int fd;
  int idx;
  gpointer data;
  GstWlShmMemory *mem;
#ifdef GST_WLSINK_ENHANCEMENT
  tbm_bo_handle virtual_addr;

  idx = self->display->tbm_bo_c_idx++;
  self->display->tbm_bufmgr =
      wayland_tbm_client_get_bufmgr (self->display->tbm_client);
  g_return_if_fail (self->display->tbm_bufmgr != NULL);

  self->display->tbm_bo[idx] =
      tbm_bo_alloc (self->display->tbm_bufmgr, size, TBM_BO_DEFAULT);
  if (!self->display->tbm_bo[idx]) {
    GST_ERROR_OBJECT (self, "alloc tbm bo(size:%d) failed: %s", size,
        strerror (errno));
    return FALSE;
  }
  GST_ERROR("display->tbm_bo[%d]=(%p)", idx, self->display->tbm_bo[idx]);
  virtual_addr.ptr = NULL;
  virtual_addr = tbm_bo_get_handle (self->display->tbm_bo[idx], TBM_DEVICE_CPU);
  if (!virtual_addr.ptr) {
    GST_ERROR_OBJECT (self, "get tbm bo handle failed: %s", strerror (errno));
    tbm_bo_unref (self->display->tbm_bo[idx]);
    self->display->tbm_bo[idx] = NULL;
	self->display->tbm_bo_c_idx--;
    return FALSE;
  }

  mem = g_slice_new0 (GstWlShmMemory);
  gst_memory_init ((GstMemory *) mem, GST_MEMORY_FLAG_NO_SHARE, allocator, NULL,
      size, 0, 0, size);
  mem->data = virtual_addr.ptr;
  GST_ERROR ("mem(%p) mem->data(%p) virtual_addr.ptr(%p) size(%d)", mem,
      mem->data, virtual_addr.ptr, size);

  return (GstMemory *) mem;
#else
  /* TODO: make use of the allocation params, if necessary */

  /* allocate shm pool */
  snprintf (filename, 1024, "%s/%s-%d-%s", g_get_user_runtime_dir (),
      "wayland-shm", init++, "XXXXXX");

  fd = g_mkstemp (filename);
  if (fd < 0) {
    GST_ERROR_OBJECT (self, "opening temp file %s failed: %s", filename,
        strerror (errno));
    return NULL;
  }
  if (ftruncate (fd, size) < 0) {
    GST_ERROR_OBJECT (self, "ftruncate failed: %s", strerror (errno));
    close (fd);
    return NULL;
  }

  data = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    GST_ERROR_OBJECT (self, "mmap failed: %s", strerror (errno));
    close (fd);
    return NULL;
  }

  unlink (filename);

  mem = g_slice_new0 (GstWlShmMemory);
  gst_memory_init ((GstMemory *) mem, GST_MEMORY_FLAG_NO_SHARE, allocator, NULL,
      size, 0, 0, size);
  mem->data = data;
  mem->fd = fd;

  return (GstMemory *) mem;
#endif
}

static void
gst_wl_shm_allocator_free (GstAllocator * allocator, GstMemory * memory)
{
  FUNCTION;
  GstWlShmMemory *shm_mem = (GstWlShmMemory *) memory;

  if (shm_mem->fd != -1)
    close (shm_mem->fd);
  munmap (shm_mem->data, memory->maxsize);

  g_slice_free (GstWlShmMemory, shm_mem);
}

static gpointer
gst_wl_shm_mem_map (GstMemory * mem, gsize maxsize, GstMapFlags flags)
{
  FUNCTION;
  return ((GstWlShmMemory *) mem)->data;
}

static void
gst_wl_shm_mem_unmap (GstMemory * mem)
{
}

static void
gst_wl_shm_allocator_class_init (GstWlShmAllocatorClass * klass)
{
  FUNCTION;
  GstAllocatorClass *alloc_class = (GstAllocatorClass *) klass;

  alloc_class->alloc = GST_DEBUG_FUNCPTR (gst_wl_shm_allocator_alloc);
  alloc_class->free = GST_DEBUG_FUNCPTR (gst_wl_shm_allocator_free);
}

static void
gst_wl_shm_allocator_init (GstWlShmAllocator * self)
{
  FUNCTION;
  self->parent_instance.mem_type = GST_ALLOCATOR_WL_SHM;
  self->parent_instance.mem_map = gst_wl_shm_mem_map;
  self->parent_instance.mem_unmap = gst_wl_shm_mem_unmap;

  GST_OBJECT_FLAG_SET (self, GST_ALLOCATOR_FLAG_CUSTOM_ALLOC);
}

void
gst_wl_shm_allocator_register (void)
{
  FUNCTION;
  gst_allocator_register (GST_ALLOCATOR_WL_SHM,
      g_object_new (GST_TYPE_WL_SHM_ALLOCATOR, NULL));
}

GstAllocator *
gst_wl_shm_allocator_get (void)
{
  FUNCTION;
  return gst_allocator_find (GST_ALLOCATOR_WL_SHM);
}

gboolean
gst_is_wl_shm_memory (GstMemory * mem)
{
  FUNCTION;
  return gst_memory_is_type (mem, GST_ALLOCATOR_WL_SHM);
}

struct wl_buffer *
gst_wl_shm_memory_construct_wl_buffer (GstMemory * mem, GstWlDisplay * display,
    const GstVideoInfo * info)
{
  FUNCTION;
  GstWlShmMemory *shm_mem = (GstWlShmMemory *) mem;
  gint width, height, stride;
  gsize size;
  enum wl_shm_format format;
  struct wl_shm_pool *wl_pool;
  struct wl_buffer *wbuffer;

#ifdef GST_WLSINK_ENHANCEMENT
  tbm_bo_handle virtual_addr;
  tbm_surface_info_s ts_info;
  int num_bo;
  void *data;

  if (display->is_native_format == TRUE) {
    width = GST_VIDEO_INFO_WIDTH (info);
    height = GST_VIDEO_INFO_HEIGHT (info);
    size = display->native_video_size;
    format = gst_video_format_to_wl_tbm_format (GST_VIDEO_INFO_FORMAT (info));
    GST_ERROR ("format %s, width(%d), height(%d), size(%d)",
        gst_wl_tbm_format_to_string (format), width, height, size);

#ifdef DUMP_BUFFER
    virtual_addr = tbm_bo_get_handle (display->bo[0], TBM_DEVICE_CPU);
    if (!virtual_addr.ptr) {
      GST_ERROR ("get tbm bo handle failed: %s", strerror (errno));
      return FALSE;
    }
    data = virtual_addr.ptr;
    int ret;
    char file_name[128];
    if (dump_cnt < 10) {
      sprintf (file_name, "/root/WLSINK_OUT_DUMP_%2.2d.dump", dump_cnt++);
      ret = _write_rawdata (file_name, virtual_addr.ptr, size);
      if (ret) {
        GST_ERROR ("_write_rawdata() failed");
      }
    }
#endif
    GST_DEBUG ("TBM bo %p %p %p", display->bo[0], display->bo[1], 0);
    ts_info.width = width;
    ts_info.height = height;
    ts_info.format = format;
    ts_info.bpp = tbm_surface_internal_get_bpp (ts_info.format);
    ts_info.num_planes = tbm_surface_internal_get_num_planes (ts_info.format);
    GST_DEBUG ("ts_info.width(%d) height(%d) format(%d) bpp(%d) num_planes(%d)",
        ts_info.width = width, ts_info.height = height, ts_info.format =
        format, ts_info.bpp, ts_info.num_planes);

    ts_info.planes[0].stride = display->stride_width[0];
    ts_info.planes[1].stride = display->stride_width[1];
    ts_info.planes[0].offset = 0;
    ts_info.planes[1].offset = (display->bo[1]) ? 0 : display->plane_size[0];
    num_bo = (display->bo[1]) ? 2 : 1;
    GST_ERROR ("num_bo(%d)", num_bo);
    display->tsurface =
        tbm_surface_internal_create_with_bos (&ts_info, display->bo, num_bo);
    GST_ERROR ("display->tsurface(%p)", display->tsurface);
    GST_ERROR ("tbm_client(%p),tsurface(%p)", display->tbm_client,
        display->tsurface);
    wbuffer =
        wayland_tbm_client_create_buffer (display->tbm_client,
        display->tsurface);
  } else {
    int idx;
    idx = display->tbm_bo_u_idx++;
    width = GST_VIDEO_INFO_WIDTH (info);
    height = GST_VIDEO_INFO_HEIGHT (info);
    stride = GST_VIDEO_INFO_PLANE_STRIDE (info, 0);
    size = GST_VIDEO_INFO_SIZE (info);

    format = gst_video_format_to_wl_tbm_format (GST_VIDEO_INFO_FORMAT (info));
    g_return_val_if_fail (gst_is_wl_shm_memory (mem), NULL);
    g_return_val_if_fail (size <= mem->size, NULL);
    g_return_val_if_fail (shm_mem->fd != -1, NULL);

    GST_DEBUG_OBJECT (mem->allocator, "Creating wl_buffer of size %"
        G_GSSIZE_FORMAT " (%d x %d, stride %d), format %s", size, width, height,
        stride, gst_wl_tbm_format_to_string (format));

#ifdef DUMP_BUFFER
        GST_DEBUG ("idx(%d)",idx);
		virtual_addr = tbm_bo_get_handle (display->tbm_bo[idx], TBM_DEVICE_CPU);
		if (!virtual_addr.ptr) {
		  GST_ERROR ("get tbm bo handle failed: %s", strerror (errno));
		  return FALSE;
		}
		data = virtual_addr.ptr;
		int ret;
		char file_name[128];
		if (dump_cnt < 10) {
		  sprintf (file_name, "/home/owner/WLSINK_OUT_DUMP_%2.2d.dump", dump_cnt++);
		  ret = _write_rawdata (file_name, virtual_addr.ptr, size);
		  if (ret) {
			GST_ERROR ("_write_rawdata() failed");
		  }
		  GST_ERROR("DUMP IMAGE");
		}
#endif
    ts_info.width = width;
    ts_info.height = height;
    ts_info.format = format;
    ts_info.bpp = tbm_surface_internal_get_bpp (ts_info.format);
    ts_info.num_planes = tbm_surface_internal_get_num_planes (ts_info.format);
    ts_info.planes[0].stride = GST_VIDEO_INFO_PLANE_STRIDE (info, 0);
    ts_info.planes[1].stride = GST_VIDEO_INFO_PLANE_STRIDE (info, 1);
    ts_info.planes[2].stride = GST_VIDEO_INFO_PLANE_STRIDE (info, 2);
    ts_info.planes[0].offset = GST_VIDEO_INFO_PLANE_OFFSET (info, 0);
    ts_info.planes[1].offset = GST_VIDEO_INFO_PLANE_OFFSET (info, 1);
    ts_info.planes[2].offset = GST_VIDEO_INFO_PLANE_OFFSET (info, 2);

	GST_ERROR("display->tbm_bo (%p)", display->tbm_bo[idx]);

    display->tsurface =
        tbm_surface_internal_create_with_bos (&ts_info, &display->tbm_bo[idx], 1);
    wbuffer =
        wayland_tbm_client_create_buffer (display->tbm_client,
        display->tsurface);
  }
  GST_DEBUG ("wayland_tbm_client_create_buffer create wl_buffer %p", wbuffer);

  return wbuffer;

#else
  width = GST_VIDEO_INFO_WIDTH (info);
  height = GST_VIDEO_INFO_HEIGHT (info);
  stride = GST_VIDEO_INFO_PLANE_STRIDE (info, 0);
  size = GST_VIDEO_INFO_SIZE (info);
  format = gst_video_format_to_wl_shm_format (GST_VIDEO_INFO_FORMAT (info));
  g_return_val_if_fail (gst_is_wl_shm_memory (mem), NULL);
  g_return_val_if_fail (size <= mem->size, NULL);
  g_return_val_if_fail (shm_mem->fd != -1, NULL);

  GST_DEBUG_OBJECT (mem->allocator, "Creating wl_buffer of size %"
      G_GSSIZE_FORMAT " (%d x %d, stride %d), format %s", size, width, height,
      stride, gst_wl_shm_format_to_string (format));

  wl_pool = wl_shm_create_pool (display->shm, shm_mem->fd, mem->size);
  wbuffer = wl_shm_pool_create_buffer (wl_pool, 0, width, height, stride,
      format);

  close (shm_mem->fd);
  shm_mem->fd = -1;
  wl_shm_pool_destroy (wl_pool);

  return wbuffer;
#endif
}

#ifdef DUMP_BUFFER
int
_write_rawdata (const char *file, const void *data, unsigned int size)
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
