//! Original-instruction checks. Stopping at SDK/CD calls does not validate those calls.

use bof3_audio::{
    catalog::loader::initialize_layout, catalog::loader::resolve,
    catalog::loader::LAYOUT_INITIALIZE, catalog::loader::LOADER_TABLE, catalog::model::AssetData,
    catalog::model::Catalog, machine::bus::Bus, machine::bus::Ram, machine::bus::Width,
    machine::cpu::Cpu, machine::executable::Executable,
};
use std::{path::PathBuf, process::Command};

fn exe() -> Executable {
    Executable::from_bytes(
        std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE")).unwrap(),
    )
    .unwrap()
}

fn run(ram: &mut Ram, entry: u32, stop: u32, a0: u32, v1: u32) -> Cpu {
    let mut cpu = Cpu::new(entry);
    cpu.set_register(4, a0);
    cpu.set_register(3, v1);
    cpu.set_register(29, 0x801f_fff0);
    cpu.set_register(31, 0x8000_1000);
    cpu.run_until(ram, stop, 1000).unwrap();
    cpu
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_layout_initializer_populates_three_distinct_layouts_with_seven_slots() {
    let exe = exe();
    let expected_spu = [
        [4112, 254112, 324112, 0, 0, 0, 0],
        [4112, 254112, 336112, 373104, 404112, 435104, 466112],
        [4112, 444112, 0, 0, 0, 0, 0],
    ];
    for selector in 0..3 {
        let layout = initialize_layout(&exe, selector).unwrap();
        assert_eq!(layout.instructions, 199);
        assert_eq!(layout.slots.len(), 7);
        assert_eq!(layout.slots[0].header_address, 0x8011_a000);
        for (index, slot) in layout.slots.iter().enumerate() {
            assert_eq!(slot.vab_id as usize, index);
            assert_eq!(slot.spu_base, expected_spu[selector as usize][index]);
            assert_eq!(slot.flags, 0);
            assert_eq!(
                slot.sequence_address,
                slot.header_address + slot.header_capacity
            );
            if index + 1 < 7 {
                assert_eq!(
                    layout.slots[index + 1].header_address,
                    slot.sequence_address + slot.sequence_capacity
                );
            }
        }
    }
    assert!(initialize_layout(&exe, 3).is_err());
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_loader_argument_paths_use_toc_for_vh_then_selected_slot_for_payloads() {
    let exe = exe();
    for selector in 0..3 {
        let layout = initialize_layout(&exe, selector).unwrap();
        for slot in &layout.slots {
            let mut ram = Ram::from_executable(&exe);
            run(
                &mut ram,
                LAYOUT_INITIALIZE,
                0x8000_1000,
                u32::from(selector),
                0,
            );
            for index in 0..7 {
                ram.write(0x8014_8fc0 + index, Width::Byte, 0xff).unwrap();
            }
            // The staged slot table points directly at the original EMI header.
            ram.write(0x8014_6844, Width::Word, 0x8001_0000).unwrap();
            ram.write(0x8014_6848, Width::Word, 0x8001_0000).unwrap();
            ram.write(0x8001_0000, Width::Word, 1).unwrap();
            ram.write(0x8001_0010, Width::Word, 0x888).unwrap();
            ram.write(0x8001_0014, Width::Word, slot.index as u32)
                .unwrap();
            ram.write(0x8001_0018, Width::Word, 0x5641_4270).unwrap();
            ram.write(0x8001_001c, Width::Half, 6).unwrap();
            let cpu = run(&mut ram, 0x8016_2b08, 0x8000_1000, 1, 0);
            assert_eq!(cpu.register(2), 1);
            assert_eq!(
                ram.read(0x8014_6458, Width::Word).unwrap(),
                slot.index as u32
            );
            assert_eq!(ram.read(0x8014_6460, Width::Half).unwrap(), 6);
            // Stop at the CD copy call, before consuming any sectors.
            run(&mut ram, 0x8016_2790, 0x8016_2c14, 0, 0);
            assert_eq!(
                ram.read(0x8014_6483, Width::Byte).unwrap(),
                slot.vab_id as u32
            );
            assert_eq!(
                ram.read(0x8014_6458, Width::Word).unwrap(),
                slot.header_address
            );
            // The auxiliary and sequence handlers ignore their own TOC argument.
            for (entry, expected) in [
                (0x8016_29f0, slot.auxiliary_address),
                (0x8016_2a6c, slot.sequence_address),
            ] {
                ram.write(0x8014_646c, Width::Word, 0).unwrap();
                ram.write(0x8014_6458, Width::Word, 0xdead_beef).unwrap();
                run(&mut ram, entry, 0x8016_2c14, 0, 0);
                assert_eq!(ram.read(0x8014_6458, Width::Word).unwrap(), expected);
            }
            assert_eq!(
                ram.read(LOADER_TABLE + slot.index as u32 * 20 + 18, Width::Half)
                    .unwrap()
                    & 2,
                2
            );
            // Execute the original argument blocks, stopping at SDK call entry.
            let cpu = run(&mut ram, 0x8016_28f0, 0x8017_3c50, 0, 0);
            assert_eq!(
                [cpu.register(4), cpu.register(5), cpu.register(6)],
                [slot.header_address, slot.vab_id as u32, slot.spu_base]
            );
            let cpu = run(
                &mut ram,
                0x8016_35b8,
                0x8016_b38c,
                0,
                slot.index as u32 * 20,
            );
            assert_eq!(
                [cpu.register(4), cpu.register(5), cpu.register(6)],
                [slot.sequence_address, slot.vab_id as u32, 4]
            );
        }
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn local_corpus_loader_mapping_resolves_every_bank_without_using_header_ids() {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").expect("set BOF3_AUDIO_CORPUS"));
    let mut catalog = Catalog::read(Some(&root), &[]).unwrap();
    let mapping = resolve(&mut catalog, &exe()).unwrap();
    assert_eq!(mapping.associations.len(), 1020);
    assert!(mapping
        .associations
        .iter()
        .all(|a| a.body_entries.len() == 1));
    assert_eq!(
        mapping
            .associations
            .iter()
            .map(|a| a.songs.len())
            .sum::<usize>(),
        119
    );
    assert_eq!(
        mapping
            .associations
            .iter()
            .map(|a| a.auxiliary_entries.len())
            .sum::<usize>(),
        904
    );
    assert_eq!(
        mapping.unresolved.len(),
        3,
        "unexpected source association gaps: {:?}",
        mapping.unresolved
    );
    let mut banks = [0usize; 7];
    for asset in &catalog.assets {
        match &asset.data {
            AssetData::Bank {
                metadata, content, ..
            } => {
                assert_eq!(metadata.header_id, 0);
                assert!(
                    content.is_some(),
                    "{} must have a validated VH/VB identity",
                    asset.id
                );
                banks[asset.game_bank_id.unwrap() as usize] += 1;
            }
            AssetData::XaStream(_) | AssetData::XaCue(_) => assert!(asset.game_bank_id.is_none()),
            _ => assert!(asset.game_bank_id.is_some()),
        }
        if !matches!(
            asset.data,
            AssetData::Song { .. } | AssetData::Sequence { .. }
        ) {
            assert!(asset.game_song_ids.is_empty());
        }
    }
    assert_eq!(banks, [119, 210, 244, 63, 74, 70, 240]);
    // Counts independently established by a Python raw-TOC/hashlib scan.
    assert_eq!(mapping.shared_banks.len(), 99);
    assert_eq!(
        mapping
            .shared_banks
            .iter()
            .map(|group| group.banks.len())
            .sum::<usize>(),
        695
    );
    for group in &mapping.shared_banks {
        let mut reference = None;
        for id in &group.banks {
            let bank = catalog.select("audio", Some("bank"), id).unwrap();
            let AssetData::Bank {
                content: Some(content),
                ..
            } = &bank.data
            else {
                panic!("missing bank content")
            };
            let source = catalog
                .sources
                .iter()
                .find(|source| source.source == bank.source)
                .unwrap();
            let body = source
                .entries
                .iter()
                .find(|entry| entry.id == content.body_entry)
                .unwrap();
            let bof3_audio::archive::MediaImage::Emi(image) =
                bof3_audio::archive::MediaImage::read(&root.join(&bank.source)).unwrap()
            else {
                panic!("expected EMI")
            };
            let pair = (
                image.entry(bank.entry.unwrap()).unwrap(),
                image.entry(body.entry).unwrap(),
            );
            if let Some((vh, vb)) = &reference {
                assert_eq!(pair.0, vh, "header mismatch for {id}");
                assert_eq!(pair.1, vb, "body mismatch for {id}");
            } else {
                reference = Some((pair.0.to_vec(), pair.1.to_vec()));
            }
        }
    }
}

#[test]
fn map_requires_a_runtime_profile_and_rejects_inventory_only_options() {
    for args in [
        vec!["map", "--mode", "music", "--archive", "missing.EMI"],
        vec!["map", "--mode", "music", "--kind", "bank"],
    ] {
        let output = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(args)
            .output()
            .unwrap();
        assert_eq!(output.status.code(), Some(2));
        assert!(output.stdout.is_empty());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn mapping_uses_loader_order_and_leaves_orphan_payloads_unresolved() {
    let archive =
        PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").expect("set BOF3_AUDIO_CORPUS"))
            .join("BIN/BGM/BGM000.EMI");
    let mut catalog = Catalog::read(None, std::slice::from_ref(&archive)).unwrap();
    for entry in &mut catalog.sources[0].entries {
        if entry.file_type != 6 {
            entry.load_argument = 0xdead_beef;
        }
    }
    let mapping = resolve(&mut catalog, &exe()).unwrap();
    assert_eq!(mapping.associations.len(), 1);
    assert_eq!(mapping.associations[0].body_entries.len(), 1);
    assert_eq!(mapping.associations[0].songs.len(), 1);
    assert_eq!(mapping.unresolved.len(), 3);
    let mut orphan = Catalog::read(None, &[archive]).unwrap();
    orphan.sources[0]
        .entries
        .retain(|entry| entry.file_type != 6);
    let mapping = resolve(&mut orphan, &exe()).unwrap();
    assert!(mapping.associations.is_empty());
    assert_eq!(mapping.unresolved.len(), 5);
    assert!(orphan
        .assets
        .iter()
        .all(|asset| asset.game_bank_id.is_none()));
}
