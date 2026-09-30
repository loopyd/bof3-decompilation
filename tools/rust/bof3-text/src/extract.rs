//! Dialogue extraction: EMI subfile discovery, bank parsing, document rendering.
//!
//! A row's bytes are its slot extent: from the slot's pointer up to the next
//! larger pointer in the table (or the end of the bank). That extent holds the
//! visible string plus, for choice rows, the option strings that follow the
//! `{end}` terminator. Extents tile the string region, so every byte belongs to
//! exactly one row (modulo shared pointers) and nothing is orphaned.

use crate::models::{
    Bank, DOCUMENT_VERSION, DialogueRow, Entry, Error, FIRST_PAYLOAD, HEADER_SIZE, MAGIC, Result,
    SECTOR, Section, TOC_ENTRY_SIZE, invalid,
};

/// A subfile that begins with this word declares two pointer banks.
const TWO_BANK_MARKER: [u8; 4] = [0x08, 0x00, 0x00, 0x00];

/// Parse the TOC of an EMI archive and return every text subfile: the entry
/// paired with its text class (`dialogue`, `battle`).
pub fn text_entries(data: &[u8]) -> Result<Vec<(Entry, crate::models::TextClass)>> {
    if data.len() < HEADER_SIZE || &data[8..16] != MAGIC {
        return invalid("not an EMI archive (missing MATH_TBL magic)");
    }
    let count = u32::from_le_bytes(data[0..4].try_into().unwrap()) as usize;
    if count == 0 || HEADER_SIZE + count * TOC_ENTRY_SIZE > data.len() {
        return invalid("EMI TOC entry count does not fit the file");
    }
    let mut entries = Vec::new();
    let mut offset = FIRST_PAYLOAD;
    for index in 0..count {
        let at = HEADER_SIZE + index * TOC_ENTRY_SIZE;
        let size = u32::from_le_bytes(data[at..at + 4].try_into().unwrap());
        let ram_ptr = u32::from_le_bytes(data[at + 4..at + 8].try_into().unwrap());
        if let Some(class) = crate::models::TextClass::from_load_argument(ram_ptr) {
            entries.push((
                Entry {
                    index,
                    offset,
                    size,
                    ram_ptr,
                },
                class,
            ));
        }
        let advanced = (size as usize)
            .checked_add(SECTOR - 1)
            .map(|value| value / SECTOR * SECTOR)
            .ok_or_else(|| Error::Invalid("EMI size overflow".into()))?;
        offset = offset
            .checked_add(advanced)
            .ok_or_else(|| Error::Invalid("EMI size overflow".into()))?;
    }
    Ok(entries)
}

/// The selected text subfile: its entry, its class, and its payload bytes.
pub fn select_entry(
    data: &[u8],
    requested: Option<usize>,
) -> Result<(Entry, crate::models::TextClass, &[u8])> {
    let entries = text_entries(data)?;
    if entries.is_empty() {
        return invalid("archive has no text subfile (load argument 0x80010000 or 0x8001A000)");
    }
    let chosen = match requested {
        Some(index) => entries
            .iter()
            .find(|(entry, _)| entry.index == index)
            .copied()
            .ok_or_else(|| Error::Invalid(format!("entry {index} is not a text subfile")))?,
        None if entries.len() == 1 => entries[0],
        None => {
            return invalid(format!(
                "archive has {} text subfiles; select one with --entry (or --entry all)",
                entries.len()
            ));
        }
    };
    let (entry, class) = chosen;
    let end = entry.offset + entry.size as usize;
    if end > data.len() {
        return invalid("text subfile escapes the archive");
    }
    Ok((entry, class, &data[entry.offset..end]))
}

/// Every text subfile in the archive, as `(entry, class, payload)`.
pub fn all_entries(data: &[u8]) -> Result<Vec<(Entry, crate::models::TextClass, &[u8])>> {
    let mut out = Vec::new();
    for (entry, class) in text_entries(data)? {
        let end = entry.offset + entry.size as usize;
        if end > data.len() {
            return invalid("text subfile escapes the archive");
        }
        out.push((entry, class, &data[entry.offset..end]));
    }
    Ok(out)
}

/// Parse one dialogue subfile into one or two banks.
pub fn parse_section(data: &[u8]) -> Result<Section> {
    if data.len() < 4 {
        return invalid("dialogue subfile is shorter than one bank header");
    }
    let bounds = bank_bounds(data)?;
    let mut banks = Vec::with_capacity(bounds.len());
    for (index, (start, end)) in bounds.iter().enumerate() {
        banks.push(parse_bank(data, *start, *end, index)?);
    }
    Ok(Section {
        prefix: data[..bounds[0].0].to_vec(),
        banks,
        size: data.len(),
    })
}

