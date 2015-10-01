%bcond_with wayland
%define gst_branch 1.0
%define _lib_gstreamer_dir %{_libdir}/gstreamer-%{gst_branch}
%define _libdebug_dir %{_libdir}/debug/usr/lib

Name:           gst-plugins-bad
Version:        1.5.90
Release:        1
Summary:        GStreamer Streaming-Media Framework Plug-Ins
License:        GPL-2.0+ and LGPL-2.1+
Group:          Multimedia/Framework
Url:            http://gstreamer.freedesktop.org/
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-bad/%{name}-%{version}.tar.xz
BuildRequires:  gettext-tools
BuildRequires:  SDL-devel
BuildRequires:  autoconf
BuildRequires:  gcc-c++
BuildRequires:  glib2-devel >= 2.31.14
BuildRequires:  gstreamer-devel >= 1.0.0
BuildRequires:  gst-plugins-base-devel >= 1.0.0
BuildRequires:  pkgconfig(orc-0.4) >= 0.4.11
BuildRequires:  python
BuildRequires:  xsltproc
BuildRequires:  pkgconfig(libusb-1.0)
BuildRequires:  pkgconfig(gudev-1.0)
BuildRequires:  pkgconfig(gio-2.0) >= 2.25.0
BuildRequires:  pkgconfig(libcrypto)
BuildRequires:  pkgconfig(libcurl) >= 7.21.0
BuildRequires:  pkgconfig(libexif) >= 0.6.16
BuildRequires:  pkgconfig(libpng) >= 1.2
BuildRequires:  pkgconfig(openssl) >= 0.9.5
BuildRequires:  pkgconfig(sndfile) >= 1.0.16
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(mm-common)
%if %{with wayland}
BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(wayland-egl) >= 9.0
BuildRequires:  pkgconfig(wayland-client) >= 1.0.0
BuildRequires:  pkgconfig(wayland-cursor) >= 1.0.0
BuildRequires:  pkgconfig(tizen-extension-client)
%endif
Requires:       gstreamer >= 1.0.2

%description
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package devel
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Requires: %{name} = %{version}-%{release}
Requires: gst-plugins-base-devel

%description devel
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%prep
%setup -q -n %{name}-%{version}

%build
export V=1
NOCONFIGURE=1 ./autogen.sh
export CFLAGS="-DGST_WLSINK_ENHANCEMENT -DGST_TBM_SUPPORT -DMESA_EGL_NO_X11_HEADERS"
%configure\
    --disable-static\
    --disable-examples\
    --enable-experimental\
    --disable-audiomixer\
    --enable-compositor\
    --disable-ivfparse\
    --disable-jp2kdecimator\
    --disable-opengl\
    --enable-egl=yes\
    --enable-wayland=yes\
    --enable-gles2=yes\
    --disable-glx\
    --disable-sndfile\
    --disable-stereo\
    --disable-videosignal\
    --disable-vmnc\
    --disable-gtk-doc
%__make %{?_smp_mflags} V=1

%install
%make_install
%find_lang %{name}-%{gst_branch}
mv %{name}-%{gst_branch}.lang %{name}.lang

%lang_package

%clean
rm -rf %{_builddir}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%license COPYING COPYING.LIB
%{_lib_gstreamer_dir}/libgstadpcmdec.so
%{_lib_gstreamer_dir}/libgstadpcmenc.so
%{_lib_gstreamer_dir}/libgstasfmux.so
%{_lib_gstreamer_dir}/libgstaudiovisualizers.so
%{_lib_gstreamer_dir}/libgstautoconvert.so
%{_lib_gstreamer_dir}/libgstbayer.so
%{_lib_gstreamer_dir}/libgstcamerabin2.so
%{_lib_gstreamer_dir}/libgstcoloreffects.so
%{_lib_gstreamer_dir}/libgstcurl.so
%{_lib_gstreamer_dir}/libgstdataurisrc.so
%{_lib_gstreamer_dir}/libgstdebugutilsbad.so
%{_lib_gstreamer_dir}/libgstdvb.so
%{_lib_gstreamer_dir}/libgstdvbsuboverlay.so
%{_lib_gstreamer_dir}/libgstfestival.so
%{_lib_gstreamer_dir}/libgstgaudieffects.so
%{_lib_gstreamer_dir}/libgstgdp.so
%{_lib_gstreamer_dir}/libgstgeometrictransform.so
%{_lib_gstreamer_dir}/libgstid3tag.so
%{_lib_gstreamer_dir}/libgstinter.so
%{_lib_gstreamer_dir}/libgstinterlace.so
%{_lib_gstreamer_dir}/libgstjpegformat.so
%{_lib_gstreamer_dir}/libgstliveadder.so
%{_lib_gstreamer_dir}/libgstmpegpsdemux.so
%{_lib_gstreamer_dir}/libgstmpegtsdemux.so
%{_lib_gstreamer_dir}/libgstmpegtsmux.so
%{_lib_gstreamer_dir}/libgstpcapparse.so
%{_lib_gstreamer_dir}/libgstpnm.so
%{_lib_gstreamer_dir}/libgstrawparse.so
%{_lib_gstreamer_dir}/libgstremovesilence.so
%{_lib_gstreamer_dir}/libgstsdpelem.so
%{_lib_gstreamer_dir}/libgstsegmentclip.so
%{_lib_gstreamer_dir}/libgstshm.so
%{_lib_gstreamer_dir}/libgstsmooth.so
%{_lib_gstreamer_dir}/libgstspeed.so
%{_lib_gstreamer_dir}/libgstvideoparsersbad.so
%{_lib_gstreamer_dir}/libgsty4mdec.so
%{_lib_gstreamer_dir}/libgstdvdspu.so
%{_lib_gstreamer_dir}/libgstfieldanalysis.so
%{_lib_gstreamer_dir}/libgstfrei0r.so
%{_lib_gstreamer_dir}/libgstsiren.so
%{_lib_gstreamer_dir}/libgstsubenc.so
%{_lib_gstreamer_dir}/libgstmpegpsmux.so
%{_lib_gstreamer_dir}/libgstdecklink.so
%{_lib_gstreamer_dir}/libgstaccurip.so
%{_lib_gstreamer_dir}/libgstaiff.so
%{_lib_gstreamer_dir}/libgstaudiofxbad.so
%{_lib_gstreamer_dir}/libgstfbdevsink.so
%{_lib_gstreamer_dir}/libgstfreeverb.so
%{_lib_gstreamer_dir}/libgstivtc.so
%{_lib_gstreamer_dir}/libgstmidi.so
%{_lib_gstreamer_dir}/libgstmxf.so
%{_lib_gstreamer_dir}/libgstrfbsrc.so
%{_lib_gstreamer_dir}/libgstvideofiltersbad.so
%{_lib_gstreamer_dir}/libgstyadif.so
%{_lib_gstreamer_dir}/libgstuvch264.so
%{_lib_gstreamer_dir}/libgstcompositor.so
%{_lib_gstreamer_dir}/libgstdtls.so
%{_lib_gstreamer_dir}/libgstfragmented.so
%{_lib_gstreamer_dir}/libgstopengl.so
%{_lib_gstreamer_dir}/libgstrtpbad.so
%{_lib_gstreamer_dir}/libgstrtponvif.so
%{_lib_gstreamer_dir}/libgstvcdsrc.so

