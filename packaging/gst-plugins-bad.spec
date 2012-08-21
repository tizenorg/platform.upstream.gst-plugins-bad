#sbs-git:slp/pkgs/g/gst-plugins-bad0.10 gst-plugins-bad 0.10.23 3e50f3f786fc9956e191c290b2169d52808cd441
Name:       gst-plugins-bad
Summary:    GStreamer plugins from the "bad" set
Version:    0.10.23
Release:    9
Group:      Applications/Multimedia
License:    LGPLv2+
Source0:    %{name}-%{version}.tar.gz
Patch0:     gst-plugins-bad-disable-gtk-doc.patch
BuildRequires:  gettext
BuildRequires:  gst-plugins-base-devel  
BuildRequires:  pkgconfig(gstreamer-0.10) 
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(liboil-0.3)
BuildRequires:  pkgconfig(libcrypto)

%description
GStreamer is a streaming media framework, based on graphs of filters
which operate on media data.  Applications using this library can do
anything from real-time sound processing to playing videos, and just
about anything else media-related.  Its plugin-based architecture means
that new data types or processing capabilities can be added simply by
installing new plug-ins.
.
GStreamer Bad Plug-ins is a set of plug-ins that aren't up to par compared
to the rest. They might be close to being good quality, but they're missing
something - be it a good code review, some documentation, a set of tests, a
real live maintainer, or some actual wide use.




%prep
%setup -q 
%patch0 -p1


%build
./autogen.sh

export CFLAGS+=" -Wall -g -fPIC\
 -DGST_EXT_HLS_MODIFICATION\
 -DGST_H263PARSE_MODIFICATION\
 -DGST_H264PARSE_MODIFICATION"

%configure  --prefix=%{_prefix}\
 --disable-static\
 --disable-nls\
 --with-html-dir=/tmp/dump\
 --disable-examples\
 --disable-adpcmdec\
 --disable-aiff\
 --disable-amrparse\
 --disable-asfmux\
 --disable-bayer\
 --disable-cdxaparse\
 --disable-dccp\
 --disable-dvdspu\
 --disable-festival\
 --disable-freeze\
 --disable-frei0r\
 --disable-hdvparse\
 --disable-librfb\
 --disable-modplug\
 --disable-mpegpsmux\
 --disable-mve\
 --disable-mxf\
 --disable-nsf\
 --disable-nuvdemux\
 --disable-pcapparse\
 --disable-pnm\
 --disable-qtmux\
 --disable-real\
 --disable-scaletempo\
 --disable-shapewipe\
 --disable-siren\
 --disable-speed\
 --disable-subenc\
 --disable-stereo\
 --disable-tta\
 --disable-videomeasure\
 --disable-videosignal\
 --disable-vmnc\
 --disable-directsound\
 --disable-directdraw\
 --disable-osx_video\
 --disable-vcd\
 --disable-assrender\
 --disable-amrwb\
 --disable-apexsink\
 --disable-bz2\
 --disable-cdaudio\
 --disable-celt\
 --disable-cog\
 --disable-dc1394\
 --disable-directfb\
 --disable-dirac\
 --disable-dts\
 --disable-divx\
 --disable-dvdnav\
 --disable-faac\
 --disable-faad\
 --disable-fbdev\
 --disable-gsm\
 --disable-ivorbis\
 --disable-jack\
 --disable-jp2k\
 --disable-kate\
 --disable-ladspa\
 --disable-lv2\
 --disable-libmms\
 --disable-modplug\
 --disable-mimic\
 --disable-mpeg2enc\
 --disable-mplex\
 --disable-musepack\
 --disable-musicbrainz\
 --disable-mythtv\
 --disable-nas\
 --disable-neon\
 --disable-ofa\
 --disable-timidity\
 --disable-wildmidi\
 --disable-sdl\
 --disable-sdltest\
 --disable-sndfile\
 --disable-soundtouch\
 --disable-spc\
 --disable-gme\
 --disable-swfdec\
 --disable-theoradec\
 --disable-xvid\
 --disable-dvb\
 --disable-oss4\
 --disable-wininet\
 --disable-acm\
 --disable-vdpau\
 --disable-schro\
 --disable-vp8\
 --disable-zbar\
 --disable-dataurisrc\
 --disable-shm\
 --disable-coloreffects\
 --disable-colorspace\
 --disable-videomaxrate\
 --disable-jp2kdecimator\
 --disable-interlace\
 --disable-gaudieffects\
 --disable-y4m\
 --disable-adpcmdec\
 --disable-adpcmenc\
 --disable-segmentclip\
 --disable-geometrictransform\
 --disable-inter\
 --disable-dvbsuboverlay\
 --disable-ivfparse\
 --disable-gsettings\
 --disable-opus\
 --disable-openal\
 --disable-snapdsp\
 --disable-teletextdec\
 --disable-audiovisualizers\
 --disable-faceoverlay\
 --disable-freeverb\
 --disable-removesilence\
 --disable-smooth\
 --disable-avc\
 --disable-d3dvideosink\
 --disable-pvr2d


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install




