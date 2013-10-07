%bcond_with wayland
Name:           gst-plugins-bad
Version:        1.0.7
Release:        0
%define gst_branch 1.0
Summary:        GStreamer Streaming-Media Framework Plug-Ins
License:        GPL-2.0+ and LGPL-2.1+
Group:          Multimedia/Audio
Url:            http://gstreamer.freedesktop.org/
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-bad/%{name}-%{version}.tar.xz
Source1001: 	gst-plugins-bad.manifest
BuildRequires:  gst-common
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
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(gio-2.0) >= 2.25.0
BuildRequires:  pkgconfig(libcrypto)
BuildRequires:  pkgconfig(libcurl) >= 7.21.0
BuildRequires:  pkgconfig(libexif) >= 0.6.16
BuildRequires:  pkgconfig(libpng) >= 1.2
BuildRequires:  pkgconfig(openssl) >= 0.9.5
BuildRequires:  pkgconfig(sndfile) >= 1.0.16
%if %{with wayland}
BuildRequires:  pkgconfig(wayland-client) >= 1.0.0
%endif
BuildRequires:  pkgconfig(x11)
Requires(post): glib2-tools
Requires(postun): glib2-tools
Requires:       gstreamer >= 1.0.2
Enhances:       gstreamer

%description
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.


%package -n libgstbasecamerabinsrc
Summary:        GStreamer Streaming-Media Framework Plug-Ins

%description -n libgstbasecamerabinsrc
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstphotography
Summary:        GStreamer Streaming-Media Framework Plug-Ins

%description -n libgstphotography
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstvdp
Summary:        GStreamer Streaming-Media Framework Plug-Ins

%description -n libgstvdp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstbasevideo
Summary:        GStreamer Streaming-Media Framework Plug-Ins

%description -n libgstbasevideo
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstcodecparsers
Summary:        GStreamer Streaming-Media Framework Plug-Ins

%description -n libgstcodecparsers
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstsignalprocessor
Summary:        GStreamer Streaming-Media Framework Plug-Ins

%description -n libgstsignalprocessor
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package devel
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Requires:       gstreamer-devel
Requires:       libgstbasecamerabinsrc = %{version}
Requires:       libgstbasevideo = %{version}
Requires:       libgstcodecparsers = %{version}
Requires:       libgstphotography = %{version}
Requires:       libgstsignalprocessor = %{version}

%description devel
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.


%prep
%setup -q -n %{name}-%{version}
cp %{SOURCE1001} .
rm -rf common
cp -a %{_datadir}/gst-common common
find common -exec touch {} \;

%build
export V=1
NOCONFIGURE=1 ./autogen.sh
%configure\
    --disable-static\
    --disable-examples\
    --enable-experimental\
    --disable-gtk-doc 
%__make %{?_smp_mflags} V=1

%install
%make_install
%find_lang %{name}-%{gst_branch}
mv %{name}-%{gst_branch}.lang %{name}.lang

%lang_package

%clean
rm -rf $RPM_BUILD_ROOT

%post
%glib2_gsettings_schema_post

%postun
%glib2_gsettings_schema_postun

%post -n libgstbasecamerabinsrc -p /sbin/ldconfig

%post -n libgstphotography -p /sbin/ldconfig

%post -n libgstbasevideo -p /sbin/ldconfig

%post -n libgstcodecparsers -p /sbin/ldconfig

%post -n libgstsignalprocessor -p /sbin/ldconfig

%post -n libgstvdp -p /sbin/ldconfig

%postun -n libgstbasecamerabinsrc -p /sbin/ldconfig

%postun -n libgstphotography -p /sbin/ldconfig

%postun -n libgstbasevideo -p /sbin/ldconfig

%postun -n libgstcodecparsers -p /sbin/ldconfig

%postun -n libgstsignalprocessor -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-, root, root)
%doc COPYING COPYING.LIB 
%{_libdir}/gstreamer-%{gst_branch}/libgstadpcmdec.so
%{_libdir}/gstreamer-%{gst_branch}/libgstadpcmenc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstasfmux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudiovisualizers.so
%{_libdir}/gstreamer-%{gst_branch}/libgstautoconvert.so
%{_libdir}/gstreamer-%{gst_branch}/libgstbayer.so
%{_libdir}/gstreamer-%{gst_branch}/libgstcamerabin2.so
%{_libdir}/gstreamer-%{gst_branch}/libgstcoloreffects.so
%{_libdir}/gstreamer-%{gst_branch}/libgstcurl.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdataurisrc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdebugutilsbad.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdtmf.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdvb.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdvbsuboverlay.so
%{_libdir}/gstreamer-%{gst_branch}/libgstfestival.so
%{_libdir}/gstreamer-%{gst_branch}/libgstfragmented.so
%{_libdir}/gstreamer-%{gst_branch}/libgstgaudieffects.so
%{_libdir}/gstreamer-%{gst_branch}/libgstgdp.so
%{_libdir}/gstreamer-%{gst_branch}/libgstgeometrictransform.so
%{_libdir}/gstreamer-%{gst_branch}/libgstid3tag.so
%{_libdir}/gstreamer-%{gst_branch}/libgstinter.so
%{_libdir}/gstreamer-%{gst_branch}/libgstinterlace.so
%{_libdir}/gstreamer-%{gst_branch}/libgstjpegformat.so
%{_libdir}/gstreamer-%{gst_branch}/libgstliveadder.so

%{_libdir}/gstreamer-%{gst_branch}/libgstmpegpsdemux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmpegtsdemux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmpegtsmux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstpcapparse.so
%{_libdir}/gstreamer-%{gst_branch}/libgstpnm.so
%{_libdir}/gstreamer-%{gst_branch}/libgstrawparse.so
%{_libdir}/gstreamer-%{gst_branch}/libgstremovesilence.so
%{_libdir}/gstreamer-%{gst_branch}/libgstrtpmux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstrtpvp8.so
%{_libdir}/gstreamer-%{gst_branch}/libgstscaletempoplugin.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsdpelem.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsegmentclip.so
%{_libdir}/gstreamer-%{gst_branch}/libgstshm.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsmooth.so
%{_libdir}/gstreamer-%{gst_branch}/libgstspeed.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvideoparsersbad.so
%{_libdir}/gstreamer-%{gst_branch}/libgsty4mdec.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdvdspu.so
%{_libdir}/gstreamer-%{gst_branch}/libgstfieldanalysis.so
%{_libdir}/gstreamer-%{gst_branch}/libgstfrei0r.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsiren.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsubenc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmpegpsmux.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdecklink.so
%{_libdir}/gstreamer-%{gst_branch}/libgsteglglessink.so
%if %{with wayland}
%{_libdir}/gstreamer-%{gst_branch}/libgstwaylandsink.so
%endif

%files -n libgstphotography
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstphotography-%{gst_branch}.so.0*

%files -n libgstbasevideo
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstbasevideo-%{gst_branch}.so.0*

%files -n libgstbasecamerabinsrc
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstbasecamerabinsrc-%{gst_branch}.so.0*

%files -n libgstcodecparsers
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstcodecparsers-%{gst_branch}.so.0*

%files -n libgstsignalprocessor
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstsignalprocessor-%{gst_branch}.so.0*


%files devel
%manifest %{name}.manifest
%defattr(-, root, root)
%{_includedir}/gstreamer-%{gst_branch}
%{_libdir}/*.so
%{_libdir}/pkgconfig/gstreamer-basevideo-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-codecparsers-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-plugins-bad-%{gst_branch}.pc

