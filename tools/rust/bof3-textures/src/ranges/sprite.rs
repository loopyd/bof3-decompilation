//! Source-bound graphics byte roles; opaque bytes remain unclassified.

use super::{Entry, NontextRange, sha256_hex, uncovered_nontext_ranges};

pub(super) const BPLU_SELECTED_SPRITE_EVIDENCE: &str = "Original US BPLU selector-2 program and sprite-cell numeric fields; exact archive and entry geometry, mode, load, checkword and payload identities; the preserved family-2 request, hint/triplet, slot/LBA supplier, successful unskipped load and GAME publisher/reselection bind the same three roots with count 1, work index 0 and clear record masks; original D910/DB98/D978 readers qualify finite fetched pairs, jump operands and selected cell-relative halfwords; original GAME 0x801BDAB8 and SLUS 0x8014D290/0x8014BA54/0x8014BC70 carry the same work into 0x8014C4D4, whose counted part loop and 0x8014CF9C consume control, signed X/Y and unsigned U/V; stable work, work[0]=1, work+0x24=1, work+0x28=0, retained work+0x2A in 0..1, work+0x48=0, valid visible geometry, queue capacity, readable prefetch slots, ordinary helper returns and nonaliased frames/packets remain conditions; compressed literals require every decoded alias in the reviewed image/program/palette/selected-cell union, with previous evidence preserved; program header byte 0, unfetched high-bit predicates, other programs/cells and allocation tails excluded; part control bits 0x70 remain opaque; no natural state production, actual invocation, loading, residency, transfer, display, rasterized-text absence or safe-edit claim";

pub(super) const BPLD_SELECTED_SPRITE_EVIDENCE: &str = "Original US BPLD selector-0 program and sprite-cell numeric fields; exact archive and entry geometry, mode, load, checkword and payload identities; the family-1 request with flag byte zero, hint/triplet, slot/LBA supplier, successful unskipped load and GAME publisher/reselection bind the same three roots with count 1, work index 0 and clear record masks; original D910/DB98/D978 readers qualify finite fetched pairs, jump operands and selected cell-relative halfwords; original GAME 0x801BDAB8 and SLUS 0x8014D290/0x8014BA54/0x8014BC70 carry the same work into 0x8014C4D4, whose counted part loop and 0x8014CF9C consume control, signed X/Y and unsigned U/V; stable work, work[0]=1, initialized work+0x24=1 and work+0x28=2, retained work+0x2A in 0..1, work+0x48=0, valid visible geometry, queue capacity, readable prefetch slots, ordinary helper returns and nonaliased frames/packets remain conditions; palette mode 2 selects original 0x8014CF10 and the renderer page-adjustment paths; compressed literals require every decoded alias in the reviewed image/program/palette/selected-cell union, with previous evidence preserved; program header byte 0, unfetched high-bit predicates, selection mode bytes, other programs/cells and allocation tails excluded; part control bits 0x70 remain opaque; no natural state production, actual invocation, loading, residency, transfer, display, rasterized-text absence or safe-edit claim";

pub(super) const BPL_FLAG_SPRITE_EVIDENCE: &str = "Original US BPL family-flag program and sprite-cell numeric fields; exact archive, entry geometry, mode, load, checkword and payload identities; GAME 0x80197B98 tests only flag bit 1 for family 1/2 and 0x801BF568 preserves the full flag as a below-128 selector when record mask 0x4000 is clear; byte 0x80146325=1 skips the unrelated coordinate lookup while retaining the request; original successful unskipped load, hint/triplet, slot/LBA supplier and phase-2/3 publisher bind each root separately with count 1 and work index 0; original D6B8/D6DC/DC08 and D910/DB98/D978 consume bounded selection triples, selected program fields, finite fetched pairs, redirect operands and cell-relative halfwords; the reviewed same-actor GAME and SLUS draw route to C4D4/CF9C consumes counted control, signed X/Y and unsigned U/V fields with initialized work+0x24=1, work+0x28=2, work+0x48=0 and retained mirror byte 0 or 1; stable flags, roots, work and code, clear record mask 0x0080, valid visible geometry, queue capacity, readable prefetch slots, ordinary noninterfering helper returns and bounded nonaliased frames/packets remain conditions; compressed literals require every decoded alias in the reviewed image/program/palette/selected-cell/family-flag union, with every earlier classification and evidence preserved; header byte zero, unfetched high-bit predicates, invalid triples/walks, other programs/cells, allocation tails and opaque part bits 0x70 remain unqualified; no natural flag or state production, actual request, invocation, loading, cadence, residency, transfer, display, rasterized-text absence or safe-edit claim";

