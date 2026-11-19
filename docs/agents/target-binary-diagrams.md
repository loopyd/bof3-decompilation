# Target binary diagrams

Each dashboard target card maps original file bytes from left to right, then down.
EXE maps include the original PS-X loader header. EMI maps start at byte zero of
the selected entry, excluding the archive header and other entries.
An EMI layout's leading `bin, header` marks its first four bytes as an entry
header. Longer blocks with that name can contain mixed data or code; the name
does not classify their remaining bytes. Untyped prefixes remain unknown.

Hover over the canvas to highlight a span and show its category, inclusive
hexadecimal bounds, byte length and evidence. Click or tap to retain a selection.
Headers preview their first 32 bytes as spaced hexadecimal pairs, with `...` when
more bytes remain. Four-byte EMI headers show all four pairs. Reviewed text rows
show the text tool's decoded characters and command notation, limited to 160
characters with `...` for truncation. Text-bank metadata has no character preview.
Selection brightens the span's fill without drawing a border over neighboring bytes.
Function details omit analyzer names. Exact lifts show ✓ and partial lifts show
⏳, with accessible status labels. The 🔗 icon opens the function's C source line
on the repository's `main` branch in a new tab. Tab can focus the link. Enter opens it.
One details tooltip is shared across cards and moves to the active diagram.
Its vertical position snaps to the top, middle or bottom for selections in the
corresponding canvas third. Pointer selection uses the pointer's y coordinate; keyboard and chevron
navigation use the span's first row. Resizing preserves that position. Clicking
outside the canvas, its navigation buttons or details closes it, except when using legend help.
Moving keyboard focus away also clears selection, with the same help exception.

Focused diagrams show left and right chevrons for adjacent spans. Arrow keys
also select adjacent spans, Home and End select the first and last spans, and
Escape clears selection. The chevrons disappear when focus leaves the diagram.
Resizing preserves physical offsets and selection.

The `?` button at the canvas's right edge opens the color legend. One help tooltip
is shared across cards, independently of span details, so both can remain open.
Clicking outside help, scrolling or pressing its button again closes it. Escape
also closes help when its button is focused. The legend appears only in help.

## Classifications

| Category | Evidence |
| --- | --- |
| Header | Verified PS-X EXE loader header or reviewed EMI entry header word |
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
`detail`. Spans partition exactly `[0, size)`.
Header and decoded text spans also carry a bounded `preview` string. Decoded
rows retain their physical byte ranges; text-bank metadata remains separate.
Code spans carry `functions` with indexed `address`, `status` and a `source` URL
for valid exact or partial lifts. Unlifted and invalid records use a null source.
Function metadata preserves separate function boundaries and overlapping records.
Header evidence wins over asset ranges, which win over code, then zero fill,
then unknown. Physical asset records can refine their enclosing container.
Conflicts at the same priority are unknown.
The browser rejects old schemas, invalid categories, gaps and overlaps.

Publication includes classification metadata, bounded header/text previews and
static page assets. Full source media, decoded asset files, catalogs and absolute
host paths stay outside the public output. Source URLs point to C files on `main`
and their function declaration lines.
