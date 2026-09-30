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
fn all_seven_handlers_preserve_their_bank_identity_and_bounded_auxiliary_state() -> Result<()> {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let cases = [
        ("BIN/BGM/BGM004.EMI", 3, 2, 0, 0x8015e994),
        ("BIN/BATTLE/COMN_SE.EMI", 2, 1, 0, 0x8015efac),
        ("BIN/WORLD02/AREA102.EMI", 2, 1, 0, 0x8015f5c8),
        ("BIN/BPLCHAR/DRG08_00.EMI", 2, 1, 4, 0x8015fbe4),
        ("BIN/BPLCHAR/DRG08_02.EMI", 2, 1, 4, 0x80160200),
        ("BIN/BPLCHAR/DRG08_01.EMI", 2, 1, 4, 0x8016081c),
        ("BIN/BENEMY/ENEMY086.EMI", 2, 1, 0, 0x80160e38),
    ];
    for (slot, (path, body, aux, row, handler)) in cases.into_iter().enumerate() {
        let image = ArchiveImage::from_bytes(fs::read(root.join(path))?)?;
        let (mut p, archive) = prepare_entries(image, 0, body);
        let controls = p.execution.bus.read(0x80148a04, Width::Word)?;
        let table = cues::stage(&mut p, &archive, aux)?;
        assert_eq!(table.game_bank_id as usize, slot);
        assert_eq!(table.address, 0x801486a0 + slot as u32 * 124);
        assert_eq!(table.capacity, 124);
        assert_eq!(p.execution.bus.read(0x80148a04, Width::Word)?, controls);
        p.execution.call(0x8015ce70, [0; 4], 100_000)?;
        p.execution
            .enable_output(output_clock::Model::PcsxReduxNtsc)?;
        let mut visited = false;
        let mut notes = Vec::new();
        cues::dispatch(&mut p, &table, row, &mut |e| {
            visited |= e.cpu.pc() == handler;
            if e.cpu.pc() == effects::KEY_ON {
                notes.push((e.cpu.register(4), e.cpu.register(5), e.cpu.register(6)));
            }
            Ok(())
        })?;
        assert!(visited, "{path} must execute its original handler");
        assert_eq!(notes.len(), if slot == 2 { 1 } else { 2 });
        assert!(notes.iter().all(|n| n.1 == slot as u32));
        let mut pcm = Vec::new();
        while p.execution.output().unwrap().frames() < 1024 {
            p.execution.call(0x801753dc, [0; 4], 100_000)?;
            pcm.extend(p.execution.take_audio_frames()?);
        }
        assert!(pcm.iter().flatten().any(|&v| v != 0), "{path} row {row}");
    }
    Ok(())
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn original_cue_attributes_can_alias_another_program_without_rewriting_sdk_selection() -> Result<()>
{
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let image = ArchiveImage::from_bytes(fs::read(root.join("BIN/BENEMY/ENEMY086.EMI"))?)?;
    let (mut p, archive) = prepare_entries(image, 0, 2);
    let table = cues::stage(&mut p, &archive, 1)?;
    assert_eq!(p.bank.programs[3].tone_block, 3);
    assert_eq!(p.execution.bus.read(0x801821e6, Width::Half)?, 0);
    let mut notes = Vec::new();
    cues::dispatch(&mut p, &table, 6, &mut |e| {
        if e.cpu.pc() == effects::KEY_ON {
            let at = ram_offset(e.cpu.register(29), 32)?;
            notes.push(std::array::from_fn::<_, 8, _>(|i| {
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
        }
        Ok(())
    })?;
    assert_eq!(
        notes,
        [[19, 6, 3, 1, 24, 67, 20, 20], [18, 6, 3, 0, 24, 67, 60, 60]]
    );
    // Corrupting the original lookup beyond the staged header must fail before
    // any guest execution, rather than reading unrelated initialized RAM.
    p.execution.bus.write(0x801821e6, Width::Half, 0xffff)?;
    let before = p.execution.bus.ram().bytes().to_vec();
    let count = p.execution.cpu.instructions();
    assert!(cues::dispatch(&mut p, &table, 6, &mut |_| panic!(
        "unbounded lookup executed"
    ))
    .unwrap_err()
    .to_string()
    .contains("VH payload"));
    assert_eq!(p.execution.bus.ram().bytes(), before);
    assert_eq!(p.execution.cpu.instructions(), count);
    Ok(())
}
