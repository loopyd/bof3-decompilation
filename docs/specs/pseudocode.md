---
type: Reference
title: Verified pseudocode index
description: Source-backed runtime and data-extraction algorithms for BOF3 PSX binaries.
tags: [algorithms, runtime, extraction, verification]
---

# Verified pseudocode index

Compact algorithms derived from reviewed source, original-byte analysis, and
owning specs. Pseudocode explains control and data flow; it is not a replacement
for exact-match C or the linked binary-layout evidence. The [format conversion
map](formats/conversion.md) identifies lossless interchange, preview formats,
validators, and provenance that each flow must retain.

## Runtime index

| Flow | Evidence owner | Confidence boundary |
| --- | --- | --- |
| EMI loading and dispatch | [EMI loader](runtime/emi-loader.md), [EMI format](formats/emi.md) | Handler roles are bounded; types 4 and 5 remain unresolved. |
| Type-3 texture upload and palettes | [EMI graphics](formats/graphics.md) | Upload geometry is verified; palette association is draw-state-owned. |
| Sprites and GPU primitives | [Frontend flow](runtime/frontend.md), reviewed source | Construction examples are verified; there is no single universal sprite format. |
| VAB, sequence, and STR/XA media | [EMI loader](runtime/emi-loader.md), [STR/XA](formats/str-xa.md) | Container roles are verified; runtime CDDA control is not recovered. |
| Monsters and formations | [Area data](data/areas.md), [archive ownership](archives/ownership.md) | Storage extraction is verified; enemy runtime lifecycle is out of scope here. |

## Runtime flows

### EMI archive loading and entry dispatch

```text
function load_emi_archive(slot):
    begin_stream(slot)
    while loader_is_not_ready():
        service_scheduler_once()

    for entry in archive_toc:
        assert entry_payload_is_0x800_aligned
        dispatch_by_type(entry)

function dispatch_by_type(entry):
    if entry.type == 0:
        copy_payload_to_ram(entry.load_argument)
    else if entry.type in {1, 2}:
        queue_ram_transfer(entry)
    else if entry.type == 3:
        queue_vram_chunks(entry)
    else if entry.type in {4, 5}:
        invoke_shared_special_handler(entry)  # semantics unresolved
    else if entry.type == 6:
        stage_vab_header(entry)
    else if entry.type == 7:
        stage_vab_body(entry)
    else if entry.type == 8:
        stage_auxiliary_audio_payload(entry)
    else if entry.type in {9, 10}:
        stage_sequence_side_payload(entry)
```

