//! Original CdGetSector with explicitly supplied output blocks and service steps.
use bof3_audio::machine::{
    bus::{Bus, Width},
    cd_data::Model,
    cpu::Cpu,
    executable::Executable,
    interconnect::Interconnect,
    profile::Profile,
};

fn executable() -> Executable {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    exe
}
fn transfer(bus: &mut Interconnect, destination: u32, words: u32, budget: usize) {
    let mut cpu = Cpu::new(0x80175a78);
    for (r, v) in [
        (4, destination),
        (5, words),
        (29, 0x801ff000),
        (31, 0x80001000),
    ] {
        cpu.set_register(r, v);
    }
    for _ in 0..20000 {
        if cpu.pc() == 0x80001000 {
            break;
        }
        cpu.step(bus).unwrap();
        bus.service_cd(budget).unwrap();
    }
    assert_eq!(cpu.pc(), 0x80001000);
    assert_eq!(cpu.register(2), 1);
    assert_eq!(
        bus.read(0x1f8010b0, Width::Word).unwrap(),
        destination & 0xffffff
    );
    assert_eq!(bus.read(0x1f8010b4, Width::Word).unwrap(), 0x10000 | words);
    assert_eq!(bus.read(0x1f8010b8, Width::Word).unwrap(), 0);
    assert_eq!(bus.read(0x1f801020, Width::Word).unwrap(), 0x1325);
}
fn check(exe: &Executable, bytes: &[u8], budget: usize, split: bool) {
    let mut bus = Interconnect::from_executable(exe);
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    bus.configure_cd_data(Model::EmulatorReference).unwrap();
    bus.present_cd_data(bytes).unwrap();
    let words = bytes.len() as u32 / 4;
    if split {
        transfer(&mut bus, 0x80210003, 16, budget);
        assert!(bus.cd_host().unwrap().data_ready());
        transfer(&mut bus, 0x80210043, words - 16, budget);
    } else {
        transfer(&mut bus, 0x80210003, words, budget);
    }
    assert_eq!(&bus.ram().bytes()[0x10000..0x10000 + bytes.len()], bytes);
    assert!(!bus.cd_host().unwrap().data_ready());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original sector transfer routine, synthetic sectors and explicit service cadence"]
fn original_cd_get_sector_copies_both_sizes_in_whole_and_split_transfers() {
    let exe = executable();
    for size in [2048, 2340] {
        let bytes: Vec<u8> = (0..size).map(|i| (i * 37 + i / 256) as u8).collect();
        for budget in [1, 17, 1024] {
            for split in [false, true] {
                check(&exe, &bytes, budget, split);
            }
        }
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK; bounded raw VOICE sectors through original CdGetSector"]
fn original_voice_prefix_bytes_survive_sector_dma() {
    let exe = executable();
    let path = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").unwrap());
    let mut disc = bof3_audio::archive::disc::DiscImage::open(&path).unwrap();
    let start = disc.files()["BIN/SCE_XA/VOICE.STR"].lba;
    for lba in start..start + 16 {
        let sector = disc.read_sector(lba).unwrap();
        assert_eq!(
            &sector[12..15],
            &bof3_audio::machine::cd_position::CdPosition::from_lba(lba as i32)
                .unwrap()
                .bcd()
        );
        check(&exe, &sector[12..], 17, false);
    }
    eprintln!("Original CdGetSector: 16 supplied VOICE data-port blocks, 37,440 bytes reproduced exactly; no drive selection/timing or XA playback claim.");
}