pub(super) const BRT_SPRITE_EVIDENCE: &str = "Original US BRT finite program and sprite-cell numeric fields; exact archive, entry geometry, mode, load, checkword and payload identities under the reviewed BRT slot/LBA and saved-root bindings; original post-load 0x801E7BA0 restores the actor, preserves request byte 8 and selects (byte8+4)&255 through 0x8014D6B8; only bounded below-128 selections, family-D request bytes below 2 or family-U bytes at least 2, and mirror bytes 0/1 qualify; original queued completion 0x801E8558 raises actor+0x40 from zero to 0x10000, then clears actor+0x48 and record+0x124 bit 0x2000; original actor dispatcher 0x801DECE0 and finite table cells carry state 6/4/4/1 through 0x801E13B8 to states 2 then 3; with flags+0x80=0, record+0x119=0, record+0x124 bits 2/0x1000 clear, globals 0x801462E0/0x801462EF/0x801463CE zero and 0x801462E8 bit 0x10 clear, 0x801DEFE4 requests the unchanged selector and the stable state-3 loop reaches 0x801DEDE4 and D978 on the same actor; original D910/DB98/D978 qualify finite fetched pairs, redirect operands and cell-relative halfwords; the reviewed same-actor GAME and SLUS draw route to C4D4/CF9C consumes counted control, signed X/Y and unsigned U/V with active work, work+0x24=1, work+0x28=2, work+0x48=0 and retained mirror 0/1; stable code, bank, roots and work, other actors inactive, ordinary noninterfering helpers, valid visible geometry, readable prefetch slots and bounded nonaliased frames, queues and packets remain conditions; compressed literals require every decoded alias in the previous reviewed image/palette or finite selected-field union and preserve previous evidence; unread BRT root-array words, program header byte zero, unfetched predicates, invalid triples/walks, other programs/cells, tails and opaque control bits 0x70 remain unqualified; no natural state or selector production, actual invocation, loading, cadence, residency, transfer, display, rasterized-text absence or safe-edit claim";

pub(super) const BRT_RESELECTED_SPRITE_EVIDENCE: &str = "Original US BRTD post-completion reselected image, program and sprite-cell fields; exact archive and entry identities, original slot/LBA supplier and retained saved-root bindings; original 0x801DEFE4 requests byte8 plus 8 with flags80=0 and global62E8 bit10 set, plus 0x14 with flags80=0/global bit10 clear/record124 bit2 set, plus 0x18 with flags80=0x40/global bit10 set, or plus 0x30 with flags80=0x800; D request byte8 is 0/1, record119=0, globals62E0/62EF/63CE zero and record124 bit1000 clear; original 0x801E7BA0 saves and restores the current flags for kind0/7, preserves those flag80 bits and record124 bit2, and clears only global62E8 bit8000; original 0x801E8558 and 0x801E13B8 preserve these selector predicates while reaching actor states2/3; 0x8014D8D4 reselects once through D6B8/D6DC/DC08, then the stable state3 loop calls D978 on the same actor; complete original image readers E6D0/E7F8/E8FC, checked image allocations and independent native record reconciliation qualify selected directory words, image headers and consumed encodings; D910/DB98/D978 qualify finite program walks and the reviewed same-actor GAME/SLUS draw route consumes counted control, signed X/Y and unsigned U/V fields with active work, work24=1/work28=2/work48=0 and mirror0/1; stable code, roots, bank, flags and work, other actors inactive, noninterfering helpers, bounded nonaliased memory and queues, valid visible geometry and readable prefetch remain conditions; every compressed literal alias must belong to the prior reviewed fields or this image/program/cell union, preserving all prior evidence; root-array words, opaque mode2 prefixes and part bits70, program header byte0, unfetched predicates, other selections and tails remain unqualified; no natural predicate production, invocation, loading, cadence, residency, transfer, display, rasterized-text absence or safe-edit claim";

