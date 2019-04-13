ARCH_GET=`uname -m`
if [ "$ARCH_GET" = "x86_64" ];
then
	ARCH=x64
else
	ARCH=i386
fi
tar -zxf ffmpeg-2.8.6.tar.gz
cd ffmpeg-2.8.6
./configure --enable-static --disable-shared --disable-everything \
--disable-muxers --disable-muxer=ffm --disable-muxer=mov \
--disable-demuxers --disable-demuxer=asf --disable-demuxer=mpegts \
--disable-demuxer=rtsp --disable-demuxer=mov --disable-demuxer=rm \
--disable-protocols --enable-protocol=file --disable-protocol=rtp \
--disable-protocol=udp --disable-protocol=http --disable-protocol=tcp \
--disable-filters --disable-filter=aformat --disable-filter=format \
--disable-filter=setpts --disable-filter=anull --disable-filter=hflip \
--disable-filter=transpose --disable-filter=atrim --disable-filter=null \
--disable-filter=trim --disable-filter=crop --disable-filter=rotate --disable-filter=vflip \
--enable-decoder=hevc --enable-decoder=h264 --enable-muxer=mp4 --enable-muxer=mov \
--enable-parser=hevc --enable-parser=h264 --disable-debug --prefix=./bulid --disable-yasm
make
make install
cp -rf ./bulid/lib/* ../../ffmpeg/lib/$ARCH
