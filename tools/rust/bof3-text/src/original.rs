//! Original-US file identity used by coverage and loader-specific payload rules.

use std::sync::OnceLock;

use serde::Deserialize;

use crate::digest::sha256_hex;
use crate::models::{Error, Result};

pub const DISC: &str = include_str!("../../../../config/original-us-disc.json");

#[derive(Deserialize)]
pub struct Disc {
    pub release: String,
    pub project_sha256: String,
    pub files: Vec<File>,
}

#[derive(Deserialize)]
pub struct File {
    pub path: String,
    pub size: usize,
    pub sha256: String,
    pub emi_entries: Option<usize>,
}

pub fn disc() -> Result<&'static Disc> {
    static ORIGINAL: OnceLock<std::result::Result<Disc, String>> = OnceLock::new();
    ORIGINAL
        .get_or_init(|| serde_json::from_str(DISC).map_err(|error| error.to_string()))
        .as_ref()
        .map_err(|error| Error::Invalid(format!("original US disc qualification: {error}")))
}

/// Whole-archive identity, independent of the input's path or regional label.
pub fn archive(data: &[u8]) -> Result<bool> {
    let original = disc()?;
    let digest = sha256_hex(data);
    Ok(original
        .files
        .iter()
        .any(|file| file.emi_entries.is_some() && file.size == data.len() && file.sha256 == digest))
}
