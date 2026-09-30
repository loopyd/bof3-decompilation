use bof3_audio::{voice::gain::Context, voice::gain::Registers};

fn neutral() -> Context {
    Context {
        bank_volume: 127,
        program_volume: 127,
        tone_volume: 127,
        velocity: 127,
        channel_volume: 127,
        sequence_volume: [127, 127],
        tone_pan: 64,
        program_pan: 64,
        channel_pan: 64,
        mono: false,
    }
}

#[test]
fn register_endpoints_and_pan_order_retain_game_quantization() {
    let mut c = neutral();
    assert_eq!(
        c.registers().unwrap(),
        Registers {
            left: 16383,
            right: 16383
        }
    );
    c.tone_pan = 63;
    assert_eq!(c.registers().unwrap(), neutral().registers().unwrap());
    c.tone_pan = 0;
    assert_eq!(
        c.registers().unwrap(),
        Registers {
            left: 16383,
            right: 0
        }
    );
    c.program_pan = 127;
    assert_eq!(c.registers().unwrap(), Registers { left: 0, right: 0 });
    c.program_pan = 64;
    c.mono = true;
    assert_eq!(c.registers().unwrap(), neutral().registers().unwrap());
    c.velocity = 0;
    assert_eq!(c.registers().unwrap(), Registers { left: 0, right: 0 });
    c.channel_pan = 128;
    assert!(c.registers().is_err());
}

mod original {
    use super::*;
    use bof3_audio::machine::{
        bus::{Bus, BusError, Ram, Width},
        cpu::Cpu,
        executable::Executable,
        profile::Profile,
    };

    const HEADER: u32 = 0x8001_0000;
    const PROGRAM: u32 = 0x8001_0100;
    const TONE: u32 = 0x8001_1000;
    const SEQUENCE: u32 = 0x8001_2000;
    const RETURN: u32 = 0x8000_1000;

    struct Observed {
        ram: Ram,
        program_mode_reads: usize,
        tone_mode_reads: usize,
        spu_writes: Vec<(u32, Width, u32)>,
    }

