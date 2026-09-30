use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    executable::Executable,
    profile::Profile,
};
use bof3_audio::{bank::Bank, sample::reference::resolve_us_pcm};

const HEADER: u32 = 0x8001_0000;
const SEQUENCE: u32 = 0x8003_0000;
const RETURN: u32 = 0x8000_1000;

fn executable() -> Executable {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    exe
}

fn call(ram: &mut Ram, entry: u32, arguments: &[u32], stop: u32) -> Cpu {
    let mut cpu = Cpu::new(entry);
    cpu.set_register(29, 0x801f_f000);
    cpu.set_register(31, RETURN);
    for (i, &value) in arguments.iter().enumerate() {
        if i < 4 {
            cpu.set_register(4 + i, value);
        } else {
            ram.write(0x801f_f010 + (i as u32 - 4) * 4, Width::Word, value)
                .unwrap();
        }
    }
    cpu.run_until(ram, stop, 20_000).unwrap();
    cpu
}

fn header(reference: i16, prefix: u16) -> Vec<u8> {
    let mut vh = vec![0; 0x820 + 512 + 512];
    vh[..4].copy_from_slice(b"pBAV");
    vh[4..8].copy_from_slice(&7u32.to_le_bytes());
    for (at, value) in [(0x12, 1u16), (0x14, 1), (0x16, 3)] {
        vh[at..at + 2].copy_from_slice(&value.to_le_bytes());
    }
    vh[0x18] = 127;
    vh[0x19] = 64;
    vh[0x20] = 1;
    vh[0x21] = 127;
    vh[0x24] = 64;
    for (at, value) in [
        (0x821, 0),
        (0x822, 127),
        (0x823, 64),
        (0x824, 60),
        (0x827, 127),
    ] {
        vh[at] = value;
    }
    vh[0x830..0x832].copy_from_slice(&0x000fu16.to_le_bytes());
    vh[0x832..0x834].copy_from_slice(&0x1fc0u16.to_le_bytes());
    vh[0x836..0x838].copy_from_slice(&reference.to_le_bytes());
    for (index, units) in [prefix, 2, 4, 6].into_iter().enumerate() {
        vh[0xa20 + index * 2..0xa22 + index * 2].copy_from_slice(&units.to_le_bytes());
    }
    vh
}

fn open(exe: &Executable, vh: &[u8], spu_base: u32) -> Ram {
    let mut ram = Ram::from_executable(exe);
    for (i, &value) in vh.iter().enumerate() {
        ram.write(HEADER + i as u32, Width::Byte, u32::from(value))
            .unwrap();
    }
    let cpu = call(&mut ram, 0x8017_3c50, &[HEADER, 0, spu_base], RETURN);
    assert_eq!(cpu.register(2), 0, "bank zero should open");
    assert_eq!(ram.read(0x8018_e100, Width::Word).unwrap(), HEADER + 0x20);
    assert_eq!(ram.read(0x8018_e1e8, Width::Word).unwrap(), HEADER + 0x820);
    assert_eq!(ram.read(0x8018_e7f8, Width::Byte).unwrap(), 2);
    ram
}

#[test]
fn qualified_sample_resolution_retains_raw_values_and_rejects_unavailable_media() {
    for (raw, selected, sample, slot, offset) in [
        (0, 0, 2, 0, 14),
        (1, 1, 1, 0, 12),
        (2, 2, 2, 0, 14),
        (3, 3, 3, 1, 12),
        (256, 0, 2, 0, 14),
        (257, 1, 1, 0, 12),
        (-256, 0, 2, 0, 14),
        (-255, 1, 1, 0, 12),
    ] {
        let r = resolve_us_pcm(raw, 3).unwrap();
        assert_eq!(
            (
                r.encoded_reference,
                r.selected_byte,
                r.sample_id,
                r.address_table_program,
                r.address_table_offset
            ),
            (raw, selected, sample, slot, offset)
        );
    }
    assert!(resolve_us_pcm(0, 1)
        .unwrap_err()
        .to_string()
        .contains("adjacent SPU media"));
    assert!(resolve_us_pcm(4, 3).is_err());
    assert!(resolve_us_pcm(1, 256).is_err());
    for noise in [255, 511, -1] {
        assert!(resolve_us_pcm(noise, 255)
            .unwrap_err()
            .to_string()
            .contains("noise"));
    }
}

