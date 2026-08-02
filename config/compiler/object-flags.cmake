#
# Per-object compiler variant override:
#   set(BOF3_OBJCOMPILER_<sanitized_src_relative_path> <catalog-id>)
# The <catalog-id> must match an entry in config/compiler/variants.json.
# CMake wraps the compile command in `cmake -E env PSX_GCC=<verified-path>`
# when this is set. No entry means canonical gcc-2.7.2-psx is used.
