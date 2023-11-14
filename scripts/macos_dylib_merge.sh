#!/usr/bin/env bash
set -e

files=(libass.9.dylib
       libavcodec.60.31.102.dylib
       libavdevice.60.3.100.dylib
       libavfilter.9.12.100.dylib
       libavformat.60.16.100.dylib
       libavutil.58.29.100.dylib
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
       libswresample.4.12.100.dylib
       libswscale.7.5.100.dylib
       libtasn1.6.dylib
       libunibreak.5.dylib
       libunistring.5.dylib
       libwebp.7.1.8.dylib)

rm -rf ./universal
mkdir -p ./universal

i=1
for file in "${files[@]}"; do
  echo $i "$file"
  arm64_ret=$(otool -l ./arm64/"$file" | { grep minos || true;} | { grep 11.0 || true;})
  x86_ret=$(otool -l ./x86_64/"$file" | { grep 10.11 || true;})

  if [ -z "$arm64_ret" ]; then
      echo -e "\t\033[31m arm64 不满足 11.0 \033[0m"
  fi

  if [ -z "$x86_ret" ]; then
      echo -e "\t\033[31m x86_64 不满足 10.11 \033[0m"
  fi

  lipo -create -output ./universal/"$file" ./x86_64/"$file" ./arm64/"$file"
  ((i++))
done

output_name="macos_dylib_ffmpeg61_mpv36"
arch_list=("arm64" "x86_64" "universal")
for i in "${arch_list[@]}";
do
  echo "$i";
  package_name="${output_name}_${i}.tar.gz"
  rm -rf "$package_name"
  tar -czvf "$package_name" "${i}"
done