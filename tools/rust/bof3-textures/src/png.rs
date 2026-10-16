//! Indexed PNG interchange through the installed FFmpeg pal8 codec.
//! Palette words and STP remain separate source metadata.

use std::io::{Read, Write};
use std::process::{Command, Stdio};
use std::thread;
use std::time::{Duration, Instant};

use crate::{Error, Result};

const PNG_MAGIC: &[u8] = b"\x89PNG\r\n\x1a\n";
pub const MAX_PIXELS: usize = 1024 * 1024;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct IndexedImage {
    pub width: usize,
    pub height: usize,
    pub indices: Vec<u8>,
    /// Exact PNG display palette in BGRA order, with 256 entries.
    pub palette_bgra: Vec<u8>,
}

impl IndexedImage {
    pub fn validate(&self) -> Result<()> {
        let pixels = dimensions(self.width, self.height)?;
        if self.indices.len() != pixels || self.palette_bgra.len() != 1024 {
            return Err(Error::Invalid(
                "indexed image must contain width*height indices and 256 BGRA colors".into(),
            ));
        }
        Ok(())
    }
}

fn dimensions(width: usize, height: usize) -> Result<usize> {
    width
        .checked_mul(height)
        .filter(|count| width > 0 && height > 0 && *count <= MAX_PIXELS)
        .ok_or_else(|| {
            Error::Invalid(format!(
                "PNG dimensions {width}x{height} exceed the nonempty {MAX_PIXELS}-pixel bound"
            ))
        })
}

pub fn encode(image: &IndexedImage) -> Result<Vec<u8>> {
    image.validate()?;
    let size = format!("{}x{}", image.width, image.height);
    let mut input = image.indices.clone();
    input.extend_from_slice(&image.palette_bgra);
    let bytes = ffmpeg(
        &[
            "-f",
            "rawvideo",
            "-pixel_format",
            "pal8",
            "-video_size",
            &size,
            "-i",
            "pipe:0",
            "-frames:v",
            "1",
            "-c:v",
            "png",
            "-pix_fmt",
            "pal8",
            "-f",
            "image2pipe",
            "pipe:1",
        ],
        input,
        MAX_PIXELS * 2 + 65536,
    )?;
    if decode(&bytes)? != *image {
        return Err(Error::Codec(
            "PNG codec did not preserve indices and palette order".into(),
        ));
    }
    Ok(bytes)
}

pub fn decode(bytes: &[u8]) -> Result<IndexedImage> {
    if bytes.len() < 33 || &bytes[..8] != PNG_MAGIC || &bytes[8..16] != b"\0\0\0\rIHDR" {
        return Err(Error::Invalid("expected a PNG with a standard IHDR".into()));
    }
    let width = u32::from_be_bytes(
        bytes[16..20]
            .try_into()
            .map_err(|_| Error::Invalid("invalid PNG width".into()))?,
    ) as usize;
    let height = u32::from_be_bytes(
        bytes[20..24]
            .try_into()
            .map_err(|_| Error::Invalid("invalid PNG height".into()))?,
    ) as usize;
    let count = dimensions(width, height)?;
    if bytes[25] != 3 || ![1, 2, 4, 8].contains(&bytes[24]) {
        return Err(Error::Invalid("editable image must be indexed PNG; RGBA conversion and automatic quantization are refused".into()));
    }
    if bytes.len() > MAX_PIXELS * 2 + 65536 {
        return Err(Error::Invalid("PNG exceeds compressed-input bound".into()));
    }
    let raw = ffmpeg(
        &[
            "-err_detect",
            "crccheck+explode",
            "-i",
            "pipe:0",
            "-frames:v",
            "1",
            "-f",
            "rawvideo",
            "-pix_fmt",
            "pal8",
            "pipe:1",
        ],
        bytes.to_vec(),
        count + 1024,
    )?;
    if raw.len() != count + 1024 {
        return Err(Error::Codec(format!(
            "PNG decode returned {} bytes, expected {}",
            raw.len(),
            count + 1024
        )));
    }
    Ok(IndexedImage {
        width,
        height,
        indices: raw[..count].to_vec(),
        palette_bgra: raw[count..].to_vec(),
    })
}

fn ffmpeg(arguments: &[&str], input: Vec<u8>, max_output: usize) -> Result<Vec<u8>> {
    let mut child = Command::new("ffmpeg")
        .args(["-nostdin", "-v", "error", "-threads", "1"])
        .args(arguments)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|error| Error::Codec(format!("cannot run installed ffmpeg: {error}")))?;
    let mut stdin = child
        .stdin
        .take()
        .ok_or_else(|| Error::Codec("missing codec stdin".into()))?;
    let stdout = child
        .stdout
        .take()
        .ok_or_else(|| Error::Codec("missing codec stdout".into()))?;
    let stderr = child
        .stderr
        .take()
        .ok_or_else(|| Error::Codec("missing codec stderr".into()))?;
    let writer = thread::spawn(move || stdin.write_all(&input));
    let reader = thread::spawn(move || {
        let mut out = Vec::new();
        stdout
            .take(max_output as u64 + 1)
            .read_to_end(&mut out)
            .map(|_| out)
    });
    let diagnostics = thread::spawn(move || {
        let mut out = Vec::new();
        stderr.take(65537).read_to_end(&mut out).map(|_| out)
    });
    let deadline = Instant::now() + Duration::from_secs(20);
    let status = loop {
        if let Some(status) = child.try_wait()? {
            break Some(status);
        }
        if Instant::now() >= deadline {
            child.kill()?;
            child.wait()?;
            break None;
        }
        thread::sleep(Duration::from_millis(10));
    };
    let written = writer
        .join()
        .map_err(|_| Error::Codec("codec input worker failed".into()))?;
    let output = reader
        .join()
        .map_err(|_| Error::Codec("codec output worker failed".into()))??;
    let errors = diagnostics
        .join()
        .map_err(|_| Error::Codec("codec diagnostic worker failed".into()))??;
    if !status.is_some_and(|status| status.success())
        || output.len() > max_output
        || written.is_err()
    {
        return Err(Error::Codec(format!(
            "PNG codec failed or exceeded its time/output bound: {}",
            String::from_utf8_lossy(&errors)
        )));
    }
    Ok(output)
}
