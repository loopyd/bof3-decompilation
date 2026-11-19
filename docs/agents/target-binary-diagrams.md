# Target binary diagrams

Each dashboard target card maps original file bytes from left to right, then down.
EXE maps include the original PS-X loader header. EMI maps start at byte zero of
the selected entry, excluding the archive header and other entries.

Hover over the canvas to highlight a span and show its category, inclusive
hexadecimal bounds, byte length and evidence. Click or tap to retain a selection.
With the canvas focused, arrow keys select adjacent spans, Home and End select
the first and last spans, and Escape clears selection. The HTML legend lists
all supported colors. Resizing preserves physical offsets and selection.

## Classifications

| Category | Evidence |
| --- | --- |
| Header | Verified PS-X EXE loader header |
| Audio | Recognized physical entry selected by the audio index/query tool |
| Text | Verified text bank or literal allocation from text index/query |
| Texture | Recognized stored graphics container or physical image/palette record |
| Code (unlifted) | Indexed function with unlifted or invalid lift status |
| Code (lifted) | Indexed function with valid exact or partial source; details retain status |
| Table | Reviewed jump table or recognized physical asset metadata table |
| Unknown | No verified classification, or conflicting evidence at equal precedence |
| Empty (padding) | At least 16 consecutive zero bytes outside stronger classifications |

Zero fill does not prove semantic padding. Generic data and rodata are not
automatically tables. Compressed graphics use their stored container range;
decoded-bank offsets are excluded. The native audio and texture catalogs cover
EMI containers; executable maps also use verified text and indexed/layout facts.
The overview can hide subpixel spans. Keyboard traversal still visits exact spans.
Byte colors do not change function counts or establish runtime behavior.

## Snapshot and publication

`bin/harness pages build --help` describes local builds; `pages preview --help`
describes exact-commit previews. Builds require original media under
`out/extracted/`, target binaries, the fresh reverse index and installed native
toolchains. Text, audio and texture tools are built from the publication checkout.
Each source is indexed/queried once per build, and its bytes are checked against
`config/original-us-disc.json` and the selected target payload. Unsupported text
layouts remain unknown. Missing sources, tool failures and inconsistent bounds
stop publication.

`status.json` uses `bof3.decomp-status/v3`. `binary_categories` contains unique
category IDs, labels and colors. Each target's `binary` contains `size`, `sha256`,
`source` (`id`, original source `sha256`, archive `offset`) and ordered `spans`.
Every span has half-open `start`/`end` file offsets, a `category` and evidence
`detail`. Spans partition exactly `[0, size)`. Header evidence wins over asset
ranges, which win over code, then zero fill, then unknown. Physical asset records
can refine their enclosing container. Conflicts at the same priority are unknown.
The browser rejects old schemas, invalid categories, gaps and overlaps.

Publication includes classification metadata and static page assets only. Original
media, decoded assets, catalogs and local paths stay outside the public output.
