//! Qualified BOF3 graphics sources, reversible formats, and asset packages.

mod aliases;
pub mod animation;
pub mod area;
pub mod bank;
pub mod bank_image;
pub mod catalog;
pub mod cell;
pub mod composition;
pub mod digest;
pub mod error;
mod field;
pub mod formats;
mod interface;
pub mod lzss;
mod media;
pub mod package;
pub mod packed;
pub mod palette;
pub mod png;
pub mod preview;
pub mod record;
pub mod resident;
pub mod save_icon;
pub mod sources;
pub mod sprite_bank;
pub mod stream_family;
pub mod verification;

pub use error::{Error, Result};
