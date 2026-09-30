use bof3_audio::machine::{
    adsr,
    bus::{Bus, Width},
    executable::Executable,
    interconnect::Interconnect,
    spu_mixer::Inputs,
    spu_sample::Model,
};

fn half(bus: &mut Interconnect, address: u32, value: u16) {
    bus.write(address, Width::Half, u32::from(value)).unwrap();
}
fn fixture() -> Interconnect {
    let mut bytes = vec![0; 0x804];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    for (at, value) in [(0x10, 0x80010000u32), (0x18, 0x80010000), (0x1c, 4)] {
        bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
    }
    let mut bus = Interconnect::from_executable(&Executable::from_bytes(bytes).unwrap());
    bus.configure_spu_voices(Model::Published, adsr::Model::Published)
        .unwrap();
    half(&mut bus, 0x1f801da6, 0x200); // Keep sample data above capture buffers.
    half(&mut bus, 0x1f801dac, 4);
    half(&mut bus, 0x1f801daa, 0xc010);
    half(&mut bus, 0x1f801da8, 0x0500); // Loop-start + end/mute flags.
    for _ in 0..7 {
        half(&mut bus, 0x1f801da8, 0x1111);
    }
    assert_eq!(bus.service_spu(8).unwrap(), 8);
    for voice in 0..24 {
        let base = 0x1f801c00 + voice * 16;
        half(&mut bus, base, 0x2000);
        half(&mut bus, base + 2, 0x6000);
        half(&mut bus, base + 4, [0, 0x1000, 0x4000][voice as usize % 3]);
        half(&mut bus, base + 6, 0x200);
        half(&mut bus, base + 8, 0x7f00); // Frozen attack for explicit ENVX.
    }
    bus
}

#[test]
fn explicit_disable_model_preserves_inactive_state_and_rejects_atomically() {
    use bof3_audio::machine::spu_voice_ports::DisableModel;
    let mut bus = fixture();
    bus.write(0x1f801d88, Width::Word, 0x0055_5555).unwrap();
    bus.step_spu_voices().unwrap();
    for voice in 0..24 {
        half(&mut bus, 0x1f801c0c + voice * 16, 0x4000 + voice as u16);
    }
    half(&mut bus, 0x1f801d88, 2); // Pending key must survive the transition.
    let endx = bus.read(0x1f801d9c, Width::Word).unwrap();
    assert!(bus
        .write(0x1f801daa, Width::Half, 0x10)
        .unwrap_err()
        .to_string()
        .contains("evidence model"));
    assert_eq!(bus.read(0x1f801daa, Width::Half).unwrap(), 0xc010);
    bus.configure_spu_disable(DisableModel::EmulatorReference)
        .unwrap();
    assert!(bus
        .configure_spu_disable(DisableModel::EmulatorReference)
        .is_err());
    // Invalid IRQ configuration and a buffered FIFO stop must fail before any
    // envelope or control-register changes, even after model selection.
    assert!(bus.write(0x1f801daa, Width::Half, 0x50).is_err());
    half(&mut bus, 0x1f801da8, 0x1234);
    assert!(bus.write(0x1f801daa, Width::Half, 0).is_err());
    for voice in 0..24 {
        assert_eq!(
            bus.read(0x1f801c0c + voice * 16, Width::Half).unwrap(),
            0x4000 + voice
        );
    }
    assert_eq!(bus.read(0x1f801daa, Width::Half).unwrap(), 0xc010);
    bus.service_spu(1).unwrap();
    half(&mut bus, 0x1f801daa, 0x10);
    assert!(
        bus.step_spu_voices().is_err(),
        "disabled-frame scheduling is still unverified"
    );
    for voice in 0..24 {
        assert_eq!(
            bus.read(0x1f801c0c + voice * 16, Width::Half).unwrap(),
            if voice % 2 == 0 { 0 } else { 0x4000 + voice }
        );
        assert_eq!(
            bus.read(0x1f801c00 + voice * 16, Width::Word).unwrap(),
            0x6000_2000
        );
        assert_eq!(
            bus.read(0x1f801c06 + voice * 16, Width::Half).unwrap(),
            0x200
        );
    }
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), endx);
    // A second write with enable already clear has no force-off effect.
    half(&mut bus, 0x1f801c0c, 0x1234);
    half(&mut bus, 0x1f801daa, 0x10);
    assert_eq!(bus.read(0x1f801c0c, Width::Half).unwrap(), 0x1234);
    half(&mut bus, 0x1f801daa, 0xc010);
    let frame = bus.step_spu_voices().unwrap();
    assert_eq!(frame.voices[1].fetched_address, Some(0x1000));
    assert_eq!(
        bus.read(0x1f801c1c, Width::Half).unwrap(),
        0,
        "pending KON consumed after re-enable"
    );
}

