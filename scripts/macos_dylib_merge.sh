#!/usr/bin/env bash
set -e

files=(libass.9.dylib
       libavcodec.61.19.100.dylib
       libavfilter.10.4.100.dylib
       libavformat.61.7.100.dylib
       libavutil.59.39.100.dylib
       libdav1d.7.dylib
       libfreetype.6.dylib
       libfribidi.0.dylib
       libharfbuzz.0.dylib
       libluajit-5.1.2.1.1727870382.dylib
       libmbedcrypto.3.6.1.dylib
       libmbedtls.3.6.1.dylib
       libmbedx509.3.6.1.dylib
       libmpv.2.dylib
       libplacebo.349.dylib
       libpng16.16.dylib
       libsharpyuv.0.1.0.dylib
       libswresample.5.3.100.dylib
       libswscale.8.3.100.dylib
       libunibreak.6.dylib
       libwebp.7.1.9.dylib)

rm -rf ./universal
mkdir -p ./universal

i=1
for file in "${files[@]}"; do
  echo $i "$file"
  arm64_ret=$(otool -l ./arm64/"$file" | { grep minos || true;} | { grep 11.0 || true;})
  x86_ret=$(otool -l ./x86_64/"$file" | { grep 10.15 || true;})

  if [ -z "$arm64_ret" ]; then
      echo -e "\t\033[31m arm64 不满足 11.0 \033[0m"
  fi

  if [ -z "$x86_ret" ]; then
      echo -e "\t\033[31m x86_64 不满足 10.11 \033[0m"
  fi

  lipo -create -output ./universal/"$file" ./x86_64/"$file" ./arm64/"$file"
  ((i++))
done

output_name="macos_dylib_ffmpeg71_mpv39"
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