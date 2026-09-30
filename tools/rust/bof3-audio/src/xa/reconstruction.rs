//! XA replacement with fixed multiplexing, capacity and explicit cue boundaries.

use crate::{
    archive::XaImage, archive::XA_SECTOR_BYTES, digest::sha256_hex,
    xa::encoder::Report as SectorReport, xa::Arithmetic, xa::Histories, xa::Stream, Result,
};
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub stream: Stream,
    pub source_sha256: String,
    pub output_sha256: String,
    pub source_completeness: &'static str,
    pub sector_indices: Vec<usize>,
    pub reused_sectors: usize,
    pub reencoded_sectors: usize,
    pub unrelated_sectors: usize,
    pub sectors: Vec<SectorReport>,
    pub limitations: Vec<&'static str>,
}

#[derive(Debug)]
pub struct Rebuilt {
    pub image: XaImage,
    pub report: Report,
}

pub fn replace_stream(
    image: &XaImage,
    stream: Stream,
    pcm: &[i16],
    arithmetic: Arithmetic,
    initial_histories: Histories,
) -> Result<Rebuilt> {
    let indices = crate::xa::selection::stream_indices(image, stream);
    replace_sectors(image, stream, &indices, pcm, arithmetic, initial_histories)
}

/// Changed cues must agree with continuous-stream entry history and leave the
/// original exit history before following audio. Unchanged cues preserve bytes.
pub fn replace_sectors(
    image: &XaImage,
    stream: Stream,
    indices: &[usize],
    pcm: &[i16],
    arithmetic: Arithmetic,
    initial_histories: Histories,
) -> Result<Rebuilt> {
    let hash = sha256_hex(image.bytes());
    let completeness = crate::xa::reference::identify(&hash);
    if completeness.as_ref().is_some_and(|r| r.is_truncated()) {
        return Err(
            "XA rebuild: known truncated source; recover the complete stream before editing".into(),
        );
    }
    let encoded =
        crate::xa::selection::encode(image, stream, indices, pcm, arithmetic, initial_histories)?;
    let mut bytes = image.bytes().to_vec();
    for (index, sector) in &encoded.patches {
        bytes[index * XA_SECTOR_BYTES..(index + 1) * XA_SECTOR_BYTES].copy_from_slice(sector);
    }
    let sector_indices = encoded.report.sector_indices;
    let reports = encoded.report.sectors;
    let reused = encoded.report.reused_sectors;
    let output = XaImage::from_bytes(bytes)?;
    if output.sectors() != image.sectors() {
        return Err("XA rebuild: sector identity or placement changed".into());
    }
    for sector in image.sectors() {
        if sector_indices.binary_search(&sector.index).is_err()
            && output.sector(sector.index)? != image.sector(sector.index)?
        {
            return Err(format!("XA rebuild: unrelated sector {} changed", sector.index).into());
        }
    }
    let report = Report {
        schema: "bof3.audio.xa-stream-rebuild/v1", stream,
        source_sha256: hash, output_sha256: sha256_hex(output.bytes()),
        source_completeness: if completeness.is_some() { "verified" } else { "unverified" },
        reused_sectors: reused, reencoded_sectors: reports.len() - reused,
        unrelated_sectors: image.sectors().len() - reports.len(),
        sector_indices, sectors: reports,
        limitations: vec![
            "Changed partial selections require matching continuous-stream entry history and unchanged exit history before following audio; game seek/reset behavior remains unverified.",
            "Unchanged decoded content reuses original sector bytes only when it also matches under the current output predictor history.",
            "No duration/rate/channel/coding changes, insertion or relocation. Known truncated sources are rejected; unknown extents remain unverified.",
            "In-memory reconstruction; XML edit detection, manifest conflicts, publication and disc-image rebuilding are not performed here.",
        ],
    };
    Ok(Rebuilt {
        image: output,
        report,
    })
}
