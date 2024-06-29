#!/usr/bin/env bash
set -e

files=(libass.9.dylib
       libavcodec.61.3.100.dylib
       libavdevice.61.1.100.dylib
       libavfilter.10.1.100.dylib
       libavformat.61.1.100.dylib
       libavutil.59.8.100.dylib
       libboost_atomic-mt.dylib
       libboost_filesystem-mt.dylib
       libcrypto.3.dylib
       libdav1d.7.dylib
       libfontconfig.1.dylib
       libfreetype.6.dylib
       libfribidi.0.dylib
       libglib-2.0.0.dylib
       libgmp.10.dylib
       libgnutls.30.dylib
       libgraphite2.3.2.1.dylib
       libharfbuzz.0.dylib
       libhogweed.6.9.dylib
       libidn2.0.dylib
       libintl.8.dylib
       libjpeg.8.3.2.dylib
       liblcms2.2.dylib
       libmpv.2.dylib
       libnettle.8.9.dylib
       libp11-kit.0.dylib
       libpcre2-8.0.dylib
       libplacebo.338.dylib
       libpng16.16.dylib
       libshaderc_shared.1.dylib
       libsharpyuv.0.1.0.dylib
       libssl.3.dylib
       libswresample.5.1.100.dylib
       libswscale.8.1.100.dylib
       libtasn1.6.dylib
       libunibreak.6.dylib
       libunistring.5.dylib
       libvulkan.1.3.280.dylib
       libwebp.7.1.9.dylib)

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

cp -r ./arm64/cmake ./universal/

output_name="macos_dylib_ffmpeg7_mpv38"
arch_list=("arm64" "x86_64" "universal")
for i in "${arch_list[@]}";
do
  echo "$i";
  mv "${i}" lib
  package_name="${output_name}_${i}.tar.gz"
  rm -rf "$package_name"
  tar -czvf "$package_name" lib include
  mv lib "${i}"
done