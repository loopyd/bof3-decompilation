use bof3_audio::{
    codec::adpcm::History, codec::edc::check_form2, codec::edc::checksum, codec::edc::Status,
    xa::encoder::Encoder, xa::Arithmetic, xa::Decoder, xa::Format, xa::Histories, xa::Stream,
};

fn stream(coding: u8) -> Stream {
    Stream {
        file: 1,
        channel: 3,
        coding,
    }
}

fn template(coding: u8, edc: bool) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, 3, 0xe4, coding, 1, 3, 0xe4, coding]);
    for group in 0..18 {
        let start = 8 + group * 128;
        bytes[start..start + 16].fill(0x1c);
        if coding & 16 != 0 {
            bytes[start + 8..start + 16].fill(0xa5);
        }
        bytes[start + 16..start + 128].fill(0x11);
    }
    bytes[2312..2332].fill(0x7a);
    if edc {
        let crc = checksum(&bytes[..2332]);
        bytes[2332..].copy_from_slice(&crc.to_le_bytes());
    }
    bytes
}

fn signal(coding: u8, phase: usize) -> Vec<i16> {
    let format = Format::from_coding(coding).unwrap();
    (0..format.frames_per_sector())
        .flat_map(|i| {
            (0..format.channels()).map(move |channel| {
                let wave = ((i + phase) as f64 * if channel == 0 { 0.041 } else { 0.081 }).sin();
                (wave * if channel == 0 { 27000.0 } else { -15000.0 }) as i16
            })
        })
        .collect()
}

#[test]
fn all_formats_and_arithmetic_preserve_layout_and_report_actual_channel_loss() {
    for arithmetic in [Arithmetic::SplitFloor, Arithmetic::CombinedRounded] {
        for coding in [0, 1, 4, 5, 16, 17, 20, 21] {
            let source = template(coding, true);
            let pcm = signal(coding, 17);
            let mut encoder =
                Encoder::new(stream(coding), arithmetic, Histories::default()).unwrap();
            let output = encoder.encode_sector(&source, &pcm).unwrap();
            let mut decoder =
                Decoder::new(stream(coding), arithmetic, Histories::default()).unwrap();
            let actual = decoder.decode_sector(&output.bytes).unwrap();
            assert_eq!(encoder.histories(), decoder.histories());
            assert!(!output.report.reused_original);
            assert_eq!(&source[..8], &output.bytes[..8]);
            assert_eq!(&source[2312..2332], &output.bytes[2312..2332]);
            assert!(matches!(
                check_form2(&output.bytes).unwrap(),
                Status::Valid(_)
            ));
            assert_ne!(&source[2332..], &output.bytes[2332..]);
            let channels = usize::from(decoder.format().channels());
            for channel in 0..channels {
                let errors: Vec<_> = pcm[channel..]
                    .iter()
                    .step_by(channels)
                    .zip(actual[channel..].iter().step_by(channels))
                    .map(|(&a, &b)| i32::from(b) - i32::from(a))
                    .collect();
                let loss = &output.report.channel_loss[channel];
                assert_eq!(loss.frames, decoder.format().frames_per_sector());
                assert_eq!(
                    loss.peak_absolute_error,
                    errors.iter().map(|e| e.unsigned_abs()).max().unwrap()
                );
                let rms = (errors.iter().map(|&e| f64::from(e).powi(2)).sum::<f64>()
                    / errors.len() as f64)
                    .sqrt();
                assert_eq!(loss.rms_error, rms);
                assert!(rms < 150.0, "coding {coding:x}: RMS {rms}");
            }
            for group in 0..18 {
                let at = 8 + group * 128;
                assert_eq!(&output.bytes[at..at + 4], &output.bytes[at + 4..at + 8]);
                if coding & 16 != 0 {
                    assert_eq!(&source[at + 8..at + 16], &output.bytes[at + 8..at + 16]);
                } else {
                    assert_eq!(
                        &output.bytes[at + 8..at + 12],
                        &output.bytes[at + 12..at + 16]
                    );
                }
            }
        }
    }
}

