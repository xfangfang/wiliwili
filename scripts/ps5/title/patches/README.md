# Patches

Small changes to the third party sources the native title links, kept here so
that a clean checkout of each dependency can be brought to the exact state the
build expects.

## ps5-opengl-sdl2-static.patch

Applied to `integration/SDL2/CMakeLists.txt` of the ps5-opengl v1.0.2 checkout
(`scripts/ps5/title/build_sdl2.sh` applies it before running their builder).

It makes three changes:

* **`SDL_AUDIO` stays on.** Upstream turns every optional SDL subsystem off for
  the native target. The media player reaches the console's audio output through
  SDL's own PS5 backend (`SDL_ps5audio.c`), which only compiles under
  `SDL_AUDIO`.
* **CMake 4 compatibility.** SDL's own `CMakeLists.txt` still declares 3.0
  compatibility, which CMake 4 refuses outright; the policy minimum is raised
  just for that configure run.
* **LLVM archiver.** Apple's `ar` silently drops members that are not Mach-O
  objects, which leaves a `libSDL2.a` with no target code in it. The archiver
  and ranlib are taken from the LLVM installation `PS5_CLANG` points at.
