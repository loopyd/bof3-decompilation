use bof3_audio::{
    soundfont::reader::Font, soundfont::reader::Limits, soundfont::Bank, soundfont::Instrument,
    soundfont::LoopMode, soundfont::Preset, soundfont::Sample, soundfont::Zone,
};
use std::io::Cursor;

fn bank() -> Bank {
    let sample = Sample {
        name: "left source".into(),
        pcm: (0..64)
            .map(|i| {
                if i == 0 {
                    i16::MIN
                } else if i == 1 {
                    i16::MAX
                } else {
                    i * 201 - 6000
                }
            })
            .collect(),
        rate: 44100,
        root_key: 60,
        correction_cents: -4,
        loop_range: Some(8..56),
    };
    let mut right = sample.clone();
    right.name = "right source".into();
    right.pcm.reverse();
    let mut left_zone = Zone::new(0);
    left_zone.loop_mode = LoopMode::Continuous;
    left_zone.pan = -500;
    let mut right_zone = left_zone.clone();
    right_zone.sample = 1;
    right_zone.pan = 500;
    Bank {
        name: "reader fixture".into(),
        samples: vec![sample, right],
        instruments: vec![Instrument {
            name: "stereo candidate".into(),
            zones: vec![left_zone, right_zone],
        }],
        presets: vec![Preset {
            name: "preset".into(),
            bank: 0,
            program: 5,
            instruments: vec![0],
        }],
    }
}
fn range(bytes: &[u8], id: &[u8; 4]) -> std::ops::Range<usize> {
    Font::from_bytes(bytes.to_vec())
        .unwrap()
        .chunks
        .iter()
        .find(|c| &c.id == id)
        .unwrap()
        .data
        .clone()
}
fn chunk(id: &[u8; 4], payload: &[u8]) -> Vec<u8> {
    let mut out = id.to_vec();
    out.extend((payload.len() as u32).to_le_bytes());
    out.extend(payload);
    if !payload.len().is_multiple_of(2) {
        out.push(0xab);
    }
    out
}
fn parts(bytes: &[u8]) -> Vec<([u8; 4], Vec<u8>)> {
    let mut at = 0;
    let mut result = Vec::new();
    while at < bytes.len() {
        let n = u32::from_le_bytes(bytes[at + 4..at + 8].try_into().unwrap()) as usize;
        result.push((
            bytes[at..at + 4].try_into().unwrap(),
            bytes[at + 8..at + 8 + n].to_vec(),
        ));
        at += 8 + n + n % 2;
    }
    result
}
fn edit(bytes: &[u8], scope: &[u8; 4], f: impl FnOnce(&mut Vec<([u8; 4], Vec<u8>)>)) -> Vec<u8> {
    let mut lists = parts(&bytes[12..]);
    let list = lists
        .iter_mut()
        .find(|(id, data)| id == b"LIST" && &data[..4] == scope)
        .unwrap();
    let mut children = parts(&list.1[4..]);
    f(&mut children);
    list.1 = scope.to_vec();
    for (id, data) in children {
        list.1.extend(chunk(&id, &data));
    }
    let mut body = b"sfbk".to_vec();
    for (id, data) in lists {
        body.extend(chunk(&id, &data));
    }
    chunk(b"RIFF", &body)
}

#[test]
fn parsed_writer_output_retains_raw_tables_layers_and_exact_pcm() {
    let bank = bank();
    let bytes = bank.encode().unwrap().bytes;
    let font = Font::from_bytes(bytes.clone()).unwrap();
    assert_eq!(font.original_bytes(), bytes);
    assert_eq!(font.version, (2, 1));
    assert_eq!(font.sample_bits, 16);
    assert_eq!(font.presets.len(), 1);
    assert_eq!((font.presets[0].bank, font.presets[0].program), (0, 5));
    assert_eq!(font.presets[0].zones[0].target, Some(0));
    assert_eq!(font.instruments[0].zones.len(), 2);
    for (index, sample) in bank.samples.iter().enumerate() {
        assert_eq!(
            font.sample_pcm24(index).unwrap(),
            sample
                .pcm
                .iter()
                .map(|p| i32::from(*p) * 256)
                .collect::<Vec<_>>()
        );
        assert_eq!(
            font.samples[index].loop_start - font.samples[index].start,
            8
        );
        assert_eq!(font.samples[index].loop_end - font.samples[index].start, 56);
        assert_eq!(font.instruments[0].zones[index].target, Some(index));
    }
    assert!(font.diagnostics.is_empty());
    assert!(font.sample_pcm24(2).is_err());
    let consumer = rustysynth::SoundFont::new(&mut Cursor::new(&bytes)).unwrap();
    assert_eq!(consumer.get_sample_headers().len(), font.samples.len());
}

