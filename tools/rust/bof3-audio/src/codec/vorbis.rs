//! Vorbis delivery encoding and independent Ogg framing/timing validation.

use crate::{interchange::wave::Wave, Result};
use oxideav_vorbis::oggfile::{encode_pcm_to_ogg, StreamEncoderConfig};
use serde::Serialize;

#[derive(Debug, PartialEq, Eq, Serialize)]
pub struct Stream {
    pub sample_rate: u32,
    pub channels: u8,
    pub frames: u64,
    pub serial: u32,
    pub pages: u32,
    pub packets: u64,
}

#[derive(Debug)]
pub struct Encoded {
    pub bytes: Vec<u8>,
    pub stream: Stream,
}

pub fn encode(wave: &Wave, quality: f32) -> Result<Encoded> {
    if !matches!(wave.channels, 1 | 2) || wave.sample_rate == 0 {
        return Err("Vorbis requires mono/stereo PCM and a nonzero sample rate".into());
    }
    if wave.pcm.is_empty() || !wave.pcm.len().is_multiple_of(usize::from(wave.channels)) {
        return Err("Vorbis requires nonempty, complete interleaved PCM frames".into());
    }
    if !quality.is_finite() || !(0.0..=1.0).contains(&quality) {
        return Err("Vorbis quality must be finite and between zero and one".into());
    }
    let channels = usize::from(wave.channels);
    let pcm: Vec<Vec<f32>> = (0..channels)
        .map(|ch| {
            wave.pcm[ch..]
                .iter()
                .step_by(channels)
                .map(|&sample| f32::from(sample) / 32768.0)
                .collect()
        })
        .collect();
    let mut config = StreamEncoderConfig::new(wave.sample_rate, wave.channels as u8);
    config.quality = quality;
    let bytes = encode_pcm_to_ogg(&pcm, &config)?;
    let stream = inspect(&bytes)?;
    if stream.sample_rate != wave.sample_rate
        || u16::from(stream.channels) != wave.channels
        || stream.frames != wave.frames() as u64
    {
        return Err("Vorbis encoder changed PCM rate, channels or presentation length".into());
    }
    Ok(Encoded { bytes, stream })
}

/// Checks a single complete logical stream's pages, CRCs, lacing, identification
/// and end granule. Audio-packet decoding/quality remain separate evidence.
pub fn inspect(bytes: &[u8]) -> Result<Stream> {
    let mut offset = 0usize;
    let mut pages = 0u32;
    let mut packets = 0u64;
    let mut serial = None;
    let mut format = None;
    let mut last_granule = 0u64;
    let mut packet = Vec::new();
    let mut ended = false;
    while offset < bytes.len() {
        let header = bytes
            .get(offset..offset + 27)
            .ok_or("truncated Ogg page header")?;
        if &header[..4] != b"OggS" || header[4] != 0 {
            return Err("invalid Ogg capture/version".into());
        }
        let flags = header[5];
        if flags & !7 != 0 || ended {
            return Err("invalid Ogg flags or bytes after end of stream".into());
        }
        if (flags & 2 != 0) != (pages == 0) || (flags & 1 != 0) != (!packet.is_empty()) {
            return Err("inconsistent Ogg beginning/continuation flags".into());
        }
        let current_serial = u32::from_le_bytes(header[14..18].try_into()?);
        if serial.is_some_and(|expected| expected != current_serial) {
            return Err("multiple Ogg logical streams are not supported".into());
        }
        serial = Some(current_serial);
        if u32::from_le_bytes(header[18..22].try_into()?) != pages {
            return Err("noncontiguous Ogg page sequence".into());
        }
        let count = usize::from(header[26]);
        let lacing = bytes
            .get(offset + 27..offset + 27 + count)
            .ok_or("truncated Ogg segment table")?;
        let body_len: usize = lacing.iter().map(|&n| usize::from(n)).sum();
        let end = offset + 27 + count + body_len;
        let page = bytes.get(offset..end).ok_or("truncated Ogg page body")?;
        if crc(page) != u32::from_le_bytes(header[22..26].try_into()?) {
            return Err("Ogg page CRC mismatch".into());
        }
        let granule = u64::from_le_bytes(header[6..14].try_into()?);
        let mut at = offset + 27 + count;
        let mut completed = 0;
        for &size in lacing {
            let size = usize::from(size);
            packet.extend_from_slice(&bytes[at..at + size]);
            at += size;
            if size < 255 {
                if packets < 3 {
                    let kind = [1, 3, 5][packets as usize];
                    if packet.len() < 7 || packet[0] != kind || &packet[1..7] != b"vorbis" {
                        return Err("invalid Vorbis header order or signature".into());
                    }
                    if packets == 0 {
                        if packet.len() != 30 || packet[7..11] != [0; 4] || packet[29] != 1 {
                            return Err("invalid Vorbis identification header".into());
                        }
                        let rate = u32::from_le_bytes(packet[12..16].try_into()?);
                        let channels = packet[11];
                        let short = packet[28] & 15;
                        let long = packet[28] >> 4;
                        if !matches!(channels, 1 | 2)
                            || rate == 0
                            || !(6..=13).contains(&short)
                            || !(short..=13).contains(&long)
                        {
                            return Err("invalid Vorbis rate, channel layout or block sizes".into());
                        }
                        format = Some((rate, channels));
                    }
                } else if packet.is_empty() || packet[0] & 1 != 0 {
                    return Err("invalid Vorbis audio packet marker".into());
                }
                packets += 1;
                completed += 1;
                packet.clear();
            }
        }
        if completed == 0 {
            if granule != u64::MAX {
                return Err("Ogg page without a completed packet needs an unset granule".into());
            }
        } else {
            if granule == u64::MAX || granule < last_granule {
                return Err("invalid Ogg granule ordering".into());
            }
            last_granule = granule;
        }
        ended = flags & 4 != 0;
        if ended && (!packet.is_empty() || packets < 4 || completed == 0 || granule == 0) {
            return Err("incomplete Vorbis end page".into());
        }
        pages = pages.checked_add(1).ok_or("Ogg page count overflow")?;
        offset = end;
    }
    if !ended {
        return Err("missing Ogg end-of-stream page".into());
    }
    let (sample_rate, channels) = format.ok_or("missing Vorbis identification")?;
    Ok(Stream {
        sample_rate,
        channels,
        frames: last_granule,
        serial: serial.unwrap(),
        pages,
        packets,
    })
}

fn crc(page: &[u8]) -> u32 {
    let mut crc = 0u32;
    for (index, &byte) in page.iter().enumerate() {
        crc ^= u32::from(if (22..26).contains(&index) { 0 } else { byte }) << 24;
        for _ in 0..8 {
            crc = (crc << 1) ^ if crc & 0x80000000 != 0 { 0x04c11db7 } else { 0 };
        }
    }
    crc
}
