#!/bin/bash

boost_version="1.85.0"
webp_version="1.4.0"
mpv_version="0.38.0"

# cd to wiliwili" "
cd "$(dirname "$0")/.." || exit

rm -rf include
mkdir -p include

boost_item=("filesystem" "system" "config" "assert" "iterator" "type_traits" "mpl" "preprocessor" "static_assert" "core" "detail" "io" "functional" "container_hash" "smart_ptr")
for item in "${boost_item[@]}"
do
   url="https://github.com/boostorg/$item/archive/refs/tags/boost-$boost_version.zip"
   curl -L -o boost.zip "$url"
   unzip -q boost.zip
   dir_name=$(unzip -Z -1 boost.zip | head -1 | cut -d " " -f 1)
   echo "$dir_name"
   cp -r "$dir_name"/include/boost include
   rm boost.zip
   rm -r "$dir_name"
done

curl -L -o webp.zip "https://github.com/webmproject/libwebp/archive/refs/tags/v$webp_version.zip"
unzip -q webp.zip
dir_name=$(unzip -Z -1 webp.zip | head -1 | cut -d " " -f 1)
echo "$dir_name"
cp -r "$dir_name"/src/webp include
rm webp.zip
rm -r "$dir_name"

curl -L -o mpv.zip "https://github.com/mpv-player/mpv/archive/refs/tags/v$mpv_version.zip"
unzip -q mpv.zip
dir_name=$(unzip -Z -1 mpv.zip | head -1 | cut -d " " -f 1)
echo "$dir_name"
cp -r "$dir_name"/libmpv include
mv include/libmpv include/mpv
rm mpv.zip
rm -r "$dir_name"