#[test]
fn shared_noise_is_independent_of_pitch_and_ignores_adpcm_end_mute() {
    let mut bus = fixture();
    bus.write(0x1f801d94, Width::Word, 0xffff_ffff).unwrap();
    assert_eq!(bus.read(0x1f801d94, Width::Word).unwrap(), 0x00ff_ffff);
    bus.write(0x1f801d88, Width::Word, 0x00ff_ffff).unwrap();
    assert!(bus
        .step_spu_voices()
        .unwrap_err()
        .to_string()
        .contains("noise"));
    bus.configure_spu_noise(0x1234, 0x20000).unwrap();
    assert!(bus.configure_spu_noise(0, 0).is_err());
    let first = bus.step_spu_voices().unwrap();
    assert_eq!(first.noise_level, Some(0x1234));
    for (i, voice) in first.voices.iter().enumerate() {
        assert_eq!(voice.fetched_address, Some(0x1000)); // Failed precheck kept KON.
        assert_eq!(voice.mono, 0);
        half(&mut bus, 0x1f801c0c + i as u32 * 16, 0x4000);
    }
    for _ in 0..60 {
        let frame = bus.step_spu_voices().unwrap();
        assert_eq!(frame.noise_level, Some(0x1234));
        assert!(frame.voices.iter().all(|v| v.mono == 2330));
    }
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), 0x00db_6db6);
    for voice in 0..24 {
        assert_eq!(
            bus.read(0x1f801c0c + voice * 16, Width::Half).unwrap(),
            0x4000
        );
    }
}

#[test]
fn noise_clocks_once_per_frame_even_without_selected_voices_and_keys_do_not_reset_it() {
    let mut bus = fixture();
    half(&mut bus, 0x1f801daa, 0xff10); // Fastest noise clock.
    bus.configure_spu_noise(0, 0).unwrap();
    assert_eq!(bus.step_spu_voices().unwrap().noise_level, Some(0));
    bus.write(0x1f801d94, Width::Word, 0x00ff_ffff).unwrap();
    bus.write(0x1f801d88, Width::Word, 0x00ff_ffff).unwrap();
    assert_eq!(bus.step_spu_voices().unwrap().noise_level, Some(1));
    bus.write(0x1f801d94, Width::Word, 0).unwrap();
    half(&mut bus, 0x1f801d88, 1);
    assert_eq!(bus.step_spu_voices().unwrap().noise_level, Some(3));
    bus.write(0x1f801d94, Width::Word, 0x00ff_ffff).unwrap();
    assert_eq!(bus.step_spu_voices().unwrap().noise_level, Some(7));
}

#[test]
fn final_output_captures_before_pan_mute_and_cd_input_gain() {
    let mut bus = fixture();
    bus.configure_spu_noise(0x1234, 0x20000).unwrap();
    half(&mut bus, 0x1f801d94, 10);
    half(&mut bus, 0x1f801d88, 10);
    // Muted stereo volume on voice 3 must not mute its capture.
    bus.write(0x1f801c30, Width::Word, 0).unwrap();
    bus.write(0xbf801d80, Width::Word, 0x20002000).unwrap();
    assert_eq!(bus.read(0x9f801db8, Width::Word).unwrap(), 0x40004000);
    assert_eq!(bus.read(0x1f801d81, Width::Byte).unwrap(), 0x20);
    bus.write(0x1f801db0, Width::Word, 0x40004000).unwrap();
    let inputs = Inputs {
        cd: [12000, -12000],
        external: [0; 2],
    };
    assert_eq!(bus.step_spu_output(inputs).unwrap(), [0; 2]);
    half(&mut bus, 0x1f801c1c, 0x4000);
    half(&mut bus, 0x1f801c3c, 0x4000);
    assert_eq!(bus.step_spu_output(inputs).unwrap(), [582, -583]);
    half(&mut bus, 0x1f801daa, 0x8011); // Mute voices, enable CD.
    assert_eq!(bus.step_spu_output(inputs).unwrap(), [3000, -3000]);
    for (base, expected) in [(0, 12000i16), (0x400, -12000), (0x800, 2330), (0xc00, 2330)] {
        for frame in [1, 2] {
            assert_eq!(
                &bus.spu_transfer().ram()[base + frame * 2..base + frame * 2 + 2],
                &expected.to_le_bytes()
            );
        }
    }
    assert_eq!(bus.spu_transfer().capture_position(), 6);
    assert_eq!(bus.spu_transfer().ram()[0x1000], 0);
    assert_eq!(bus.spu_transfer().ram()[0x1001], 5);
}

