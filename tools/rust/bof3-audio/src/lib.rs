//! Host audio tooling. Recovered game C remains independently owned.

pub mod archive;
pub mod bank;
pub mod catalog;
mod catalog_cli;
pub mod cli;
pub mod codec;
pub mod digest;
pub mod document;
mod extract_cli;
pub mod interchange;
pub mod machine;
pub mod music;
pub mod pack;
mod pack_cli;
pub mod pc_archive;
pub mod pc_render;
pub mod psx_render;
mod publication;
mod render_cli;
pub mod sample;
pub mod sequence;
pub mod soundfont;
pub mod verify;
pub mod voice;
pub mod xa;

pub type Result<T> = std::result::Result<T, Box<dyn std::error::Error + Send + Sync>>;

pub mod driver;
