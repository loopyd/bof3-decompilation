use bof3_audio::{
    codec::adpcm::encoder::{encode, Options, Padding},
    codec::adpcm::{decode_sample, Termination},
};

fn sine(frames: usize) -> Vec<i16> {
    (0..frames)
        .map(|i| (24000.0 * (i as f64 * 0.067).sin()) as i16)
        .collect()
}

fn independent_error(input: &[i16], output: &[i16]) -> (u32, f64, f64) {
    assert_eq!(input.len(), output.len());
    let errors: Vec<_> = input
        .iter()
        .zip(output)
        .map(|(&a, &b)| i32::from(b) - i32::from(a))
        .collect();
    let peak = errors.iter().map(|e| e.unsigned_abs()).max().unwrap_or(0);
    let divisor = input.len().max(1) as f64;
    let rms = (errors.iter().map(|&e| f64::from(e).powi(2)).sum::<f64>() / divisor).sqrt();
    let mean = errors.iter().map(|&e| f64::from(e)).sum::<f64>() / divisor;
    (peak, rms, mean)
}

#[test]
fn silence_empty_and_representable_endpoints_have_exact_reported_results() {
    for input in [vec![], vec![0; 84], vec![-32768; 56], vec![28672; 56]] {
        let result = encode(&input, &Options::default()).unwrap();
        let decoded = decode_sample(&result.bytes).unwrap();
        assert_eq!(decoded.pcm, input);
        assert_eq!(result.report.loss.peak_absolute_error, 0);
        assert_eq!(result.report.loss.rms_error, 0.0);
        assert_eq!(result.report.loss.mean_error, 0.0);
        assert_eq!(result.report.loss.signal_to_noise_db, None);
        assert_eq!(result.bytes.len(), input.len() / 28 * 16);
        assert_eq!(
            decoded.termination,
            if input.is_empty() {
                Termination::Empty
            } else {
                Termination::EndMute
            }
        );
        assert!(serde_json::to_string(&result.report).is_ok());
    }
}

#[test]
fn deterministic_filtered_encoding_reports_measured_loss_and_uses_available_capacity() {
    let input = sine(28 * 80);
    let options = Options {
        capacity_bytes: Some(80 * 16),
        ..Options::default()
    };
    let result = encode(&input, &options).unwrap();
    let decoded = decode_sample(&result.bytes).unwrap();
    assert_eq!(result.bytes, encode(&input, &options).unwrap().bytes);
    let (peak, rms, mean) = independent_error(&input, &decoded.pcm);
    assert_eq!(peak, result.report.loss.peak_absolute_error);
    assert!((rms - result.report.loss.rms_error).abs() < 1e-10);
    assert!((mean - result.report.loss.mean_error).abs() < 1e-10);
    assert!(result.report.predictor_blocks[1..].iter().sum::<usize>() > 60);
    // A smooth, slowly varying signal should benefit materially from prediction.
    assert!(rms < 100.0, "tonal RMS error {rms}");
    assert!(result.report.loss.signal_to_noise_db.unwrap() > 40.0);
    assert_eq!(result.report.padded_frames, 0);
    assert_eq!(result.report.encoded_bytes, 80 * 16);
    for block in result.bytes.as_chunks::<16>().0 {
        assert!(block[0] >> 4 <= 4 && block[0] & 15 <= 12);
    }
}

#[test]
fn loops_keep_endpoints_and_pcm_across_multiple_history_carrying_traversals() {
    for (frames, start) in [(28, 0), (84, 0), (28 * 40, 28 * 3)] {
        let input = sine(frames);
        let result = encode(
            &input,
            &Options {
                sample_loop: Some(start..frames),
                ..Options::default()
            },
        )
        .unwrap();
        let decoded = decode_sample(&result.bytes).unwrap();
        let loop_ = result.report.sample_loop.unwrap();
        assert_eq!(
            (loop_.start_frame, loop_.end_frame_exclusive),
            (start, frames)
        );
        assert!(loop_.pcm_repeat_is_stable);
        assert_eq!(result.bytes[start / 28 * 16] >> 4, 0);
        assert_eq!(result.bytes[result.bytes.len() - 15] & 3, 3);
        let mut history = decoded.final_history;
        for _ in 0..8 {
            let mut repeated = Vec::new();
            for block in result.bytes[start / 28 * 16..].as_chunks::<16>().0 {
                repeated.extend(history.decode_block(block).unwrap());
            }
            assert!(repeated == decoded.pcm[start..]);
        }
    }
}