#[test]
fn unsupported_reverb_and_capture_modes_reject_before_consuming_pending_keys() {
    let mut bus = fixture();
    half(&mut bus, 0x1f801d88, 2);
    half(&mut bus, 0x1f801daa, 0xc090);
    assert!(bus
        .step_spu_output(Inputs::default())
        .unwrap_err()
        .to_string()
        .contains("reverb"));
    half(&mut bus, 0x1f801daa, 0xc010);
    half(&mut bus, 0x1f801d84, 1);
    assert!(bus
        .step_spu_output(Inputs::default())
        .unwrap_err()
        .to_string()
        .contains("tail"));
    half(&mut bus, 0x1f801d84, 0);
    half(&mut bus, 0x1f801dac, 0);
    assert!(bus
        .step_spu_output(Inputs::default())
        .unwrap_err()
        .to_string()
        .contains("capture"));
    assert_eq!(bus.spu_transfer().capture_position(), 0);
    half(&mut bus, 0x1f801dac, 4);
    assert_eq!(
        bus.step_spu_voices().unwrap().voices[1].fetched_address,
        Some(0x1000)
    );
}

#[test]
fn voice_and_main_sweeps_use_old_gain_then_advance_without_key_reset() {
    let mut bus = fixture();
    bus.configure_spu_noise(0x1234, 0x20000).unwrap();
    half(&mut bus, 0x1f801d94, 2);
    half(&mut bus, 0x1f801d88, 2);
    bus.step_spu_voices().unwrap();
    half(&mut bus, 0x1f801c1c, 0x4000);
    half(&mut bus, 0x1f801c10, 0x8000); // Left increase from +16384.
    half(&mut bus, 0x1f801c12, 0x9000); // Right increase from -16384.
    bus.write(0x1f801d80, Width::Word, 0x20002000).unwrap();
    half(&mut bus, 0x1f801d80, 0x8000);
    assert_eq!(bus.step_spu_output(Inputs::default()).unwrap(), [582, -583]);
    assert_eq!(bus.read(0x1f801e04, Width::Word).unwrap(), 0x80007800);
    assert_eq!(bus.read(0x1f801db8, Width::Half).unwrap(), 30720);
    assert_eq!(
        bus.step_spu_output(Inputs::default()).unwrap(),
        [2047, -1165]
    );
    assert_eq!(bus.read(0x1f801e04, Width::Word).unwrap(), 0x80007fff);
    assert_eq!(
        bus.step_spu_output(Inputs::default()).unwrap(),
        [2328, -1165]
    );
    half(&mut bus, 0x1f801d88, 2);
    assert_eq!(bus.step_spu_output(Inputs::default()).unwrap(), [0; 2]);
    assert_eq!(bus.read(0x1f801e04, Width::Word).unwrap(), 0x80007fff);
}

#[test]
fn current_voice_volume_ports_preserve_signed_values_and_clock_inactive_sweeps() {
    let mut bus = fixture();
    for voice in 0..24 {
        let port = 0x1f801e00 + voice * 4;
        let raw = 0x80000000 | (voice * 2);
        bus.write(0x1f801c00 + voice * 16, Width::Word, 0x40000000 | voice)
            .unwrap();
        assert_eq!(bus.read(port | 0xa0000000, Width::Word).unwrap(), raw);
        assert!(bus.write(port, Width::Word, 0).is_err());
        assert_eq!(bus.read(port, Width::Word).unwrap(), raw);
        assert_eq!(
            bus.read(0x1f801c00 + voice * 16, Width::Half).unwrap(),
            voice
        );
    }
    half(&mut bus, 0x1f801d70, 0x8000); // Voice 23, never keyed on.
    bus.step_spu_voices().unwrap();
    assert_eq!(bus.read(0x1f801e5c, Width::Half).unwrap(), 14382);
    bus.write(0x1f801e5d, Width::Byte, 0).unwrap(); // Odd byte writes ignored.
    assert_eq!(bus.read(0x1f801e5c, Width::Half).unwrap(), 14382);
    assert!(bus.read(0x1f801e60, Width::Half).is_err());
    assert!(bus.write(0x1f801e61, Width::Half, 0).is_err());
}