pub(super) fn bpl_selected_sprite_profile(entry: Entry, data: &[u8]) -> Option<(u8, &'static str)> {
    if entry.transfer_mode != 1 || entry.ram_ptr != 0x8003_b800 || entry.size as usize != data.len()
    {
        return None;
    }
    let digest = sha256_hex(data);
    let identity = (
        entry.index,
        entry.offset,
        entry.check_word,
        data.len(),
        digest.as_str(),
    );
    if matches!(
        identity,
        (
            7,
            98_304,
            368_652,
            164_994,
            "e2a34bb38b0a2f4640b12edcf99abfbdfbfe94899e16dbedcf4f19654bb46dc2"
        ) | (
            7,
            98_304,
            331_788,
            171_761,
            "6b07b7680c06ab63614b5d824df4482b293d7f19ed7af15dfa9506c25c151202"
        ) | (
            7,
            98_304,
            331_788,
            124_778,
            "09a3ac9e9463bbd28d007c0fa1e12d6349b3ad21f81c40726887b1e1df6a918f"
        ) | (
            7,
            98_304,
            368_652,
            192_570,
            "8a3d4406db576778f6c28da82cc74da24d043a0a9814eba7573f09bb3572b716"
        ) | (
            7,
            98_304,
            368_652,
            145_584,
            "3a3756665db495864c14e4d3ed169a56e599f0da6562da57a439ad28a5ea30ec"
        ) | (
            7,
            98_304,
            331_788,
            134_569,
            "2b9702dd8b5a617ac209ac4463a7c2f352fc190487ce826f970a36f0012d3f66"
        ) | (
            7,
            96_256,
            331_788,
            152_690,
            "dca408e86ea40dbb6cb9b411fd82d4ebf59ae7dff4438e69f111d6de17145d4c"
        ) | (
            7,
            96_256,
            368_652,
            196_697,
            "e346011032cbdeb18efbe5519dfed41ee48379d9f122aa021b757bdefd45edd6"
        ) | (
            7,
            98_304,
            368_652,
            208_309,
            "04c55b4f45a62a7cbb50e400e643553f8d3e76cef8929bec6853877ca1758d65"
        ) | (
            7,
            100_352,
            368_652,
            161_332,
            "49b75da89fb00ed33e435221f0a44ed40a065e405d3b267f1c5068432bc97af2"
        ) | (
            7,
            98_304,
            368_652,
            203_533,
            "e89f0b3b8ed58b12352cba5516071aaa14dcd825a604a234b7adf2ed8b824fd6"
        ) | (
            7,
            98_304,
            275_460,
            133_381,
            "d7dfb508ba4366c13a87fba1d91af0166170839f651f4952dd6d54fa44c815d7"
        ) | (
            7,
            94_208,
            331_788,
            203_819,
            "53db0e87ff007d5f285f369e8fe831c69ade60fb52fae4f324ac0bf30256ad3c"
        ) | (
            7,
            96_256,
            331_788,
            156_785,
            "3dc9d693b853e79393b206dddd5215c8bb94ff88cfe9805d1afa0543cfdf6ddd"
        ) | (
            7,
            94_208,
            331_788,
            198_974,
            "dd9c4770ba3eab3f49cabedeeb8b8939876b9e0cdf403edf0ccd59ca0d3d3428"
        ) | (
            7,
            98_304,
            331_788,
            168_385,
            "6b299fd8cea814cf90575f7b0f803a9b300f8cc6cf0b1d2c10b10555cb20f063"
        ) | (
            7,
            96_256,
            331_788,
            210_574,
            "0eaa04778217afe8fe7793e4666a48bbd8ecf5f92cf9324d25c6aca5c3edcbfc"
        ) | (
            7,
            98_304,
            331_788,
            163_590,
            "5d47579d411fd999b25bf7627e0a246e84653f8450e955b57ab75c7bee013a75"
        )
    ) {
        Some((2, BPLU_SELECTED_SPRITE_EVIDENCE))
    } else if matches!(
        identity,
        (
            7,
            98_304,
            368_652,
            166_846,
            "053c5e252a85d53b7ddedd9043fe09427c21a6869d7df79e6e27f049a318058b"
        ) | (
            7,
            98_304,
            331_788,
            168_143,
            "58818212b2924ea74f946de57fcb2ca5217df55df59c13c7efc64b712c1e8441"
        ) | (
            7,
            98_304,
            331_788,
            126_764,
            "5788a3234f6ce4d8ee0075d9345b4b4a9958f2adf7d319b95dacefc2b6385268"
        ) | (
            7,
            98_304,
            368_652,
            197_048,
            "b266a31ac21b11a086c4fe548fb5bdc170b15fd5f9e2d4601dc90930bc9b0e7b"
        ) | (
            7,
            98_304,
            368_652,
            155_687,
            "3ae2a261a6fac405171fc8051dc88247c5290806c9d06c247099cdd3631ad7b7"
        ) | (
            7,
            98_304,
            331_788,
            129_525,
            "e4dcdf27c2d790c1731de7c1d3a4186a2427ba64243bcc2e40d9270a3bbf65bf"
        ) | (
            7,
            96_256,
            331_788,
            157_020,
            "a08ce37780b173281c607a3cdff5c4291458fbce794d9a571198aae46092515a"
        ) | (
            7,
            96_256,
            368_652,
            199_085,
            "21e3a59a7b0771d43f42babd465142899935f87e50188b7c6829fba8959c4ee1"
        ) | (
            7,
            98_304,
            368_652,
            213_807,
            "b627b7492e8462589d8ed191e365b6c53c3595c86d8efa4209cf59128b963a45"
        ) | (
            7,
            100_352,
            368_652,
            172_578,
            "d96a0b1fc1f3e9cb0d19c349459d60e336a114a9d402261d6d6b83cd52c5b018"
        ) | (
            7,
            98_304,
            368_652,
            210_377,
            "145a87d311cce9f157630b5c77b4bd97f54afc5d10fcf5f5ed3ce5d3e53afa8a"
        ) | (
            7,
            98_304,
            269_856,
            138_325,
            "b9dd8cb2698064159c49a3bb1abb550c7163ac7622d23a3fe07d3033f73d5dd0"
        ) | (
            0,
            2_048,
            331_788,
            129_413,
            "b9229f2ef691a502bf1c9d5a94626f2ed97b9922d92ff1e6db96e6a03662f8e4"
        ) | (
            7,
            94_208,
            331_788,
            200_599,
            "b682d37a2ec37adf08dde1a514f98b85ae4aa0ac68e616cdbb697e140ba5b858"
        ) | (
            7,
            96_256,
            331_788,
            159_254,
            "0a2c91b60f2eaf4ec493d6014c0eabacc06979c481d07fab1367710eaeace1f8"
        ) | (
            7,
            94_208,
            331_788,
            196_922,
            "a48d4b434e176f043f2482050906d13788c979bdebe252807952b1e148922802"
        ) | (
            7,
            98_304,
            331_788,
            173_974,
            "03cf547e8cfedee08e9581d4202e1f41425d83e6daebb6c4495098f464e7d3af"
        ) | (
            7,
            96_256,
            331_788,
            211_642,
            "4b321cc1ff09b75bad5cd7a34cc1ef806fffcc7c26adb618279d04c07e0857cc"
        ) | (
            7,
            98_304,
            331_788,
            170_285,
            "69136a4e291a626a5285c1168642a726bedfc670a593589b3f5fec9c3998cb02"
        )
    ) {
        Some((0, BPLD_SELECTED_SPRITE_EVIDENCE))
    } else {
        None
    }
}