    impl Bus for Observed {
        fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
            let covers = |target| address <= target && target < address + width.bytes() as u32;
            self.program_mode_reads += usize::from(covers(PROGRAM + 3));
            self.tone_mode_reads += usize::from(covers(TONE + 1));
            self.ram.read(address, width)
        }
        fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
            if (0x1f80_1c00..0x1f80_1e00).contains(&address) {
                // A write witness only, not an emulated SPU or audio oracle.
                self.spu_writes.push((address, width, value));
                return Ok(());
            }
            self.ram.write(address, width, value)
        }
        fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
            self.ram.write_masked(address, value, lanes)
        }
    }

    fn executable() -> Executable {
        let exe = Executable::from_bytes(
            std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap(),
        )
        .unwrap();
        Profile::identify(&exe).unwrap();
        exe
    }

    fn fixture(exe: &Executable, c: Context, program_mode: u8, tone_mode: u8) -> Observed {
        let mut ram = Ram::from_executable(exe);
        // Controlled post-bank-open state: real routines and pitch table remain intact.
        for (address, value) in [
            (0x8018_e1a0, HEADER),
            (0x8018_e100, PROGRAM),
            (0x8018_e1e8, TONE),
            (0x8019_0308, SEQUENCE),
        ] {
            ram.write(address, Width::Word, value).unwrap();
        }
        for (address, value) in [
            (0x8018_e24c, 128),
            (HEADER + 0x12, 1),
            (SEQUENCE + 0x4e, u32::from(c.channel_volume)),
            (SEQUENCE + 0x74, u32::from(c.sequence_volume[0])),
            (SEQUENCE + 0x76, u32::from(c.sequence_volume[1])),
            (0x8018_e246, u32::from(c.mono)),
            (TONE + 0x10, 0x000f),
            (TONE + 0x12, 0x1fc0),
            (TONE + 0x16, 1),
            (PROGRAM + 0xc, 0x200),
        ] {
            ram.write(address, Width::Half, value).unwrap();
        }
        for (address, value) in [
            (0x8018_e7f8, 1),
            (0x8018_e264, 24),
            (HEADER + 0x18, c.bank_volume),
            (PROGRAM, 1),
            (PROGRAM + 1, c.program_volume),
            (PROGRAM + 3, program_mode),
            (PROGRAM + 4, c.program_pan),
            (TONE, 64),
            (TONE + 1, tone_mode),
            (TONE + 2, c.tone_volume),
            (TONE + 3, c.tone_pan),
            (TONE + 4, 60),
            (TONE + 7, 127),
        ] {
            ram.write(address, Width::Byte, u32::from(value)).unwrap();
        }
        Observed {
            ram,
            program_mode_reads: 0,
            tone_mode_reads: 0,
            spu_writes: Vec::new(),
        }
    }

    fn execute(bus: &mut Observed, c: Context) -> (Registers, u32) {
        let mut cpu = Cpu::new(0x8017_102c);
        // handle 0, bank 0, program 0, key 60; velocity and pan on caller's stack.
        cpu.set_register(7, 60);
        cpu.set_register(29, 0x801f_f000);
        cpu.set_register(31, RETURN);
        bus.ram
            .write(0x801f_f010, Width::Word, u32::from(c.velocity))
            .unwrap();
        bus.ram
            .write(0x801f_f014, Width::Word, u32::from(c.channel_pan))
            .unwrap();
        cpu.run_until(bus, RETURN, 5000).unwrap();
        if c.velocity != 0 {
            assert_eq!(
                cpu.register(2).count_ones(),
                1,
                "one voice must be selected"
            );
        }
        let read = |bus: &mut Observed, address| bus.ram.read(address, Width::Half).unwrap();
        let voice = read(bus, 0x8018_e7f2);
        assert!(voice < 24);
        let registers = Registers {
            left: read(bus, 0x8018_e808 + voice * 16) as u16,
            right: read(bus, 0x8018_e80a + voice * 16) as u16,
        };
        let reverb = read(bus, 0x8018_db54) | (read(bus, 0x8018_db56) << 16);
        (registers, reverb)
    }

    #[test]
    #[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
    fn initial_voice_registers_match_original_note_on_across_all_seven_bit_inputs() {
        let exe = executable();
        let mut checks = 0;
        for field in 0..11 {
            for value in 0..128 {
                let mut c = neutral();
                match field {
                    0 => c.bank_volume = value,
                    1 => c.program_volume = value,
                    2 => c.tone_volume = value,
                    3 => c.velocity = value,
                    4 => c.channel_volume = value,
                    5 => c.sequence_volume[0] = value,
                    6 => c.sequence_volume[1] = value,
                    7 => c.tone_pan = value,
                    8 => c.program_pan = value,
                    9 => c.channel_pan = value,
                    10 => {
                        c.mono = true;
                        c.tone_pan = value;
                    }
                    _ => unreachable!(),
                }
                let mut bus = fixture(&exe, c, 0, 0);
                let (actual, _) = execute(&mut bus, c);
                assert_eq!(
                    actual,
                    c.registers().unwrap(),
                    "field {field}, value {value}"
                );
                checks += 1;
            }
        }
        // Mixed controls expose intermediate truncation that isolated sweeps miss.
        let mut seed = 0x1234_5678u32;
        for _ in 0..1024 {
            let mut next = || {
                seed = seed.wrapping_mul(1664525).wrapping_add(1013904223);
                (seed >> 25) as u8
            };
            let c = Context {
                bank_volume: next(),
                program_volume: next(),
                tone_volume: next(),
                velocity: next(),
                channel_volume: next(),
                sequence_volume: [next(), next()],
                tone_pan: next(),
                program_pan: next(),
                channel_pan: next(),
                mono: next() & 1 != 0,
            };
            let (actual, _) = execute(&mut fixture(&exe, c, 0, 0), c);
            assert_eq!(actual, c.registers().unwrap(), "{c:?}");
            checks += 1;
        }
        eprintln!("{checks} original-US initial note-on gain contexts agree exactly");
    }

    #[test]
    #[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
    fn mode_bytes_have_distinct_observed_effects_on_original_note_on() {
        let exe = executable();
        let c = neutral();
        for mode in 0..=255 {
            let mut bus = fixture(&exe, c, mode, 0);
            let base = execute(&mut bus, c);
            assert_eq!(base, (c.registers().unwrap(), 0));
            assert_eq!(bus.program_mode_reads, 0);
            assert!(bus.tone_mode_reads > 0);

            let mut bus = fixture(&exe, c, 0, mode);
            // Set another voice's bit and the selected voice's bit to check clear/set.
            bus.ram.write(0x8018_db54, Width::Half, 3).unwrap();
            let (registers, mask) = execute(&mut bus, c);
            assert_eq!(registers, base.0);
            assert_eq!(mask, 2 | u32::from(mode & 4 != 0));
            assert_eq!(
                bus.ram.read(0x8018_e7ec, Width::Byte).unwrap(),
                u32::from(mode)
            );
            assert_eq!(bus.program_mode_reads, 0);
        }
    }

    #[test]
    #[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
    fn tone_reverb_mask_reaches_both_spu_register_halves_for_every_voice() {
        let exe = executable();
        let c = neutral();
        for voice in 0..24 {
            for mode in [0, 4] {
                let mut bus = fixture(&exe, c, 0, mode);
                // Occupy earlier voices at higher priority so the next one is free.
                for occupied in 0..voice {
                    bus.ram
                        .write(0x8018_dc0b + occupied * 52, Width::Byte, 1)
                        .unwrap();
                    bus.ram
                        .write(0x8018_dbf6 + occupied * 52, Width::Half, 1)
                        .unwrap();
                    bus.ram
                        .write(0x8018_dc08 + occupied * 52, Width::Half, 127)
                        .unwrap();
                }
                bus.ram.write(0x8018_db54, Width::Half, 0xffff).unwrap();
                bus.ram.write(0x8018_db56, Width::Half, 0xff).unwrap();
                let (gain, mask) = execute(&mut bus, c);
                assert_eq!(bus.ram.read(0x8018_e7f2, Width::Half).unwrap(), voice);
                assert_eq!(gain, c.registers().unwrap());
                let expected = if mode == 4 {
                    0xff_ffff
                } else {
                    0xff_ffff & !(1 << voice)
                };
                assert_eq!(mask, expected);
                assert!(
                    bus.spu_writes.is_empty(),
                    "note-on only stages register writes"
                );
                assert_eq!(bus.ram.read(0x8018_4458, Width::Word).unwrap(), 0x1f80_1c00);
                // Execute the original final flush block up to its stack epilogue.
                let mut cpu = Cpu::new(0x8017_09c4);
                cpu.run_until(&mut bus, 0x8017_0a34, 100).unwrap();
                assert_eq!(
                    bus.spu_writes,
                    vec![
                        (0x1f80_1d8c, Width::Half, 0),
                        (0x1f80_1d8e, Width::Half, 0),
                        (0x1f80_1d88, Width::Half, (1 << voice) & 0xffff),
                        (0x1f80_1d8a, Width::Half, (1 << voice) >> 16),
                        (0x1f80_1d98, Width::Half, expected & 0xffff),
                        (0x1f80_1d9a, Width::Half, expected >> 16),
                    ]
                );
            }
        }
    }
}
