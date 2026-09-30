//! Passive catalog identities and evidence reports; no loading or execution.

use crate::{
    bank::{Bank, Sample},
    machine::profile::Profile,
    sequence::{Sequence, SequenceSet},
};
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize)]
pub struct Catalog {
    pub schema: &'static str,
    pub sources: Vec<Source>,
    pub assets: Vec<Asset>,
    pub unresolved: Vec<&'static str>,
    pub runtime_mapping: Option<Report>,
}

#[derive(Debug, Serialize)]
pub struct Source {
    pub source: String,
    pub bytes: usize,
    pub container: &'static str,
    pub entries: Vec<Entry>,
    /// Whole-file identity pins every input, including unrelated entries and padding.
    pub sha256: Option<String>,
}

#[derive(Debug, Serialize)]
pub struct Entry {
    pub id: String,
    pub entry: usize,
    pub file_type: u16,
    pub offset: u64,
    pub bytes: u32,
    pub load_argument: u32,
    /// Hash of the complete declared type-7 payload, excluding archive padding.
    pub body_sha256: Option<String>,
}

#[derive(Debug, Serialize)]
pub struct Asset {
    pub id: String,
    pub source: String,
    pub entry: Option<usize>,
    pub game_bank_id: Option<u16>,
    pub game_song_ids: Vec<u16>,
    pub content_type: &'static str,
    #[serde(flatten)]
    pub data: AssetData,
}

#[derive(Debug, Serialize)]
#[serde(tag = "kind", content = "metadata", rename_all = "snake_case")]
pub enum AssetData {
    Bank {
        header_sha256: String,
        body_candidates: Vec<String>,
        content: Option<Content>,
        #[serde(flatten)]
        metadata: Bank,
    },
    Sample {
        bank: String,
        #[serde(flatten)]
        metadata: Sample,
    },
    Song {
        bank_candidates: Vec<String>,
        #[serde(flatten)]
        metadata: SequenceSet,
    },
    Sequence {
        song: String,
        #[serde(flatten)]
        metadata: Sequence,
    },
    XaStream(XaStream),
    XaCue(XaCue),
}

#[derive(Debug, Serialize)]
pub struct XaStream {
    pub file: u8,
    pub channel: u8,
    pub coding: u8,
    pub channels: Option<u8>,
    pub sample_rate: Option<u32>,
    pub bits_per_sample: Option<u8>,
    pub sector_ranges: Vec<SectorRange>,
    pub eof_sectors: Vec<usize>,
    pub cue_boundaries: &'static str,
}

#[derive(Debug, Serialize)]
pub struct SectorRange {
    pub start: usize,
    pub end_exclusive: usize,
}