#[test]
fn unknown_chunks_padding_terminal_payloads_and_duplicate_names_survive() {
    let original = bank().encode().unwrap().bytes;
    let mut bytes = edit(&original, b"INFO", |chunks| {
        chunks.push((*b"Xtra", vec![1, 2, 3]))
    });
    let headers = range(&bytes, b"shdr");
    bytes.copy_within(headers.start..headers.start + 20, headers.start + 46);
    // Terminal fields are ignored by SoundFont traversal but remain opaque bytes.
    let terminal = headers.end - 46;
    bytes[terminal..terminal + 20].fill(0xff);
    bytes[terminal + 20..headers.end].fill(0x71);
    let gen = range(&bytes, b"igen");
    bytes[gen.end - 4..gen.end].copy_from_slice(&[0x34, 0x12, 0x78, 0x56]);
    let pcm = range(&bytes, b"smpl");
    bytes[pcm.start + 64 * 2] = 1; // Nonzero first guard point is retained and reported.
    let font = Font::from_bytes(bytes.clone()).unwrap();
    assert_eq!(font.original_bytes(), bytes);
    let unknown = font.chunks.iter().find(|c| c.id == *b"Xtra").unwrap();
    assert_eq!(&bytes[unknown.data.clone()], [1, 2, 3]);
    assert_eq!(bytes[unknown.data.end], 0xab);
    assert_eq!(font.samples[0].name, font.samples[1].name);
    assert!(font.diagnostics.iter().any(|s| s.contains("sample guard")));
}

#[test]
fn twenty_four_bit_samples_retain_low_bytes_and_signed_extremes() {
    let original = bank().encode().unwrap().bytes;
    let points = range(&original, b"smpl").len() / 2;
    let mut bytes = edit(&original, b"INFO", |chunks| {
        chunks.iter_mut().find(|(id, _)| id == b"ifil").unwrap().1 = vec![2, 0, 4, 0]
    });
    bytes = edit(&bytes, b"sdta", |chunks| {
        let mut low = vec![0; points + points % 2];
        low[0] = 0x7f;
        low[1] = 0xff;
        low[2] = 1;
        chunks.push((*b"sm24", low));
    });
    let font = Font::from_bytes(bytes.clone()).unwrap();
    assert_eq!(font.sample_bits, 24);
    assert_eq!(
        &font.sample_pcm24(0).unwrap()[..3],
        [-8388481, 8388607, (2 * 201 - 6000) * 256 + 1]
    );
    assert_eq!(font.original_bytes(), bytes);
    let invalid = edit(&bytes, b"sdta", |chunks| {
        chunks
            .iter_mut()
            .find(|(id, _)| id == b"sm24")
            .unwrap()
            .1
            .pop()
            .map(|_| ())
            .unwrap()
    });
    assert!(Font::from_bytes(invalid)
        .unwrap_err()
        .to_string()
        .contains("sm24"));
    let old_version = edit(&bytes, b"INFO", |chunks| {
        chunks.iter_mut().find(|(id, _)| id == b"ifil").unwrap().1 = vec![2, 0, 1, 0]
    });
    assert!(Font::from_bytes(old_version)
        .unwrap_err()
        .to_string()
        .contains("sm24"));
}

#[test]
fn first_global_zone_and_custom_modulators_are_retained_without_synthesis_claims() {
    let original = bank().encode().unwrap().bytes;
    let bytes = edit(&original, b"pdta", |tables| {
        let get = |id: &[u8; 4]| tables.iter().position(|(key, _)| key == id).unwrap();
        let phdr = get(b"phdr");
        let pbag = get(b"pbag");
        let pgen = get(b"pgen");
        let pmod = get(b"pmod");
        // Preset global attenuation and one custom modulator, then local link.
        tables[phdr].1[38 + 24..38 + 26].copy_from_slice(&2u16.to_le_bytes());
        tables[pbag].1 = vec![0, 0, 0, 0, 1, 0, 1, 0, 2, 0, 1, 0];
        tables[pgen].1 = vec![48, 0, 30, 0, 41, 0, 0, 0, 0, 0, 0, 0];
        tables[pmod].1 = vec![
            2, 0, 48, 0, 0x9c, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        ];
    });
    let font = Font::from_bytes(bytes.clone()).unwrap();
    let zones = &font.presets[0].zones;
    assert_eq!(zones.len(), 2);
    assert_eq!(zones[0].target, None);
    assert_eq!(zones[1].target, Some(0));
    assert_eq!(zones[0].generators[0].operator, 48);
    assert_eq!(zones[0].modulators[0].amount, -100);
    assert_eq!(font.original_bytes(), bytes);
    rustysynth::SoundFont::new(&mut Cursor::new(&bytes)).unwrap();
    let unknown = edit(&bytes, b"pdta", |tables| {
        tables.iter_mut().find(|(id, _)| id == b"pgen").unwrap().1[..2]
            .copy_from_slice(&65000u16.to_le_bytes())
    });
    assert_eq!(
        Font::from_bytes(unknown).unwrap().presets[0].zones[0].generators[0].operator,
        65000
    );
}