The archive is never the executable target: classify each extracted entry
independently. Type `0` permits a RAM copy but does not prove code. Exact
handlers and addresses are listed in the [loader dispatch table](runtime/emi-loader.md#entry-dispatch).

Testable invariants:

- `bin/emi-ex` can extract and inspect an EMI archive without changing tracked
  target facts; generated evidence remains under `out/`.
- Every payload offset is `0x800`-aligned and every next offset uses
  `(size + 0x7ff) & ~0x7ff`.
- A promoted entry's archive path, slot, payload hash, and load address must
  agree with its target manifest before any function diff is meaningful.

### Type-3 VRAM upload and separate palette mapping

```text
function queue_vram_chunks(entry):
    descriptor = entry.load_argument
    base_x_words = ((descriptor >> 24) & 0x3f) * 32
    base_y_rows = ((descriptor >> 16) & 0x1f) * 32
    chunks_per_row = (descriptor >> 8) & 0x3f

    for each 0x800-byte payload chunk with chunk_index:
        column = chunk_index % chunks_per_row
        row = chunk_index // chunks_per_row
        destination = {
            x_words: base_x_words + column * 32,
            y_rows: base_y_rows + row * 32,
            width_words: 32,
            height_rows: 32,
        }
        upload_0x800_bytes_to_vram(destination)

function decode_psx_color(raw_u16):
    return {
        red5: raw_u16 & 0x1f,
        green5: (raw_u16 >> 5) & 0x1f,
        blue5: (raw_u16 >> 10) & 0x1f,
        stp: (raw_u16 >> 15) & 1,
    }
```

Each chunk is `32x32` 16-bit VRAM words: `128x32` pixels at 4bpp, `64x32`
at 8bpp, or `32x32` at 16bpp. CLUT bytes commonly arrive through a separate,
small type-`0` RAM payload. The texture payload does not identify its palette;
primitive/draw data supplies the CLUT selection. See [EMI graphics](formats/graphics.md#palette).

Testable invariants:

- Each queued full chunk consumes exactly `0x800` bytes and covers `32 * 32`
  16-bit VRAM words.
- A 4bpp CLUT row is `0x20` bytes; an 8bpp CLUT row is `0x200` bytes.
- Reconstructing the same texture with a different CLUT may change displayed
  colors but must not change the type-`3` texture bytes.

### Sprite and primitive construction

```text
function draw_indexed_sprite(x, y, sprite_id, flags):
    rectangle = lookup_sprite_rectangle(sprite_id, flags & 1)
    primitive = allocate_gt_quad()
    initialize_gpu_primitive(primitive)
    set_semitransparency(primitive, disabled)
    set_xy_and_uv_from_rectangle(primitive, x, y, rectangle)
    primitive.clut = choose_clut(flags bit 1)
    append_primitive_to_ordering_table(primitive)

function tint_primitive(primitive, alpha):
    primitive.red = alpha
    primitive.green = alpha
    primitive.blue = alpha
```

The first flow is evidenced by exact-matching `GAME.EMI#0 @ 0x801af2a0`;
the tint helper is exact at `GAME.EMI#1 @ 0x801d18e8`. Frontend glyph geometry
is table-driven, but its constructor at `0x801d17d8` is not yet an exact C
match. Do not generalize these layouts to every PSX primitive.

Executable checks:

```sh
bin/asm-diff emi/etc/game/00@0x801AF2A0
bin/byte-match emi/etc/game/00@0x801AF2A0
bin/asm-diff emi/etc/game/01@0x801D18E8
bin/byte-match emi/etc/game/01@0x801D18E8
```

Both checks must report exact instruction and byte matches. A visual render is
supporting evidence only; it does not replace the canonical binary diff.

### Audio and sector media

```text
function rewrap_extracted_str_xa(sector_2336):
    assert len(sector_2336) == 2336
    assert sector_2336 == xa_subheader_8 + payload_2324 + edc_4
    return raw_cd_sync_12 + raw_cd_header_4 + sector_2336

function align_desktop_audio(video_frames, fps, audio_samples, sample_rate):
    video_seconds = video_frames / fps
    audio_seconds = audio_samples / sample_rate
    pad_samples = max(0, round((video_seconds - audio_seconds) * sample_rate))
    padded_audio = append_silence_per_channel(audio_samples, pad_samples)
    assert durations_equal_within_one_sample(video_seconds, padded_audio)
    return padded_audio
```

EMI types `6` and `7` form VAB header/body pairs; type `10` is sequence data.
Extracted STR/XA files omit the outer 16 raw-sector bytes, so generic 2352-byte
tools require rewrapping without changing the inner payload. Channel/file
demultiplexing and decoding remain tool concerns. No source-backed CDDA runtime
control algorithm is currently recovered; do not treat XA and CDDA as synonyms.

Testable invariants:

- Extracted STR/XA size is divisible by `2336`; rewrapped size is sector count
  times `2352`; each inner 2336-byte sector is preserved byte-for-byte.
- Preserve the original extracted bytes and hash as the archival source. The
  desktop derivative is Matroska with lossless H.264 (`libx264 -qp 0`) plus
  FLAC, without scaling or pixel-format/range changes.
- A decoder-generated A/V file belongs under `out/analysis/media/`. Compare
  decoded timing with a bounded probe:

  ```sh
  ffprobe -v error -count_frames \
    -show_entries stream=index,codec_type,nb_read_frames,duration,r_frame_rate,sample_rate \
    -of json out/analysis/media/CAPCOM30.mkv
  ```

- The measured `CAPCOM30.STR` has exactly 1155 2336-byte sectors. Lossless
  wrapping yields 231 video frames and 143 main stereo XA packets at 37800 Hz.
  ffmpeg's default time base reproduces the naïve desktop half-speed symptom,
  but it is not a canonical duration for the asset.
- Missing end padding is not supported: the extracted size is exact and the
  wrapper preserves all sectors. As a worked example, 231 frames at 30 fps and
  1155 sectors at 2x CD rate each give 7.700 seconds. Applying the generic
  formula to the 7.626667-second main stereo decode derives 0.073333 seconds of
  padding, or 2772 samples per channel at 37800 Hz; both mux tracks then measure
  7.700 seconds within the stated tolerance.
- `INFERRED:` 30 fps/2x delivery is the strongly supported conversion setting,
  not yet a proven game-runtime contract. Verify scheduler behavior in the
  unresolved LOGO functions `0x801ce760` and `0x801cea98` before promoting it.
  `ffprobe` cannot read the extracted 2336-byte STR directly, so probing raw
  `CAPCOM30.STR` is not a valid timing test.

### Enemy-data boundary

Enemy facts in this page are storage algorithms: [monster extraction](#3-monster-id-extraction),
[formation decoding](#9-formation-record-decoding), and [BENEMY mapping](#19-benemy-file-mapping).
Area archives own monster and formation records. `BENEMY` archives are audio
banks, not executable enemy overlays. Runtime AI, spawning, and battle lifecycle
need separate reviewed call-chain evidence and are not inferred from these tables.

Testable invariants:

- Monster records are exactly `0x88` bytes and formation records are exactly
  9 bytes; every non-`0xff` formation slot is an area-local monster index, not
  a global monster ID.
- Repeated pointers may resolve to the same monster ID. Conflicting IDs must be
  reported as variants rather than silently deduplicated.
- The `monster ID N -> ENEMY{N-1}.EMI` relation remains bounded by the verified
  corpus; confirm a runtime caller and bounds before encoding it in game code.

## Data extraction algorithms

## 1. Name decoding (custom encoding)

The game uses a custom character encoding for item/weapon/armor/accessory/
ability names. The decoding algorithm:

```
function decode_name(raw_bytes):
    result = ""
    for each byte b in raw_bytes:
        if b == 0x00:           # end of string
            break
        if b == 0xFF:           # space
            result += " "
        else if b == 0x8E:      # apostrophe
            result += "'"
        else if b == 0x3D:      # hyphen
            result += "-"
        else if b == 0x3E:      # period
            result += "."
        else if b == 0x8B:      # plus
            result += "+"
        else if b == 0x05:      # color tag (2-byte sequence)
            skip next byte      # 0x02=RED, 0x03=BLUE
        else if b == 0x06:      # NOCOLOR (reset)
            pass
        else if b == 0x01:      # newline
            pass                # skip (display formatting)
        else if 0x20 <= b < 0x7F:  # printable ASCII
            result += chr(b)
        else:
            result += "[XX]"    # unknown byte
    return result.strip()
```

**How it was built**: Started with known item names from the randomizer
(e.g., "Green Apple" = `47 72 65 65 6E FF 41 70 70 6C 65`), compared
raw bytes against expected characters, and iteratively discovered the
swap table. The 0xFF=space mapping was obvious (appears between words).
Other mappings were found by testing names with known punctuation.

## 2. Pointer table parsing

Pointer files map offsets into archive files. The parsing algorithm:

```
function parse_pointer_table(filepath):
    entries = []
    for each line in file:
        line = line.strip()
        if line is empty: continue
        
        # Format: "HEXX@Path/to/ARCHIVE.EMI [# optional comment]"
        parts = line.split("@", 2)
        if len(parts) < 2: continue
        
        offset = int(parts[0], 16)
        archive = parts[1].split("#")[0].strip()  # strip comments
        
        if offset == 0x00000 or offset == 0xFFFFF:
            continue  # empty slot
        
        entries.append((offset, archive))
    
    return entries
```

**How it was built**: The vast-violence reference provided pointer files.
Verified by reading bytes at each offset and checking that the first
8 bytes decode to a valid monster/item name.

## 3. Monster ID extraction

Monsters are embedded in area archives, not in a central table. The
extraction algorithm:

```
function extract_monster_ids(pointer_table):
    monster_map = {}  # name -> (id, stats)
    
    for (offset, archive) in pointer_table:
        data = read_file(archive)
        record = data[offset : offset + 136]
        
        hp = read_uint16(record, 32)
        if hp == 0:
            continue  # empty slot
        
        name = decode_monster_name(record[0:8])
        monster_id = read_uint16(record, 8)
        
        if name not in monster_map:
            monster_map[name] = (monster_id, record)
        else:
            # Verify consistency across areas
            existing_id = monster_map[name][0]
            if existing_id != monster_id:
                log("VARIANT: " + name + " has IDs " + existing_id + " and " + monster_id)
    
    return monster_map
```

**How it was built**: Read the monster struct definition from
vast-violence, then verified by extracting all 1400 pointer entries
and checking that the same monster name always has the same ID
(163/168 consistent; 5 have legitimate variants like boss forms).

## 4. Stat decoding (signed bytes)

Master and base stats use signed bytes where 0x80-0xFF = negative:

```
function decode_stat(byte_value):
    if byte_value < 0x80:
        return byte_value        # positive or zero
    else:
        return byte_value - 256  # negative (0xFF = -1, 0x80 = -128)
```

**How it was built**: Cross-referenced master stats against known game
behavior (e.g., Ladon is "glass cannon" with -6 HP, +2 PWR). Verified
that the randomizer's `MasterStatsObject.cleanup()` uses the same
conversion.

## 5. Level growth decoding (nibble packing)

Level growth records pack two 4-bit values into one byte:

```
function decode_level_growth(record):
    exp = read_uint16(record, 0)        # 2 bytes: experience threshold
    hp = record[2]                       # 1 byte: HP growth
    ap = record[3]                       # 1 byte: AP growth
    pwr = (record[4] >> 4) & 0x0F       # high nibble: power growth
    dfn = record[4] & 0x0F              # low nibble: defense growth
    agi = (record[5] >> 4) & 0x0F       # high nibble: agility growth
    intel = record[5] & 0x0F            # low nibble: intellect growth
    ability = record[6]                  # ability learned at this level
    unknown = record[7]
    
    char_index = record_index // 99      # which character (0-7)
    level = (record_index % 99) + 1     # level (1-99)
    
    return { exp, hp, ap, pwr, dfn, agi, intel, ability }
```

**How it was built**: The randomizer's `Level_growth` table definition
showed the nibble splitting. Verified by checking that Ryu's level 1
stats match his base stats in START.EMI.

## 6. Master skill decoding

Master skills are packed as 2-byte values:

```
function decode_master_skills(record):
    skills = []
    for i in 0..5:
        value = read_uint16(record, i * 2)
        if value == 0x63FF:
            skills.append(null)  # empty slot
        else:
            level = (value >> 8) & 0xFF  # high byte = required level
            skill_id = value & 0xFF       # low byte = ability ID
            skills.append({ level, skill_id })
    return skills
```

**How it was built**: The randomizer's `struct_master_skills.txt` defined
the format. Verified by checking that Deis (index 9) has 5 skills
(unlike other masters with 3-4), matching her known skill set.

## 7. Shop item reference decoding

Shop items reference the item/weapon/armor/accessory tables:

```
function decode_shop_item(record):
    item_type = record[0]        # 0=Item, 1=Weapon, 2=Armor, 3=Accessory
    item_index = record[1]       # index into the respective table
    # Special cases:
    #   item_type=4, item_index=0  → Key Item
    #   item_type=0xFF             → empty slot
    
    return lookup_table(item_type, item_index)
```

**How it was built**: The randomizer's shop randomization logic showed
the type→table mapping. Verified by cross-referencing shop contents
against known game shops.

## 8. Chest record decoding

Chests contain 3-byte records:

```
function decode_chest(record):
    memory_byte = record[0]     # 0xFF = empty chest
    item_index = record[1]      # index into item table
    item_type = record[2]       # 0=Item, 1=Weapon, 2=Armor, 3=Accessory
    
    if memory_byte == 0xFF:
        return null  # empty
    
    zenny_value = item_index * 40  # zenny chests use this formula
    
    return { item_index, item_type, zenny_value }
```

**How it was built**: The randomizer's chest randomization logic showed
the 3-byte format. The `item_index * 40` zenny formula was derived
from the randomizer's `rewrite_chests()` function.

## 9. Formation record decoding

Formation records define enemy encounters:

```
function decode_formation(record):
    monster_indexes = record[0:8]  # up to 8 monster slots
    appearance_rate = record[8]    # 0 = boss/inactive
    
    return { monster_indexes, appearance_rate }
```

**How it was built**: The randomizer's formation randomization showed
the 9-byte format. Verified by checking that boss formations have
appearance_rate=0.

## 10. Monster resistance/condition decoding

Monster resistances are 9 bytes, one per element:

```
RESISTANCE_NAMES = [
    "Fire", "Frost", "Thunder", "Earth",
    "Wind", "Holy", "Psionic", "Status", "Death"
]

function decode_resistances(record):
    resistances = {}
    for i in 0..8:
        value = record[i]
        if value == 0x63:
            continue  # unused
        resistances[RESISTANCE_NAMES[i]] = value  # 0-7 scale
    return resistances
```

**How it was built**: The randomizer's monster randomization showed the
9-byte resistance format. The 0x63=unused sentinel was derived from
the struct definition.

## 11. Fairy record decoding

Fairy records are 9 bytes (5-byte name + 4-byte stats):

```
function decode_fairy(record):
    name = decode_name(record[0:5])
    stats = record[5:9]  # 4 stat bytes
    return { name, stats }
```

**How it was built**: The randomizer's fairy randomization showed the
format. Fairy names use the same custom encoding as items.

## 12. Manillo item decoding

Manillo stock items are 8 bytes:

```
function decode_manillo_item(record):
    item_index = record[0]
    item_type = record[1]
    fish_indexes = record[2:5]    # 3 fish types for trade
    fish_quantities = record[5:8] # quantities needed
    return { item_index, item_type, fish_indexes, fish_quantities }
```

## 13. Table discovery by string search

When the location of a data table is unknown, use string search:

```
function find_table_location(known_names, archive_file):
    data = read_file(archive_file)
    
    for name in known_names:
        # Try both encoded and plain ASCII
        encoded = encode_custom(name)
        plain = name.encode('ascii')
        
        for pattern in [encoded, plain]:
            offset = data.find(pattern)
            if offset != -1:
                log("Found '" + name + "' at 0x" + hex(offset))
                # Nearby tables are likely at offset +/- delta
                return offset
    
    return null
```

**How it was built**: Used `strings -t x` and Python's `bytes.find()`
to locate item names in GAME.EMI. Once one table was anchored, nearby
tables were found by computing offsets from the known layout.

## 14. Struct inference by comparison

When a struct definition is unknown, compare multiple records:

```
function infer_struct(records):
    # Find bytes that are constant across all records
    constants = []
    for byte_pos in 0..record_size:
        values = set(r[byte_pos] for r in records)
        if len(values) == 1:
            constants.append((byte_pos, values[0]))
    
    # Find bytes that vary systematically
    variables = []
    for byte_pos in 0..record_size:
        values = [r[byte_pos] for r in records]
        if len(set(values)) > 1:
            # Check if values correlate with known properties
            variables.append((byte_pos, values))
    
    return { constants, variables }
```

## 15. Verification pipeline

All extracted data goes through:

1. **Cross-reference**: Compare extracted values against randomizer code
2. **Consistency check**: Same monster should have same ID across areas
3. **Range validation**: Stats should be within plausible ranges
4. **Decode roundtrip**: Encode a decoded name and verify it matches raw
5. **Pointer validation**: Verify pointer offsets point to valid records

```
function verify_all():
    for each table in tables:
        records = extract_table(table)
        assert records.count == table.expected_count
        for record in records:
            assert decode_name(record.name) is not null
            assert record.stats within expected_ranges
    
    for each pointer_set in pointer_sets:
        entries = parse_pointer_table(pointer_set.file)
        for entry in entries:
            data = read_file(entry.archive)
            assert data[entry.offset] looks like valid record
```

## 16. EMI header parsing

All 880 EMI files share a common header format:

```
function parse_emi_header(data):
    entry_count = read_uint32(data, 0)
    version = read_uint32(data, 4)          # always 1
    signature = data[8:16]                   # "MATH_TBL"
    
    if signature != b'MATH_TBL':
        error("Not a valid EMI file")
    
    entries = []
    offset = 16
    for i in 0..entry_count:
        size = read_uint32(data, offset)
        load_addr = read_uint32(data, offset + 4)
        tag = read_uint32(data, offset + 8)
        unknown = read_uint16(data, offset + 12)
        padding = read_uint16(data, offset + 14)  # always 0x2E2E
        
        entries.append({ size, load_addr, tag, unknown })
        offset += 16
    
    return { version, signature, entries }
```

Load addresses are PSX RAM addresses (0x80000000–0x801FFFFF).

## 17. Finding unknown table locations

When you know what data should exist but not where:

```
function find_unknown_table(known_pattern, archive_file):
    data = read_file(archive_file)
    
    # Method 1: Search for known values
    # If you know the table has N records of size S,
    # search for N consecutive values of a known type
    
    # Method 2: Search for structural patterns
    # Tables often have repeating patterns (e.g., every 18 bytes)
    for record_size in [8, 12, 16, 18, 20, 22, 24]:
        for offset in range(0, len(data), record_size):
            if matches_pattern(data[offset:offset+record_size]):
                log("Potential table at 0x" + hex(offset) + 
                    " with record size " + str(record_size))
    
    # Method 3: Cross-reference with other files
    # If a pointer file references offset X in file A,
    # check if similar offsets exist in file B
```

## 18. Data cross-referencing

To verify data consistency across files:

```
function cross_reference_data():
    # Example: Verify master names match between files
    afldkwa_names = read_master_names('AFLDKWA.EMI')
    first_names = read_master_names('FIRST.EMI')
    
    for i in range(17):
        assert afldkwa_names[i] == first_names[i], \
            f"Master {i} name mismatch: {afldkwa_names[i]} vs {first_names[i]}"
    
    # Example: Verify base stats match
    start_stats = read_base_stats('START.EMI')
    status_stats = read_base_stats('STATUS.EMI')
    
    for i in range(8):
        assert start_stats[i] == status_stats[i], \
            f"Character {i} stats mismatch"
```

## 19. BENEMY file mapping

Enemy audio banks are mapped by monster ID:

```
function map_benemy_files():
    # Monster ID N maps to ENEMY{N-1}.EMI
    # 200 files, only 67 unique contents
    
    for file in BENEMY directory:
        monster_id = int(file.name.replace("ENEMY", "").replace(".EMI", "")) + 1
        hash = md5(file.contents)
        yield { monster_id, file.name, hash }

    # Group by hash to find duplicates
    # 45 files share content (ENEMY147-199)
    # 44 files share content (ENEMY036-133)
    # 9 files share content (ENEMY003-019)
```

**How it was built**: Hashed all 200 BENEMY files and grouped by MD5.
Verified that the monster_id-to-file mapping is consistent with the
monster ID extraction from area archives.

## 20. Data duplication detection

To find duplicate data across EMI files:

```
function find_duplicates():
    # Hash each EMI file
    hashes = {}
    for file in all_emi_files:
        h = md5(file.contents)
        if h not in hashes:
            hashes[h] = []
        hashes[h].append(file.name)
    
    # Report groups with more than one file
    for h, files in hashes.items():
        if len(files) > 1:
            log(f"Duplicate group: {files}")
    
    # Known duplications:
    # - START.EMI ↔ STATUS.EMI: base stats (byte-identical)
    # - AFLDKWA.EMI ↔ FIRST.EMI: master names
    # - BATTLE.EMI ↔ BATTLE2.EMI: battle system
    # - Every BOSS*.EMI contains a battle engine copy
    # - BENEMY: 155 of 200 files are duplicates
```

**Why duplication exists**: PSX CD-ROM seek time (200-800ms) meant
loading data from nearby disc locations was faster than seeking to
one distant location. Developers deliberately duplicated data to
minimize seek times during gameplay.

## 21. Master data extraction

Master data spans multiple EMI files:

```
function extract_masters():
    # Names from AFLDKWA.EMI @ 0x1BE0
    names = []
    data = read_file('AFLDKWA.EMI')
    offset = 0x1BE0
    for i in 0..16:
        name = read_null_terminated_ascii(data, offset)
        names.append(name)
        offset += len(name) + 1
    
    # Skills from SISYOU.EMI @ 0x3C88
    skills = []
    data = read_file('SISYOU.EMI')
    for i in 0..16:
        record = data[0x3C88 + i*12 : 0x3C88 + (i+1)*12]
        master_skills = decode_master_skills(record)
        skills.append(master_skills)
    
    # Stats from SISYOU.EMI @ 0x3D54
    stats = []
    for i in 0..16:
        record = data[0x3D54 + i*6 : 0x3D54 + (i+1)*6]
        master_stats = [decode_stat(b) for b in record]
        stats.append(master_stats)
    
    return { names, skills, stats }
```

## 22. Formation appearance rate analysis

Boss detection uses formation appearance rates:

```
function find_bosses(pointer_table):
    # A monster is a "boss" if it never appears in any formation
    # with a nonzero appearance rate
    
    monster_formation_counts = {}
    
    for (offset, archive) in pointer_table:
        data = read_file(archive)
        formation = data[offset : offset + 9]
        
        monster_indexes = formation[0:8]
        appearance_rate = formation[8]
        
        if appearance_rate == 0:
            continue  # boss/inactive formation
        
        for idx in monster_indexes:
            if idx != 0xFF:
                monster_formation_counts[idx] = \
                    monster_formation_counts.get(idx, 0) + 1
    
    # Monsters with zero active formations are bosses
    bosses = []
    for monster_id in all_monster_ids:
        if monster_formation_counts.get(monster_id, 0) == 0:
            bosses.append(monster_id)
    
    return bosses
```

## Traps to avoid

- **Two encoding systems**: Don't assume plain ASCII for game-data names.
  The custom encoding uses special bytes for punctuation and colors.
- **Name field padding**: Names shorter than 12 bytes are padded with
  `0x00`. Trailing non-zero bytes after visible text are struct fields,
  not name characters.
- **Signed vs unsigned**: Stat values (master stats, base stats) use
  signed bytes. Raw hex `0xFE` = -2, not 254.
- **Pointer validity**: Not all pointer slots are filled. `0x00000` and
  `0xFFFFF` indicate empty/unused entries.
- **Cross-version differences**: v1.0 and v1.1 have different offsets
  for some tables (e.g., Manillo stock at 0x3E53A vs 0x3E53E).
- **Monster ID ≠ pointer index**: The pointer list index is not the
  monster ID. Multiple pointers can reference the same monster.
- **Master assignment**: Characters have `master=0xFF` in base stats.
  Master assignment is determined at runtime, not stored in the record.
