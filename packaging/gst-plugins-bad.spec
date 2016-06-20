%bcond_with wayland
#%bcond_with x
%define gst_branch 1.0

Name:           gst-plugins-bad
Version:        1.6.1
Release:        5
Summary:        GStreamer Streaming-Media Framework Plug-Ins
License:        LGPL-2.0+
Group:          Multimedia/Framework
Url:            http://gstreamer.freedesktop.org/
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-bad/%{name}-%{version}.tar.xz
Source100:      common.tar.gz
BuildRequires:  gettext-tools
#BuildRequires:  SDL-devel
BuildRequires:  autoconf
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(glib-2.0) >= 2.31.14
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires:  pkgconfig(orc-0.4) >= 0.4.11
BuildRequires:  python
BuildRequires:  xsltproc
#BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(libusb-1.0)
BuildRequires:  pkgconfig(gudev-1.0)
BuildRequires:  pkgconfig(gio-2.0) >= 2.25.0
BuildRequires:  pkgconfig(libcrypto)
BuildRequires:  pkgconfig(libcurl) >= 7.21.0
BuildRequires:  pkgconfig(libexif) >= 0.6.16
BuildRequires:  pkgconfig(openssl) >= 0.9.5
BuildRequires:  pkgconfig(sndfile) >= 1.0.16
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:	pkgconfig(mm-common)
#BuildRequires:  mesa-libGLESv2
#BuildRequires:  mesa-libEGL
%if %{with wayland}
#BuildRequires:  opengl-es-devel
BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(wayland-egl) >= 9.0
BuildRequires:  pkgconfig(wayland-client) >= 1.0.0
BuildRequires:  pkgconfig(wayland-cursor) >= 1.0.0
BuildRequires:  pkgconfig(wayland-tbm-client)
BuildRequires:  pkgconfig(tizen-extension-client)
BuildRequires:  pkgconfig(libxml-2.0)
%endif
%if %{with x}
BuildRequires:  pkgconfig(x11)
%endif
Requires:       gstreamer >= 1.0.2
#Enhances:       gstreamer

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
%setup -q -T -D -a 100

%build
export V=1
NOCONFIGURE=1 ./autogen.sh
export CFLAGS+=" -Wall -g -fPIC -DGST_WLSINK_ENHANCEMENT -DGST_TBM_SUPPORT -DMESA_EGL_NO_X11_HEADERS -DGST_EXT_AVOID_PAD_SWITCHING -DGST_ADAPTIVE_MODIFICATION"
%configure\
	--disable-static\
	--disable-examples\
	--enable-experimental\
%if "%{?profile}" == "tv"
	--enable-tv\
	--disable-autoconvert\
	--disable-camerabin2\
	--disable-dash\
	--disable-hls\
	--disable-id3tag\
	--disable-jpegformat\
	--disable-mpegdemux\
	--disable-smoothstreaming\
	--disable-coloreffects\
	--disable-rtp\
	--disable-gl\
%endif
	--disable-accurip\
	--disable-adpcmdec\
	--disable-audiofxbad\
	--disable-decklink\
	--disable-dvb\
	--disable-fieldanalysis\
	--disable-gdp\
	--disable-ivtc\
	--disable-liveadder\
	--disable-rawparse\
	--disable-videofilters\
	--disable-yadif\
	--disable-curl\
	--disable-dtls\
	--disable-fbdev\
	--disable-uvch264\
	--disable-vcd\
	--disable-y4m\
	--disable-adpcmenc\
	--disable-aiff\
	--disable-asfmux\
	--disable-audiomixer\
	--disable-compositor\
	--disable-audiovisualizers\
	--disable-bayer\
	--disable-cdxaparse\
	--disable-dataurisrc\
	--disable-dccp\
	--disable-dvbsuboverlay\
	--disable-dvdspu\
	--disable-faceoverlay\
	--disable-festival\
	--disable-freeverb\
	--disable-frei0r\
	--disable-gaudieffects\
	--disable-geometrictransform\
	--disable-hdvparse\
	--disable-inter\
	--disable-interlace\
	--disable-ivfparse\
	--disable-jp2kdecimator\
	--disable-librfb\
	--disable-mve\
	--disable-mxf\
	--disable-nuvdemux\
	--disable-onvif\
	--disable-pcapparse\
	--disable-pnm\
	--disable-removesilence\
	--disable-sdi\
	--disable-segmentclip\
	--disable-siren\
	--disable-smooth\
	--disable-speed\
	--disable-subenc\
	--disable-stereo\
	--disable-tta\
	--disable-videomeasure\
	--disable-videosignal\
	--disable-vmnc\
	--disable-opengl\
	--enable-egl=yes\
	--enable-wayland=yes\
	--enable-gles2=yes\
	--disable-glx\
	--disable-sndfile\
	--disable-gtk-doc\
	--disable-warnings-as-errors