%files
%defattr(-,root,root,-)
%{_libdir}/gstreamer-0.10/libgstautoconvert.so
%{_libdir}/gstreamer-0.10/libgstvideofiltersbad.so
%{_libdir}/gstreamer-0.10/libgstrtpvp8.so
%{_libdir}/gstreamer-0.10/libgstdtmf.so
%{_libdir}/gstreamer-0.10/libgstfieldanalysis.so
%{_libdir}/gstreamer-0.10/libgstmpegtsdemux.so
%{_libdir}/gstreamer-0.10/libgstmpegtsmux.so
%{_libdir}/gstreamer-0.10/libgstfragmented.so
%{_libdir}/gstreamer-0.10/libgstvideoparsersbad.so
%{_libdir}/gstreamer-0.10/libgstrtpmux.so
%{_libdir}/gstreamer-0.10/libgstliveadder.so
%{_libdir}/gstreamer-0.10/libgstpatchdetect.so
%{_libdir}/gstreamer-0.10/libgstmpegdemux.so
%{_libdir}/gstreamer-0.10/libgstlegacyresample.so
%exclude %{_libdir}/gstreamer-0.10/libgstcamerabin.so
%exclude %{_libdir}/gstreamer-0.10/libgstcamerabin2.so
%{_libdir}/gstreamer-0.10/libgsth264parse.so
%{_libdir}/gstreamer-0.10/libgstjpegformat.so
%{_libdir}/gstreamer-0.10/libgstmpegvideoparse.so
%{_libdir}/gstreamer-0.10/libgstlinsys.so
%{_libdir}/gstreamer-0.10/libgstsdi.so
%{_libdir}/gstreamer-0.10/libgstdecklink.so
%{_libdir}/gstreamer-0.10/libgstid3tag.so
%{_libdir}/gstreamer-0.10/libgstsdpelem.so
%{_libdir}/gstreamer-0.10/libgstrawparse.so
%{_libdir}/gstreamer-0.10/libgstdebugutilsbad.so
%{_libdir}/libgstbasevideo-0.10.so*
%exclude %{_libdir}/libgstphotography-0.10.so*
%exclude %{_libdir}/libgstsignalprocessor-0.10.so*
%{_libdir}/libgstcodecparsers-0.10.so*
%exclude %{_libdir}/libgstbasecamerabinsrc-0.10.so*
%exclude %{_includedir}/gstreamer-0.10/gst/basecamerabinsrc/gstbasecamerasrc.h
%exclude %{_includedir}/gstreamer-0.10/gst/basecamerabinsrc/gstcamerabin-enum.h
%exclude %{_includedir}/gstreamer-0.10/gst/basecamerabinsrc/gstcamerabinpreview.h
%exclude %{_includedir}/gstreamer-0.10/gst/codecparsers/gsth264parser.h
%exclude %{_includedir}/gstreamer-0.10/gst/codecparsers/gstmpeg4parser.h
%exclude %{_includedir}/gstreamer-0.10/gst/codecparsers/gstmpegvideoparser.h
%exclude %{_includedir}/gstreamer-0.10/gst/codecparsers/gstvc1parser.h
%exclude %{_includedir}/gstreamer-0.10/gst/interfaces/photography-enumtypes.h
%exclude %{_includedir}/gstreamer-0.10/gst/interfaces/photography.h
%exclude %{_includedir}/gstreamer-0.10/gst/signalprocessor/gstsignalprocessor.h
%exclude %{_includedir}/gstreamer-0.10/gst/video/gstbasevideocodec.h
%exclude %{_includedir}/gstreamer-0.10/gst/video/gstbasevideodecoder.h
%exclude %{_includedir}/gstreamer-0.10/gst/video/gstbasevideoencoder.h
%exclude %{_includedir}/gstreamer-0.10/gst/video/gstbasevideoutils.h
%exclude %{_includedir}/gstreamer-0.10/gst/video/gstsurfacebuffer.h
%exclude %{_includedir}/gstreamer-0.10/gst/video/gstsurfaceconverter.h
%exclude %{_includedir}/gstreamer-0.10/gst/video/videocontext.h
%exclude %{_libdir}/pkgconfig/gstreamer-plugins-bad-0.10.pc
%exclude %{_libdir}/pkgconfig/gstreamer-basevideo-0.10.pc
%exclude %{_libdir}/pkgconfig/gstreamer-codecparsers-0.10.pc

