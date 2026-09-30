use crate::fixture as bank_fixture;
use bof3_audio::{
    bank::Bank, codec::adpcm, soundfont::bank as binding, soundfont::bank::Gain,
    soundfont::packing::sample, soundfont::reader::Font,
};

fn fixture(silent: bool) -> (Vec<u8>, binding::Bound) {
    let (mut vh, mut vb) = bank_fixture::fixture();
    vb.extend(b"opaque body tail");
    if silent {
        vh[0x18] = 0;
    }
    let mut contexts = bank_fixture::contexts(&vh);
    if silent {
        let bank = Bank::parse(&vh).unwrap();
        for context in &mut contexts {
            let program = bank
                .programs
                .iter()
                .find(|p| p.program == context.program)
                .unwrap();
            context.gain = Gain::from_bank(
                &bank,
                program,
                &program.tones[context.tone],
                bof3_audio::soundfont::gain::Model::SpecificationScale,
            )
            .unwrap();
        }
    }
    let bound = binding::bind(
        bank_fixture::identity(),
        &vh,
        &vb,
        &contexts,
        &bank_fixture::options(),
    )
    .unwrap();
    (vb, bound)
}
fn offset(bytes: &[u8], id: &[u8; 4]) -> usize {
    Font::from_bytes(bytes.to_vec())
        .unwrap()
        .chunks
        .iter()
        .find(|c| &c.id == id)
        .unwrap()
        .data
        .start
}
fn pack(bound: &binding::Bound, body: &[u8], bytes: Vec<u8>) -> bof3_audio::Result<sample::Packed> {
    sample::pack(
        &bound.report,
        body,
        &Font::from_bytes(bound.bytes.clone())?,
        &Font::from_bytes(bytes)?,
    )
}
fn modes(bytes: &mut [u8], mode: u16, all: bool) {
    let font = Font::from_bytes(bytes.to_vec()).unwrap();
    let chunk = font.chunks.iter().find(|c| c.id == *b"igen").unwrap();
    let rows: Vec<_> = bytes[chunk.data.clone()]
        .as_chunks::<4>()
        .0
        .iter()
        .map(|row| {
            (
                u16::from_le_bytes([row[0], row[1]]),
                u16::from_le_bytes([row[2], row[3]]),
            )
        })
        .collect();
    for (i, row) in rows.iter().enumerate() {
        if row.0 != 54 {
            continue;
        }
        let sample = rows[i..].iter().find(|r| r.0 == 53).unwrap().1 as usize;
        if font.samples[sample].name.starts_with(b"Empty VAB programs") {
            continue;
        }
        let at = chunk.data.start + i * 4 + 2;
        bytes[at..at + 2].copy_from_slice(&mode.to_le_bytes());
        if !all {
            break;
        }
    }
}
fn pcm24(bytes: &[u8], values: &[(usize, u8)]) -> Vec<u8> {
    let font = Font::from_bytes(bytes.to_vec()).unwrap();
    let mut result = bytes.to_vec();
    let ifil = font.chunks.iter().find(|c| c.id == *b"ifil").unwrap();
    result[ifil.data.start + 2] = 4;
    let sdta = font
        .chunks
        .iter()
        .find(|c| c.id == *b"LIST" && &bytes[c.data.start..c.data.start + 4] == b"sdta")
        .unwrap();
    let points = font.pcm24_pool().len();
    let mut low = vec![0; points + points % 2];
    for &(i, v) in values {
        low[i] = v;
    }
    let mut chunk = b"sm24".to_vec();
    chunk.extend((low.len() as u32).to_le_bytes());
    chunk.extend(low);
    let old = u32::from_le_bytes(
        result[sdta.data.start - 4..sdta.data.start]
            .try_into()
            .unwrap(),
    );
    result[sdta.data.start - 4..sdta.data.start]
        .copy_from_slice(&(old + chunk.len() as u32).to_le_bytes());
    result.splice(sdta.data.end..sdta.data.end, chunk);
    let size = result.len() - 8;
    result[4..8].copy_from_slice(&(size as u32).to_le_bytes());
    result
}