%__make %{?_smp_mflags} V=1

%install
%make_install
%find_lang %{name}-%{gst_branch}
mv %{name}-%{gst_branch}.lang %{name}.lang

%lang_package


%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%defattr(-, root, root)
%license COPYING.LIB
%if "%{?profile}" != "tv"
%{_libdir}/gstreamer-%{gst_branch}/libgstautoconvert.so
%{_libdir}/gstreamer-%{gst_branch}/libgstcamerabin2.so
%{_libdir}/gstreamer-%{gst_branch}/libgstcoloreffects.so
%{_libdir}/gstreamer-%{gst_branch}/libgstid3tag.so
%{_libdir}/gstreamer-%{gst_branch}/libgstjpegformat.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmpegpsdemux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsmoothstreaming.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdashdemux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstopengl.so
%{_libdir}/gstreamer-%{gst_branch}/libgstrtpbad.so
%{_libdir}/gstreamer-%{gst_branch}/libgstfragmented.so

%{_libdir}/libgstinsertbin-%{gst_branch}.so.0*
%{_libdir}/libgstphotography-%{gst_branch}.so.0*
%{_libdir}/libgstbasecamerabinsrc-%{gst_branch}.so.0*
%{_libdir}/libgstbadbase-%{gst_branch}.so.0*
%{_libdir}/libgstbadvideo-%{gst_branch}.so.0*
%{_libdir}/libgsturidownloader-%{gst_branch}.so.0*
%{_libdir}/libgstadaptivedemux-1.0.so.0
%{_libdir}/libgstadaptivedemux-1.0.so.0.601.0
%{_libdir}/libgstgl-1.0.so.0
%{_libdir}/libgstgl-1.0.so.0.601.0
%endif
#%{_libdir}/gstreamer-%{gst_branch}/libgstadpcmdec.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstadpcmenc.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstasfmux.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstaudiovisualizers.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstbayer.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstcurl.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdataurisrc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdebugutilsbad.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdvb.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdvbsuboverlay.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstfestival.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstgaudieffects.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstgdp.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstgeometrictransform.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstinter.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstinterlace.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstliveadder.so

%{_libdir}/gstreamer-%{gst_branch}/libgstmpegtsdemux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmpegtsmux.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstpcapparse.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstpnm.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstrawparse.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstremovesilence.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsdpelem.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstsegmentclip.so
%{_libdir}/gstreamer-%{gst_branch}/libgstshm.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstsmooth.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstspeed.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvideoparsersbad.so
#%{_libdir}/gstreamer-%{gst_branch}/libgsty4mdec.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdvdspu.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstfieldanalysis.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstfrei0r.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstsiren.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstsubenc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmpegpsmux.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdecklink.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstaccurip.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstaiff.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstaudiofxbad.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstfbdevsink.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstfreeverb.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstivtc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmidi.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstmxf.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstrfbsrc.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstvideofiltersbad.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstyadif.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstuvch264.so

%if %{with wayland}
%{_libdir}/libgstwayland-%{gst_branch}.so.0*
%{_libdir}/gstreamer-%{gst_branch}/libgstwaylandsink.so
%endif

%{_libdir}/libgstcodecparsers-%{gst_branch}.so.0*
%{_libdir}/libgstmpegts-%{gst_branch}.so.0*
#%{_libdir}/gstreamer-%{gst_branch}/libgstcompositor.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdtls.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstrtponvif.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstvcdsrc.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstwaylandsink.so
#%{_libdir}/libgstwayland-1.0.so.0
#%{_libdir}/libgstwayland-1.0.so.0.601.0
#/usr/share/gstreamer-%{gst_branch}/presets/GstFreeverb.prs


%files devel
%manifest %{name}.manifest
%defattr(-, root, root)
%{_includedir}/gstreamer-%{gst_branch}
%if "%{?profile}" != "tv"
%{_libdir}/gstreamer-%{gst_branch}/include/gst/gl/gstglconfig.h
%endif
%{_libdir}/*.so
%{_libdir}/pkgconfig/gstreamer-codecparsers-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-plugins-bad-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-insertbin-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-mpegts-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-gl-1.0.pc
%if %{with wayland}
%{_libdir}/pkgconfig/gstreamer-wayland-%{gst_branch}.pc
%{_includedir}/gstreamer-%{gst_branch}/gst/wayland/wayland.h
%endif

