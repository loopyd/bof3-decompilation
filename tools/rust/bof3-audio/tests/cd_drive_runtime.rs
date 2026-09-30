//! Original US SDK instructions coupled to the functional drive, at explicit
//! boundaries. No GPU, BIOS startup or drive/CPU timing acceptance is implied.
use bof3_audio::{
    archive::disc::DiscImage,
    machine::{
        bus::{Bus, BusError, Width},
        cd_audio::{Model, XaAudio},
        cd_drive::{Delivery, Drive, Response},
        cd_host::Command,
        cd_position::CdPosition,
        cpu::Cpu,
        executable::Executable,
        interconnect::Interconnect,
        profile::Profile,
    },
    xa::{Arithmetic, Histories, Stream},
};

struct FixtureBus(Interconnect);
impl Bus for FixtureBus {
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        // VSync(-1) reads GPUSTAT even though it returns the software counter.
        if address == 0x1f801814 && width == Width::Word {
            return Ok(0);
        }
        self.0.read(address, width)
    }
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        self.0.write(address, width, value)
    }
    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
        self.0.write_masked(address, value, lanes)
    }
}

fn execute(bus: &mut FixtureBus, entry: u32, args: &[u32]) -> Cpu {
    let mut cpu = Cpu::new(entry);
    cpu.set_register(29, 0x801ff000);
    cpu.set_register(31, 0x80001000);
    for (i, value) in args.iter().enumerate() {
        cpu.set_register(4 + i, *value);
    }
    for _ in 0..20000 {
        if cpu.pc() == 0x80001000 {
            break;
        }
        cpu.step(bus).unwrap();
        bus.0.service_cd(17).unwrap();
    }
    assert_eq!(cpu.pc(), 0x80001000);
    cpu
}
fn deliver(bus: &mut FixtureBus, response: &Response) {
    bus.0
        .respond_cd(response.interrupt, &response.bytes)
        .unwrap();
    execute(bus, 0x80175c60, &[]);
    let index = bus.read(0x1f801800, Width::Byte).unwrap() & 3;
    bus.write(0x1f801800, Width::Byte, 1).unwrap();
    assert_eq!(bus.read(0x1f801803, Width::Byte).unwrap() & 7, 0);
    bus.write(0x1f801800, Width::Byte, index).unwrap();
    assert_eq!(bus.read(0x1f801800, Width::Byte).unwrap() & 0x20, 0);
}
fn submit(bus: &mut FixtureBus, drive: &mut Drive, opcode: u8, parameters: &[u8]) -> Response {
    for (i, value) in parameters.iter().enumerate() {
        bus.write(0x80010000 + i as u32, Width::Byte, u32::from(*value))
            .unwrap();
    }
    let cpu = execute(bus, 0x80176734, &[u32::from(opcode), 0x80010000, 0, 1]);
    assert_eq!(cpu.register(2), 0, "command {opcode:#04x}");
    let command = bus.0.take_cd_command().unwrap().unwrap();
    assert_eq!(command.opcode, opcode);
    assert_eq!(command.parameters, parameters);
    let response = drive.command(&command).unwrap();
    deliver(bus, &response);
    response
}
fn context() -> (FixtureBus, DiscImage, u32) {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    let path = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").unwrap());
    let disc = DiscImage::open(&path).unwrap();
    let start = disc.files()["BIN/SCE_XA/VOICE.STR"].lba;
    let mut bus = FixtureBus(Interconnect::from_executable(&exe));
    bus.0.configure_cd_host([128, 0, 128, 0]).unwrap();
    bus.0
        .configure_cd_data(bof3_audio::machine::cd_data::Model::EmulatorReference)
        .unwrap();
    // Explicit previous completion establishes the SDK's idle state through
    // original instructions; no SDK completion flag is patched.
    deliver(
        &mut bus,
        &Response {
            interrupt: 2,
            bytes: vec![2],
        },
    );
    (bus, disc, start)
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK; original SDK and raw VOICE read transactions"]
fn original_sdk_commands_drive_filtering_position_and_pause() {
    let (mut bus, mut disc, start) = context();
    let position = CdPosition::from_lba(start as i32).unwrap();
    let mut drive = Drive::ready(position, 0, [0, 0]).unwrap();
    for (op, params) in [
        (1, vec![]),
        (0x0e, vec![0xc8]),
        (0x0d, vec![1, 0]),
        (2, position.bcd().to_vec()),
    ] {
        assert_eq!(submit(&mut bus, &mut drive, op, &params).bytes, [2]);
    }
    assert_eq!(
        submit(&mut bus, &mut drive, 0x0f, &[]).bytes,
        [2, 0xc8, 0, 1, 0]
    );
    submit(&mut bus, &mut drive, 0x16, &[]);
    deliver(&mut bus, &drive.complete().unwrap().unwrap());
    submit(&mut bus, &mut drive, 0x1b, &[]);
    assert!(drive.complete().unwrap().is_none());
    let (mut selected, mut data_count) = (0, 0);
    for lba in start..start + 16 {
        let sector = disc.read_sector(lba).unwrap();
        let result = drive.sector(&sector).unwrap();
        if sector[18] & 0x44 != 0x44 {
            let Delivery::Data { bytes, response } = result else {
                panic!("data sector was lost")
            };
            assert_eq!(bytes, sector[24..2072]);
            bus.0.present_cd_data(&bytes).unwrap();
            deliver(&mut bus, &response);
            let cpu = execute(
                &mut bus,
                0x80175a78,
                &[0x80011000, (bytes.len() / 4) as u32],
            );
            assert_eq!(cpu.register(2), 1);
            assert_eq!(&bus.0.ram().bytes()[0x11000..0x11000 + bytes.len()], &bytes);
            data_count += 1;
        } else if sector[16..18] == [1, 0] {
            assert_eq!(result, Delivery::Xa(sector[16..].to_vec()));
            selected += 1;
        } else {
            assert_eq!(result, Delivery::Filtered);
        }
        assert_eq!(
            submit(&mut bus, &mut drive, 0x10, &[]).bytes,
            sector[12..20]
        );
    }
    assert_eq!(selected, 1);
    assert_eq!(data_count, 11);
    assert_eq!(submit(&mut bus, &mut drive, 9, &[]).bytes, [0x22]);
    let complete = drive.complete().unwrap().unwrap();
    assert_eq!(complete.bytes, [2]);
    deliver(&mut bus, &complete);
    assert_eq!(submit(&mut bus, &mut drive, 1, &[]).bytes, [2]);
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK; raw VOICE prefix through drive routing and XA resampling"]
fn raw_voice_routing_preserves_selected_decoder_input_and_history() {
    let (_, mut disc, start) = context();
    let position = CdPosition::from_lba(start as i32).unwrap();
    let mut drive = Drive::ready(position, 0xe8, [1, 0]).unwrap();
    drive
        .command(&Command {
            opcode: 6,
            parameters: vec![],
        })
        .unwrap();
    drive.complete().unwrap();
    let first = disc.read_sector(start).unwrap();
    let stream = Stream {
        file: first[16],
        channel: first[17],
        coding: first[19],
    };
    assert_eq!([stream.file, stream.channel], [1, 0]);
    let mut routed = XaAudio::new(
        stream,
        Arithmetic::SplitFloor,
        Histories::default(),
        Model::EmulatorReference,
    )
    .unwrap();
    let mut direct = XaAudio::new(
        stream,
        Arithmetic::SplitFloor,
        Histories::default(),
        Model::EmulatorReference,
    )
    .unwrap();
    let (mut selected, mut frames, mut data_count) = (0, 0, 0);
    for lba in start..start + 256 {
        let raw = disc.read_sector(lba).unwrap();
        match drive.sector(&raw).unwrap() {
            Delivery::Xa(bytes) => {
                assert_eq!(&bytes, &raw[16..]);
                let actual = routed.sector(&bytes).unwrap();
                let expected = direct.sector(&raw[16..]).unwrap();
                assert_eq!(actual, expected);
                selected += 1;
                frames += actual.len();
            }
            Delivery::Filtered => assert_ne!(raw[16..18], [1, 0]),
            Delivery::Data { bytes, response } => {
                assert_ne!(raw[18] & 0x44, 0x44);
                assert_eq!(bytes, raw[12..]);
                assert_eq!(response.interrupt, 1);
                data_count += 1;
            }
        }
    }
    assert_eq!(selected, 16);
    assert_eq!(frames, 75264);
    assert_eq!(data_count, 176);
    eprintln!("Raw VOICE drive routing: 256 sectors, 16 selected XA, 176 data, 75,264 stereo frames; comparison proves routing/history preservation, not independent hardware PCM or timing.");
}