#[test]
fn unchanged_pcm_and_lossless_twenty_four_bit_reencoding_reuse_all_original_bytes() {
    for silent in [false, true] {
        let (body, bound) = fixture(silent);
        for bytes in [bound.bytes.clone(), pcm24(&bound.bytes, &[])] {
            let same = bytes == bound.bytes;
            let packed = pack(&bound, &body, bytes).unwrap();
            assert_eq!(packed.body, body);
            assert_eq!(packed.report.soundfont_byte_equal, same);
            assert!(packed
                .report
                .samples
                .iter()
                .all(|s| !s.content_changed && !s.encoded_changed && s.encoding.is_none()));
        }
    }
}

#[test]
fn pcm_and_loop_edits_encode_once_for_all_layered_tones_and_preserve_body_tail() {
    let (body, bound) = fixture(false);
    let mut bytes = bound.bytes.clone();
    let pcm = offset(&bytes, b"smpl");
    for index in 0..112 {
        let value = ((index as f64 * 0.2).sin() * 10000.0) as i16;
        bytes[pcm + index * 2..pcm + index * 2 + 2].copy_from_slice(&value.to_le_bytes());
    }
    let header = offset(&bytes, b"shdr");
    bytes[header + 28..header + 32].copy_from_slice(&28u32.to_le_bytes());
    let input = Font::from_bytes(bytes.clone()).unwrap();
    let packed = pack(&bound, &body, bytes).unwrap();
    assert!(!packed.report.samples[0].content_changed);
    let report = &packed.report.samples[1];
    assert!(report.content_changed && report.encoded_changed);
    assert!(report.encoding.is_some());
    assert_eq!(&packed.body[64..], &body[64..]);
    let decoded = adpcm::decode_sample(&packed.body[..64]).unwrap();
    let sample_loop = decoded.sample_loop.unwrap();
    assert_eq!(
        (
            sample_loop.start_frame,
            sample_loop.end_frame_exclusive,
            sample_loop.pcm_repeat_is_stable
        ),
        (28, 112, true)
    );
    let peak = input
        .sample_pcm24(0)
        .unwrap()
        .iter()
        .zip(&decoded.pcm)
        .map(|(&a, &b)| (a - i32::from(b) * 256).unsigned_abs())
        .max()
        .unwrap();
    assert_eq!(
        report
            .pcm24_total_loss
            .as_ref()
            .unwrap()
            .peak_absolute_error,
        peak
    );
    assert_eq!(
        report
            .pcm24_rounding_loss
            .as_ref()
            .unwrap()
            .peak_absolute_error,
        0
    );
    assert_eq!(report.encoding.as_ref().unwrap().capacity_bytes, Some(64));
}

#[test]
fn removing_loop_requires_all_shared_zones_to_agree_and_release_only_fails() {
    let (body, bound) = fixture(false);
    let mut partial = bound.bytes.clone();
    modes(&mut partial, 0, false);
    assert!(pack(&bound, &body, partial)
        .err()
        .unwrap()
        .to_string()
        .contains("conflicting loop modes"));
    let mut all = bound.bytes.clone();
    modes(&mut all, 0, true);
    let packed = pack(&bound, &body, all).unwrap();
    assert!(adpcm::decode_sample(&packed.body[..64])
        .unwrap()
        .sample_loop
        .is_none());
    let mut release = bound.bytes.clone();
    modes(&mut release, 3, true);
    assert!(pack(&bound, &body, release)
        .err()
        .unwrap()
        .to_string()
        .contains("release-only"));
    for start in [1u32, 113] {
        let mut bytes = bound.bytes.clone();
        let h = offset(&bytes, b"shdr");
        bytes[h + 28..h + 32].copy_from_slice(&start.to_le_bytes());
        assert!(pack(&bound, &body, bytes).is_err());
    }
    let mut tail = bound.bytes.clone();
    let header = offset(&tail, b"shdr");
    tail[header + 32..header + 36].copy_from_slice(&84u32.to_le_bytes());
    assert!(pack(&bound, &body, tail)
        .err()
        .unwrap()
        .to_string()
        .contains("post-loop tails"));

    let (vh, mut body) = bank_fixture::fixture();
    body[1] = 0;
    body[49] = 1;
    let bound = binding::bind(
        bank_fixture::identity(),
        &vh,
        &body,
        &bank_fixture::contexts(&vh),
        &bank_fixture::options(),
    )
    .unwrap();
    let mut added = bound.bytes.clone();
    let header = offset(&added, b"shdr");
    added[header + 32..header + 36].copy_from_slice(&112u32.to_le_bytes());
    modes(&mut added, 1, true);
    let packed = pack(&bound, &body, added).unwrap();
    assert!(packed.report.samples[1].content_changed);
    let loop_ = adpcm::decode_sample(&packed.body)
        .unwrap()
        .sample_loop
        .unwrap();
    assert_eq!((loop_.start_frame, loop_.end_frame_exclusive), (0, 112));
}

