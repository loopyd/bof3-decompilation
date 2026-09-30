//! Original scheduler instructions with explicit injected CD responses.
//! These checks establish control flow, not hardware response timing or PCM.
use bof3_audio::{
    machine::bus::Bus, machine::bus::Ram, machine::bus::Width, machine::cd_position::CdPosition,
    machine::cpu::Cpu, machine::executable::Executable, xa::cue,
};

const RETURN: u32 = 0x8000_1000;
const RESPONSE: u32 = 0x8000_2000;
const CALLBACK: u32 = 0x8016_3858;
const TICK: u32 = 0x8016_38b0;
const STATE: u32 = 0x8014_682d;
const COMPLETION: u32 = 0x8014_682e;
const TIMER: u32 = 0x8014_681c;
const POLL: u32 = 0x8014_681e;
const RETRIES: u32 = 0x8014_6820;
const RESULT: u32 = 0x8014_6824;
const LAST_COMMAND: u32 = 0x8018_57a1;

fn exe() -> Executable {
    Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
        .unwrap()
}

fn cpu(entry: u32) -> Cpu {
    let mut cpu = Cpu::new(entry);
    cpu.set_register(29, 0x801f_fff0);
    cpu.set_register(31, RETURN);
    cpu
}

fn write_bytes(ram: &mut Ram, address: u32, bytes: &[u8]) {
    for (index, byte) in bytes.iter().enumerate() {
        ram.write(address + index as u32, Width::Byte, u32::from(*byte))
            .unwrap();
    }
}

fn read_bytes<const N: usize>(ram: &mut Ram, address: u32) -> [u8; N] {
    std::array::from_fn(|index| ram.read(address + index as u32, Width::Byte).unwrap() as u8)
}

