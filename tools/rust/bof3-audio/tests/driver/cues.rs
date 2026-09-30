use super::fixtures::prepare;
use bof3_audio::{
    machine::{
        bus::{Bus, Width},
        cues, effects,
        executable::ram_offset,
        output_clock,
    },
    Result,
};

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn game_cues_preserve_ordered_layers_program_selection_and_silent_samples() -> Result<()> {
    let cases: [(&str, usize, &[[u32; 8]], bool); 3] = [
        (
            "COMN_SE",
            0,
            &[[23, 1, 0, 1, 24, 0, 20, 20], [22, 1, 0, 0, 24, 0, 25, 25]],
            true,
        ),
        (
            "COMN_SE",
            8,
            &[[23, 1, 1, 1, 24, 0, 13, 66], [22, 1, 1, 0, 24, 0, 67, 12]],
            false,
        ),
        (
            "BATL_SE",
            2,
            &[
                [18, 1, 0, 6, 28, 10, 0, 89],
                [17, 1, 0, 6, 28, 10, 0, 89],
                [16, 1, 0, 4, 28, 10, 90, 0],
            ],
            true,
        ),
    ];
    for (name, row, expected, audible) in cases {
        let (mut p, archive) = prepare(name);
        let table = cues::stage(&mut p, &archive, 1)?;
        assert_eq!(table.address, 0x8014871c);
        assert_eq!(table.capacity, 124);
        p.execution.call(0x8015ce70, [0; 4], 100_000)?;
        p.execution
            .enable_output(output_clock::Model::PcsxReduxNtsc)?;
        let mut notes = Vec::new();
        let mut volumes = Vec::new();
        cues::dispatch(&mut p, &table, row, &mut |e| {
            if e.cpu.pc() == effects::KEY_ON {
                let at = ram_offset(e.cpu.register(29), 32)?;
                notes.push(std::array::from_fn::<_, 8, _>(|i| {
                    if i < 4 {
                        e.cpu.register(i + 4)
                    } else {
                        let off = at + i * 4;
                        u32::from_le_bytes(e.bus.ram().bytes()[off..off + 4].try_into().unwrap())
                    }
                }));
            }
            if e.cpu.pc() == 0x8016f8f8 {
                volumes.push(std::array::from_fn::<_, 3, _>(|i| e.cpu.register(i + 4)));
            }
            Ok(())
        })?;
        assert_eq!(notes, expected, "{name} row {row}");
        assert_eq!(
            volumes,
            expected
                .iter()
                .map(|n| [n[0], 6143, 6143])
                .collect::<Vec<_>>()
        );
        let mut pcm = Vec::new();
        while p.execution.output().unwrap().frames() < 1024 {
            p.execution.call(0x801753dc, [0; 4], 100_000)?;
            pcm.extend(p.execution.take_audio_frames()?);
        }
        assert_eq!(
            pcm.iter().flatten().any(|&v| v != 0),
            audible,
            "{name} row {row}"
        );
    }
    Ok(())
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn cue_identity_capacity_and_record_failures_do_not_execute_guest_code() -> Result<()> {
    let (mut p, archive) = prepare("COMN_SE");
    let original = p.execution.bus.ram().bytes().to_vec();
    let instructions = p.execution.cpu.instructions();
    assert!(cues::stage(&mut p, &archive, 0)
        .unwrap_err()
        .to_string()
        .contains("type 8"));
    assert!(cues::stage(&mut p, &archive, 99).is_err());
    let changed = archive.replace_entries(&[(1, &[0; 128])])?;
    assert!(cues::stage(&mut p, &changed, 1)
        .unwrap_err()
        .to_string()
        .contains("differs"));
    assert_eq!(p.execution.bus.ram().bytes(), original);
    // The loader boundary comes from the executed layout table. A smaller
    // boundary must reject staging before any auxiliary byte is written.
    let pointer = 0x8014677c + 2 * 20 + 8;
    let old = p.execution.bus.read(pointer, Width::Word)?;
    p.execution
        .bus
        .write(pointer, Width::Word, p.identity.slot.auxiliary_address + 4)?;
    let before = p.execution.bus.ram().bytes().to_vec();
    assert!(cues::stage(&mut p, &archive, 1)
        .unwrap_err()
        .to_string()
        .contains("capacity"));
    assert_eq!(p.execution.bus.ram().bytes(), before);
    p.execution.bus.write(pointer, Width::Word, old)?;
    let mut table = cues::stage(&mut p, &archive, 1)?;
    let before = p.execution.bus.ram().bytes().to_vec();
    let pc = p.execution.cpu.pc();
    for row in [table.records.len(), 256] {
        assert!(
            cues::dispatch(&mut p, &table, row, &mut |_| panic!("invalid row executed")).is_err()
        );
    }
    let saved = table.records[0];
    for (record, diagnostic) in [
        ([2, 128, 10, 54], "cross-bank"),
        ([0, 128, 10, 255], "flags"),
        ([0, 128, 10, 63], "voice"),
        ([0, 127, 10, 54], "empty"),
        ([0, 128, 250, 54], "layered"),
        ([0, 128, 26, 54], "changed"),
    ] {
        table.records[0] = record;
        let error = cues::dispatch(&mut p, &table, 0, &mut |_| {
            panic!("invalid record executed")
        })
        .unwrap_err();
        assert!(error.to_string().contains(diagnostic), "{error}");
    }
    table.records[0] = saved;
    table.archive_sha256.clear();
    assert!(cues::dispatch(&mut p, &table, 0, &mut |_| panic!(
        "foreign identity executed"
    ))
    .is_err());
    assert_eq!(p.execution.bus.ram().bytes(), before);
    assert_eq!(p.execution.cpu.instructions(), instructions);
    assert_eq!(p.execution.cpu.pc(), pc);
    Ok(())
}
