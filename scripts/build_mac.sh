set -e

# Do not run this script manually

# Use cmake to build a macOS app
# cmake -B build -DPLATFORM_DESKTOP=ON -DCMAKE_BUILD_TYPE=Release
# make -C build wiliwili.app -j$(nproc)

# If you want to package and share it with others,
# you need to install the dependency: dylibbundler, and then re-execute the make command.

BUILD_DIR=$(pwd)

# cd to wiliwili
cd "$(dirname "$0")/.."

APP_PATH="${BUILD_DIR}"/wiliwili.app

rm -rf "${APP_PATH}"

mkdir -p "${APP_PATH}"/Contents
mkdir -p "${APP_PATH}"/Contents/MacOS
mkdir -p "${APP_PATH}"/Contents/Resources


cp ./scripts/mac/Info.plist "${APP_PATH}"/Contents/Info.plist

version=$3
git_tag=$(git rev-parse --short HEAD)

/usr/bin/sed -i '' '35s/1.0/'"${version}"'/' "${APP_PATH}"/Contents/Info.plist
/usr/bin/sed -i '' '39s/1.0/'"${git_tag}"'/' "${APP_PATH}"/Contents/Info.plist

cp ./scripts/mac/AppIcon.icns "${APP_PATH}"/Contents/Resources/AppIcon.icns
cp "${BUILD_DIR}"/wiliwili "${APP_PATH}"/Contents/MacOS/wiliwili
cp -r ./resources "${APP_PATH}"/Contents/Resources/

if ! command -v dylibbundler >/dev/null 2>&1; then
    echo -e "\033[31m\"dylibbundler\" is not installed. The application you built can only be used locally\033[0m"
    echo -e "For more information, please refer to: \033[36mhttps://github.com/xfangfang/wiliwili/issues/83#issuecomment-1415858949\033[0m"
else
  bundle_deps=""
  if [[ "$1" != "-nb" ]]; then
    echo "bundle deps"
    bundle_deps="-b"
  else
    echo "bundle deps: $2"
    cp -r "$2" "${APP_PATH}"/Contents/MacOS/lib
  fi
  dylibbundler -cd ${bundle_deps} -x "${APP_PATH}"/Contents/MacOS/wiliwili \
    -d "${APP_PATH}"/Contents/MacOS/lib/ -p @executable_path/lib/
  codesign --sign - --force "${APP_PATH}"/Contents/MacOS/lib/*
fi
