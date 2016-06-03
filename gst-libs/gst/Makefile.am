
if USE_OPENGL
GL_DIR = gl
endif
if USE_GLES2
GL_DIR = gl
endif

if USE_WAYLAND
WAYLAND_DIR=wayland
endif

if TV_PROFILE
SUBDIRS = codecparsers mpegts $(WAYLAND_DIR)
else
SUBDIRS = uridownloader adaptivedemux interfaces basecamerabinsrc codecparsers \
	 insertbin mpegts base video $(GL_DIR) $(WAYLAND_DIR)
endif

noinst_HEADERS = gst-i18n-plugin.h gettext.h glib-compat-private.h

if TV_PROFILE
DIST_SUBDIRS = codecparsers mpegts wayland
else
DIST_SUBDIRS = uridownloader adaptivedemux interfaces gl basecamerabinsrc \
	codecparsers insertbin mpegts wayland base video
endif

#dependencies
video: base

adaptivedemux: uridownloader

if TV_PROFILE
INDEPENDENT_SUBDIRS = \
	codecparsers mpegts $(WAYLAND_DIR)
else
INDEPENDENT_SUBDIRS = \
	interfaces basecamerabinsrc codecparsers insertbin uridownloader \
	mpegts base $(GL_DIR) $(WAYLAND_DIR)
endif

.PHONY: independent-subdirs $(INDEPENDENT_SUBDIRS)

independent-subdirs: $(INDEPENDENT_SUBDIRS)

$(INDEPENDENT_SUBDIRS):
	$(MAKE) -C $@

all-recursive: independent-subdirs
