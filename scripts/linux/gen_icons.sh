#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

icon_dirs=(16 24 32 48 64 128 256)

for icon_dir in "${icon_dirs[@]}"; do
    icon_path="icons/${icon_dir}x${icon_dir}"
    mkdir -p "icons/${icon_dir}x${icon_dir}"
    icon_path=${icon_path}/cn.xfangfang.wiliwili.png
    vips thumbnail ../../resources/icon/icon.png "${icon_path}" "${icon_dir}"
done
