%bcond_with divx4linux
Name:           gst-plugins-bad
Version:        1.0.2
Release:        0
%define gst_branch 1.0
Summary:        GStreamer Streaming-Media Framework Plug-Ins
License:        GPL-2.0+ and LGPL-2.1+
Group:          Productivity/Multimedia/Other
Url:            http://gstreamer.freedesktop.org/
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-bad/%{name}-%{version}.tar.xz
Source1:        %{name}-patch-source.sh
Patch0:         gstreamer-plugins-bad-real.patch
BuildRequires:  SDL-devel
BuildRequires:  autoconf
BuildRequires:  gcc-c++
BuildRequires:  glib2-devel >= 2.31.14
BuildRequires:  gstreamer-devel >= 1.0.0
BuildRequires:  gstreamer-plugins-base-devel >= 1.0.0
BuildRequires:  gtk-doc
#BuildRequires:  libjasper-devel
BuildRequires:  pkgconfig(orc-0.4) >= 0.4.11
BuildRequires:  python-base
BuildRequires:  xsltproc
BuildRequires:  pkgconfig(cairo)
#BuildRequires:  pkgconfig(dirac) >= 0.10
BuildRequires:  pkgconfig(gio-2.0) >= 2.25.0
#BuildRequires:  pkgconfig(kate) >= 0.1.7
#BuildRequires:  pkgconfig(libass) >= 0.9.4
#BuildRequires:  pkgconfig(libcdaudio)
BuildRequires:  pkgconfig(libcrypto)
BuildRequires:  pkgconfig(libcurl) >= 7.21.0
BuildRequires:  pkgconfig(libexif) >= 0.6.16
#BuildRequires:  pkgconfig(libmms) >= 0.4
BuildRequires:  pkgconfig(libpng) >= 1.2
#BuildRequires:  pkgconfig(librsvg-2.0) >= 2.14
#BuildRequires:  pkgconfig(mjpegtools)
BuildRequires:  pkgconfig(openssl) >= 0.9.5
#BuildRequires:  pkgconfig(schroedinger-1.0) >= 1.0.10
BuildRequires:  pkgconfig(sndfile) >= 1.0.16
#BuildRequires:  pkgconfig(wayland-client) >= 0.1
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
Group:          Productivity/Multimedia/Other

%description -n libgstbasecamerabinsrc
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstphotography
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Productivity/Multimedia/Other

%description -n libgstphotography
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstvdp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Productivity/Multimedia/Other

%description -n libgstvdp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstbasevideo
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Productivity/Multimedia/Other

%description -n libgstbasevideo
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstcodecparsers
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Productivity/Multimedia/Other

%description -n libgstcodecparsers
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstsignalprocessor
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Productivity/Multimedia/Other

%description -n libgstsignalprocessor
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package devel
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Development/Libraries/C and C++
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

%package doc
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}

%description doc
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%prep
%setup -q -n %{_name}-%{version}

%build
%configure\
    --disable-static\
    --disable-examples\
    --enable-experimental\
    --enable-gtk-doc \
    --with-gtk=3.0
%__make %{?_smp_mflags} V=1

%install
%make_install
%find_lang %{_name}-%{gst_branch}
mv %{_name}-%{gst_branch}.lang %{name}.lang

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
%if 0%{?BUILD_ORIG}
%if 0%{?BUILD_ORIG_ADDON}


%files -n libgstphotography
%defattr(-, root, root)
%{_libdir}/libgstphotography-%{gst_branch}.so.0*

%files -n libgstbasevideo
%defattr(-, root, root)
%{_libdir}/libgstbasevideo-%{gst_branch}.so.0*

%files -n libgstbasecamerabinsrc
%defattr(-, root, root)
%{_libdir}/libgstbasecamerabinsrc-%{gst_branch}.so.0*

%files -n libgstcodecparsers
%defattr(-, root, root)
%{_libdir}/libgstcodecparsers-%{gst_branch}.so.0*

%files -n libgstsignalprocessor
%defattr(-, root, root)
%{_libdir}/libgstsignalprocessor-%{gst_branch}.so.0*

%if %{build_gstvdp}
%files -n libgstvdp
%defattr(-, root, root)
%{_libdir}/libgstvdp-%{gst_branch}.so.0*
%endif

%files devel
%defattr(-, root, root)
%{_includedir}/gstreamer-%{gst_branch}
%{_libdir}/*.so
%{_libdir}/pkgconfig/gstreamer-basevideo-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-codecparsers-%{gst_branch}.pc
%{_libdir}/pkgconfig/gstreamer-plugins-bad-%{gst_branch}.pc

%files doc
%defattr(-, root, root)
%{_datadir}/gtk-doc/html/gst-plugins-bad-plugins-%{gst_branch}/
%{_datadir}/gtk-doc/html/gst-plugins-bad-libs-%{gst_branch}/