pub(super) fn bpl_sprite_roots(data: &[u8]) -> Option<[usize; 3]> {
    if data.len() > 1024 * 1024 {
        return None;
    }
    let word = |at: usize| {
        Some(u32::from_le_bytes(data.get(at..at.checked_add(4)?)?.try_into().ok()?) as usize)
    };
    let roots = [word(0)?, word(4)?, word(8)?];
    (roots[0] == 12 && roots[0] < roots[1] && roots[1] < roots[2] && roots[2] < data.len())
        .then_some(roots)
}

pub(super) fn bpl_root_sprite_fields(
    data: &[u8],
    root: usize,
    root_end: usize,
    selector: u8,
) -> Option<Vec<(usize, usize)>> {
    let word = |at: usize| {
        Some(u32::from_le_bytes(data.get(at..at.checked_add(4)?)?.try_into().ok()?) as usize)
    };
    let half = |at: usize| {
        Some(usize::from(u16::from_le_bytes(
            data.get(at..at.checked_add(2)?)?.try_into().ok()?,
        )))
    };
    let mut fields = Vec::new();
    let image = root.checked_add(word(root)?)?;
    let palette = root.checked_add(word(root + 4)?)?;
    let programs = root.checked_add(word(root + 8)?)?;
    let selections = root.checked_add(word(root + 12)?)?;
    if !(root + 16 <= image
        && image < palette
        && palette < programs
        && programs < selections
        && selections < root_end)
    {
        return None;
    }
    let selection = selections.checked_add(3 * usize::from(selector))?;
    let program_index = usize::from(*data.get(selection + 1)?);
    let directory_size = half(programs)?;
    let directory_end = programs.checked_add(directory_size)?;
    if directory_size == 0
        || directory_size % 2 != 0
        || directory_end > selections
        || program_index >= directory_size / 2
        || selection + 3 > root_end
    {
        return None;
    }
    let starts = (0..directory_size / 2)
        .map(|index| programs.checked_add(half(programs + 2 * index)?))
        .collect::<Option<Vec<_>>>()?;
    if starts
        .iter()
        .any(|&start| !(directory_end..selections).contains(&start))
    {
        return None;
    }
    let program = starts[program_index];
    let end = starts
        .iter()
        .copied()
        .filter(|&start| start > program)
        .min()
        .unwrap_or(selections);
    let cells = usize::from(*data.get(program)?);
    let steps = usize::from(*data.get(program + 1)?);
    let pairs = program.checked_add(2)?;
    let table = pairs.checked_add(steps.checked_mul(2)?)?;
    let table_end = table.checked_add(cells.checked_mul(2)?)?;
    if cells == 0 || steps == 0 || table_end > end {
        return None;
    }
    let offsets = (0..cells)
        .map(|index| table.checked_add(half(table + 2 * index)?))
        .collect::<Option<Vec<_>>>()?;
    if offsets[0] != table_end
        || offsets
            .iter()
            .any(|&cell| !(table_end..end).contains(&cell))
    {
        return None;
    }
    let mut distinct = offsets.clone();
    distinct.sort_unstable();
    distinct.dedup();
    for (index, &cell) in distinct.iter().enumerate() {
        let allocation_end = distinct.get(index + 1).copied().unwrap_or(end);
        if cell.checked_add(1 + 5 * usize::from(*data.get(cell)?))? > allocation_end {
            return None;
        }
    }
    fields.extend([
        (root + 8, root + 16),
        (selection, selection + 2),
        (
            programs + 2 * program_index,
            programs + 2 * program_index + 2,
        ),
        (program + 1, program + 2),
    ]);
    let mut visited = vec![false; steps];
    let mut index = 0;
    while !*visited.get(index)? {
        visited[index] = true;
        let pair = pairs + 2 * index;
        let operand = usize::from(*data.get(pair + 1)?);
        let cell = *offsets.get(operand)?;
        fields.extend([
            (pair, pair + 2),
            (table + 2 * operand, table + 2 * operand + 2),
            (cell, cell + 1 + 5 * usize::from(*data.get(cell)?)),
        ]);
        index += 1;
        if index == steps {
            index = 0;
        } else if *data.get(pairs + 2 * index)? >= 0x80 {
            let jump = pairs + 2 * index + 1;
            fields.push((jump, jump + 1));
            index = usize::from(*data.get(jump)?);
        }
    }
    Some(fields)
}

