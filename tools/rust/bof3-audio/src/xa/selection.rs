//! Selected XA encoding; callers own complete-source identity and publication.
use crate::{
    archive::XaImage, xa::encoder::Encoder, xa::encoder::Report as SectorReport, xa::Arithmetic,
    xa::Decoder, xa::Format, xa::Histories, xa::Stream, Result,
};
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub stream: Stream,
    pub sector_indices: Vec<usize>,
    pub reused_sectors: usize,
    pub reencoded_sectors: usize,
    pub sectors: Vec<SectorReport>,
    pub boundary_policy: &'static str,
}

pub struct Encoded {
    pub patches: Vec<(usize, Vec<u8>)>,
    pub report: Report,
}

pub(crate) fn stream_indices(image: &XaImage, stream: Stream) -> Vec<usize> {
    image
        .sectors()
        .iter()
        .filter(|s| {
            s.is_audio()
                && (s.file, s.channel, s.coding) == (stream.file, stream.channel, stream.coding)
        })
        .map(|s| s.index)
        .collect()
}

pub fn encode(
    image: &XaImage,
    stream: Stream,
    indices: &[usize],
    pcm: &[i16],
    arithmetic: Arithmetic,
    initial_histories: Histories,
) -> Result<Encoded> {
    let all_indices = stream_indices(image, stream);
    let sector_indices = indices.to_vec();
    if sector_indices.is_empty() {
        return Err("XA rebuild: selected stream has no audio sectors".into());
    }
    let first = all_indices
        .binary_search(&sector_indices[0])
        .map_err(|_| "XA rebuild: first selected sector is not in stream")?;
    let end = first
        .checked_add(sector_indices.len())
        .ok_or("XA rebuild: selection overflow")?;
    if all_indices.get(first..end) != Some(indices) {
        return Err("XA rebuild: selected sectors must be one ordered, contiguous slice of the multiplexed stream".into());
    }
    let format = Format::from_coding(stream.coding)?;
    let samples_per_sector = format.frames_per_sector() * usize::from(format.channels());
    let required = sector_indices
        .len()
        .checked_mul(samples_per_sector)
        .ok_or("XA rebuild: capacity overflow")?;
    if pcm.len() != required {
        return Err(format!("XA rebuild: selected capacity is {required} interleaved samples, received {}; duration edits require an explicit cue/padding policy", pcm.len()).into());
    }
    let mut encoder = Encoder::new(stream, arithmetic, initial_histories)?;
    let mut original_decoder = Decoder::new(stream, arithmetic, initial_histories)?;
    let mut patches = Vec::new();
    let mut reports = Vec::new();
    for (&index, source) in sector_indices
        .iter()
        .zip(pcm.chunks_exact(samples_per_sector))
    {
        if end != all_indices.len() {
            original_decoder.decode_sector(image.sector(index)?)?;
        }
        let encoded = encoder
            .encode_sector(image.sector(index)?, source)
            .map_err(|e| format!("XA rebuild sector {index}: {e}"))?;
        patches.push((index, encoded.bytes));
        reports.push(encoded.report);
    }
    let reused = reports.iter().filter(|r| r.reused_original).count();
    let changed = patches
        .iter()
        .any(|(index, bytes)| image.sector(*index).ok() != Some(bytes.as_slice()));
    if changed {
        if first != 0 {
            let mut prefix = Decoder::new(stream, arithmetic, Histories::default())?;
            for &index in &all_indices[..first] {
                prefix.decode_sector(image.sector(index)?)?;
            }
            if prefix.histories() != initial_histories {
                return Err("XA rebuild: edited cue entry history differs from the continuous stream; seek/reset context is unsupported".into());
            }
        }
        if end != all_indices.len() && encoder.histories() != original_decoder.histories() {
            return Err("XA rebuild: edited cue exit history would change following unselected audio; preserve a convergent tail or edit the complete stream".into());
        }
    }
    Ok(Encoded { patches, report: Report {
        schema: "bof3.xa-selection-encoding/v1", stream, sector_indices,
        reused_sectors: reused, reencoded_sectors: reports.len() - reused, sectors: reports,
        boundary_policy: "edited partial selections require matching continuous-stream entry and original exit history before following audio; game seek/reset behavior is unverified",
    } })
}
