---
type: Reference
title: Reverse engineering algorithms
description: Algorithms and processes used to extract and verify game data from BOF3 PSX binary archives.
tags: [algorithms, extraction, verification]
---

# Pseudocode: reverse engineering algorithms

Algorithms and processes used to extract and verify game data from BOF3
PSX binary archives. Each section documents the technique, not the code.

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