/// Only finite walks selected by pinned original BPL profiles qualify.
pub(super) fn bpl_selected_sprite_fields(data: &[u8], selector: u8) -> Option<Vec<(usize, usize)>> {
    let roots = bpl_sprite_roots(data)?;
    let mut fields = vec![(0, 12)];
    for (ordinal, &root) in roots.iter().enumerate() {
        let root_end = roots.get(ordinal + 1).copied().unwrap_or(data.len());
        fields.extend(bpl_root_sprite_fields(data, root, root_end, selector)?);
    }
    Some(merge_bpl_sprite_fields(fields))
}

/// The loader consumes flag bit 1; below-128 reselection preserves the other bits.
pub(super) fn bpl_flag_sprite_fields(data: &[u8], family_flag: u8) -> Option<Vec<(usize, usize)>> {
    let mut fields = sprite_selector_fields(data, |selector| selector & 2 == family_flag)?;
    fields.push((0, 12));
    Some(merge_bpl_sprite_fields(fields))
}

pub(super) fn sprite_selector_fields(
    data: &[u8],
    accepts_selector: impl Fn(u8) -> bool,
) -> Option<Vec<(usize, usize)>> {
    let roots = bpl_sprite_roots(data)?;
    let word = |at: usize| {
        Some(u32::from_le_bytes(data.get(at..at.checked_add(4)?)?.try_into().ok()?) as usize)
    };
    let mut fields = Vec::new();
    for (ordinal, &root) in roots.iter().enumerate() {
        let root_end = roots.get(ordinal + 1).copied().unwrap_or(data.len());
        let image = root.checked_add(word(root)?)?;
        let palette = root.checked_add(word(root + 4)?)?;
        let selections = root.checked_add(word(root + 12)?)?;
        let image_directory_size = word(image)?;
        if image_directory_size == 0
            || image_directory_size % 4 != 0
            || image.checked_add(image_directory_size)? > palette
        {
            return None;
        }
        let before = fields.len();
        for selector in 0_u8..128 {
            if !accepts_selector(selector) {
                continue;
            }
            let selection = selections.checked_add(3 * usize::from(selector))?;
            let end = selection.checked_add(3)?;
            if end > root_end {
                continue;
            }
            let triple = data.get(selection..end)?;
            if usize::from(triple[0]) >= image_directory_size / 4 || triple[2] > 1 {
                continue;
            }
            if let Some(selected) = bpl_root_sprite_fields(data, root, root_end, selector) {
                fields.extend(selected);
                fields.push((selection + 2, end));
            }
        }
        if fields.len() == before {
            return None;
        }
    }
    Some(merge_bpl_sprite_fields(fields))
}

