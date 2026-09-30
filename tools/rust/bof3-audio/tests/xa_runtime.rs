//! XA cue selection executes original instructions; scheduler stop timing is separate.
use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    executable::Executable,
    profile::Profile,
};

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn all_supported_xa_extents_agree_with_original_selector() {
    let exe = Executable::from_bytes(
        std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE")).unwrap(),
    )
    .unwrap();
    let cues = bof3_audio::xa::cue::read_cues(&exe).unwrap();
    assert_eq!(cues.len(), 896);
    for (stream, count) in [(0, 11), (1, 880), (2, 5)] {
        assert_eq!(cues.iter().filter(|c| c.stream == stream).count(), count);
    }
    for cue in cues {
        let mut ram = Ram::from_executable(&exe);
        let mut cpu = Cpu::new(bof3_audio::xa::cue::SELECTOR);
        cpu.set_register(4, u32::from(cue.packed_id));
        cpu.set_register(29, 0x801f_fff0);
        cpu.set_register(31, 0x8000_1000);
        cpu.run_until(&mut ram, 0x8000_1000, 100_000)
            .unwrap_or_else(|error| panic!("XA cue {:#06x}: {error}", cue.packed_id));
        for (address, width, expected) in [
            (0x8014_6834, Width::Byte, u32::from(cue.filter_file)),
            (0x8014_6835, Width::Byte, u32::from(cue.channel)),
            (0x8014_680c, Width::Word, cue.runtime_start_lba),
            (0x8014_6814, Width::Word, cue.runtime_start_lba),
            (0x8014_6810, Width::Word, cue.runtime_stop_threshold_lba),
        ] {
            assert_eq!(
                ram.read(address, width).unwrap(),
                expected,
                "XA cue {:#06x}, address {address:#010x}",
                cue.packed_id
            );
        }
        assert_eq!(
            cue.last_sector,
            cue.sector_start + (cue.sector_count - 1) * cue.sector_stride
        );
        assert_eq!(cue.scheduler_endpoint, "not_verified");
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_voice_cue_selector_sets_file_channel_start_and_stop_threshold() {
    let exe = Executable::from_bytes(
        std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE")).unwrap(),
    )
    .unwrap();
    Profile::identify(&exe).unwrap();
    let base_lba = 106214;
    for (cue, groups) in [71, 63, 61, 60, 42].into_iter().enumerate() {
        let mut ram = Ram::from_executable(&exe);
        let mut cpu = Cpu::new(0x8016_3744);
        cpu.set_register(4, 0x2000 + cue as u32);
        cpu.set_register(29, 0x801f_fff0);
        cpu.set_register(31, 0x8000_1000);
        cpu.run_until(&mut ram, 0x8000_1000, 1000).unwrap();
        assert_eq!(ram.read(0x8014_6834, Width::Byte).unwrap(), 1);
        assert_eq!(ram.read(0x8014_6835, Width::Byte).unwrap(), cue as u32);
        assert_eq!(
            ram.read(0x8014_680c, Width::Word).unwrap(),
            base_lba + cue as u32
        );
        assert_eq!(
            ram.read(0x8014_6814, Width::Word).unwrap(),
            base_lba + cue as u32
        );
        assert_eq!(
            ram.read(0x8014_6810, Width::Word).unwrap(),
            base_lba + groups * 16 + cue as u32 - 150
        );
    }
}