impl Asset {
    pub fn kind(&self) -> &'static str {
        match self.data {
            AssetData::Bank { .. } => "bank",
            AssetData::Sample { .. } => "sample",
            AssetData::Song { .. } => "song",
            AssetData::Sequence { .. } => "sequence",
            AssetData::XaStream(_) => "xa_stream",
            AssetData::XaCue(_) => "xa_cue",
        }
    }

    pub(super) fn numeric_id(&self) -> Option<usize> {
        match &self.data {
            AssetData::Bank { .. } | AssetData::Song { .. } => self.entry,
            AssetData::Sample { metadata, .. } => Some(usize::from(metadata.sample_id)),
            AssetData::Sequence { metadata, .. } => Some(usize::from(metadata.sequence_id)),
            AssetData::XaStream(stream) => Some(usize::from(stream.channel)),
            AssetData::XaCue(cue) => Some(usize::from(cue.packed_id)),
        }
    }

    pub fn matches_mode(&self, mode: &str) -> bool {
        match mode {
            "audio" => !matches!(
                self.data,
                AssetData::Song { .. } | AssetData::Sequence { .. }
            ),
            "music" => !matches!(self.data, AssetData::XaStream(_) | AssetData::XaCue(_)),
            _ => false,
        }
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct Layout {
    pub selector: u8,
    pub instructions: u64,
    pub slots: Vec<Slot>,
}

#[derive(Clone, Debug, Serialize)]
pub struct Slot {
    pub index: usize,
    pub spu_base: u32,
    pub header_address: u32,
    pub auxiliary_address: u32,
    pub sequence_address: u32,
    pub vab_id: u16,
    pub flags: u16,
    pub header_capacity: u32,
    pub sequence_capacity: u32,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub profile: Profile,
    pub layouts: Vec<Layout>,
    pub associations: Vec<Association>,
    pub shared_banks: Vec<Group>,
    pub references: Vec<Reference>,
    pub unresolved: Vec<String>,
    pub music: MusicMap,
    pub xa: XaMap,
}

#[derive(Debug, Serialize)]
pub struct Reference {
    pub target: &'static str,
    pub address: u32,
    pub executable_file_offset: usize,
    pub symbol: Option<&'static str>,
    pub role: &'static str,
}

#[derive(Debug, Serialize)]
pub struct Association {
    pub bank: String,
    pub load_slot: u32,
    pub game_bank_id: u16,
    pub body_entries: Vec<String>,
    pub auxiliary_entries: Vec<String>,
    pub songs: Vec<String>,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct MusicFile {
    pub slot: u16,
    pub path: String,
    pub lba: u32,
    pub bytes: usize,
    pub sha256: String,
}

#[derive(Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct DiscEvidence {
    pub schema: String,
    pub executable_sha256: String,
    pub track_sha256: String,
    pub file_lba_table: u32,
    pub files: Vec<MusicFile>,
}

#[derive(Debug, Serialize)]
pub struct MusicMap {
    pub cue_table: u32,
    pub supported_cues: u16,
    pub extent_evidence: &'static str,
    pub disc_evidence: DiscEvidence,
    pub cues: Vec<Cue>,
}

#[derive(Debug, Serialize)]
pub struct Cue {
    pub id: String,
    pub game_song_id: u16,
    pub record_address: u32,
    pub file_slot: u16,
    pub disc_lba: u32,
    pub disc_path: String,
    pub game_bank_id: u8,
    pub sequence_index: u8,
    pub sequence_assets: Vec<String>,
    pub resolution: &'static str,
}

#[derive(Clone, Debug, Serialize)]
pub struct XaCue {
    pub packed_id: u16,
    pub stream: u8,
    pub cue_index: u16,
    pub table_address: u32,
    pub disc_path: &'static str,
    pub disc_file_slot: u16,
    pub filter_file: u8,
    pub channel: u8,
    pub sector_start: usize,
    pub sector_stride: usize,
    pub sector_count: usize,
    pub last_sector: usize,
    pub runtime_start_lba: u32,
    pub runtime_start_bcd: [u8; 3],
    pub runtime_stop_threshold_lba: u32,
    pub runtime_stop_threshold_bcd: [u8; 3],
    pub scheduler_endpoint: &'static str,
}

#[derive(Debug, Serialize)]
pub struct Binding {
    pub source: String,
    pub source_completeness: &'static str,
    pub missing_selected_sectors: usize,
    pub asset: Option<String>,
}

#[derive(Debug, Serialize)]
pub struct CueMapping {
    pub cue: XaCue,
    pub bindings: Vec<Binding>,
}

#[derive(Debug, Serialize)]
pub struct XaMap {
    pub selector: u32,
    pub file_slots: u32,
    pub table_pointers: u32,
    pub extent_evidence: &'static str,
    pub cues: Vec<CueMapping>,
    pub scheduler: SchedulerEvidence,
}

#[derive(Debug, Serialize)]
pub struct SchedulerEvidence {
    pub start_entry: u32,
    pub completion_callback: u32,
    pub tick_entry: u32,
    pub state_table: u32,
    pub state_entries: [u32; 9],
    pub position_poll_period_ticks: u16,
    pub watchdog_threshold_ticks: u16,
    pub stop_condition: &'static str,
    pub timing_evidence: &'static str,
}

#[derive(Debug, Serialize)]
pub struct Content {
    pub body_entry: String,
    pub body_sha256: String,
    pub header_bytes: u32,
    pub body_bytes: u32,
    pub declared_sample_bytes: usize,
}

#[derive(Debug, Serialize)]
pub struct Group {
    pub header_sha256: String,
    pub body_sha256: String,
    pub header_bytes: u32,
    pub body_bytes: u32,
    pub banks: Vec<String>,
    pub equivalence: &'static str,
}