#[test]
fn explicit_padding_changes_duration_but_loss_excludes_added_frames() {
    let input = sine(29);
    assert!(encode(&input, &Options::default())
        .unwrap_err()
        .to_string()
        .contains("padding"));
    let result = encode(
        &input,
        &Options {
            padding: Padding::Zero,
            ..Options::default()
        },
    )
    .unwrap();
    let decoded = decode_sample(&result.bytes).unwrap();
    assert_eq!(
        (
            result.report.input_frames,
            result.report.encoded_frames,
            result.report.padded_frames
        ),
        (29, 56, 27)
    );
    assert_eq!(result.report.loss.frames, 29);
    let padding = result.report.padding_loss.as_ref().unwrap();
    assert_eq!(padding.frames, 27);
    assert_eq!(
        padding.rms_error,
        independent_error(&[0; 27], &decoded.pcm[29..]).1
    );
    assert_eq!(
        result.report.loss.peak_absolute_error,
        independent_error(&input, &decoded.pcm[..29]).0
    );
    assert!(result.report.sample_loop.is_none());
}

#[test]
fn capacity_alignment_and_unrepresentable_loops_fail_without_truncation() {
    let input = sine(84);
    for range in [0..0, 28..28, 29..84, 0..83, 0..112, 84..84, 0..56] {
        assert!(encode(
            &input,
            &Options {
                sample_loop: Some(range),
                padding: Padding::Zero,
                ..Options::default()
            }
        )
        .is_err());
    }
    for capacity in [0, 16, 32, 47, 49] {
        assert!(encode(
            &input,
            &Options {
                capacity_bytes: Some(capacity),
                ..Options::default()
            }
        )
        .is_err());
    }
    let roomy = encode(
        &input,
        &Options {
            capacity_bytes: Some(96),
            ..Options::default()
        },
    )
    .unwrap();
    assert_eq!(
        roomy.bytes.len(),
        48,
        "unused allocation bytes are not fabricated"
    );
    assert_eq!(roomy.report.capacity_bytes, Some(96));
}

#[test]
fn full_scale_discontinuities_and_noise_do_not_overflow_or_hide_loss() {
    let mut state = 0x12345678u32;
    let input: Vec<_> = (0..28 * 64)
        .map(|i| {
            state = state.wrapping_mul(1664525).wrapping_add(1013904223);
            match i % 4 {
                0 => i16::MIN,
                1 => i16::MAX,
                _ => (state >> 16) as i16,
            }
        })
        .collect();
    let result = encode(&input, &Options::default()).unwrap();
    let decoded = decode_sample(&result.bytes).unwrap();
    let (peak, rms, mean) = independent_error(&input, &decoded.pcm);
    assert_eq!(result.report.loss.peak_absolute_error, peak);
    assert_eq!(result.report.loss.rms_error, rms);
    assert_eq!(result.report.loss.mean_error, mean);
    assert!(peak > 0 && rms > 0.0);
    assert_eq!(decoded.trailing_bytes, 0);
}