#[test]
fn twenty_four_bit_rounding_loss_is_reported_even_when_encoded_bytes_are_reused() {
    let (body, bound) = fixture(false);
    let bytes = pcm24(&bound.bytes, &[(0, 1), (1, 127)]);
    let packed = pack(&bound, &body, bytes).unwrap();
    let report = &packed.report.samples[1];
    assert!(report.content_changed);
    assert!(!report.encoded_changed);
    assert_eq!(packed.body, body);
    assert!(report.encoding.is_none());
    assert_eq!(
        report
            .pcm24_rounding_loss
            .as_ref()
            .unwrap()
            .peak_absolute_error,
        127
    );
    assert_eq!(
        report
            .pcm24_total_loss
            .as_ref()
            .unwrap()
            .peak_absolute_error,
        127
    );
    let bytes = pcm24(&bound.bytes, &[(0, 128), (1, 255)]);
    let packed = pack(&bound, &body, bytes).unwrap();
    assert!(
        packed.report.samples[1]
            .pcm24_rounding_loss
            .as_ref()
            .unwrap()
            .peak_absolute_error
            <= 128
    );
}

#[test]
fn unsupported_metadata_geometry_guards_and_synthetic_silence_are_never_discarded() {
    let (body, bound) = fixture(false);
    for (id, at) in [
        (b"INAM", 0),
        (b"shdr", 24),
        (b"shdr", 36),
        (b"igen", 2 * 4 + 2),
    ] {
        let mut bytes = bound.bytes.clone();
        let start = offset(&bytes, id);
        bytes[start + at] ^= 1;
        assert!(pack(&bound, &body, bytes).is_err());
    }
    let mut guard = bound.bytes.clone();
    let pcm = offset(&guard, b"smpl");
    guard[pcm + 112 * 2] = 1;
    assert!(pack(&bound, &body, guard)
        .err()
        .unwrap()
        .to_string()
        .contains("guard or unowned"));
    let (body, bound) = fixture(true);
    let font = Font::from_bytes(bound.bytes.clone()).unwrap();
    let silence = bound.report.silence_sample.unwrap();
    let mut bytes = bound.bytes.clone();
    let pcm = offset(&bytes, b"smpl");
    bytes[pcm + font.samples[silence].start as usize * 2] = 1;
    assert!(pack(&bound, &body, bytes)
        .err()
        .unwrap()
        .to_string()
        .contains("generated silence"));

    let (body, mut bound) = fixture(false);
    // Retained odd opaque metadata must compare its RIFF padding as well.
    bound.bytes.extend(b"JUNK\x01\x00\x00\x00\x07\x99");
    let size = bound.bytes.len() as u32 - 8;
    bound.bytes[4..8].copy_from_slice(&size.to_le_bytes());
    assert_eq!(pack(&bound, &body, bound.bytes.clone()).unwrap().body, body);
    let mut changed_padding = bound.bytes.clone();
    *changed_padding.last_mut().unwrap() ^= 1;
    assert!(pack(&bound, &body, changed_padding).is_err());
}
