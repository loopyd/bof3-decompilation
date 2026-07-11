---
type: Media format
title: STR playback
description: BOF3 extracted STR and XA sector representation.
tags: [str, xa, media]
---

# STR playback

BOF3 stores extracted STR/XA data as `2336` byte sectors without the outer raw
CD sync and header:

```text
[XA subheader 8][payload 2324][EDC 4]
```

Generic PSX media tools commonly expect `2352` byte raw sectors. Rewrap the
sector before decoding; do not alter the inner payload.

## Known files

| Path | Content |
| --- | --- |
| `LOGO/CAPCOM30.STR` | MDEC video plus stereo XA |
| `BIN/BMAG_XA/MAGIC00.STR` | multiplexed mono XA |
| `BIN/SCE_XA/S_XA00.STR` | multiplexed stereo XA |
| `BIN/SCE_XA/VOICE.STR` | multiplexed mono XA |

`CAPCOM30.STR` contains standard inner STR/MDEC chunk headers and plays at about
30 fps; the three `BIN/` files are audio banks, not movies.
