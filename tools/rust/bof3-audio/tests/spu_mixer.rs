use bof3_audio::machine::{
    spu_mixer::{Inputs, Mixer, Sends},
    spu_transfer::Transfer,
    spu_voice_ports::Frame,
    spu_voices::Output,
};

fn voices(stereo: [i16; 2], mask: u32) -> Frame {
    Frame {
        voices: [Output {
            stereo,
            ..Output::default()
        }; 24],
        reverb_mask: mask,
        noise_level: None,
    }
}
fn half_gain() -> Mixer {
    let mut mixer = Mixer::default();
    mixer.write(0x180, 0x2000).unwrap();
    mixer.write(0x182, 0x2000).unwrap();
    mixer
}

#[test]
fn voice_sums_stay_wide_until_clipping_before_main_gain() {
    let mut mixer = half_gain();
    let sends = mixer.prepare(
        &voices([30000, -30000], 0x00ff_ffff),
        0xc000,
        Inputs::default(),
    );
    assert_eq!(sends.dry, [720000, -720000]);
    assert_eq!(sends.reverb, [32767, -32768]);
    assert_eq!(mixer.finish(sends, [0; 2]).unwrap(), [16383, -16384]);
    let sends = mixer.prepare(&voices([30000; 2], 1 << 23), 0xc000, Inputs::default());
    assert_eq!(sends.reverb, [30000; 2]);
}

#[test]
fn input_enable_and_reverb_routes_are_independent_of_voice_mute() {
    let mut mixer = half_gain();
    for offset in [0x1b0, 0x1b2] {
        mixer.write(offset, 0x7fff).unwrap();
    }
    mixer.write(0x1b4, 0x4000).unwrap();
    mixer.write(0x1b6, 0xc000).unwrap();
    let frame = voices([1234; 2], 0x00ff_ffff);
    let inputs = Inputs {
        cd: [-10000, 10000],
        external: [20000; 2],
    };
    let cd = mixer.prepare(&frame, 0x8005, inputs);
    assert_eq!(
        cd,
        Sends {
            dry: [-10000, 9999],
            reverb: [-10000, 9999]
        }
    );
    assert_eq!(mixer.finish(cd, [0; 2]).unwrap(), [-5000, 4999]);
    for control in [0x800c, 0x000c] {
        // Sends alone cannot enable either input.
        assert_eq!(
            mixer.prepare(&frame, control, inputs),
            Sends {
                dry: [0; 2],
                reverb: [0; 2]
            }
        );
    }
    let external = mixer.prepare(&frame, 0x800a, inputs);
    assert_eq!(
        external,
        Sends {
            dry: [10000, -10000],
            reverb: [10000, -10000]
        }
    );
    let both = mixer.prepare(&frame, 0x8003, inputs);
    assert_eq!(
        both,
        Sends {
            dry: [0, -1],
            reverb: [0; 2]
        }
    );
    // The mixer component also models CD routing with the voice engine disabled;
    // the interconnect still rejects that unimplemented execution transition.
    assert_eq!(mixer.prepare(&frame, 5, inputs), cd);
}

#[test]
fn signed_reverb_return_and_main_registers_preserve_distinct_scales() {
    let mut mixer = half_gain();
    mixer.write(0x184, 0x4000).unwrap();
    mixer.write(0x186, 0x4000).unwrap();
    let sends = Sends {
        dry: [1000, -1000],
        reverb: [0; 2],
    };
    assert_eq!(mixer.finish(sends, [20000, -20000]).unwrap(), [5500, -5500]);
    assert!(mixer.require_dry(0xc000).is_err()); // Disabled reverb can have a tail.
    mixer.write(0x184, 0).unwrap();
    mixer.write(0x186, 0).unwrap();
    assert!(mixer.require_dry(0xc080).is_err());
    mixer.require_dry(0xc000).unwrap();
    mixer.write(0x1b8, 0xe000).unwrap();
    assert_eq!(mixer.read(0x180).unwrap(), 0x2000);
    assert_eq!(mixer.read(0x1b8).unwrap(), 0xe000);
    assert_eq!(mixer.finish(sends, [0; 2]).unwrap(), [-250, -500]);
    assert!(mixer.write(0x180, 0x8000).is_err());
    assert_eq!(mixer.read(0x1b8).unwrap(), 0xe000);
    for offset in [0x181, 0x188, 0x1bc] {
        assert!(mixer.write(offset, 0).is_err());
        assert!(mixer.read(offset).is_err());
    }
    mixer.write(0x180, 0x4000).unwrap(); // Signed fifteen-bit -16384 -> -32768.
    assert_eq!(mixer.read(0x1b8).unwrap(), 0x8000);
    let error = mixer
        .finish(
            Sends {
                dry: [-32768, 0],
                reverb: [0; 2],
            },
            [0; 2],
        )
        .unwrap_err();
    assert!(error.to_string().contains("+32768 gain rail"));
}

#[test]
fn capture_preserves_four_independent_rings_and_half_status() {
    let mut transfer = Transfer::default();
    assert!(transfer.capture_frame([1; 2], [1; 2]).is_err());
    assert_eq!(transfer.capture_position(), 0);
    assert!(transfer.ram().iter().all(|&v| v == 0));
    transfer.write(12, 4).unwrap();
    for frame in 0..512i16 {
        transfer
            .capture_frame([frame, -frame], [1000 + frame, -1000 - frame])
            .unwrap();
        assert_eq!(transfer.capture_position(), ((frame as u16 + 1) * 2) % 1024);
        assert_eq!(transfer.status() & 0x800 != 0, (255..511).contains(&frame));
    }
    let sample = |transfer: &Transfer, base, frame| {
        i16::from_le_bytes(
            transfer.ram()[base + frame * 2..base + frame * 2 + 2]
                .try_into()
                .unwrap(),
        )
    };
    for frame in 0..512usize {
        for (base, expected) in [
            (0, frame as i16),
            (0x400, -(frame as i16)),
            (0x800, 1000 + frame as i16),
            (0xc00, -1000 - frame as i16),
        ] {
            assert_eq!(sample(&transfer, base, frame), expected);
        }
    }
    transfer
        .capture_frame([32767, -32768], [-12345, 23456])
        .unwrap();
    for (base, expected) in [(0, 32767), (0x400, -32768), (0x800, -12345), (0xc00, 23456)] {
        assert_eq!(sample(&transfer, base, 0), expected);
    }
    assert_eq!(sample(&transfer, 0, 1), 1);
    assert_eq!(transfer.capture_position(), 2);
    assert!(transfer.ram()[0x1000..].iter().all(|&v| v == 0));
    transfer.write(12, 0).unwrap();
    assert!(transfer.capture_frame([0; 2], [0; 2]).is_err());
    assert_eq!(sample(&transfer, 0, 0), 32767);
    assert_eq!(transfer.capture_position(), 2);
}