#[test]
#[ignore = "requires installed FFmpeg as a development-only independent VAG consumer"]
fn independent_vag_consumer_accepts_encoder_output_with_documented_decoder_difference() {
    use std::{
        io::Write,
        process::{Command, Stdio},
    };
    let result = encode(&sine(28 * 80), &Options::default()).unwrap();
    let mut vag = vec![0; 48];
    vag[..4].copy_from_slice(b"VAGp");
    vag[4..8].copy_from_slice(&0x20u32.to_be_bytes());
    vag[12..16].copy_from_slice(&(result.bytes.len() as u32).to_be_bytes());
    vag[16..20].copy_from_slice(&44100u32.to_be_bytes());
    vag.extend(&result.bytes);
    let mut child = Command::new("ffmpeg")
        .args([
            "-hide_banner",
            "-loglevel",
            "error",
            "-f",
            "vag",
            "-i",
            "pipe:0",
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
    child.stdin.take().unwrap().write_all(&vag).unwrap();
    let output = child.wait_with_output().unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    let pcm: Vec<_> = output
        .stdout
        .as_chunks::<2>()
        .0
        .iter()
        .map(|b| i16::from_le_bytes(*b))
        .collect();
    assert_eq!(output.stdout.len(), result.report.encoded_frames * 2);
    // FFmpeg 6.1.1 combines predictor terms with truncation toward zero and
    // retains unclipped history. The SPU model separately floors terms and
    // clips feedback. Therefore filtered PCM need not match between consumers.
    let mut history = [0i32; 2];
    let coefficients = [(0, 0), (60, 0), (115, -52), (98, -55), (122, -60)];
    let mut reference = Vec::new();
    for block in result.bytes.as_chunks::<16>().0 {
        let (a, b) = coefficients[usize::from(block[0] >> 4)];
        for i in 0..28 {
            let n = (block[2 + i / 2] >> (i % 2 * 4)) & 15;
            let value = (i32::from((u16::from(n) << 12) as i16) >> (block[0] & 15))
                + (history[0] * a + history[1] * b) / 64;
            reference.push(value.clamp(-32768, 32767) as i16);
            history = [value, history[0]];
        }
    }
    assert!(
        pcm == reference,
        "independent VAG consumer arithmetic mismatch"
    );
    let spu = decode_sample(&result.bytes).unwrap().pcm;
    let (peak, rms, _) = independent_error(&spu, &pcm);
    eprintln!("Independent VAG consumer: {} frames; FFmpeg/SPU-model difference peak={peak}, RMS={rms:.6}", pcm.len());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS; encodes bounded edited sample excerpts"]
fn corpus_edited_sample_excerpts_encode_and_report_loss_within_original_capacity() {
    use bof3_audio::{
        archive::MediaImage, catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
        digest::sha256_hex, machine::executable::Executable,
    };
    use std::{collections::BTreeSet, path::PathBuf};
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut catalog = Catalog::read(Some(&root), &[]).unwrap();
    loader::resolve(&mut catalog, &exe).unwrap();
    let mut seen = BTreeSet::new();
    let (mut samples, mut frames, mut peak, mut squared_error) = (0, 0, 0, 0f64);
    for asset in &catalog.assets {
        let AssetData::Bank {
            metadata,
            content: Some(content),
            ..
        } = &asset.data
        else {
            continue;
        };
        let MediaImage::Emi(image) = MediaImage::read(&root.join(&asset.source)).unwrap() else {
            unreachable!()
        };
        let body_index = catalog
            .sources
            .iter()
            .find(|s| s.source == asset.source)
            .unwrap()
            .entries
            .iter()
            .find(|e| e.id == content.body_entry)
            .unwrap()
            .entry;
        let body = image.entry(body_index).unwrap();
        for sample in &metadata.samples {
            let bytes = &body[sample.body_offset..sample.body_offset + sample.encoded_bytes];
            if bytes.is_empty() || !seen.insert(sha256_hex(bytes)) {
                continue;
            }
            let decoded = decode_sample(bytes).unwrap();
            let mut pcm = decoded.pcm[..decoded.pcm.len().min(28 * 8)].to_vec();
            for x in pcm.iter_mut().step_by(17) {
                *x = x.saturating_add(257);
            }
            let encoded = encode(
                &pcm,
                &Options {
                    capacity_bytes: Some(bytes.len()),
                    ..Options::default()
                },
            )
            .unwrap();
            let check = decode_sample(&encoded.bytes).unwrap();
            assert_eq!(check.pcm.len(), pcm.len());
            let error = independent_error(&pcm, &check.pcm);
            assert_eq!(error.0, encoded.report.loss.peak_absolute_error);
            assert_eq!(error.1, encoded.report.loss.rms_error);
            samples += 1;
            frames += pcm.len();
            peak = peak.max(error.0);
            squared_error += error.1.powi(2) * pcm.len() as f64;
        }
    }
    assert!(samples > 1000, "unexpectedly narrow corpus: {samples}");
    eprintln!("Edited corpus excerpts: {samples} unique nonempty allocations, {frames} frames; peak={peak}, weighted RMS={:.6}; at most eight blocks per source sample, no whole-corpus packing claim", (squared_error / frames as f64).sqrt());
}
