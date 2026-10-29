//! Counted draw-state records consumed by SLUS_004.22@0x8014DD3C.

use serde::{Deserialize, Serialize};

use crate::{Error, Result};

pub const SCHEMA: &str = "bof3.sprite-definitions/v1";

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Definition {
    pub offset: usize,
    pub id: u16,
    pub clut_selector: u8,
    pub opaque: u8,
    pub page: u8,
    pub v_base: u8,
    pub indexed_8bit: u8,
    pub group: u8,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Document {
    pub schema: String,
    pub definitions: Vec<Definition>,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub definitions: usize,
    pub tail_bytes: usize,
}

fn invalid(reason: &str) -> Error {
    Error::Codec(format!("sprite definitions: {reason}"))
}

impl Document {
    /// Match the original first-ID lookup, without assigning an active entity.
    pub fn select(&self, id: u16) -> Option<&Definition> {
        self.definitions.iter().find(|record| record.id == id)
    }

    pub fn info(&self, source: &[u8]) -> Result<Info> {
        let used = self
            .definitions
            .len()
            .checked_mul(8)
            .and_then(|n| n.checked_add(4))
            .ok_or_else(|| invalid("record size overflow"))?;
        let tail_bytes = source
            .len()
            .checked_sub(used)
            .ok_or_else(|| invalid("records exceed source allocation"))?;
        Ok(Info {
            definitions: self.definitions.len(),
            tail_bytes,
        })
    }
}

/// Decode the byte count at 0x800E3800 and eight-byte records at +4.
/// The other header bytes, record byte three and trailing bytes stay opaque.
pub fn decode(source: &[u8]) -> Result<Document> {
    if source.len() > 1024 * 1024 {
        return Err(invalid("source exceeds 1 MiB"));
    }
    let header = source.get(..4).ok_or_else(|| invalid("missing header"))?;
    let count = usize::from(header[0]);
    let records = source
        .get(4..4 + count * 8)
        .ok_or_else(|| invalid("count exceeds the source allocation"))?;
    Ok(Document {
        schema: SCHEMA.into(),
        definitions: records
            .as_chunks::<8>()
            .0
            .iter()
            .enumerate()
            .map(|(index, bytes)| Definition {
                offset: 4 + 8 * index,
                id: u16::from_le_bytes([bytes[0], bytes[1]]),
                clut_selector: bytes[2],
                opaque: bytes[3],
                page: bytes[4],
                v_base: bytes[5],
                indexed_8bit: bytes[6],
                group: bytes[7],
            })
            .collect(),
    })
}

/// Edit only the four consumed draw-state bytes in existing records.
/// IDs, groups, ordering, allocation and all opaque bytes remain unchanged.
pub fn encode(source: &[u8], edited: &Document) -> Result<Vec<u8>> {
    let original = decode(source)?;
    if edited.schema != SCHEMA || edited.definitions.len() != original.definitions.len() {
        return Err(invalid("edit changes schema or record allocation"));
    }
    let mut output = source.to_vec();
    for (record, raw) in edited.definitions.iter().zip(&original.definitions) {
        if (record.offset, record.id, record.opaque, record.group)
            != (raw.offset, raw.id, raw.opaque, raw.group)
        {
            return Err(invalid(
                "edit changes an ID, group, opaque byte or record offset",
            ));
        }
        output[raw.offset + 2] = record.clut_selector;
        output[raw.offset + 4] = record.page;
        output[raw.offset + 5] = record.v_base;
        output[raw.offset + 6] = record.indexed_8bit;
    }
    Ok(output)
}