pub(super) fn merge_bpl_sprite_fields(mut fields: Vec<(usize, usize)>) -> Vec<(usize, usize)> {
    fields.sort_unstable();
    let mut merged: Vec<(usize, usize)> = Vec::new();
    for (start, end) in fields {
        if let Some(previous) = merged.last_mut()
            && start <= previous.1
        {
            previous.1 = previous.1.max(end);
            continue;
        }
        merged.push((start, end));
    }
    merged
}

pub(super) const US_DRG_SPRITE_FIELD_EVIDENCE: &str = "Original US DRG selected program and cell fields. Exact original DRG01..08 raw entry-3 profiles retain their documented source/root bindings and finite selectors. Original selection, initialization, forward and draw readers preserve the same actor, program and relative cell table. Complete native directory, allocation and finite forward/direct-fetch checks qualify only consumed numeric fields and counted five-byte cell records. Retained mode 2 and mirror 0/1 use the reviewed renderer paths. Successful unskipped loading, stable original bytes and actor/root/program, no overriding request, repeated same-actor calls, bounded geometry/lists/packets and ordinary noninterfering helper returns remain conditions. Header byte zero, unfetched predicates, other programs/cells and tails retain prior status. No natural producer or schedule, observed invocation, cadence, live CLUT, transfer, display, rasterized-text absence or safe-edit claim.";
pub(super) const US_DRG_MIRROR_EVIDENCE: &str = "Original US DRG selected mirror bytes. Exact original DRG00..08 raw entry-3 source/root profiles and finite selectors bind selection byte 2 to the load at 0x8014DD00, same-actor store at 0x8014DD08 and renderer read/dispatch at 0x8014C720..0x8014C730. Native selected values 0/1 select the reviewed counted-cell paths with retained mode 2. Successful loading, unchanged source, retained same-actor root/program/control, bounded draw state/storage and ordinary noninterfering helpers remain conditions. Other selectors and control values retain prior status. No natural selection, scheduling, invocation, live CLUT, display, rasterized-text absence or safe-edit claim.";

pub(super) const US_RAW_SPRITE_FIELD_EVIDENCE: &str = "Original US CRYU, PAPY, RYU and RT selected program and cell fields. Exact original raw entry-3 profiles retain documented source/root bindings and finite selectors. Original initialization, forward/direct-fetch and same-actor draw readers bind consumed numeric fields and counted five-byte cell records. Complete native directory, allocation and finite-walk checks preserve opaque headers, unfetched predicates, other records and tails. Retained palette mode 2 and mirror 0/1 use reviewed renderer paths. RT requires an active actor and changed selector, no matching effect pointer, actor halfword +0x80 bit 0x80 clear and bounded nonaliased palette staging. Successful unskipped loading, stable source/code/root/program/control, no overriding request, repeated same-actor calls, visible bounded geometry, lists/packets and ordinary noninterfering helpers remain conditions. No natural producer, schedule, observed invocation, cadence, live CLUT, transfer, display, rasterized-text absence or safe-edit claim.";
pub(super) const US_RAW_SPRITE_MIRROR_EVIDENCE: &str = "Original US CRYU, PAPY, RYU and RT selected mirror bytes. Exact original raw entry-3 source/root profiles and finite selectors bind selection byte 2 to load 0x8014DD00, same-actor store 0x8014DD08 and renderer read/dispatch 0x8014C720..0x8014C730. Native values 0/1 select reviewed counted-cell paths with retained palette mode 2. The documented source, initialization, post-reader helper and bounded same-actor draw conditions remain required. Other selectors/control values retain prior status. No natural selection, scheduling, invocation, live CLUT, display, rasterized-text absence or safe-edit claim.";

