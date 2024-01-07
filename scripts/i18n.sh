#!/usr/bin/env bash

# cd to wiliwili/resources/i18n
cd "$(dirname "$0")/../resources/i18n" || exit

LANG_PATH="$HOME/Downloads/wiliwili (translations)"

if [ -n "$1" ] ;then
  LANG_PATH=$1
fi

echo "Load language files from: $LANG_PATH"

cp "$LANG_PATH/ja/hints.json" ./ja
cp "$LANG_PATH/ja/wiliwili.json" ./ja
cp "$LANG_PATH/ry/hints.json" ./ja-RYU
cp "$LANG_PATH/ry/wiliwili.json" ./ja-RYU
cp "$LANG_PATH/ko/hints.json" ./ko
cp "$LANG_PATH/ko/wiliwili.json" ./ko
cp "$LANG_PATH/zh-CN/hints.json" ./zh-Hans
cp "$LANG_PATH/zh-CN/wiliwili.json" ./zh-Hans
cp "$LANG_PATH/zh-TW/hints.json" ./zh-Hant
cp "$LANG_PATH/zh-TW/wiliwili.json" ./zh-Hant
cp "$LANG_PATH/it/hints.json" ./it
cp "$LANG_PATH/it/wiliwili.json" ./it

echo "Load language files done"

rm -r "$LANG_PATH"