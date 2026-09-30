use bof3_audio::{
    archive::disc::DiscImage,
    digest::sha256_hex,
    xa::{Arithmetic, Decoder, Histories, Stream},
};
use std::{
    collections::BTreeMap,
    io::Write,
    path::PathBuf,
    process::{Command, Stdio},
};

#[test]
#[ignore = "requires original BOF3_AUDIO_TRACK and installed FFmpeg independent decoder"]
fn all_complete_disc_xa_streams_match_ffmpeg_and_report_rounding_differences() {
    let path = PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").expect("set BOF3_AUDIO_TRACK"));
    let mut disc = DiscImage::open(&path).unwrap();
    let mut reports = Vec::new();
    let (mut total_sectors, mut total_frames, mut total_samples) = (0, 0, 0);
    let mut total_differences = 0u64;
    for name in [
        "BIN/BMAG_XA/MAGIC00.STR",
        "BIN/SCE_XA/S_XA00.STR",
        "BIN/SCE_XA/VOICE.STR",
        "LOGO/CAPCOM30.STR",
    ] {
        let image = disc.read_xa(name).unwrap();
        let reference = bof3_audio::xa::reference::identify(&sha256_hex(image.bytes())).unwrap();
        assert!(!reference.is_truncated());
        let mut selections = BTreeMap::<Stream, Vec<usize>>::new();
        for sector in image.sectors().iter().filter(|s| s.is_audio()) {
            selections
                .entry(Stream {
                    file: sector.file,
                    channel: sector.channel,
                    coding: sector.coding,
                })
                .or_default()
                .push(sector.index);
        }
        for (stream, selection) in selections {
            let mut rounded =
                Decoder::new(stream, Arithmetic::CombinedRounded, Histories::default()).unwrap();
            let mut split =
                Decoder::new(stream, Arithmetic::SplitFloor, Histories::default()).unwrap();
            let mut input = Vec::new();
            let mut expected = Vec::new();
            let mut split_bytes = Vec::new();
            let mut differences = 0u64;
            let mut squared_error = 0u64;
            let mut max_error = 0;
            for &index in &selection {
                let sector = image.sector(index).unwrap();
                // Mode-2 wrapper for the independent demuxer. Subheader/audio
                // bytes are exact original disc bytes; unrelated streams are
                // excluded so FFmpeg gets precisely this selected context.
                input.extend_from_slice(&[
                    0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 2, 0, 2,
                ]);
                input.extend_from_slice(sector);
                let a = rounded
                    .decode_sector(sector)
                    .unwrap_or_else(|e| panic!("{name} sector {index}: {e}"));
                let b = split.decode_sector(sector).unwrap();
                for (&a, &b) in a.iter().zip(&b) {
                    let difference = (i32::from(a) - i32::from(b)).unsigned_abs();
                    differences += u64::from(difference != 0);
                    squared_error += u64::from(difference).pow(2);
                    max_error = max_error.max(difference);
                }
                expected.extend(a.into_iter().flat_map(i16::to_le_bytes));
                split_bytes.extend(b.into_iter().flat_map(i16::to_le_bytes));
            }
            let mut child = Command::new("ffmpeg")
                .args([
                    "-hide_banner",
                    "-loglevel",
                    "error",
                    "-f",
                    "psxstr",
                    "-i",
                    "pipe:0",
                    "-map",
                    "0:a:0",
                    "-f",
                    "s16le",
                    "-acodec",
                    "pcm_s16le",
                    "pipe:1",
                ])
                .stdin(Stdio::piped())
                .stdout(Stdio::piped())
                .stderr(Stdio::piped())
                .spawn()
                .unwrap();
            let mut stdin = child.stdin.take().unwrap();
            let writer = std::thread::spawn(move || stdin.write_all(&input));
            let actual = child.wait_with_output().unwrap();
            writer.join().unwrap().unwrap();
            assert!(
                actual.status.success(),
                "{name} {stream:?}: {}",
                String::from_utf8_lossy(&actual.stderr)
            );
            assert_eq!(
                actual.stdout.len(),
                expected.len(),
                "{name} {stream:?}: PCM byte count"
            );
            let first_difference = actual
                .stdout
                .iter()
                .zip(&expected)
                .position(|(a, b)| a != b);
            assert!(
                first_difference.is_none(),
                "{name} {stream:?}: independent decoder differs at byte {first_difference:?}"
            );
            let frames = expected.len() / 2 / usize::from(rounded.format().channels());
            assert_eq!(
                frames,
                selection.len() * rounded.format().frames_per_sector()
            );
            reports.push(serde_json::json!({
                "source": name, "stream": stream, "sectors": selection.len(), "frames": frames,
                "rate": rounded.format().sample_rate(), "initial_history": "explicit_zero",
                "combined_rounded_sha256": sha256_hex(&expected), "split_floor_sha256": sha256_hex(&split_bytes),
                "ffmpeg_byte_equal": true, "different_samples": differences, "max_absolute_difference": max_error,
                "squared_difference_sum": squared_error,
                "rms_difference": (squared_error as f64 / (expected.len() / 2) as f64).sqrt(),
            }));
            total_sectors += selection.len();
            total_frames += frames;
            total_samples += expected.len() / 2;
            total_differences += differences;
        }
    }
    assert_eq!(reports.len(), 31);
    assert_eq!(
        (total_sectors, total_frames, total_samples),
        (26599, 84174048, 107247168)
    );
    assert!(
        total_differences > 0,
        "reference arithmetic differences must remain visible"
    );
    println!("{}", serde_json::to_string(&serde_json::json!({
        "schema": "bof3.xa-decoder-corpus-evidence/v1", "streams": reports,
        "sectors": total_sectors, "frames": total_frames, "samples": total_samples,
        "different_samples": total_differences,
        "limits": "encoded-rate codec comparison; game seek history, drive timing, de-emphasis, resampling and hardware fidelity not established",
    })).unwrap());
}
