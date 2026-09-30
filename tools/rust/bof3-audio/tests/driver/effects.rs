use super::fixtures::prepare;
use bof3_audio::{
    machine::{
        bus::{Bus, Width},
        effects::{self, Note},
        output_clock,
    },
    Result,
};

fn note(voice: u8, key: u8) -> Note {
    Note {
        voice,
        program: 0,
        tone: 0,
        key,
        fine: 0,
        left: 100,
        right: 100,
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn effects_banks_upload_nonzero_id_and_play_explicit_voices_without_sequences() -> Result<()> {
    for (name, voice, key, tone, audible) in [
        ("COMN_SE", 7, 24, 0, true),
        ("BATL_SE", 23, 41, 2, true),
        ("BATL_SE", 0, 36, 0, false),
    ] {
        let (mut p, _) = prepare(name);
        assert_eq!(p.identity.game_bank_id, 1);
        assert_eq!(p.identity.sequence_entry, None);
        let transfers: Vec<_> = p.calls.iter().filter(|c| c.entry == 0x80174354).collect();
        assert_eq!(transfers.last().unwrap().result, 1);
        assert!(transfers[..transfers.len() - 1]
            .iter()
            .all(|c| c.result == 0xffff_fffe));
        p.execution.call(0x8015ce70, [0; 4], 100_000)?;
        p.execution
            .enable_output(output_clock::Model::PcsxReduxNtsc)?;
        let sp = p.execution.cpu.register(29);
        let mut entry_arguments = None;
        let call = effects::key_on(
            &mut p,
            Note {
                tone,
                ..note(voice, key)
            },
            &mut |e| {
                if e.cpu.pc() == effects::KEY_ON {
                    let at = (e.cpu.register(29) & 0x1fff_ffff) as usize + 16;
                    entry_arguments = Some(std::array::from_fn::<_, 4, _>(|i| {
                        u32::from_le_bytes(
                            e.bus.ram().bytes()[at + i * 4..at + i * 4 + 4]
                                .try_into()
                                .unwrap(),
                        )
                    }));
                }
                Ok(())
            },
        )?;
        assert_eq!(entry_arguments, Some([key.into(), 0, 100, 100]));
        assert_eq!(call.result, u32::from(voice));
        assert_eq!(p.execution.cpu.register(29), sp);
        let e = &mut p.execution;
        let mut pcm = Vec::new();
        let mut keyed = 0;
        while e.output().unwrap().frames() < 1024 {
            e.call(0x801753dc, [0; 4], 100_000)?;
            pcm.extend(e.take_audio_frames()?);
            // Later flushes write zero: collect latches throughout playback.
            keyed |=
                e.bus.read(0x1f801d88, Width::Half)? | (e.bus.read(0x1f801d8a, Width::Half)? << 16);
        }
        assert_eq!(
            pcm.iter().flatten().any(|&v| v != 0),
            audible,
            "{name} tone {tone}"
        );
        assert_eq!(
            keyed,
            1 << voice,
            "only the explicitly selected voice keys on"
        );
        effects::key_off(&mut p, voice, &mut |_| Ok(()))?;
        let e = &mut p.execution;
        let end = e.output().unwrap().frames() + 8192;
        let mut tail = Vec::new();
        let mut released = 0;
        while e.output().unwrap().frames() < end {
            e.call(0x801753dc, [0; 4], 100_000)?;
            tail.extend(e.take_audio_frames()?);
            released |=
                e.bus.read(0x1f801d8c, Width::Half)? | (e.bus.read(0x1f801d8e, Width::Half)? << 16);
        }
        assert_eq!(released, 1 << voice, "only the selected voice is released");
        assert!(tail[tail.len() - 512..].iter().flatten().all(|&v| v == 0));
        assert_eq!(
            e.bus
                .read(0x1f801c0c + u32::from(voice) * 16, Width::Half)?,
            0
        );
    }
    Ok(())
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn invalid_effect_context_preserves_guest_and_observer_failure_restores_stack() {
    let (mut p, _) = prepare("COMN_SE");
    let original = p.execution.bus.ram().bytes().to_vec();
    let registers: Vec<_> = (0..32).map(|r| p.execution.cpu.register(r)).collect();
    let instructions = p.execution.cpu.instructions();
    assert!(effects::key_off(&mut p, 24, &mut |_| panic!("invalid release executed")).is_err());
    for invalid in [
        Note {
            voice: 24,
            ..note(0, 24)
        },
        Note {
            tone: 16,
            ..note(0, 24)
        },
        Note {
            program: 127,
            ..note(0, 24)
        },
        Note {
            key: 128,
            ..note(0, 24)
        },
        Note {
            fine: 128,
            ..note(0, 24)
        },
        Note {
            left: 128,
            ..note(0, 24)
        },
        Note {
            right: 128,
            ..note(0, 24)
        },
    ] {
        let result = effects::key_on(&mut p, invalid, &mut |_| panic!("invalid context executed"));
        assert!(result.is_err());
        assert_eq!(p.execution.bus.ram().bytes(), original);
        assert_eq!(
            (0..32)
                .map(|r| p.execution.cpu.register(r))
                .collect::<Vec<_>>(),
            registers
        );
        assert_eq!(p.execution.cpu.instructions(), instructions);
    }
    let error =
        effects::key_on(&mut p, note(0, 24), &mut |_| Err("observer failure".into())).unwrap_err();
    assert!(error.to_string().contains("observer failure"));
    assert_eq!(p.execution.cpu.register(29), registers[29]);
    assert_eq!(p.execution.cpu.instructions(), instructions);
}
