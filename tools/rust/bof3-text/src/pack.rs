//! Dialogue packing: in-place row replacement inside a fixed allocation.
//!
//! The offset tables are preserved verbatim and each row is rewritten only inside
//! its own extent, so an unedited extract -> pack round trip is byte-identical.

use crate::models::{Entry, Result, STRING_TERMINATOR, Section, invalid};
use crate::syntax::{self, Document};

/// Rebuild a section from edited text, never exceeding the original allocation.
pub fn rebuild(section: &Section, document: &Document) -> Result<Vec<u8>> {
    if document.banks.len() != section.banks.len() {
        return invalid(format!(
            "expected {} banks, found {}",
            section.banks.len(),
            document.banks.len()
        ));
    }
    let mut output = vec![0u8; section.size];
    // The two-bank header sits before the first bank; preserve it verbatim so a
    // rebuild cannot zero it out.
    output[..section.prefix.len()].copy_from_slice(&section.prefix);
    for (bank, doc_bank) in section.banks.iter().zip(&document.banks) {
        if doc_bank.len() != bank.rows.len() {
            return invalid(format!(
                "bank {}: expected {} entries, found {}",
                bank.index,
                bank.rows.len(),
                doc_bank.len()
            ));
        }
        let plans = plan_bank(bank, doc_bank)?;
        let mut body = bank.body.clone();
        for (row, plan) in bank.rows.iter().zip(&plans) {
            let (Some((from, to)), Some(encoded)) = (row.span, plan.as_ref()) else {
                continue;
            };
            let offset = from.checked_sub(bank.pointer_bytes).ok_or_else(|| {
                crate::models::Error::Invalid(format!(
                    "bank {} entry {} points inside the offset table; refusing to rebuild",
                    bank.index, row.number
                ))
            })?;
            let length = to - from;
            if offset + length > body.len() {
                return invalid(format!(
                    "bank {} entry {} extends past the bank body; refusing to rebuild",
                    bank.index, row.number
                ));
            }
            body[offset..offset + length].fill(STRING_TERMINATOR);
            body[offset..offset + encoded.len()].copy_from_slice(encoded);
        }
        let mut bank_bytes = Vec::with_capacity(bank.size);
        for offset in &bank.offsets {
            bank_bytes.extend_from_slice(&offset.to_le_bytes());
        }
        bank_bytes.extend_from_slice(&body);
        bank_bytes.resize(bank.size, 0);
        output[bank.start..bank.start + bank.size].copy_from_slice(&bank_bytes);
    }
    Ok(output)
}

/// Resolve and validate every row edit in one bank.
fn plan_bank(
    bank: &crate::models::Bank,
    doc_bank: &[syntax::DocumentRow],
) -> Result<Vec<Option<Vec<u8>>>> {
    let mut plans: Vec<Option<Vec<u8>>> = Vec::with_capacity(bank.rows.len());
    for (row, document_row) in bank.rows.iter().zip(doc_bank) {
        let Some((from, to)) = row.span else {
            // A trailing/empty slot has no allocated bytes. Accepting text for it
            // would be silently discarded, so refuse instead of dropping the edit.
            if document_row.text.is_some() {
                return invalid(format!(
                    "bank {} entry {} has no allocated span in the original bank, so it cannot \
                     hold text; only existing rows can be edited",
                    bank.index, row.number
                ));
            }
            plans.push(None);
            continue;
        };
        let encoded = match &document_row.text {
            None => Vec::new(),
            Some(text) => syntax::check_row(text).map_err(|error| {
                crate::models::Error::Invalid(format!(
                    "bank {} entry {}: {error}",
                    bank.index, row.number
                ))
            })?,
        };
        let available = to - from;
        if encoded.len() > available {
            return invalid(format!(
                "bank {} entry {}: {} bytes needed, {available} available in its extent \
                 (the extent may also hold its option list)",
                bank.index,
                row.number,
                encoded.len()
            ));
        }
        plans.push(Some(encoded));
    }

    // Slots that share one extent are the same row and must stay in step.
    for (index, row) in bank.rows.iter().enumerate() {
        let Some(span) = row.span else { continue };
        for (other, other_row) in bank.rows.iter().enumerate().skip(index + 1) {
            if other_row.span == Some(span) && plans[index] != plans[other] {
                return invalid(format!(
                    "bank {} entries {} and {} share one extent but were edited differently",
                    bank.index, row.number, other_row.number
                ));
            }
        }
    }
    Ok(plans)
}

/// Replace one payload in an EMI archive, preserving sector alignment.
pub fn replace_payload(data: &[u8], entry: Entry, payload: &[u8]) -> Result<Vec<u8>> {
    if payload.len() != entry.size as usize {
        return invalid("replacement payload changed size; relocation is not supported");
    }
    let end = entry.offset + payload.len();
    if end > data.len() {
        return invalid("replacement payload escapes the archive");
    }
    let mut output = data.to_vec();
    output[entry.offset..end].copy_from_slice(payload);
    Ok(output)
}