%if %{with wayland}
%{_libdir}/libgstwayland-%{gst_branch}.so.0
%{_lib_gstreamer_dir}/libgstwaylandsink.so
%endif

%define so_version so.0.590.0
%define so_version_debug %{so_version}.debug

%{_libdebug_dir}/libgstadaptivedemux-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstbadbase-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstbadvideo-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstbasecamerabinsrc-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstcodecparsers-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstgl-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstinsertbin-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstmpegts-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstphotography-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgsturidownloader-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstwayland-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstadaptivedemux-%{gst_branch}.%{so_version_debug}
%{_libdebug_dir}/libgstbadbase-%{gst_branch}.%{so_version_debug}

%{_libdir}/libgstbadvideo-%{gst_branch}.%{so_version}
%{_libdir}/libgstbasecamerabinsrc-%{gst_branch}.%{so_version}
%{_libdir}/libgstcodecparsers-%{gst_branch}.%{so_version}
%{_libdir}/libgstgl-%{gst_branch}.%{so_version}
%{_libdir}/libgstinsertbin-%{gst_branch}.%{so_version}
%{_libdir}/libgstmpegts-%{gst_branch}.%{so_version}
%{_libdir}/libgstphotography-%{gst_branch}.%{so_version}
%{_libdir}/libgsturidownloader-%{gst_branch}.%{so_version}
%{_libdir}/libgstwayland-%{gst_branch}.%{so_version}
%{_libdir}/libgstadaptivedemux-%{gst_branch}.%{so_version}
%{_libdir}/libgstbadbase-%{gst_branch}.%{so_version}

%{_libdir}/libgstadaptivedemux-%{gst_branch}.so.0
%{_libdir}/libgstbadbase-%{gst_branch}.so.0
%{_libdir}/libgstbadvideo-%{gst_branch}.so.0
%{_libdir}/libgstbasecamerabinsrc-%{gst_branch}.so.0
%{_libdir}/libgstcodecparsers-%{gst_branch}.so.0
%{_libdir}/libgstgl-%{gst_branch}.so.0
%{_libdir}/libgstinsertbin-%{gst_branch}.so.0
%{_libdir}/libgstmpegts-%{gst_branch}.so.0
%{_libdir}/libgstphotography-%{gst_branch}.so.0
%{_libdir}/libgsturidownloader-%{gst_branch}.so.0

%{_datadir}/gstreamer-%{gst_branch}/presets/GstFreeverb.prs

%files devel
%manifest %{name}.manifest
%{_includedir}/gstreamer-%{gst_branch}
%{_libdir}/libgstadaptivedemux-%{gst_branch}.so
%{_libdir}/libgstbadbase-%{gst_branch}.so
%{_libdir}/libgstbadvideo-%{gst_branch}.so
%{_libdir}/libgstbasecamerabinsrc-%{gst_branch}.so
%{_libdir}/libgstcodecparsers-%{gst_branch}.so
%{_libdir}/libgstgl-%{gst_branch}.so
%{_libdir}/libgstinsertbin-%{gst_branch}.so
%{_libdir}/libgstmpegts-%{gst_branch}.so
%{_libdir}/libgstphotography-%{gst_branch}.so
%{_libdir}/libgsturidownloader-%{gst_branch}.so
%{_libdir}/libgstwayland-%{gst_branch}.so
%{_libdir}/pkgconfig/gstreamer-codecparsers-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-plugins-bad-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-insertbin-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-mpegts-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-gl-%{gst_branch}.pc
%{_lib_gstreamer_dir}/include/gst/gl/gstglconfig.h