#[test]
fn stereo_links_are_validated_and_rom_pcm_is_explicitly_unavailable() {
    let mut bytes = bank().encode().unwrap().bytes;
    let r = range(&bytes, b"shdr");
    bytes[r.start + 42..r.start + 46].copy_from_slice(&[1, 0, 4, 0]);
    bytes[r.start + 46 + 42..r.start + 46 + 46].copy_from_slice(&[0, 0, 2, 0]);
    let font = Font::from_bytes(bytes.clone()).unwrap();
    assert_eq!(font.samples[0].link, 1);
    assert_eq!(font.samples[1].kind, 2);
    rustysynth::SoundFont::new(&mut Cursor::new(&bytes)).unwrap();
    let mut bad = bytes.clone();
    bad[r.start + 46 + 42] = 1;
    assert!(Font::from_bytes(bad)
        .unwrap_err()
        .to_string()
        .contains("reciprocal"));
    let mut rom = edit(&bytes, b"INFO", |chunks| {
        chunks.push((*b"irom", b"unavailable ROM\0".to_vec()))
    });
    let r = range(&rom, b"shdr");
    rom[r.start + 45] = 0x80;
    rom[r.start + 46 + 45] = 0x80;
    let font = Font::from_bytes(rom).unwrap();
    assert!(font
        .sample_pcm24(0)
        .unwrap_err()
        .to_string()
        .contains("ROM"));
}

#[test]
fn malformed_framing_indices_links_and_resource_limits_fail_without_panics() {
    let original = bank().encode().unwrap().bytes;
    for end in 0..original.len() {
        assert!(Font::from_bytes(original[..end].to_vec()).is_err());
        if end >= 12 {
            let mut truncated = original[..end].to_vec();
            truncated[4..8].copy_from_slice(&((end - 8) as u32).to_le_bytes());
            assert!(Font::from_bytes(truncated).is_err());
        }
    }
    for offset in 0..original.len() {
        let mut changed = original.clone();
        changed[offset] ^= 0xff;
        assert!(
            std::panic::catch_unwind(|| Font::from_bytes(changed)).is_ok(),
            "parser panicked on byte {offset}"
        );
    }
    for (id, offset, value, reason) in [
        (*b"phdr", 24, 1u16, "header/bag"),
        (*b"phdr", 38 + 24, 65535, "header/bag"),
        (*b"pbag", 0, 1, "bag/generator"),
        (*b"pbag", 6, 1, "bag/modulator"),
        (*b"pgen", 2, 1, "link exceeds"),
        (*b"igen", 15 * 4 + 2, 2, "link exceeds"),
        (*b"igen", 0, 44, "duplicate generator"),
    ] {
        let mut bytes = original.clone();
        let r = range(&bytes, &id);
        bytes[r.start + offset..r.start + offset + 2].copy_from_slice(&value.to_le_bytes());
        let error = Font::from_bytes(bytes).unwrap_err().to_string();
        assert!(error.contains(reason), "{id:?}: {error}");
    }
    let duplicate = edit(&original, b"pdta", |tables| tables.push(tables[0].clone()));
    assert!(Font::from_bytes(duplicate)
        .unwrap_err()
        .to_string()
        .contains("duplicate"));
    let missing = edit(&original, b"pdta", |tables| {
        tables.remove(2);
    });
    assert!(Font::from_bytes(missing)
        .unwrap_err()
        .to_string()
        .contains("missing pmod"));
    let bad_len = edit(&original, b"pdta", |tables| {
        tables[0].1.pop();
    });
    assert!(Font::from_bytes(bad_len)
        .unwrap_err()
        .to_string()
        .contains("record framing"));
    for limits in [
        Limits {
            bytes: 10,
            ..Default::default()
        },
        Limits {
            chunks: 1,
            ..Default::default()
        },
        Limits {
            records: 2,
            ..Default::default()
        },
        Limits {
            sample_points: 1,
            ..Default::default()
        },
    ] {
        assert!(Font::with_limits(original.clone(), limits)
            .unwrap_err()
            .to_string()
            .contains("limit"));
    }
    let mut oversized = original.clone();
    oversized[16..20].copy_from_slice(&u32::MAX.to_le_bytes());
    assert!(Font::from_bytes(oversized)
        .unwrap_err()
        .to_string()
        .contains("exceeds parent"));
}
