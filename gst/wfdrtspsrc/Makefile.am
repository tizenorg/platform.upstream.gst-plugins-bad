
plugin_LTLIBRARIES = libgstwfdrtspsrc.la

libgstwfdrtspsrc_la_SOURCES = gstwfdrtspsrc.c gstwfdrtspext.c wfdconfigmessage.c

libgstwfdrtspsrc_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) \
				$(GST_CFLAGS) \
				$(GST_INTERFACE_CFLAGS) \
				$(GST_APP_CFLAGS)

libgstwfdrtspsrc_la_LIBADD = $(GST_LIBS) $(GSTPB_BASE_LIBS) $(GST_BASE_LIBS) \
				$(GST_INTERFACE_LIBS) \
				$(GST_APP_LIBS) \
				$(GST_PLUGINS_BASE_LIBS) \
               -lgstrtp-@GST_API_VERSION@ -lgstrtsp-@GST_API_VERSION@ -lglib-2.0

libgstwfdrtspsrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstwfdrtspsrc_la_LIBTOOLFLAGS = $(GST_PLUGIN_LIBTOOLFLAGS)

noinst_HEADERS = gstwfdrtspsrc.h \
                 gstwfdrtspext.h \
                 wfdconfigmessage.h \
                 wfdrtspstream.h
