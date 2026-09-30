//! Selected-SEP PC rendering through the canonical editable MIDI/SF2 translation.
use crate::{
    archive::MediaImage, catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
    interchange::wave::Wave, machine::executable::Executable, music::document::Selection,
    music::document::SongReport, pc_render, sequence::midi as translation,
    sequence::timeline::Limits, sequence::timeline::Stop, sequence::Sequence,
    voice::tuning::Reference, Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use std::collections::BTreeMap;

pub struct Options {
    pub sequence: usize,
    /// Infinite-loop traversals; None means two, or enough for a fixed duration.
    pub loops: Option<u32>,
    pub playback: pc_render::Options,
    pub allow_approximations: bool,
}

#[derive(Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub executable_sha256: String,
    pub sequence_index: usize,
    pub requested_loops: Option<u32>,
    pub expanded_loop_limit: u32,
    pub song: SongReport,
    pub playback: pc_render::Report,
    pub limitations: Vec<&'static str>,
}

pub struct Rendered {
    pub wave: Wave,
    pub report: Report,
}

impl Options {
    pub fn validate(&self) -> Result<()> {
        if !self.allow_approximations {
            return Err(
                "PC archive render requires --allow-approximations for the MIDI/SF2 translation"
                    .into(),
            );
        }
        if self.loops == Some(0) || self.playback.duration_frames == Some(0) {
            return Err("PC archive render: duration and loop count must be positive".into());
        }
        if self.loops.is_some() && self.playback.duration_frames.is_some() {
            return Err("PC archive render: --duration and --loops are mutually exclusive".into());
        }
        if self.playback.repeats != 1 {
            return Err("PC archive render: expanded MIDI must be rendered once; --repeats is for MIDI input".into());
        }
        if self.playback.release_frames >= self.playback.safety_frames
            || self
                .playback
                .duration_frames
                .is_some_and(|n| n > self.playback.safety_frames - self.playback.release_frames)
        {
            return Err("PC archive render: duration plus tail exceeds audio safety limit".into());
        }
        Ok(())
    }
}

pub fn render(
    executable: &Executable,
    source: &str,
    image: &ArchiveImage,
    options: &Options,
) -> Result<Rendered> {
    options.validate()?;
    let mut catalog = Catalog::from_media(source, MediaImage::Emi(image.clone()))?;
    let mapping = loader::resolve(&mut catalog, executable)?;
    let songs: Vec<_> = catalog
        .assets
        .iter()
        .filter(|a| a.kind() == "song")
        .collect();
    let [song] = songs.as_slice() else {
        return Err("PC archive render requires exactly one song entry; archive selection is ambiguous or empty".into());
    };
    let AssetData::Song { metadata, .. } = &song.data else {
        unreachable!()
    };
    let sequence = metadata
        .sequences
        .get(options.sequence)
        .ok_or("PC archive render: selected sequence is absent")?;
    let sep = image.entry(
        song.entry
            .ok_or("PC archive render: song entry is absent")?,
    )?;
    let bytes = sep
        .get(sequence.data_offset..sequence.data_offset + sequence.data_bytes)
        .ok_or("PC archive render: selected sequence exceeds entry")?;
    let loops = match options.playback.duration_frames {
        Some(frames) => duration_loops(sequence, bytes, frames, options.playback.sample_rate)?,
        None => options.loops.unwrap_or(2),
    };
    let prepared = crate::music::document::prepare_selection(
        &catalog,
        song,
        image,
        &mapping,
        &mut Reference::from_executable(executable)?,
        &mut BTreeMap::new(),
        Selection {
            loops,
            sequence: Some(options.sequence),
        },
    )?;
    let midi = &prepared.files[&prepared.report.sequences[0].path];
    let rendered = pc_render::render(midi, &prepared.files["bank.sf2"], &options.playback)?;
    Ok(Rendered {
        wave: rendered.wave,
        report: Report {
            schema: "bof3.audio.pc-archive-render/v1",
            executable_sha256: mapping.profile.exe_sha256,
            sequence_index: options.sequence,
            requested_loops: options.loops,
            expanded_loop_limit: loops,
            song: prepared.report,
            playback: rendered.report,
            limitations: crate::music::document::LIMITATIONS.to_vec(),
        },
    })
}

fn duration_loops(sequence: &Sequence, bytes: &[u8], frames: u64, rate: u32) -> Result<u32> {
    let translated = translation::translate(
        sequence,
        bytes,
        &Limits {
            infinite_traversals: 1,
            ..Default::default()
        },
    )?;
    let first = pc_render::schedule(&translated.midi, rate)?.frames;
    if frames <= first || translated.timeline.stop == Stop::EndMarker {
        return Ok(1);
    }
    let second = translation::translate(sequence, bytes, &Limits::default())?;
    let period = pc_render::schedule(&second.midi, rate)?
        .frames
        .checked_sub(first)
        .filter(|&n| n != 0)
        .ok_or("PC archive render: loop has no positive output duration")?;
    // Tempo state can persist between traversals. Use this estimate only to
    // choose the next bounded expansion, then verify its actual scheduled end.
    let mut count: u32 = (1 + (frames - first).div_ceil(period))
        .try_into()
        .map_err(|_| "PC archive render: required loop count exceeds u32")?;
    loop {
        let translated = translation::translate(
            sequence,
            bytes,
            &Limits {
                infinite_traversals: count,
                ..Default::default()
            },
        )?;
        if pc_render::schedule(&translated.midi, rate)?.frames >= frames {
            return Ok(count);
        }
        count = count
            .checked_mul(2)
            .ok_or("PC archive render: loop expansion overflow")?;
    }
}