#[test]
fn configured_reverb_reads_shared_ram_and_returns_stored_audio_with_writes_disabled() {
    use bof3_audio::machine::spu_reverb::Model as ReverbModel;
    let mut bus = fixture();
    assert!(bus.write(0x1f801da2, Width::Half, 0xc000).is_err());
    bus.configure_spu_reverb(ReverbModel::EmulatorReference)
        .unwrap();
    assert!(bus
        .configure_spu_reverb(ReverbModel::EmulatorReference)
        .is_err());
    half(&mut bus, 0xbf801da2, 0xc000);
    assert_eq!(bus.read(0x9f801da2, Width::Half).unwrap(), 0xc000);
    for i in 0..32 {
        let port = 0x1f801dc0 + i * 2;
        half(&mut bus, port, 0xab00 + i as u16);
        assert_eq!(bus.read(port, Width::Half).unwrap(), 0xab00 + i);
        half(&mut bus, port, 0);
    }
    half(&mut bus, 0x1f801da6, 0xc000);
    for _ in 0..32 {
        half(&mut bus, 0x1f801da8, 1000);
    }
    bus.service_spu(32).unwrap();
    let before = bus.spu_transfer().ram()[0x60000..0x60040].to_vec();
    bus.write(0x1f801d80, Width::Word, 0x20002000).unwrap();
    bus.write(0x1f801d84, Width::Word, 0x40004000).unwrap();
    for frame in 0..58 {
        let sample = bus.step_spu_output(Inputs::default()).unwrap();
        if frame > 40 {
            assert_eq!(sample, [if frame & 1 == 0 { 250 } else { 249 }; 2]);
        }
    }
    assert_eq!(&bus.spu_transfer().ram()[0x60000..0x60040], before);
    assert_eq!(bus.spu_transfer().capture_position(), 116);
    half(&mut bus, 0x1f801daa, 0xc090);
    bus.step_spu_output(Inputs::default()).unwrap();
    bus.step_spu_output(Inputs::default()).unwrap();
    assert_eq!(&bus.spu_transfer().ram()[0x6003a..0x6003c], &[0, 0]);
    assert_eq!(bus.spu_transfer().ram()[0x1001], 5); // Uploaded sample remains intact.
}

#[test]
fn decoded_xa_passes_drive_routing_before_spu_gain_and_capture() {
    use bof3_audio::{
        machine::cd_audio::{Model as CdModel, Volume as CdVolume, XaAudio},
        xa::{Arithmetic, Histories, Stream},
    };
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, 3, 0x64, 1, 1, 3, 0x64, 1]);
    for group in 0..18 {
        bytes[8 + group * 128 + 16..8 + (group + 1) * 128].fill(0x21);
    }
    let mut xa = XaAudio::new(
        Stream {
            file: 1,
            channel: 3,
            coding: 1,
        },
        Arithmetic::SplitFloor,
        Histories::default(),
        CdModel::EmulatorReference,
    )
    .unwrap();
    let frames = xa.sector(&bytes).unwrap();
    let volume = CdVolume::new([0, 128, 0, 128]).unwrap(); // Swap before SPU gain.
    let mut bus = fixture();
    half(&mut bus, 0x1f801daa, 0xc011);
    bus.write(0x1f801db0, Width::Word, 0x40004000).unwrap();
    bus.write(0x1f801d80, Width::Word, 0x20002000).unwrap();
    let mut signal = false;
    for (frame, input) in frames.into_iter().take(300).enumerate() {
        let cd = volume.apply(input, true);
        let output = bus
            .step_spu_output(Inputs {
                cd,
                external: [0; 2],
            })
            .unwrap();
        assert_eq!(output, cd.map(|v| ((i32::from(v) >> 1) >> 1) as i16));
        signal |= output != [0; 2];
        for (channel, sample) in cd.into_iter().enumerate() {
            let at = channel * 1024 + frame * 2;
            assert_eq!(&bus.spu_transfer().ram()[at..at + 2], &sample.to_le_bytes());
        }
    }
    assert!(signal);
}