pub(super) const US_REI_SPRITE_FIELD_EVIDENCE: &str = "Original US REI selected program and cell fields. Four exact original raw entry-3 profiles retain their documented source/root bindings and finite selectors. Original initialization, forward/direct-fetch and same-actor draw readers bind consumed numeric fields and counted five-byte cells. Complete original post-reader and stat-rebuild bodies preserve actor/root/program/control with character index below eight, battle actor count at most three, bounded terminated stat lists or the explicit skip case, valid nonaliased character/enemy records and intact frames. Require an active actor, no matching effect pointer, actor halfword +0x80 bit 0x80 clear and bounded palette staging. Native directory, allocation and finite-fetch checks exclude opaque headers, unfetched predicates, other records and tails. Successful unskipped loads, stable original source/code/tables, retained palette mode 2 and mirror 0/1, no overriding request, repeated same-actor reads, bounded geometry/lists/packets and ordinary noninterfering palette helpers remain conditions. No natural producer, schedule, invocation, cadence, live CLUT, transfer, display, rasterized-text absence or safe-edit claim.";
pub(super) const US_REI_MIRROR_EVIDENCE: &str = "Original US REI selected mirror bytes. Exact original raw entry-3 source/root profiles and finite selectors bind selection byte 2 to load 0x8014DD00, same-actor store 0x8014DD08 and renderer read/dispatch 0x8014C720..0x8014C730. Native selected values 0/1 use reviewed counted-cell paths with retained palette mode 2. The documented bounded post-reader stat rebuild, source, initialization, same-actor retention and draw conditions remain required. Other selectors/control values retain prior status. No natural selection, scheduling, invocation, live CLUT, display, rasterized-text absence or safe-edit claim.";

