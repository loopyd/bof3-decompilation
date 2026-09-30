//! Inspect emitted MP3 headers and granule allocations without decoding audio.
use bof3_audio::Result;
use oxideav_mp3::{
    frame::{parse_header, MpegVersion},
    side_info::parse_side_info,
};
use serde_json::json;

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 1 {
        return Err("usage: bitstream MPEG1_MP3".into());
    }
    let bytes = std::fs::read(&args[0])?;
    let mut offset = 0;
    let mut frames = Vec::new();
    while offset < bytes.len() {
        let header = parse_header(&bytes[offset..])?;
        if header.version != MpegVersion::Mpeg1 || header.crc_protected {
            return Err("probe expects MPEG-1 without CRC".into());
        }
        let size = header
            .frame_len()
            .ok_or("free format is not supported by this probe")?;
        let end = offset + size;
        let frame = bytes.get(offset..end).ok_or("truncated frame")?;
        let side = parse_side_info(&header, &frame[4..])?;
        let granules: Vec<_> = side
            .granules
            .iter()
            .map(|channels| {
                channels.iter().take(header.channel_count() as usize).map(|gc|json!({
            "bits":gc.part2_3_length,"pairs":gc.big_values,"gain":gc.global_gain,
            "scalefactor_compress":gc.scalefac_compress,"window":gc.window_switching_flag,
            "type":format!("{:?}",gc.block_type),"mixed":gc.mixed_block_flag,
            "tables":gc.table_select,"regions":[gc.region0_count,gc.region1_count],
            "count1_table":gc.count1table_select
        })).collect::<Vec<_>>()
            })
            .collect();
        frames.push(
            json!({"offset":offset,"bytes":size,"bitrate":header.bitrate_kbps,
            "sample_rate":header.sample_rate_hz,"channels":header.channel_count(),
            "back_pointer":side.main_data_begin,"scfsi":side.scfsi,"granules":granules}),
        );
        offset = end;
    }
    println!("{}", serde_json::to_string_pretty(&frames)?);
    Ok(())
}
