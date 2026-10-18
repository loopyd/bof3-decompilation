//! Indexed and exact color-word PNG interchange through installed FFmpeg.

use std::io::{Read, Write};
use std::process::{Command, Stdio};
use std::thread;
use std::time::{Duration, Instant};

use crate::{Error, Result};

const PNG_MAGIC: &[u8] = b"\x89PNG\r\n\x1a\n";
pub const MAX_PIXELS: usize = 1024 * 1024;
pub const MAX_WORD_PNG_BYTES: usize = MAX_PIXELS * 5 + 65536;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct WordImage {
    pub width: usize,
    pub height: usize,
    pub words: Vec<u16>,
}

impl WordImage {
    pub fn validate(&self) -> Result<()> {
        if self.words.len() != dimensions(self.width, self.height)? {
            return Err(Error::Invalid(
                "color-word image must contain width*height words".into(),
            ));
        }
        Ok(())
    }
}

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

fn header(bytes: &[u8], max_bytes: usize) -> Result<(usize, usize)> {
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
    dimensions(width, height)?;
    if bytes.len() > max_bytes {
        return Err(Error::Invalid("PNG exceeds compressed-input bound".into()));
    }
    Ok((width, height))
}

pub fn decode(bytes: &[u8]) -> Result<IndexedImage> {
    let (width, height) = header(bytes, MAX_PIXELS * 2 + 65536)?;
    let count = width * height;
    if bytes[25] != 3 || ![1, 2, 4, 8].contains(&bytes[24]) {
        return Err(Error::Invalid("editable image must be indexed PNG; RGBA conversion and automatic quantization are refused".into()));
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

/// RGB uses exact 5-to-8-bit expansion. Alpha encodes a source bit, not blending:
/// zero word = 0, STP set = 128, all other words = 255.
pub fn encode_words(image: &WordImage) -> Result<Vec<u8>> {
    image.validate()?;
    let size = format!("{}x{}", image.width, image.height);
    let input = image
        .words
        .iter()
        .flat_map(|word| {
            let mut rgba = crate::formats::color_rgba(*word);
            if word & 0x8000 != 0 {
                rgba[3] = 128;
            }
            rgba
        })
        .collect();
    let bytes = ffmpeg(
        &[
            "-f",
            "rawvideo",
            "-pixel_format",
            "rgba",
            "-video_size",
            &size,
            "-i",
            "pipe:0",
            "-frames:v",
            "1",
            "-c:v",
            "png",
            "-pix_fmt",
            "rgba",
            "-f",
            "image2pipe",
            "pipe:1",
        ],
        input,
        MAX_WORD_PNG_BYTES,
    )?;
    if decode_words(&bytes)? != *image {
        return Err(Error::Codec(
            "PNG codec did not preserve color words and STP".into(),
        ));
    }
    Ok(bytes)
}

pub fn decode_words(bytes: &[u8]) -> Result<WordImage> {
    let (width, height) = header(bytes, MAX_WORD_PNG_BYTES)?;
    if bytes[24] != 8 || bytes[25] != 6 {
        return Err(Error::Invalid(
            "color-word image must be an 8-bit RGBA PNG; conversion and quantization are refused"
                .into(),
        ));
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
            "rgba",
            "pipe:1",
        ],
        bytes.to_vec(),
        width * height * 4,
    )?;
    if raw.len() != width * height * 4 {
        return Err(Error::Codec(
            "PNG decode returned an incomplete RGBA plane".into(),
        ));
    }
    let words = raw
        .as_chunks::<4>()
        .0
        .iter()
        .enumerate()
        .map(|(pixel, rgba)| {
            let mut word = 0u16;
            for (channel, value) in rgba[..3].iter().enumerate() {
                let component = value >> 3;
                if (component << 3) | (component >> 2) != *value {
                    return Err(Error::Invalid(format!(
                        "color-word pixel {pixel}: RGB channel is not an exact expanded 5-bit value"
                    )));
                }
                word |= u16::from(component) << (channel * 5);
            }
            match rgba[3] {
                0 if word == 0 => Ok(0),
                128 => Ok(word | 0x8000),
                255 if word != 0 => Ok(word),
                _ => Err(Error::Invalid(format!(
                    "color-word pixel {pixel}: alpha must be 0 for zero, 128 for STP, or 255 for a nonzero word without STP"
                ))),
            }
        })
        .collect::<Result<_>>()?;
    Ok(WordImage {
        width,
        height,
        words,
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