pub(super) fn raw_sprite_program_ranges(
    entry: Entry,
    data: &[u8],
    previous: &[NontextRange],
) -> Option<Vec<NontextRange>> {
    if entry.index != 3
        || entry.transfer_mode != 0
        || entry.check_word != 0x10
        || usize::try_from(entry.size).ok() != Some(data.len())
    {
        return None;
    }
    let (selectors, evidence, mirror_evidence): (&[u8], &str, &str) = match (
        entry.offset,
        entry.ram_ptr,
        data.len(),
        sha256_hex(data).as_str(),
    ) {
        (
            40_960,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            44_404,
            "1effd0ac6e77259713f70e51130634e848ed75bc66ddb3a5961d9476697d92d1",
        ) => (
            &[6, 4],
            "Original US DRG00 selected program and cell fields. Five exact raw entry-3 originals bind selectors 6 and 4 to programs at payload offsets 31254 and 21080. The original retained-root selector, initialization and forward readers preserve the same actor and cell-relative table; each finite eight-pair walk selects eight bounded cell records. The original GAME actor-array and draw route reaches the counted five-byte reader with publisher-retained mode 2. Stable original bytes, successful unskipped loading, retained actor/root/program, no overriding request, repeated same-actor forward calls, bounded visible geometry, lists/packets and ordinary noninterfering helper returns remain conditions. Header byte zero, unselected programs/cells, mirror bytes and tails retain prior status. No natural producer or schedule, observed invocation, cadence, live CLUT, transfer, display, rasterized-text absence or safe-edit claim.",
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            38_844,
            "2d8dada5f5112fbc1363e54a2b12ed8d31cd12df53f7456d42015d65e1c47c33",
        ) => (
            &[4, 6, 12, 14, 44, 46, 16, 18],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            20_480,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            72_420,
            "46af6dbcedb4eab31ae5d2db9195c3dbc968840c316d08d04b1ca95a1762efb3",
        ) => (
            &[4, 6, 44, 12, 46, 14, 16, 18],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            76_072,
            "0d918e6fdd6852c3d184213423c02b2aa46ea0cee36d156153e22b55a546d574",
        ) => (
            &[4, 6, 12, 14, 44, 46, 16, 18],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            26_624,
            0x8003_b80c,
            313_320,
            "6669621720080f29b7a431d150ade295210ecef49d1dec3085b4e828c962f538",
        ) => (
            &[4, 6, 12, 14, 44, 46, 16, 18],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            38_912,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            70_700,
            "4297311b37aa27df957dbacaca840945875fafab5c4fab1238a34252938e6ac7",
        ) => (
            &[4, 6, 12, 14, 44, 46, 16, 18],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            34_816,
            0x8003_b80c,
            233_124,
            "ccc2b49c546625cb6fbdf3d51996d3eba0ec25bb2b3c71fdf8e5b753a9d872a6",
        ) => (
            &[4, 16, 6, 44, 46, 12, 14],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            38_912,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            42_264,
            "c8743de97beea25057b3db3bdf89819fc7e43ce89483fe30d8ea54c297a2338f",
        ) => (
            &[4, 6, 12, 14, 44, 46, 16, 18],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            20_480,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            84_732,
            "ecb3e431738d1e065eb0e90a47c392368be4fa84a08fb90ec928659c40d19b36",
        ) => (
            &[4, 12, 44, 16, 6, 14, 46, 18],
            US_DRG_SPRITE_FIELD_EVIDENCE,
            US_DRG_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8003_b80c,
            110_592,
            "0cb3bd788cbdf439572efa1e6f33c544d0ce1ee9cef6c0aa3ca55d605d9c7db9",
        ) => (
            &[16, 40, 0, 4, 8, 36, 20, 12, 32, 24, 56, 6, 44, 60, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            43_008,
            0x8003_b80c,
            110_592,
            "8e109d9442d43c46aab8f73e6dbaddbe3e9dc4daddee891a960ea9dd94bbdbcd",
        ) => (
            &[16, 40, 0, 4, 8, 36, 20, 12, 32, 24, 56, 6, 44, 60, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8003_b80c,
            110_592,
            "440f35b120293916a9d19a7c60a6589dfe47800b9e8dbab95de24d25a4d4f58e",
        ) => (
            &[6, 8, 12, 20, 16, 24, 32, 36, 4, 40, 44, 56, 60, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            43_008,
            0x8003_b80c,
            110_592,
            "bcf1849fd598d529fd441f4d09561ccc7c143c03babbf3142d24d051ce8b81fe",
        ) => (
            &[6, 8, 12, 20, 16, 24, 32, 36, 4, 40, 44, 56, 60, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            32_768,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            11_796,
            "feea0916b287895af705b3598c55581a2fdaa3657ce7e0b3efff68a47f1c0338",
        ) => (
            &[4, 16, 28, 6],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            32_768,
            0x8003_b80c | 0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            17_400,
            "f9d984db05b512d943860a31776800a6678c772bf322f265f4339f0f3704b864",
        ) => (
            &[4, 6, 12, 16, 28],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            26_624,
            0x8003_b80c | 0x8005_f80c,
            110_592,
            "436640506a37ffea2605d20e1b1f63983d8cd019399279ed3f62e5fb17be9e18",
        ) => (
            &[0, 1],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            26_624,
            0x8003_b80c | 0x8005_f80c,
            110_592,
            "834705a61dc821f942d47bc68d3a9a08c5c924f1266527aec14173ac9aa532e1",
        ) => (
            &[4, 6, 16, 48, 56, 58],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            110_592,
            "c5b66a9d3aa3aa53f0a01ffdd83227ff46656c424318444f26e5a9080920bcf5",
        ) => (
            &[4, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 60, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            61_260,
            "79ab35585a155a4d67210af5b64140f4dfb971e4cd8f97517ac75c51b758b048",
        ) => (
            &[4, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            110_592,
            "fdfe8bd3a62f759adba71e370483e2b6d3c70a0419ed152c3518ad10fd68b8c1",
        ) => (
            &[4, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 60, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            40_960,
            0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
            57_480,
            "10dafbb38ae2aa7d798a2b6d02f35b88d89aa833f61015415ad8185a1c0f2f4d",
        ) => (
            &[4, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 48],
            US_RAW_SPRITE_FIELD_EVIDENCE,
            US_RAW_SPRITE_MIRROR_EVIDENCE,
        ),
        (
            38_912,
            0x8003_b80c | 0x8005_f80c,
            110_592,
            "30da9d48edceff573fa4d61982f76f79faa0a0f0b10095d5aee55428df03b271",
        ) => (&[4], US_REI_SPRITE_FIELD_EVIDENCE, US_REI_MIRROR_EVIDENCE),
        (
            38_912,
            0x8003_b80c | 0x8005_f80c,
            110_592,
            "674e66c3fb9779917786c30c579456ebeace8419c19a9a11907037a0b96a7e40",
        ) => (
            &[6, 0, 16, 20, 32, 28, 12, 8, 36, 40, 44, 56, 48],
            US_REI_SPRITE_FIELD_EVIDENCE,
            US_REI_MIRROR_EVIDENCE,
        ),
        _ => return None,
    };
    let mut fields = Vec::new();
    let mut mirrors = Vec::new();
    for &selector in selectors {
        fields.extend(bpl_root_sprite_fields(data, 0, data.len(), selector)?);
        let selection_table = u32::from_le_bytes(data[12..16].try_into().ok()?) as usize;
        let mirror = selection_table + 3 * usize::from(selector) + 2;
        if *data.get(mirror)? > 1 {
            return None;
        }
        mirrors.push(mirror);
    }
    let mut ranges: Vec<_> = merge_bpl_sprite_fields(fields)
        .into_iter()
        .map(|(start, end)| NontextRange {
            offset: start,
            length: end - start,
            evidence,
        })
        .collect();
    ranges.extend(mirrors.into_iter().map(|offset| NontextRange {
        offset,
        length: 1,
        evidence: mirror_evidence,
    }));
    uncovered_nontext_ranges(data.len(), ranges, previous)
}