#[test]
fn size_table_prefix_precedes_declared_sample_allocations() {
    let bank = Bank::parse(&header(0, 2)).unwrap();
    assert_eq!(bank.body_prefix_bytes, 16);
    assert_eq!(
        bank.samples
            .iter()
            .map(|s| (s.body_offset, s.encoded_bytes))
            .collect::<Vec<_>>(),
        [(16, 16), (32, 32), (64, 48)]
    );
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_loader_and_note_on_resolve_zero_and_truncated_sample_references() {
    let exe = executable();
    for prefix in [0, 2] {
        for reference in [
            0i16,
            1,
            2,
            3,
            4,
            256,
            257,
            -256,
            -255,
            0xa500u16 as i16,
            0xa503u16 as i16,
        ] {
            let mut ram = open(&exe, &header(reference, prefix), 0x1000);
            let starts = [
                0x1000 + u32::from(prefix) * 8,
                0x1010 + u32::from(prefix) * 8,
                0x1030 + u32::from(prefix) * 8,
                0x1060 + u32::from(prefix) * 8,
            ];
            for (index, &start) in starts.iter().enumerate() {
                let offset = (index / 2) * 16 + if index % 2 == 0 { 12 } else { 14 };
                assert_eq!(
                    ram.read(HEADER + 0x20 + offset as u32, Width::Half)
                        .unwrap(),
                    start / 8
                );
            }
            // Controlled post-transfer ready state; no DMA or SPU PCM is simulated.
            ram.write(0x8018_e7f8, Width::Byte, 1).unwrap();
            ram.write(0x8018_e264, Width::Byte, 24).unwrap();
            ram.write(0x8019_0308, Width::Word, SEQUENCE).unwrap();
            for at in [0x4e, 0x74, 0x76] {
                ram.write(SEQUENCE + at, Width::Half, 127).unwrap();
            }
            let cpu = call(&mut ram, 0x8017_102c, &[0, 0, 0, 60, 127, 64], RETURN);
            assert_eq!(cpu.register(2), 1);
            let byte = u32::from(reference as u8);
            let index = if byte == 0 { 1 } else { byte as usize - 1 };
            assert_eq!(ram.read(0x8018_e7f0, Width::Half).unwrap(), byte);
            assert_eq!(
                ram.read(0x8018_e80e, Width::Half).unwrap(),
                starts[index] / 8,
                "reference {reference}, prefix {prefix}"
            );
            if byte == 4 {
                assert!(
                    resolve_us_pcm(reference, 3).is_err(),
                    "end-of-bank address is not an allocated sample"
                );
            } else {
                let resolved = resolve_us_pcm(reference, 3).unwrap();
                let parsed = Bank::parse(&header(reference, prefix)).unwrap();
                let sample = &parsed.samples[usize::from(resolved.sample_id) - 1];
                assert_eq!(starts[index], 0x1000 + sample.body_offset as u32);
            }
        }
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_sample_byte_255_enters_noise_path_instead_of_pcm_pitch_path() {
    let exe = executable();
    for reference in [255i16, 511, -1] {
        let mut ram = open(&exe, &header(reference, 0), 0x1000);
        ram.write(0x8018_e7f8, Width::Byte, 1).unwrap();
        ram.write(0x8018_e264, Width::Byte, 24).unwrap();
        ram.write(0x8019_0308, Width::Word, SEQUENCE).unwrap();
        for at in [0x4e, 0x74, 0x76] {
            ram.write(SEQUENCE + at, Width::Half, 127).unwrap();
        }
        call(&mut ram, 0x8017_102c, &[0, 0, 0, 60, 127, 64], 0x8017_1c1c);
        assert_eq!(ram.read(0x8018_e7f0, Width::Half).unwrap(), 255);
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn original_bank_loader_agrees_with_all_corpus_sample_offsets_and_tone_blocks() {
    use bof3_audio::{archive::MediaImage, catalog::model::AssetData, catalog::model::Catalog};
    use std::path::PathBuf;
    let exe = executable();
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let catalog = Catalog::read(Some(&root), &[]).unwrap();
    let (mut banks, mut samples, mut nonzero_prefixes) = (0, 0, 0);
    for asset in &catalog.assets {
        let AssetData::Bank { metadata, .. } = &asset.data else {
            continue;
        };
        let MediaImage::Emi(image) = MediaImage::read(&root.join(&asset.source)).unwrap() else {
            unreachable!()
        };
        let vh = image.entry(asset.entry.unwrap()).unwrap();
        let mut ram = open(&exe, vh, 0);
        for program in &metadata.programs {
            assert_eq!(
                ram.read(
                    HEADER + 0x20 + u32::from(program.program) * 16 + 8,
                    Width::Word
                )
                .unwrap(),
                program.tone_block as u32
            );
        }
        for sample in &metadata.samples {
            let slot = usize::from(sample.sample_id - 1);
            let address =
                HEADER + 0x20 + (slot / 2 * 16 + if slot % 2 == 0 { 12 } else { 14 }) as u32;
            assert_eq!(
                ram.read(address, Width::Half).unwrap() * 8,
                sample.body_offset as u32,
                "{} sample {}",
                asset.id,
                sample.sample_id
            );
            samples += 1;
        }
        let extent = metadata
            .samples
            .last()
            .map_or(metadata.body_prefix_bytes, |s| {
                s.body_offset + s.encoded_bytes
            });
        assert_eq!(
            ram.read(0x8019_0b90, Width::Word).unwrap(),
            extent as u32,
            "{}",
            asset.id
        );
        banks += 1;
        nonzero_prefixes += usize::from(metadata.body_prefix_bytes != 0);
    }
    assert_eq!((banks, samples), (1020, 8385));
    eprintln!("Original bank loader agrees with {banks} banks / {samples} sample starts; {nonzero_prefixes} nonzero size-table prefixes");
}
