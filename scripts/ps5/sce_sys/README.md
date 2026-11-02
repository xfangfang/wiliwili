# Home screen artwork

The console reads these from `sce_sys/` once the title folder is published; the
build copies the ones it finds there into the title (`scripts/ps5/title/build_title.sh`).

| File | Shown | Format |
|---|---|---|
| `icon0.png` | the tile on the home screen | 512 × 512 PNG, no transparency |
| `pic0.dds` | background while the title is selected | 3840 × 2160 BC7_UNORM DX10 DDS, no mipmaps |
| `pic1.dds` | background while the title launches | same as `pic0.dds` |
| `background-source.png` | nothing — the editable source of both backgrounds | PNG |

The console wants both backgrounds or neither, so they are committed rather
than produced during a build: a BC7 encoder is not part of the toolchain.

`background-source.png` is the source image cropped to 16:9 and scaled to
3840 × 2160. To regenerate the pair from it, encode each 4 × 4 block to BC7 and
wrap the result in a DDS header with `DXGI_FORMAT_BC7_UNORM` (98), a single
`D3D10_RESOURCE_DIMENSION_TEXTURE2D` (3) surface, `mipMapCount` 1 and a linear
size of `width / 4 * height / 4 * 16` — the profile the loader validates.
