use super::fixtures::prepare_entries;
use bof3_audio::{
    machine::{
        bus::{Bus, Width},
        cues, effects,
        executable::ram_offset,
        output_clock,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{fs, path::PathBuf};

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn source_cue_needing_sample_outside_the_loaded_bank_is_rejected() -> Result<()> {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let archive = ArchiveImage::from_bytes(fs::read(root.join("BIN/BPLCHAR/DRG04_00.EMI"))?)?;
    let (mut p, archive) = prepare_entries(archive, 0, 2);
    let table = cues::stage(&mut p, &archive, 1)?;
    assert_eq!(p.bank.declared_samples, 2);
    // The original loader writes a terminal address after the two allocations.
    // Sample 3 resolves to that endpoint, not an unreported third sample.
    let header = p.identity.slot.header_address;
    let terminal = p.execution.bus.read(header + 0x20 + 16 + 12, Width::Half)? * 8;
    assert_eq!(
        terminal,
        p.identity.slot.spu_base + p.identity.body_bytes as u32
    );
    assert_eq!(terminal, 0x5ee10);
    let sizes = header + 0x820 + u32::from(p.bank.declared_programs) * 512;
    assert_eq!(p.execution.bus.read(sizes + 6, Width::Half)?, 0);
    let mut calls = Vec::new();
    let error = cues::dispatch(&mut p, &table, 5, &mut |e| {
        if e.cpu.pc() == effects::KEY_ON {
            calls.push([
                e.cpu.register(4),
                e.cpu.register(5),
                e.cpu.register(6),
                e.cpu.register(7),
            ]);
        }
        Ok(())
    })
    .unwrap_err();
    assert!(error.to_string().contains("sample reference 3"), "{error}");
    assert!(error.to_string().contains("bank's 2 samples"), "{error}");
    assert_eq!(calls, [[19, 3, 2, 1], [18, 3, 2, 0]]);
    assert_eq!(p.execution.cpu.pc(), 0x8016e5dc);
    Ok(())
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn physical_tone_slots_preserve_original_skips_and_undeclared_playable_tones() -> Result<()> {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let cases = [
        (
            "BIN/BMAGIC/MAGIC018.EMI",
            0,
            2,
            1,
            [[23, 1, 0, 4, 26, 0, 0, 0], [22, 1, 0, 3, 26, 0, 15, 15]],
            [u32::MAX, 22],
            true,
        ),
        (
            "BIN/BPLCHAR/BPLD015.EMI",
            8,
            10,
            4,
            [[19, 5, 1, 1, 24, 0, 30, 30], [18, 5, 1, 0, 24, 0, 50, 50]],
            [19, u32::MAX],
            true,
        ),
        (
            "BIN/WORLD02/AREA078.EMI",
            0,
            2,
            11,
            [[17, 2, 2, 7, 0, 0, 0, 0], [16, 2, 2, 6, 0, 0, 0, 0]],
            [u32::MAX, u32::MAX],
            false,
        ),
    ];
    for (path, header, body, row, expected, returns, audible) in cases {
        let archive = ArchiveImage::from_bytes(fs::read(root.join(path))?)?;
        let (mut p, archive) = prepare_entries(archive, header, body);
        let table = cues::stage(&mut p, &archive, header + 1)?;
        p.execution.call(0x8015ce70, [0; 4], 100_000)?;
        p.execution
            .enable_output(output_clock::Model::PcsxReduxNtsc)?;
        let mut calls = Vec::new();
        let mut results = Vec::new();
        let mut return_pc = None;
        cues::dispatch(&mut p, &table, row, &mut |e| {
            if e.cpu.pc() == effects::KEY_ON {
                let at = ram_offset(e.cpu.register(29), 32)?;
                calls.push(std::array::from_fn::<_, 8, _>(|i| {
                    if i < 4 {
                        e.cpu.register(i + 4)
                    } else {
                        u32::from_le_bytes(
                            e.bus.ram().bytes()[at + i * 4..at + i * 4 + 4]
                                .try_into()
                                .unwrap(),
                        )
                    }
                }));
                return_pc = Some(e.cpu.register(31));
            } else if Some(e.cpu.pc()) == return_pc {
                results.push(e.cpu.register(2));
                return_pc = None;
            }
            Ok(())
        })?;
        assert_eq!(calls, expected, "{path}");
        assert_eq!(results, returns, "{path}");
        let mut pcm = p.execution.take_audio_frames()?;
        while p.execution.output().unwrap().frames() < 1024 {
            p.execution.call(0x801753dc, [0; 4], 100_000)?;
            pcm.extend(p.execution.take_audio_frames()?);
        }
        assert_eq!(pcm.iter().flatten().any(|&v| v != 0), audible, "{path}");
    }
    Ok(())
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn original_sdk_reads_are_guarded_before_outside_tone_or_sample_access() -> Result<()> {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    for outside in [false, true] {
        let archive = ArchiveImage::from_bytes(fs::read(root.join("BIN/BMAGIC/MAGIC018.EMI"))?)?;
        let (mut p, archive) = prepare_entries(archive, 0, 2);
        let table = cues::stage(&mut p, &archive, 1)?;
        if outside {
            let end =
                p.identity.slot.header_address + 0x820 + u32::from(p.bank.declared_programs) * 512;
            // Change the original SDK's bank tone-base pointer, not host metadata.
            p.execution.bus.write(
                0x8018e1e8 + u32::from(p.identity.game_bank_id) * 4,
                Width::Word,
                end,
            )?;
        } else {
            let sample = p.identity.slot.header_address + 0x820 + 4 * 32 + 22;
            p.execution
                .bus
                .write(sample, Width::Half, u32::from(p.bank.declared_samples) + 1)?;
        }
        let before = p.execution.cpu.instructions();
        let error = cues::dispatch(&mut p, &table, 1, &mut |e| {
            assert_ne!(
                e.cpu.pc(),
                0x8016e5dc,
                "unsafe first tone load passed the guard"
            );
            Ok(())
        })
        .unwrap_err();
        assert!(
            error.to_string().contains(if outside {
                "tone read"
            } else {
                "sample reference"
            }),
            "{error}"
        );
        assert_eq!(p.execution.cpu.pc(), 0x8016e5dc);
        assert!(p.execution.cpu.instructions() > before);
    }
    Ok(())
}
