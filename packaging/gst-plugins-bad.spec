%bcond_with divx4linux

# Disabled for now in upstream tarball, not ported
%define build_gstvdp 0

Name:           gst-plugins-bad
# Patched code is built by default.
# Use rpmbuild -D 'BUILD_ORIG 1' to build original code.
# Use rpmbuild -D 'BUILD_ORIG 1' -D 'BUILD_ORIG_ADDON 1' to build patched build plus original as addon.
%define _name gst-plugins-bad
Version:        1.0.2
Release:        0
# FIXME: missing BuildRequires on pkgconfig(wayland-client)
# FIXME: re-enable opencv BuildRequires once bnc#748666 is fixed (we don't want to depend on libxine!)
# FIXME: missing BuildRequires on libtiger (http://code.google.com/p/libtiger/)
# FIXME: missing BuildRequires on chromaprint (http://acoustid.org/chromaprint)
# FIXME: missing BuildRequires on zbar (http://zbar.sourceforge.net/)
%define gst_branch 1.0
Summary:        GStreamer Streaming-Media Framework Plug-Ins
License:        GPL-2.0+ and LGPL-2.1+
Group:          Productivity/Multimedia/Other
Url:            http://gstreamer.freedesktop.org/
%if 0%{?BUILD_ORIG}
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-bad/%{_name}-%{version}.tar.xz
%else
# WARNING: This is not a comment, but the real command to repack source:
#%(bash %{_sourcedir}/%{name}-patch-source.sh %{_name}-%{version}.tar.xz)
Source:         %{_name}-%{version}-patched.tar.xz
# If the above patch source script fails due to a change in source tarball contents, you need
# to unpack the sources (this should already have happened) and examine the new plugin source for
# blacklisted content and if it's clean, edit the script and add to the allowed list in the script
%endif
Source1:        %{name}-patch-source.sh
Patch0:         gstreamer-plugins-bad-real.patch
BuildRequires:  SDL-devel
# needed for patch0
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
# Not yet in openSUSE
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
%if 0%{?BUILD_ORIG}
# FIXME: Enable more modules.
BuildRequires:  faac-devel
BuildRequires:  faad2-devel
BuildRequires:  libaudio-devel
BuildRequires:  libdca-devel
BuildRequires:  libgmyth-devel
BuildRequires:  librtmp-devel
BuildRequires:  libxvidcore-devel
BuildRequires:  pkgconfig(vo-aacenc) >= 0.1.0
BuildRequires:  pkgconfig(vo-amrwbenc) >= 0.1.0
%ifarch %ix86
# binary only so make conditional
%if %{with divx4linux}
BuildRequires:  divx4linux-devel
%endif
%endif
%endif
Requires(post): glib2-tools
Requires(postun): glib2-tools
%define gstreamer_plugins_bad_req %(xzgrep --text "^GST.*_REQ" %{S:0} | sort -u | sed 's/GST_REQ=/gstreamer >= /;s/GSTPB_REQ=/gstreamer-plugins-base >= /' | tr '\\n' ' ')
Requires:       %gstreamer_plugins_bad_req
Enhances:       gstreamer
# Generic name, never used in SuSE:
Provides:       gst-plugins-bad = %{version}
%if 0%{?BUILD_ORIG}
%if 0%{?BUILD_ORIG_ADDON}
Provides:       patched_subset
%else
Provides:       %{name}-orig-addon = %{version}
Obsoletes:      %{name}-orig-addon
%endif
%else
Provides:       patched_subset
%endif
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%if 0%{?BUILD_ORIG}
%if 0%{?BUILD_ORIG_ADDON}

%package orig-addon
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          Productivity/Multimedia/Other
Requires:       %{name} >= %{version}
Supplements:    %{name}
# Generic name, never used in SuSE:
Provides:       gst-plugins-bad:%{_libdir}/gstreamer-%{gst_branch}/libgstxvid.so = %{version}

%description orig-addon
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related,from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%endif
%endif

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
%if %{build_gstvdp}
Requires:       libgstvdp = %{version}
%endif

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
%patch0

%build
# Needed for patch0
autoconf
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

%postun -n libgstvdp -p /sbin/ldconfig

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

%files orig-addon
%defattr(-, root, root)
%endif
%ifarch %ix86
%if %{with divx4linux}
%{_libdir}/gstreamer-%{gst_branch}/libgstdivxdec.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdivxenc.so
%endif
%endif
%{_datadir}/gstreamer-%{gst_branch}/presets/GstAmrwbEnc.prs
%{_libdir}/gstreamer-%{gst_branch}/libgstamrwbenc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdtsdec.so
%{_libdir}/gstreamer-%{gst_branch}/libgstdvdspu.so
%{_libdir}/gstreamer-%{gst_branch}/libgstfaac.so
%{_libdir}/gstreamer-%{gst_branch}/libgstfaad.so
%{_libdir}/gstreamer-%{gst_branch}/libgstmythtvsrc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstnassink.so
%{_libdir}/gstreamer-%{gst_branch}/libgstrtmp.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsiren.so
%{_libdir}/gstreamer-%{gst_branch}/libgstxvid.so
%endif

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
