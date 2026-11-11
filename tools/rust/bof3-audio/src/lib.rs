//! Host audio tooling. Recovered game C remains independently owned.

pub mod archive;
pub mod bank;
pub mod catalog;
pub mod cli;
pub mod codec;
pub mod digest;
pub mod document;
pub mod interchange;
pub mod machine;
pub mod music;
pub mod packing;
pub mod player;
mod publication;
pub mod render;
pub mod sample;
pub mod sequence;
pub mod soundfont;
pub mod verification;
pub mod voice;
pub mod xa;

pub type Result<T> = std::result::Result<T, Box<dyn std::error::Error + Send + Sync>>;

pub mod driver;
