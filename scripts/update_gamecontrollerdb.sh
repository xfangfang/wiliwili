#!/usr/bin/env bash

# cd to wiliwili/resources/gamepad
cd "$(dirname "$0")/../resources/gamepad" || exit

wget https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt -O gamecontrollerdb.txt