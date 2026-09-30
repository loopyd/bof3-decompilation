//! Original driver instructions with explicit host-register state and response input.
use bof3_audio::machine::{
    bus::{Bus, BusError, Width},
    cpu::Cpu,
    executable::Executable,
    interconnect::Interconnect,
    profile::Profile,
};

// Explicit input for VSync(-1)'s unconditional status read. This supplies no
// GPU commands, clocks, display interrupts or production fallback.
struct FixtureBus(Interconnect);
impl Bus for FixtureBus {
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
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
    cpu.run_until(bus, 0x80001000, 20000).unwrap();
    cpu
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original CD command submission with explicit host state"]
fn original_us_command_writer_submits_parameters_through_mmio() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    for (command, parameters) in [
        (1, vec![]),
        (2, vec![0x01, 0x02, 0x03]),
        (0x0e, vec![0xa8]),
        (0x0d, vec![3, 2]),
    ] {
        let mut bus = FixtureBus(Interconnect::from_executable(&exe));
        bus.0.configure_cd_host([128, 0, 128, 0]).unwrap();
        // Explicit prior completion input establishes the SDK's idle sync state
        // through its original response handler, not a patched guest global.
        bus.0.respond_cd(2, &[2]).unwrap();
        execute(&mut bus, 0x80175c60, &[]);
        assert_eq!(bus.read(0x80185a5c, Width::Byte).unwrap(), 2);
        for (i, &byte) in parameters.iter().enumerate() {
            bus.write(0x80010000 + i as u32, Width::Byte, u32::from(byte))
                .unwrap();
        }
        let cpu = execute(&mut bus, 0x80176734, &[command, 0x80010000, 0, 1]);
        assert_eq!(cpu.register(2), 0);
        let sent = bus.0.take_cd_command().unwrap().unwrap();
        assert_eq!(sent.opcode, command as u8);
        assert_eq!(sent.parameters, parameters);
        assert!(bus.0.cd_host().unwrap().busy());
        bus.0.respond_cd(3, &[2]).unwrap();
        execute(&mut bus, 0x80175c60, &[]);
        assert_eq!(bus.read(0x1f801803, Width::Byte).unwrap() & 7, 0);
        assert_eq!(bus.read(0x1f801800, Width::Byte).unwrap() & 0x20, 0);
    }
}
