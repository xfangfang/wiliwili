set -e

files=(libass.9.dylib
       libavcodec.60.3.100.dylib
       libavdevice.60.1.100.dylib
       libavfilter.9.3.100.dylib
       libavformat.60.3.100.dylib
       libavutil.58.2.100.dylib
       libboost_atomic-mt.dylib
       libboost_filesystem-mt.dylib
       libcrypto.3.dylib
       libdav1d.6.dylib
       libfontconfig.1.dylib
       libfreetype.6.dylib
       libfribidi.0.dylib
       libglib-2.0.0.dylib
       libgmp.10.dylib
       libgnutls.30.dylib
       libgraphite2.3.2.1.dylib
       libharfbuzz.0.dylib
       libhogweed.6.8.dylib
       libidn2.0.dylib
       libintl.8.dylib
       libmpv.2.dylib
       libnettle.8.8.dylib
       libp11-kit.0.dylib
       libpcre2-8.0.dylib
       libpng16.16.dylib
       libsharpyuv.0.0.1.dylib
       libssl.3.dylib
       libswresample.4.10.100.dylib
       libswscale.7.1.100.dylib
       libtasn1.6.dylib
       libunibreak.5.dylib
       libunistring.5.dylib
       libwebp.7.1.8.dylib)

for file in "${files[@]}"; do
  echo "$file"
  lipo -create -output ./universal/"$file" ./x86_64/"$file" ./arm64/"$file"
done
