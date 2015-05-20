#ifndef TIZEN_SUBSURFACEPROTOCOL_H
#define TIZEN_SUBSURFACEPROTOCOL_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

  struct wl_client;
  struct wl_resource;

  struct tizen_subsurface;

  extern const struct wl_interface tizen_subsurface_interface;

#define TIZEN_SUBSURFACE_PLACE_BELOW_PARENT	0

  static inline void
      tizen_subsurface_set_user_data (struct tizen_subsurface *tizen_subsurface,
      void *user_data)
  {
    wl_proxy_set_user_data ((struct wl_proxy *) tizen_subsurface, user_data);
  }

  static inline void *tizen_subsurface_get_user_data (struct tizen_subsurface
      *tizen_subsurface)
  {
    return wl_proxy_get_user_data ((struct wl_proxy *) tizen_subsurface);
  }

  static inline void
      tizen_subsurface_destroy (struct tizen_subsurface *tizen_subsurface)
  {
    wl_proxy_destroy ((struct wl_proxy *) tizen_subsurface);
  }

  static inline void
      tizen_subsurface_place_below_parent (struct tizen_subsurface
      *tizen_subsurface, struct wl_subsurface *subsurface)
  {
    wl_proxy_marshal ((struct wl_proxy *) tizen_subsurface,
        TIZEN_SUBSURFACE_PLACE_BELOW_PARENT, subsurface);
  }

#ifdef  __cplusplus
}
#endif

#endif
