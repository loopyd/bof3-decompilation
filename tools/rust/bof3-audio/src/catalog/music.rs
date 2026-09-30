//! US music cue lookup with independently verified disc-file identities.
//! Original executable/media are supplied by the user; no payloads are embedded.

use crate::catalog::model::{Cue, DiscEvidence, MusicMap};
use crate::{
    catalog::model::AssetData, catalog::model::Catalog, machine::bus::Bus, machine::bus::Ram,
    machine::bus::Width, machine::executable::Executable, machine::profile::Profile,
    machine::profile::US_EXE_SHA256, Result,
};
use std::collections::{BTreeMap, BTreeSet};

pub const CUE_TABLE: u32 = 0x8018_1eb8;
pub const FILE_TABLE: u32 = 0x8018_2444;
pub const SUPPORTED_CUES: u16 = 165;

pub fn disc_evidence() -> Result<DiscEvidence> {
    let evidence: DiscEvidence = serde_json::from_str(include_str!("music.json"))?;
    if evidence.schema != "bof3.audio-us-music-files/v1"
        || evidence.executable_sha256 != US_EXE_SHA256
        || evidence.file_lba_table != FILE_TABLE
        || evidence.files.len() != 81
    {
        return Err("invalid compiled US disc identity metadata".into());
    }
    let mut slots = BTreeSet::new();
    for file in &evidence.files {
        if !slots.insert(file.slot)
            || !(209..=289).contains(&file.slot)
            || file.sha256.len() != 64
            || !file.sha256.bytes().all(|b| b.is_ascii_hexdigit())
        {
            return Err("invalid or duplicate compiled US music identity".into());
        }
    }
    Ok(evidence)
}

pub fn resolve(catalog: &mut Catalog, executable: &Executable) -> Result<MusicMap> {
    let profile = Profile::identify(executable)?;
    let evidence = disc_evidence()?;
    for asset in &mut catalog.assets {
        asset.game_song_ids.clear();
    }
    let mut ram = Ram::from_executable(executable);
    let files: BTreeMap<_, _> = evidence
        .files
        .iter()
        .map(|file| (file.slot, file))
        .collect();
    for file in &evidence.files {
        if ram.read(FILE_TABLE + u32::from(file.slot) * 4, Width::Word)? != file.lba {
            return Err(format!(
                "music file slot {} disagrees with verified ISO extent",
                file.slot
            )
            .into());
        }
    }
    let mut cues = Vec::new();
    let mut reached_files = BTreeSet::new();
    for game_song_id in 0..SUPPORTED_CUES {
        let address = CUE_TABLE + u32::from(game_song_id) * 4;
        let file_slot = ram.read(address, Width::Half)? as u16;
        let bank = ram.read(address + 2, Width::Byte)? as u8;
        let sequence = ram.read(address + 3, Width::Byte)? as u8;
        let file = files
            .get(&file_slot)
            .ok_or("US cue points outside the verified music file slots")?;
        if bank >= 7 || sequence >= 4 {
            return Err("US music cue has unsupported bank/sequence fields".into());
        }
        reached_files.insert(file_slot);
        let mut sources: BTreeSet<_> = catalog
            .sources
            .iter()
            .filter(|source| {
                source.bytes == file.bytes && source.sha256.as_deref() == Some(file.sha256.as_str())
            })
            .map(|source| source.source.as_str())
            .collect();
        // Distinct shipped filenames can have byte-identical contents (BGMBAT00/02).
        // Preserve a supplied disc-qualified identity before offering content aliases.
        let suffix = format!("/{}", file.path);
        let qualified: BTreeSet<_> = sources
            .iter()
            .copied()
            .filter(|source| *source == file.path || source.ends_with(&suffix))
            .collect();
        let path_matched = !qualified.is_empty();
        if path_matched {
            sources = qualified;
        }
        let mut sequence_assets = Vec::new();
        let mut songs = BTreeSet::new();
        for asset in &mut catalog.assets {
            if !sources.contains(asset.source.as_str())
                || asset.game_bank_id != Some(u16::from(bank))
            {
                continue;
            }
            if let AssetData::Sequence { song, metadata } = &asset.data {
                if metadata.sequence_index == usize::from(sequence) {
                    asset.game_song_ids.push(game_song_id);
                    sequence_assets.push(asset.id.clone());
                    songs.insert(song.clone());
                }
            }
        }
        for asset in &mut catalog.assets {
            if songs.contains(&asset.id) {
                asset.game_song_ids.push(game_song_id);
            }
        }
        let resolution = match sequence_assets.len() {
            0 if sources.is_empty() => "original_source_not_supplied",
            0 => "sequence_association_unresolved",
            1 if path_matched => "disc_path_and_whole_file_identity_match",
            1 => "whole_file_identity_match",
            _ => "ambiguous_supplied_sources",
        };
        cues.push(Cue {
            id: format!("{}#cue={game_song_id}", profile.id),
            game_song_id,
            record_address: address,
            file_slot,
            disc_lba: file.lba,
            disc_path: file.path.clone(),
            game_bank_id: bank,
            sequence_index: sequence,
            sequence_assets,
            resolution,
        });
    }
    if reached_files.len() != 81 {
        return Err("US supported cues do not cover all verified music files".into());
    }
    let next_slot = ram.read(CUE_TABLE + u32::from(SUPPORTED_CUES) * 4, Width::Half)? as u16;
    if files.contains_key(&next_slot) {
        return Err("US cue prefix ends before another recognized music record".into());
    }
    for asset in &mut catalog.assets {
        asset.game_song_ids.sort_unstable();
        asset.game_song_ids.dedup();
    }
    Ok(MusicMap { cue_table: CUE_TABLE, supported_cues: SUPPORTED_CUES,
        extent_evidence: "165 consecutive media-backed records cover all 81 BGM files; next record is not a supported music file; caller bounds remain unverified",
        disc_evidence: evidence, cues })
}
