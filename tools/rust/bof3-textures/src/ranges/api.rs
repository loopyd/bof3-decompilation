//! Physical graphics roles bound to original source bytes and entry descriptors.

use std::sync::OnceLock;

use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;

use super::{Entry, NontextRange, payload_ranges, sha256_hex, vram_tile_prefix_bytes};
use crate::{Error, Result, sources};

const SCHEMA: &str = "bof3.texture-ranges/v1";

#[derive(Serialize)]
pub struct Identity {
    schema: &'static str,
    interpretation_sha256: String,
    qualification_sha256: String,
}

#[derive(Serialize)]
pub struct Report {
    #[serde(flatten)]
    identity: &'static Identity,
    source: Option<Source>,
}

#[derive(Serialize)]
struct Source {
    sha256: String,
    size: usize,
    executable_ranges: Vec<NontextRange>,
    entries: Vec<Payload>,
}

#[derive(Serialize)]
struct Payload {
    entry: Entry,
    sha256: String,
    whole_vram_tiles: bool,
    ranges: Vec<NontextRange>,
}

pub fn identity() -> &'static Identity {
    static VALUE: OnceLock<Identity> = OnceLock::new();
    VALUE.get_or_init(|| Identity {
        schema: SCHEMA,
        interpretation_sha256: sha256_hex(
            [
                SCHEMA,
                sources::QUALIFICATION,
                include_str!("api.rs"),
                include_str!("mod.rs"),
                include_str!("resident.rs"),
                include_str!("compressed.rs"),
                include_str!("sprite.rs"),
                include_str!("copies.rs"),
                include_str!("../../../emi-ex/src/lib.rs"),
                include_str!("../../../emi-ex/src/image.rs"),
            ]
            .join("\n")
            .as_bytes(),
        ),
        qualification_sha256: sha256_hex(sources::QUALIFICATION.as_bytes()),
    })
}

pub fn classify(bytes: Vec<u8>) -> Result<Report> {
    if bytes.len() as u64 > sources::MAX_FILE_BYTES {
        return Err(Error::Invalid(
            "graphics range input exceeds the source size limit".into(),
        ));
    }
    let digest = sha256_hex(&bytes);
    let known = sources::qualification()?;
    let Some(file) = known
        .files
        .iter()
        .find(|file| file.size == bytes.len() as u64 && file.sha256 == digest)
    else {
        return Ok(Report {
            identity: identity(),
            source: None,
        });
    };
    let mut source = Source {
        sha256: digest,
        size: bytes.len(),
        executable_ranges: Vec::new(),
        entries: Vec::new(),
    };
    if file.path == "SLUS_004.22" {
        source.executable_ranges = executable_ranges();
    }
    if file.emi_entries.is_some() {
        let image = ArchiveImage::from_bytes(bytes)?;
        for (index, descriptor) in image.entries().iter().enumerate() {
            let data = image.entry(index)?;
            let entry = Entry {
                index,
                offset: usize::try_from(descriptor.offset).map_err(|_| {
                    Error::Invalid("graphics entry offset exceeds the host range".into())
                })?,
                size: descriptor.size,
                ram_ptr: descriptor.ram_ptr,
                transfer_mode: descriptor.file_type,
                check_word: descriptor.first4,
            };
            source.entries.push(Payload {
                entry,
                sha256: sha256_hex(data),
                whole_vram_tiles: vram_tile_prefix_bytes(entry, data) == Some(data.len()),
                ranges: payload_ranges(entry, data),
            });
        }
    }
    Ok(Report {
        identity: identity(),
        source: Some(source),
    })
}

fn executable_ranges() -> Vec<NontextRange> {
    vec![NontextRange {
        offset: 955_708,
        length: 32,
        evidence: EXECUTABLE_DIMENSIONS_EVIDENCE,
    }]
}

const EXECUTABLE_DIMENSIONS_EVIDENCE: &str = "original US SLUS 0x8017F53C..0x8017F55C unsigned width/height pairs; bounded byte-pair consumer 0x8014C7F0..0x8014C818; exact executable identity; stored numeric representation only";
