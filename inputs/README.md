Use `inputs/external/` as the one local-media location: place either one complete
CUE/BIN set there or `BreathOfFireIIIv1.1.7z`. `just setup` discovers and
validates the CUE/BIN set in place; if only the archive is present, it extracts
it into the ignored `inputs/external/private-assets/` cache.

This directory is user-owned rather than repo-owned. The private-assets cache
also holds the PsyQ 4.7 runtime and converted object downloads. It is not a
repository submodule and must not contain tracked user-owned media.
