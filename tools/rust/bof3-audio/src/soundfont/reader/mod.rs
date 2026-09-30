//! Preservation-aware SF2 structure and PCM reader. Parsing does not approve game edits.
pub mod model;

use crate::soundfont::reader::model::{Instrument, Preset, Sample};
use crate::{soundfont::tables, soundfont::tables::u16_at, soundfont::tables::u32_at, Result};
use std::{collections::BTreeMap, ops::Range};

#[derive(Clone, Copy, Debug)]
pub struct Limits {
    pub bytes: usize,
    pub chunks: usize,
    pub records: usize,
    pub sample_points: usize,
}
impl Default for Limits {
    fn default() -> Self {
        Self {
            bytes: 256 * 1024 * 1024,
            chunks: 65536,
            records: 1_000_000,
            sample_points: 32 * 1024 * 1024,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Chunk {
    pub scope: [u8; 4],
    pub id: [u8; 4],
    pub data: Range<usize>,
}
#[derive(Debug)]
pub struct Font {
    pub version: (u16, u16),
    pub sample_bits: u8,
    pub chunks: Vec<Chunk>,
    pub presets: Vec<Preset>,
    pub instruments: Vec<Instrument>,
    pub samples: Vec<Sample>,
    pub diagnostics: Vec<String>,
    bytes: Vec<u8>,
    pcm24: Vec<i32>,
}
impl Font {
    pub fn from_bytes(bytes: Vec<u8>) -> Result<Self> {
        Self::with_limits(bytes, Limits::default())
    }
    pub fn with_limits(bytes: Vec<u8>, limits: Limits) -> Result<Self> {
        if bytes.len() > limits.bytes {
            return Err("SF2: byte limit exceeded".into());
        }
        if bytes.len() < 12 || &bytes[..4] != b"RIFF" || &bytes[8..12] != b"sfbk" {
            return Err("SF2: expected RIFF sfbk header".into());
        }
        if u32_at(&bytes, 4) as usize != bytes.len() - 8 {
            return Err("SF2: RIFF size does not match file length".into());
        }
        let mut budget = limits.chunks;
        let roots = chunks(&bytes, 12..bytes.len(), *b"sfbk", &mut budget)?;
        let mut all = roots.clone();
        let mut lists = BTreeMap::new();
        let mut order = Vec::new();
        for chunk in &roots {
            if chunk.id != *b"LIST" {
                continue;
            }
            if chunk.data.len() < 4 {
                return Err("SF2: LIST missing type".into());
            }
            let kind: [u8; 4] = bytes[chunk.data.start..chunk.data.start + 4]
                .try_into()
                .unwrap();
            if [*b"INFO", *b"sdta", *b"pdta"].contains(&kind) {
                if lists.contains_key(&kind) {
                    return Err("SF2: duplicate required LIST".into());
                }
                let children = chunks(
                    &bytes,
                    chunk.data.start + 4..chunk.data.end,
                    kind,
                    &mut budget,
                )?;
                all.extend(children.clone());
                lists.insert(kind, children);
                order.push(kind);
            }
        }
        if order != [*b"INFO", *b"sdta", *b"pdta"] {
            return Err("SF2: required INFO/sdta/pdta lists missing or out of order".into());
        }
        let info = &lists[b"INFO"];
        let sdta = &lists[b"sdta"];
        let pdta = &lists[b"pdta"];
        let version = required(&bytes, info, b"ifil")?;
        if version.len() != 4 {
            return Err("SF2: invalid ifil version record".into());
        }
        let version = (u16_at(version, 0), u16_at(version, 2));
        if !matches!(version, (2, 0) | (2, 1) | (2, 4)) {
            return Err(format!("SF2: unsupported version {}.{}", version.0, version.1).into());
        }
        for id in [b"isng", b"INAM"] {
            let text = required(&bytes, info, id)?;
            if text.is_empty() || !text.contains(&0) {
                return Err("SF2: required INFO string lacks NUL terminator".into());
            }
        }
        let smpl = optional(&bytes, sdta, b"smpl")?.unwrap_or(&[]);
        if smpl.len() % 2 != 0 {
            return Err("SF2: odd PCM16 sample byte count".into());
        }
        let points = smpl.len() / 2;
        if points > limits.sample_points {
            return Err("SF2: sample point limit exceeded".into());
        }
        let sm24 = optional(&bytes, sdta, b"sm24")?;
        if let Some(low) = sm24 {
            if version != (2, 4) || smpl.is_empty() || low.len() != points + (points % 2) {
                return Err("SF2: sm24 requires version 2.04 and one low byte per PCM16 point with even payload alignment".into());
            }
        }
        let pcm24 = smpl
            .as_chunks::<2>()
            .0
            .iter()
            .enumerate()
            .map(|(i, word)| {
                i32::from(i16::from_le_bytes(*word)) * 256 + i32::from(sm24.map_or(0, |s| s[i]))
            })
            .collect();
        let mut tables = BTreeMap::new();
        let mut table_order = Vec::new();
        for (id, _) in tables::TABLES {
            tables.insert(id, required(&bytes, pdta, &id)?);
        }
        for chunk in pdta {
            if tables.contains_key(&chunk.id) {
                table_order.push(chunk.id);
            }
        }
        if table_order != tables::TABLES.map(|(id, _)| id) {
            return Err("SF2: hydra tables out of order".into());
        }
        let (presets, instruments, samples) = tables::read(&tables, limits.records)?;
        let mut result = Self {
            version,
            sample_bits: if sm24.is_some() { 24 } else { 16 },
            chunks: all,
            presets,
            instruments,
            samples,
            diagnostics: Vec::new(),
            bytes,
            pcm24,
        };
        let has_rom = optional(&result.bytes, info, b"irom")?
            .is_some_and(|value| !value.is_empty() && value.contains(&0));
        result.validate_samples(has_rom)?;
        Ok(result)
    }

    /// Original file bytes, including all opaque chunks, padding and terminal
    /// records. This is not a serializer for mutations of the public parsed view.
    pub fn original_bytes(&self) -> &[u8] {
        &self.bytes
    }
    /// Entire signed PCM24 pool, including guard and unreferenced points.
    pub fn pcm24_pool(&self) -> &[i32] {
        &self.pcm24
    }
    /// Signed 24-bit values: PCM16 points are scaled by 256, never rounded.
    /// Header sample bounds only; zone address-offset generators are not applied.
    pub fn sample_pcm24(&self, index: usize) -> Result<&[i32]> {
        let sample = self
            .samples
            .get(index)
            .ok_or("SF2: sample index out of range")?;
        if sample.kind & 0x8000 != 0 {
            return Err("SF2: ROM sample PCM is unavailable".into());
        }
        self.pcm24
            .get(sample.start as usize..sample.end as usize)
            .ok_or_else(|| "SF2: sample bounds exceed PCM pool".into())
    }

    fn validate_samples(&mut self, has_rom: bool) -> Result<()> {
        for (index, sample) in self.samples.iter().enumerate() {
            let kind = sample.kind & 0x7fff;
            if !matches!(kind, 1 | 2 | 4 | 8)
                || sample.rate == 0
                || (sample.root_key > 127 && sample.root_key != 255)
            {
                return Err(format!("SF2 sample {index}: invalid type, rate or root key").into());
            }
            if sample.start >= sample.end {
                return Err(format!("SF2 sample {index}: empty/reversed sample range").into());
            }
            if sample.kind & 0x8000 != 0 {
                if !has_rom {
                    return Err(
                        format!("SF2 sample {index}: ROM sample requires irom metadata").into(),
                    );
                }
                self.diagnostics.push(format!(
                    "sample {index}: ROM bytes and playback are unavailable"
                ));
            } else {
                if sample.end as usize > self.pcm24.len()
                    || sample.loop_start as usize > self.pcm24.len()
                    || sample.loop_end as usize > self.pcm24.len()
                {
                    return Err(format!("SF2 sample {index}: bounds exceed PCM pool").into());
                }
                let end = sample.end as usize;
                if !self
                    .pcm24
                    .get(end..end.saturating_add(46))
                    .is_some_and(|s| s.iter().all(|&v| v == 0))
                {
                    self.diagnostics.push(format!(
                        "sample {index}: missing/nonzero 46-point sample guard"
                    ));
                }
            }
            if kind != 1 {
                let target = self.samples.get(usize::from(sample.link)).ok_or_else(|| {
                    format!("SF2 sample {index}: linked sample index out of range")
                })?;
                if matches!(kind, 2 | 4)
                    && (target.kind != (sample.kind ^ 6)
                        || usize::from(target.link) != index
                        || usize::from(sample.link) == index)
                {
                    return Err(format!("SF2 sample {index}: stereo links must be reciprocal left/right records of the same storage type").into());
                }
            }
            if sample.loop_start > sample.loop_end {
                return Err(format!("SF2 sample {index}: reversed loop range").into());
            }
            if sample.loop_start < sample.start || sample.loop_end > sample.end {
                self.diagnostics.push(format!("sample {index}: header loop is outside sample bounds; loop mode and address-offset generators require semantic validation"));
            }
            if sample.end - sample.start < 48 || !(400..=50000).contains(&sample.rate) {
                self.diagnostics.push(format!(
                    "sample {index}: sample length/rate outside portable recommendations"
                ));
            }
        }
        Ok(())
    }
}

fn chunks(
    bytes: &[u8],
    range: Range<usize>,
    scope: [u8; 4],
    budget: &mut usize,
) -> Result<Vec<Chunk>> {
    let mut at = range.start;
    let mut result = Vec::new();
    while at < range.end {
        if *budget == 0 {
            return Err("SF2: chunk count limit exceeded".into());
        }
        *budget -= 1;
        if range.end - at < 8 {
            return Err("SF2: truncated chunk header".into());
        }
        let size = u32_at(bytes, at + 4) as usize;
        let start = at + 8;
        let end = start.checked_add(size).ok_or("SF2: chunk size overflow")?;
        let next = end
            .checked_add(size % 2)
            .ok_or("SF2: chunk padding overflow")?;
        if next > range.end {
            return Err("SF2: chunk payload/padding exceeds parent".into());
        }
        result.push(Chunk {
            scope,
            id: bytes[at..at + 4].try_into().unwrap(),
            data: start..end,
        });
        at = next;
    }
    Ok(result)
}
fn optional<'a>(bytes: &'a [u8], chunks: &[Chunk], id: &[u8; 4]) -> Result<Option<&'a [u8]>> {
    let mut matching = chunks.iter().filter(|c| &c.id == id);
    let first = matching.next();
    if matching.next().is_some() {
        return Err(format!("SF2: duplicate {} chunk", String::from_utf8_lossy(id)).into());
    }
    Ok(first.map(|c| &bytes[c.data.clone()]))
}
fn required<'a>(bytes: &'a [u8], chunks: &[Chunk], id: &[u8; 4]) -> Result<&'a [u8]> {
    optional(bytes, chunks, id)?
        .ok_or_else(|| format!("SF2: missing {} chunk", String::from_utf8_lossy(id)).into())
}
