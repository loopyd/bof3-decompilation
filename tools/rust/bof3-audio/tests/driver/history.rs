use super::fixtures::{prepare, prepare_image};
use bof3_audio::{
    machine::{
        bank::Prepared,
        bus::{Bus, Width},
        cues, effects, output_clock,
    },
    Result,
};

fn trigger(p: &mut Prepared, table: &cues::Table, row: usize) -> Result<usize> {
    let mut count = 0;
    cues::dispatch(p, table, row, &mut |e| {
        if e.cpu.pc() == effects::KEY_ON {
            count += 1;
        }
        Ok(())
    })?;
    Ok(count)
}

fn advance(p: &mut Prepared, frames: u64) -> Result<()> {
    while p.execution.output().unwrap().frames() < frames {
        p.execution.call(0x801753dc, [0; 4], 100_000)?;
        p.execution.take_audio_frames()?;
    }
    Ok(())
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn original_status_poll_controls_lower_priority_retrigger_without_host_state_patch() -> Result<()> {
    let (_, original) = prepare("COMN_SE");
    let mut data = original.entry(1)?.to_vec();
    // Preserve the existing tones/voices; change only two arbitration nibbles
    // in an in-memory archive. No original media or guest status is patched.
    data[6] = (data[6] & 0xf0) | 9;
    data[10] = (data[10] & 0xf0) | 8;
    let edited = original.replace_entries(&[(1, &data)])?;
    let (mut p, archive) = prepare_image(edited);
    let table = cues::stage(&mut p, &archive, 1)?;
    p.execution.call(0x8015ce70, [0; 4], 100_000)?;
    p.execution
        .enable_output(output_clock::Model::PcsxReduxNtsc)?;
    assert_eq!(trigger(&mut p, &table, 0)?, 2);
    advance(&mut p, 1024)?;
    assert!(p.execution.bus.read(0x1f801d6c, Width::Half)? > 0);
    assert_eq!(cues::state(&p)?.voice_status, [0; 24]);
    let mut masks = Vec::new();
    cues::refresh(&mut p, &mut |e| {
        if e.cpu.pc() == 0x801682e0 {
            masks.push(e.cpu.register(4));
        }
        Ok(())
    })?;
    assert_eq!(masks, (0..24).map(|i| 1 << i).collect::<Vec<_>>());
    let state = cues::state(&p)?;
    assert_eq!(&state.voice_status[..22], &[0; 22]);
    assert_eq!(&state.voice_status[22..], &[2, 2]);
    assert_eq!(
        trigger(&mut p, &table, 0)?,
        2,
        "equal arbitration retriggers"
    );
    assert_eq!(
        trigger(&mut p, &table, 1)?,
        0,
        "lower arbitration is suppressed on active same voice"
    );
    let state = cues::state(&p)?;
    assert_eq!((state.previous_voice, state.previous_arbitration), (22, 10));
    assert_eq!((state.current_voice, state.current_arbitration), (22, 9));
    for voice in [22, 23] {
        effects::key_off(&mut p, voice, &mut |_| Ok(()))?;
    }
    advance(&mut p, 3072)?;
    assert_eq!(p.execution.bus.read(0x1f801d6c, Width::Half)?, 0);
    assert_eq!(
        cues::state(&p)?.voice_status[22],
        2,
        "release does not refresh game snapshot"
    );
    assert_eq!(
        trigger(&mut p, &table, 1)?,
        0,
        "stale active snapshot still suppresses"
    );
    cues::refresh(&mut p, &mut |_| Ok(()))?;
    assert_eq!(cues::state(&p)?.voice_status, [0; 24]);
    assert_eq!(
        trigger(&mut p, &table, 1)?,
        2,
        "zero refreshed status permits lower arbitration"
    );
    assert_eq!(cues::state(&p)?.previous_arbitration, 9);
    advance(&mut p, 4096)?;
    cues::refresh(&mut p, &mut |_| Ok(()))?;
    assert_eq!(cues::state(&p)?.voice_status[22], 2);
    assert_eq!(
        trigger(&mut p, &table, 2)?,
        2,
        "different first voice permits lower arbitration"
    );
    let state = cues::state(&p)?;
    assert_eq!((state.previous_voice, state.previous_arbitration), (18, 8));
    Ok(())
}