#[test]
fn cd_positions_cover_lead_in_boundaries_and_reject_invalid_bcd() {
    for (lba, bcd) in [
        (-150, [0, 0, 0]),
        (-1, [0, 1, 0x74]),
        (0, [0, 2, 0]),
        (74, [0, 2, 0x74]),
        (75, [0, 3, 0]),
        (4350, [1, 0, 0]),
        (CdPosition::MAX_LBA, [0x99, 0x59, 0x74]),
    ] {
        assert_eq!(CdPosition::from_lba(lba).unwrap().bcd(), bcd);
        assert_eq!(CdPosition::from_bcd(bcd).unwrap().lba(), lba);
    }
    for lba in [i32::MIN, -151, CdPosition::MAX_LBA + 1, i32::MAX] {
        assert!(CdPosition::from_lba(lba).is_err());
    }
    for bcd in [
        [0xa0, 0, 0],
        [0x0a, 0, 0],
        [0, 0x60, 0],
        [0, 0x0f, 0],
        [0, 0, 0x75],
        [0, 0, 0x1a],
    ] {
        assert!(CdPosition::from_bcd(bcd).is_err());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE original US executable"]
fn position_conversion_and_cue_initialization_agree_with_linked_sdk() {
    let exe = exe();
    let cues = cue::read_cues(&exe).unwrap();
    let mut positions = vec![-150, -1, 0, 74, 75, 4350, CdPosition::MAX_LBA];
    for cue in &cues {
        positions.extend([
            cue.runtime_start_lba as i32,
            cue.runtime_stop_threshold_lba as i32,
        ]);
    }
    for lba in positions {
        let mut ram = Ram::from_executable(&exe);
        let mut to_position = cpu(0x8017_5adc);
        to_position.set_register(4, lba as u32);
        to_position.set_register(5, RESPONSE);
        to_position.run_until(&mut ram, RETURN, 1000).unwrap();
        assert_eq!(
            read_bytes::<3>(&mut ram, RESPONSE),
            CdPosition::from_lba(lba).unwrap().bcd()
        );
        let mut to_lba = cpu(0x8017_5be0);
        to_lba.set_register(4, RESPONSE);
        to_lba.run_until(&mut ram, RETURN, 1000).unwrap();
        assert_eq!(to_lba.register(2) as i32, lba);
    }
    for cue in cues {
        let mut ram = Ram::from_executable(&exe);
        let mut start = cpu(0x8016_36a0);
        start.set_register(4, u32::from(cue.packed_id));
        start.run_until(&mut ram, RETURN, 100_000).unwrap();
        assert_eq!(ram.read(STATE, Width::Byte).unwrap(), 1);
        assert_eq!(ram.read(RETRIES, Width::Half).unwrap(), 0);
        assert_eq!(ram.read(0x8018_5780, Width::Word).unwrap(), CALLBACK);
        assert_eq!(
            read_bytes::<3>(&mut ram, 0x8014_6838),
            CdPosition::from_lba(cue.runtime_start_lba as i32)
                .unwrap()
                .bcd()
        );
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE original US executable"]
fn original_poll_enters_pause_state_at_threshold_for_every_supported_cue() {
    let exe = exe();
    for cue in cue::read_cues(&exe).unwrap() {
        for delta in [-1i32, 0, 1] {
            let position = cue.runtime_stop_threshold_lba as i32 + delta;
            let mut ram = Ram::from_executable(&exe);
            ram.write(STATE, Width::Byte, 5).unwrap();
            ram.write(POLL, Width::Half, 0).unwrap();
            ram.write(TIMER, Width::Half, 27).unwrap();
            ram.write(0x8014_682c, Width::Byte, 0).unwrap();
            ram.write(0x8014_6810, Width::Word, cue.runtime_stop_threshold_lba)
                .unwrap();
            ram.write(LAST_COMMAND, Width::Byte, 0x10).unwrap();
            let position_bcd = CdPosition::from_lba(position).unwrap().bcd();
            let response = [
                position_bcd[0],
                position_bcd[1],
                position_bcd[2],
                2,
                1,
                cue.channel,
                0x64,
                0,
            ];
            write_bytes(&mut ram, RESPONSE, &response);
            let mut callback = cpu(CALLBACK);
            callback.set_register(4, 2);
            callback.set_register(5, RESPONSE);
            callback.run_until(&mut ram, RETURN, 1000).unwrap();
            assert_eq!(read_bytes::<8>(&mut ram, RESULT), response);
            cpu(TICK).run_until(&mut ram, RETURN, 1000).unwrap();
            assert_eq!(
                ram.read(STATE, Width::Byte).unwrap(),
                if delta < 0 { 5 } else { 6 },
                "cue {:#06x}, delta {delta}",
                cue.packed_id
            );
            assert_eq!(ram.read(0x8014_6814, Width::Word).unwrap() as i32, position);
            assert_eq!(ram.read(TIMER, Width::Half).unwrap(), 1);
            assert_eq!(ram.read(COMPLETION, Width::Byte).unwrap(), 0);
        }
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE original US executable"]
fn callback_failures_poll_cadence_and_unrelated_commands_do_not_fabricate_positions() {
    let exe = exe();
    for code in 0..=5 {
        let mut ram = Ram::from_executable(&exe);
        write_bytes(&mut ram, RESULT, &[0x55; 8]);
        write_bytes(&mut ram, RESPONSE, &[1, 2, 3, 4, 5, 6, 7, 8]);
        let mut callback = cpu(CALLBACK);
        callback.set_register(4, code);
        callback.set_register(5, RESPONSE);
        callback.run_until(&mut ram, RETURN, 1000).unwrap();
        assert_eq!(
            ram.read(COMPLETION, Width::Byte).unwrap(),
            if code == 2 { 1 } else { 255 }
        );
        assert_eq!(
            read_bytes::<8>(&mut ram, RESULT),
            if code == 2 {
                [1, 2, 3, 4, 5, 6, 7, 8]
            } else {
                [0x55; 8]
            }
        );
    }
    // A polling tick without a usable response requests GetlocL. Stop before
    // the original SDK executes hardware I/O; never fake a successful command.
    for completion in [0, 255] {
        let mut ram = Ram::from_executable(&exe);
        ram.write(POLL, Width::Half, 0).unwrap();
        ram.write(COMPLETION, Width::Byte, completion).unwrap();
        let mut poll = cpu(0x8016_3c58);
        poll.run_until(&mut ram, 0x8017_57e8, 1000).unwrap();
        assert_eq!(poll.register(4), 0x10);
        assert_eq!(poll.register(5), 0);
        assert_eq!(ram.read(COMPLETION, Width::Byte).unwrap(), 0);
    }
    for (counter, last_command, expected_completion) in [(1, 0x10, 1), (0, 0x0e, 0)] {
        let mut ram = Ram::from_executable(&exe);
        ram.write(POLL, Width::Half, counter).unwrap();
        ram.write(STATE, Width::Byte, 5).unwrap();
        ram.write(COMPLETION, Width::Byte, 1).unwrap();
        ram.write(LAST_COMMAND, Width::Byte, last_command).unwrap();
        ram.write(0x8014_6814, Width::Word, 12345).unwrap();
        cpu(0x8016_3c58).run_until(&mut ram, RETURN, 1000).unwrap();
        assert_eq!(ram.read(STATE, Width::Byte).unwrap(), 5);
        assert_eq!(ram.read(0x8014_6814, Width::Word).unwrap(), 12345);
        assert_eq!(
            ram.read(COMPLETION, Width::Byte).unwrap(),
            expected_completion
        );
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE original US executable"]
fn original_tick_cancellation_and_timeout_use_state_retries_and_status() {
    let exe = exe();
    for (timer, retries, status, cancel, expected_state, expected_retries) in [
        (179, 0, 0, 0, 5, 0),
        (180, 0, 0, 0, 1, 1),
        (180, 5, 0, 0, 1, 6),
        (180, 6, 0, 0, 8, 7),
        (180, 0, 0x10, 0, 8, 1),
        (0, 0, 0, 1, 8, 0),
    ] {
        let mut ram = Ram::from_executable(&exe);
        for (address, width, value) in [
            (STATE, Width::Byte, 5),
            (POLL, Width::Half, 1),
            (TIMER, Width::Half, timer),
            (RETRIES, Width::Half, retries),
            (RESULT, Width::Byte, status),
            (0x8014_682c, Width::Byte, cancel),
        ] {
            ram.write(address, width, value).unwrap();
        }
        cpu(TICK).run_until(&mut ram, RETURN, 1000).unwrap();
        assert_eq!(ram.read(STATE, Width::Byte).unwrap(), expected_state);
        assert_eq!(ram.read(RETRIES, Width::Half).unwrap(), expected_retries);
    }
}