/// Bank extent(s) within a subfile.
///
/// A subfile whose first word is `0x00000008` is a **two-bank** file: an 8-byte
/// header (`u32 8`, `u32 block1_start`) precedes the first bank, and the second
/// bank runs from that word to the end. Verified on 51 US battle/system text
/// subfiles (load argument `0x8001A000`); both banks obey the same rules as a
/// dialogue bank. The header is preserved verbatim by packing.
fn bank_bounds(data: &[u8]) -> Result<Vec<(usize, usize)>> {
    if data[..4] != TWO_BANK_MARKER {
        return Ok(vec![(0, data.len())]);
    }
    if data.len() < 8 {
        return invalid("two-bank subfile is shorter than its header");
    }
    let first = u32::from_le_bytes(data[0..4].try_into().unwrap()) as usize;
    let second = u32::from_le_bytes(data[4..8].try_into().unwrap()) as usize;
    if first < 8 || second < first || second >= data.len() {
        return invalid(format!(
            "invalid two-bank header: block0 0x{first:x}..0x{second:x} of {}",
            data.len()
        ));
    }
    Ok(vec![(first, second), (second, data.len())])
}

/// Parse one bank, whose bytes are `data[start..end]`.
fn parse_bank(data: &[u8], start: usize, end: usize, index: usize) -> Result<Bank> {
    let available = end - start;
    if available < 4 {
        return invalid(format!(
            "bank {index}: {} byte(s) at subfile offset {start} are shorter than a bank header",
            available
        ));
    }
    let pointer_bytes = u16::from_le_bytes(data[start..start + 2].try_into().unwrap()) as usize;
    if pointer_bytes < 4 || pointer_bytes % 2 != 0 || pointer_bytes > available {
        return invalid(format!(
            "bank {index}: invalid pointer_bytes {pointer_bytes} at subfile offset {start}"
        ));
    }
    let count = pointer_bytes / 2;
    let mut offsets = Vec::with_capacity(count);
    for slot in 0..count {
        let at = start + slot * 2;
        offsets.push(u16::from_le_bytes(data[at..at + 2].try_into().unwrap()));
    }
    if offsets[0] as usize != pointer_bytes {
        return invalid(format!(
            "bank {index}: offset table is not self-describing ({} != {pointer_bytes})",
            offsets[0]
        ));
    }
    // Every distinct in-range pointer, used to find each slot's extent end.
    let mut starts: Vec<usize> = offsets
        .iter()
        .map(|value| *value as usize)
        .filter(|value| *value <= available)
        .collect();
    starts.sort_unstable();
    starts.dedup();

    let mut rows = Vec::with_capacity(count);
    for (slot, offset) in offsets.iter().enumerate() {
        let number = slot + 1;
        let from = *offset as usize;
        // A pointer into the offset table itself is malformed: rows live after it.
        // Rejecting here is what keeps packing from subtracting past the table.
        if from < pointer_bytes && from != available {
            return invalid(format!(
                "bank {index}: row {number} points inside the offset table ({from} < {pointer_bytes})"
            ));
        }
        if from >= available {
            if from > available {
                return invalid(format!(
                    "bank {index}: row {number} points past the subfile"
                ));
            }
            rows.push(DialogueRow {
                number,
                span: None,
                encoded: Vec::new(),
            });
            continue;
        }
        let to = starts
            .iter()
            .copied()
            .find(|value| *value > from)
            .unwrap_or(available);
        rows.push(DialogueRow {
            number,
            span: Some((from, to)),
            encoded: data[start + from..start + to].to_vec(),
        });
    }
    let body = data[start + pointer_bytes..end].to_vec();
    Ok(Bank {
        index,
        start,
        size: available,
        pointer_bytes,
        offsets,
        rows,
        body,
    })
}

/// Build the JSON text-object document for a section.
pub fn document(section: &Section, source: &crate::models::Source) -> crate::syntax::Document {
    crate::syntax::Document {
        version: DOCUMENT_VERSION,
        source: source.clone(),
        fingerprint: Some(fingerprint(section)),
        latch: None,
        banks: section
            .banks
            .iter()
            .map(|bank| {
                bank.rows
                    .iter()
                    .map(|row| crate::syntax::DocumentRow {
                        number: row.number,
                        text: if row.is_empty() {
                            None
                        } else {
                            Some(crate::command::deserialize(&row.encoded))
                        },
                    })
                    .collect()
            })
            .collect(),
    }
}

/// Number of rows whose extent carries text after its `{end}` (option lists).
pub fn option_rows(section: &Section) -> usize {
    section
        .banks
        .iter()
        .flat_map(|bank| bank.rows.iter())
        .filter(|row| row.encoded[..row.encoded.len().saturating_sub(1)].contains(&0x00))
        .count()
}

/// A dependency-free fingerprint of a section's row text, for document headers.
pub fn fingerprint(section: &Section) -> String {
    let mut hash: u64 = 0xcbf2_9ce4_8422_2325;
    let mut mix = |byte: u8| {
        hash ^= u64::from(byte);
        hash = hash.wrapping_mul(0x0000_0100_0000_01b3);
    };
    for bank in &section.banks {
        mix(0x1e);
        for row in &bank.rows {
            for byte in &row.encoded {
                mix(*byte);
            }
            mix(0x1f);
        }
    }
    format!("{hash:016x}")
}
