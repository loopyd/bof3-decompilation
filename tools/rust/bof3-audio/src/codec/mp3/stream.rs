//! Validate the framing and coded-data bounds of the emitted MPEG-1 CBR profile.

use crate::Result;
use oxideav_mp3::{
    frame::{parse_header, Layer, MpegVersion},
    huffman::decode_huffman,
    scalefactors::{decode_scalefactors, MainDataReader, Reservoir},
    side_info::parse_side_info,
};
use serde::Serialize;

#[derive(Debug, PartialEq, Eq, Serialize)]
pub struct Stream {
    pub sample_rate: u32,
    pub channels: u16,
    pub bitrate_kbps: u32,
    /// Includes any Info carrier. This is not the presentation length.
    pub mpeg_frames: u32,
}

/// Validates raw, complete MPEG-1 Layer III CBR frames with no ID3/trailing tags.
/// Checks reservoir bounds, scalefactor budgets and complete big-value symbols.
/// Gapless tags, PCM reconstruction and perceptual fidelity are separate checks.
pub fn inspect(bytes: &[u8]) -> Result<Stream> {
    let mut offset = 0usize;
    let mut stream: Option<Stream> = None;
    let mut reservoir = Reservoir::new();
    let mut slot_bytes = 0u64;
    let mut consumed_bits = 0u64;
    while offset < bytes.len() {
        let header = parse_header(&bytes[offset..])?;
        if header.version != MpegVersion::Mpeg1
            || header.layer != Layer::LayerIII
            || header.crc_protected
        {
            return Err(format!("MP3 byte {offset}: requires MPEG-1 Layer III without CRC").into());
        }
        let size = header.frame_len().ok_or("MP3 free format is unsupported")?;
        let end = offset
            .checked_add(size)
            .ok_or("MP3 frame offset overflow")?;
        let frame = bytes.get(offset..end).ok_or("truncated MP3 frame")?;
        let bitrate = header
            .bitrate_kbps
            .ok_or("MP3 free format is unsupported")?;
        let current = Stream {
            sample_rate: header.sample_rate_hz,
            channels: u16::from(header.channel_count()),
            bitrate_kbps: bitrate,
            mpeg_frames: 0,
        };
        let stream = stream.get_or_insert(current);
        if stream.sample_rate != header.sample_rate_hz
            || stream.channels != u16::from(header.channel_count())
            || stream.bitrate_kbps != bitrate
        {
            return Err(
                format!("MP3 byte {offset}: inconsistent CBR rate/channels/bitrate").into(),
            );
        }
        let side = parse_side_info(&header, &frame[4..])?;
        let slot = frame
            .get(4 + side.byte_len()..)
            .ok_or("truncated MP3 side information")?;
        let back = u64::from(side.main_data_begin);
        let start_bits = slot_bytes
            .checked_sub(back)
            .ok_or("MP3 reservoir references unavailable history")?
            * 8;
        if start_bits < consumed_bits {
            return Err(
                format!("MP3 byte {offset}: reservoir overlaps previously consumed data").into(),
            );
        }
        let run = reservoir.assemble(usize::from(side.main_data_begin), slot)?;
        let bits: usize = side
            .granules
            .iter()
            .flat_map(|g| g.iter().take(stream.channels as usize))
            .map(|gc| usize::from(gc.part2_3_length))
            .sum();
        if bits > run.len() * 8 {
            return Err(
                format!("MP3 byte {offset}: granule lengths exceed available main data").into(),
            );
        }
        let factors = decode_scalefactors(&header, &side, &run)?;
        let mut cursor = 0usize;
        for (gr, granule) in side.granules.iter().enumerate() {
            for (ch, gc) in granule.iter().take(stream.channels as usize).enumerate() {
                let length = usize::from(gc.part2_3_length);
                let part2 = factors.part2_bits[gr][ch] as usize;
                if part2 > length {
                    return Err(format!("MP3 byte {offset}, granule {gr}, channel {ch}: scalefactors exceed coded length").into());
                }
                let mut reader = MainDataReader::new(&run);
                let mut skip = cursor + part2;
                while skip > 0 {
                    let step = skip.min(32);
                    reader.read(step as u32);
                    skip -= step;
                }
                // With zero count1 budget, this primitive reads only the
                // explicitly declared big-value pairs. Check their full wire
                // extent ourselves: the upstream decoder does not cap them at
                // part2_3_length and can otherwise read into the next channel.
                decode_huffman(&mut reader, gc, 0, header.sample_rate_hz, header.version)?;
                if reader.bit_pos() > cursor + length || reader.exhausted() {
                    return Err(format!("MP3 byte {offset}, granule {gr}, channel {ch}: big-value symbols exceed coded length").into());
                }
                cursor += length;
            }
        }
        consumed_bits = start_bits + bits as u64;
        slot_bytes += slot.len() as u64;
        stream.mpeg_frames = stream
            .mpeg_frames
            .checked_add(1)
            .ok_or("MP3 frame count overflow")?;
        offset = end;
    }
    stream.ok_or_else(|| "empty MP3 stream".into())
}
