//! Recognize supplied executables by whole-file identity. Recognition does not
//! authorize an unimplemented bootstrap or prove full audio runtime fidelity.

use super::executable::Executable;
use crate::{digest::sha256_hex, Result};
use serde::Serialize;

pub const US_EXE_SHA256: &str = "0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a";
pub const US_PITCH_SHA256: &str =
    "293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2";

#[derive(Clone, Debug, Serialize)]
pub struct Profile {
    pub id: &'static str,
    pub target: &'static str,
    pub exe_sha256: String,
    pub pitch_entry: u32,
    pub pitch_table: u32,
    pub pitch_table_entries: usize,
    pub spu_pitch_entry: u32,
    pub spu_pitch_table: u32,
    pub cue_dispatch: u32,
    pub sound_initialize: u32,
    pub sequence_table_setup: u32,
    pub set_tick_mode: u32,
    pub bootstrap: &'static str,
}

impl Profile {
    pub fn identify(executable: &Executable) -> Result<Self> {
        let hash = sha256_hex(executable.bytes());
        if hash != US_EXE_SHA256 {
            return Err(format!("unsupported runtime profile: executable SHA-256 {hash}; only original US SLUS_004.22 is identified").into());
        }
        Ok(Self {
            id: "bof3-us-slus-00422",
            target: "exe/slus_004_22",
            exe_sha256: hash,
            pitch_entry: 0x8017_1b20,
            pitch_table: 0x8018_445c,
            pitch_table_entries: 193,
            spu_pitch_entry: 0x8016_986c,
            spu_pitch_table: 0x8018_4284,
            cue_dispatch: 0x8016_1c20,
            sound_initialize: 0x8015_cd00,
            sequence_table_setup: 0x8016_d7ec,
            set_tick_mode: 0x8016_d9cc,
            bootstrap: "bios_audio_reference_clock_available_fidelity_unverified",
        })
    }
}
