//! Explicit frozen adjacent data for isolated pitch evaluation.
use crate::{digest::sha256_hex, machine::image::profile::US_PITCH_SHA256, Result};
use serde::{Deserialize, Serialize};
use std::ops::Range;

pub(crate) const ADJACENT_RANGE: Range<u32> = 0x8018_4406..0x8018_445c;
const ROUTINE_SHA256: &str = "7d0ddc9ba2af236e6c483444f6def9d03bf4917d10906ce1060d4785144b9679";

#[derive(Clone, Debug, Serialize)]
pub struct RuntimeContext {
    pub ram_sha256: String,
    pub adjacent_data_sha256: String,
    pub pitch_routine_sha256: String,
    pub pitch_table_sha256: &'static str,
    pub scope: &'static str,
}

#[derive(Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
struct Document {
    schema: String,
    ram_sha256: String,
    adjacent_data_sha256: String,
    pitch_routine_sha256: String,
    pitch_table_sha256: String,
    adjacent_data: Vec<u8>,
}

pub struct FrozenPitchContext(Document);

impl FrozenPitchContext {
    pub fn from_bytes(bytes: &[u8]) -> Result<Self> {
        let document: Document = serde_json::from_slice(bytes)?;
        if document.schema != "bof3.audio-pitch-context/v1"
            || document.adjacent_data.len() != 86
            || document.ram_sha256.len() != 64
            || !document
                .ram_sha256
                .bytes()
                .all(|byte| byte.is_ascii_hexdigit())
            || document.adjacent_data_sha256 != sha256_hex(&document.adjacent_data)
            || document.pitch_routine_sha256 != ROUTINE_SHA256
            || document.pitch_table_sha256 != US_PITCH_SHA256
        {
            return Err("tuning context: invalid frozen US pitch context".into());
        }
        Ok(Self(document))
    }

    pub fn to_bytes(&self) -> Result<Vec<u8>> {
        Ok(serde_json::to_vec_pretty(&self.0)?)
    }

    pub(crate) fn from_capture(ram: &[u8]) -> Result<Self> {
        let offset = crate::machine::image::executable::ram_offset(ADJACENT_RANGE.start, 86)?;
        let adjacent_data = ram[offset..offset + 86].to_vec();
        let document = Document {
            schema: "bof3.audio-pitch-context/v1".into(),
            ram_sha256: sha256_hex(ram),
            adjacent_data_sha256: sha256_hex(&adjacent_data),
            pitch_routine_sha256: ROUTINE_SHA256.into(),
            pitch_table_sha256: US_PITCH_SHA256.into(),
            adjacent_data,
        };
        Self::from_bytes(&serde_json::to_vec(&document)?)
    }

    pub(super) fn adjacent(&self) -> &[u8] {
        &self.0.adjacent_data
    }

    pub(super) fn evidence(&self) -> RuntimeContext {
        RuntimeContext {
            ram_sha256: self.0.ram_sha256.clone(),
            adjacent_data_sha256: self.0.adjacent_data_sha256.clone(),
            pitch_routine_sha256: self.0.pitch_routine_sha256.clone(),
            pitch_table_sha256: US_PITCH_SHA256,
            scope: "isolated source-qualified pitch routine with a frozen supplied runtime gap; capture history, natural reachability, concurrent scheduling and PCM fidelity are unverified",
        }
    }
}