#[test]
fn unchanged_decoded_content_reuses_every_original_byte_and_keeps_history() {
    for coding in [0, 1, 16, 17] {
        let source = template(coding, false);
        let history = Histories {
            left: History {
                previous: -25000,
                previous_previous: 1234,
            },
            right: History {
                previous: 32000,
                previous_previous: -4000,
            },
        };
        let mut decoder = Decoder::new(stream(coding), Arithmetic::SplitFloor, history).unwrap();
        let pcm = decoder.decode_sector(&source).unwrap();
        let mut encoder = Encoder::new(stream(coding), Arithmetic::SplitFloor, history).unwrap();
        let output = encoder.encode_sector(&source, &pcm).unwrap();
        assert_eq!(output.bytes, source);
        assert!(output.report.reused_original);
        assert_eq!(encoder.histories(), decoder.histories());
        assert!(output
            .report
            .channel_loss
            .iter()
            .all(|l| l.peak_absolute_error == 0));
        assert_eq!(output.report.output_edc, Status::Absent);
    }
}

#[test]
fn history_crosses_sector_and_eof_boundaries_until_explicit_reset() {
    let coding = 1;
    let mut encoder = Encoder::new(
        stream(coding),
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .unwrap();
    let mut decoder = Decoder::new(
        stream(coding),
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .unwrap();
    for phase in [0, 7, 81] {
        let before = encoder.histories();
        let output = encoder
            .encode_sector(&template(coding, false), &signal(coding, phase))
            .unwrap();
        assert_eq!(output.report.initial_histories, before);
        decoder.decode_sector(&output.bytes).unwrap();
        assert_eq!(encoder.histories(), decoder.histories());
        assert_eq!(&output.bytes[2332..], &[0; 4]);
    }
    assert_ne!(encoder.histories(), Histories::default());
    encoder.reset(Histories::default());
    let a = encoder
        .encode_sector(&template(coding, false), &signal(coding, 0))
        .unwrap();
    let b = Encoder::new(
        stream(coding),
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .unwrap()
    .encode_sector(&template(coding, false), &signal(coding, 0))
    .unwrap();
    assert_eq!(a.bytes, b.bytes);
}

#[test]
fn malformed_templates_capacity_and_format_changes_fail_without_advancing_history() {
    let initial = Histories {
        left: History {
            previous: 7,
            previous_previous: -9,
        },
        right: History::default(),
    };
    let mut encoder = Encoder::new(stream(0), Arithmetic::SplitFloor, initial).unwrap();
    let good = template(0, true);
    let pcm = signal(0, 0);
    for length in [0, pcm.len() - 1, pcm.len() + 1] {
        assert!(encoder.encode_sector(&good, &vec![0; length]).is_err());
        assert_eq!(encoder.histories(), initial);
    }
    for offset in [0, 4, 8, 2312, 2332] {
        let mut bad = good.clone();
        bad[offset] ^= 1;
        assert!(encoder.encode_sector(&bad, &pcm).is_err());
        assert_eq!(encoder.histories(), initial);
    }
    assert!(encoder.encode_sector(&good[..2335], &pcm).is_err());
    assert!(encoder.encode_sector(&template(1, false), &pcm).is_err());
    let mut bad = template(0, false);
    bad[8 + 17 * 128..24 + 17 * 128].fill(0x4c);
    assert!(encoder.encode_sector(&bad, &pcm).is_err());
    assert_eq!(encoder.histories(), initial);
    for coding in [0x40, 2, 8, 0x20, 0x80] {
        assert!(Encoder::new(stream(coding), Arithmetic::SplitFloor, initial).is_err());
    }
}

#[test]
#[ignore = "requires installed FFmpeg as independent development-only 4-bit XA decoder"]
fn independently_decoded_encoded_streams_match_selected_combined_rounding() {
    use std::{
        io::Write,
        process::{Command, Stdio},
    };
    for coding in [0, 1, 4, 5] {
        let mut encoder = Encoder::new(
            stream(coding),
            Arithmetic::CombinedRounded,
            Histories::default(),
        )
        .unwrap();
        let mut decoder = Decoder::new(
            stream(coding),
            Arithmetic::CombinedRounded,
            Histories::default(),
        )
        .unwrap();
        let mut input = Vec::new();
        let mut expected = Vec::new();
        for phase in [0, 73, 196] {
            let encoded = encoder
                .encode_sector(&template(coding, true), &signal(coding, phase))
                .unwrap();
            expected.extend(
                decoder
                    .decode_sector(&encoded.bytes)
                    .unwrap()
                    .into_iter()
                    .flat_map(i16::to_le_bytes),
            );
            input.extend([
                0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 2, 0, 2,
            ]);
            input.extend(encoded.bytes);
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
        let output = child.wait_with_output().unwrap();
        writer.join().unwrap().unwrap();
        assert!(
            output.status.success(),
            "{}",
            String::from_utf8_lossy(&output.stderr)
        );
        assert!(
            output.stdout == expected,
            "XA encoder consumer difference, coding {coding:x}"
        );
    }
}
