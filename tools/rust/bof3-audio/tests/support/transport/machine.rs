//! Original XA guest calls with explicitly selected kernel/drive inputs.
use bof3_audio::machine::{boot, firmware::Image};
use bof3_audio::{
    machine::bus::Bus, machine::bus::BusError, machine::bus::Width, machine::cd_drive::Activity,
    machine::cd_drive::Drive, machine::cd_drive::Response, machine::cd_position::CdPosition,
    machine::cpu::Cpu, machine::cpu::FaultKind, machine::exception_calls::Environment,
    machine::exception_calls::MissingHook, machine::executable::Executable,
    machine::interconnect::Interconnect, machine::kernel::Kernel, machine::profile::Profile,
    xa::cue,
};

#[derive(Clone, Copy, Debug)]
pub(crate) enum Vsync {
    Original(u32),
    Candidate,
}

#[derive(Debug, PartialEq, Eq, serde::Serialize)]
pub(crate) struct Proof {
    instructions: usize,
    vsync_calls: usize,
    pc_sha256: String,
    returns_sha256: String,
    ram_sha256: String,
    irq_entries: usize,
    irq_returns: usize,
}

pub(crate) struct BusFixture {
    pub(crate) inner: Interconnect,
    vsync: Vsync,
    pub(crate) gpu_reads: usize,
}
impl Bus for BusFixture {
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        if address == 0x1f801814 && width == Width::Word {
            self.gpu_reads += 1;
            return match self.vsync {
                Vsync::Original(gpu) => Ok(gpu),
                Vsync::Candidate => Err(BusError {
                    address,
                    detail: "GPU unavailable to the fixture-only VSync candidate".into(),
                }),
            };
        }
        if address == 0x1f801110 && matches!(self.vsync, Vsync::Candidate) {
            return Err(BusError {
                address,
                detail: "Timer-1 counter unavailable to the fixture-only VSync candidate".into(),
            });
        }
        self.inner.read(address, width)
    }
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        self.inner.write(address, width, value)
    }
    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
        self.inner.write_masked(address, value, lanes)
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) enum Dispatch {
    Direct,
    Kernel,
    Firmware,
    Initialized,
}

impl Dispatch {
    fn is_rom(self) -> bool {
        matches!(self, Self::Firmware | Self::Initialized)
    }
}

pub(crate) struct Fixture {
    pub(crate) bus: BusFixture,
    pub(crate) drive: Drive,
    pub(crate) pending: Option<Response>,
    pub(crate) commands: Vec<(u8, Vec<u8>)>,
    pub(crate) transitions: usize,
    pub(crate) kernel: Kernel,
    pub(crate) dispatch: Dispatch,
    pub(crate) irq_entries: usize,
    pub(crate) irq_returns: usize,
    pub(crate) interrupted: Option<Cpu>,
    pub(crate) evidence: Option<super::Evidence>,
    pcs: Vec<u8>,
    returns: Vec<u8>,
    vsync_calls: usize,
}

impl Fixture {
    pub(crate) fn proof(&self) -> Proof {
        let mut ram = self.bus.inner.ram().bytes().to_vec();
        // Normalize only the two declared fixture substitutions.
        for (address, word) in [(0x80174720, 0x8c500000u32), (0x80174724, 0x8c620000)] {
            let at = bof3_audio::machine::executable::ram_offset(address, 4).unwrap();
            ram[at..at + 4].copy_from_slice(&word.to_le_bytes());
        }
        Proof {
            instructions: self.pcs.len() / 4,
            vsync_calls: self.vsync_calls,
            pc_sha256: bof3_audio::digest::sha256_hex(&self.pcs),
            returns_sha256: bof3_audio::digest::sha256_hex(&self.returns),
            ram_sha256: bof3_audio::digest::sha256_hex(&ram),
            irq_entries: self.irq_entries,
            irq_returns: self.irq_returns,
        }
    }
    fn initialize_irq_hook(&mut self) {
        if let Some(evidence) = &mut self.evidence {
            evidence.begin_call(0x801748e4);
        }
        // Original callback setup stops at unsupported CD BIOS removal. Do not
        // skip that service or treat this prefix as completed initialization.
        let mut cpu = Cpu::new(0x801748e4);
        cpu.set_register(29, 0x801ff000);
        cpu.set_register(31, 0x80001000);
        for _ in 0..10000 {
            if let Some(evidence) = &mut self.evidence {
                evidence.observe("hook initialization", &cpu, &self.bus.inner);
            }
            if Kernel::is_call(cpu.pc()) {
                if cpu.pc() & 0x1fff_ffff == 0xa0 && cpu.register(9) == 0x72 {
                    assert_eq!(self.kernel.entry_hook(), Some(0x80184640));
                    println!("Original callback setup reaches A0:72 after {} instructions; SDK hook installed, bootstrap still incomplete", cpu.instructions());
                    return;
                }
                self.kernel.dispatch(&mut cpu, &mut self.bus).unwrap();
            } else {
                cpu.step(&mut self.bus).unwrap();
            }
        }
        panic!("original callback setup exceeded instruction bound");
    }

    fn configure_kernel_fixture(&mut self) {
        // Supplied empty priority chain and one live thread, not Sony BIOS
        // allocations. The SDK hook itself is installed by original code.
        for (address, value) in [
            (0x100, 0x80003000),
            (0x104, 32),
            (0x108, 0x80006000),
            (0x10c, 4),
            (0x110, 0x80005000),
            (0x114, 0xc0),
            (0x80006000, 0x80005000),
            (0x80005000, 0x4000),
        ] {
            self.bus.write(address, Width::Word, value).unwrap();
        }
        for offset in (0..32).step_by(4) {
            self.bus.write(0x80003000 + offset, Width::Word, 0).unwrap();
        }
    }

    fn finish_interrupt(&mut self, cpu: &Cpu) {
        let mut expected = self.interrupted.take().unwrap();
        expected.cop0_mut().return_from_exception();
        for register in 1..32 {
            if register != 26 {
                assert_eq!(
                    cpu.register(register),
                    expected.register(register),
                    "restored r{register}"
                );
            }
        }
        assert_eq!((cpu.hi(), cpu.lo()), (expected.hi(), expected.lo()));
        assert_eq!(cpu.pc(), expected.cop0().read(14).unwrap());
        assert_eq!(cpu.cop0().status(), expected.cop0().status());
        self.irq_returns += 1;
        if matches!(
            self.drive.activity(),
            Activity::Seeking { .. } | Activity::Pausing { .. } | Activity::Resetting
        ) {
            self.pending = self.drive.complete().unwrap();
        }
    }

    pub(crate) fn call(&mut self, entry: u32, args: &[u32], handler: bool) -> u32 {
        if let Some(evidence) = &mut self.evidence {
            evidence.begin_call(entry);
        }
        let mut cpu = Cpu::new(entry);
        cpu.set_register(29, if handler { 0x801fe000 } else { 0x801ff000 });
        cpu.set_register(31, 0x80001000);
        cpu.cop0_mut()
            .write(12, if handler { 0 } else { 0x401 })
            .unwrap();
        for (i, value) in args.iter().enumerate() {
            cpu.set_register(4 + i, *value);
        }
        for _ in 0..200000 {
            if cpu.pc() == 0x80001000
                && !self.kernel.pending_exception()
                && self.interrupted.is_none()
            {
                for value in (0..32).map(|i| cpu.register(i)).chain([
                    cpu.hi(),
                    cpu.lo(),
                    cpu.cop0().status(),
                    cpu.pc(),
                ]) {
                    self.returns.extend_from_slice(&value.to_le_bytes());
                }
                return cpu.register(2);
            }
            self.transitions += 1;
            self.pcs.extend_from_slice(&cpu.pc().to_le_bytes());
            if cpu.pc() == 0x80174700 {
                self.vsync_calls += 1;
                if matches!(self.bus.vsync, Vsync::Candidate) {
                    assert!(
                        (cpu.register(4) as i32) < 0,
                        "candidate excludes nonnegative VSync callers"
                    );
                }
            }
            assert!(self.transitions < 2_000_000, "fixture transition bound");
            if let Some(evidence) = &mut self.evidence {
                evidence.observe("scheduler/IRQ", &cpu, &self.bus.inner);
            }
            if !self.dispatch.is_rom() && Kernel::is_call(cpu.pc()) {
                let active = self.kernel.pending_exception();
                let call = self.kernel.dispatch(&mut cpu, &mut self.bus).unwrap();
                assert!(!call.waiting);
                if active && !self.kernel.pending_exception() {
                    self.finish_interrupt(&cpu);
                }
            } else {
                let before = self
                    .evidence
                    .as_ref()
                    .map(|_| bof3_audio::driver::retirement::Before::capture(&cpu));
                match cpu.step(&mut self.bus) {
                    Ok(step) => {
                        if let (Some(evidence), Some(before)) =
                            (&mut self.evidence, before.as_ref())
                        {
                            evidence.observe_retired(before, &step);
                        }
                    }
                    Err(fault) if fault.kind == FaultKind::Syscall => {
                        if let Some(evidence) = &mut self.evidence {
                            evidence.observe_syscall(&fault);
                        }
                        if self.dispatch.is_rom() {
                            cpu.enter_exception(&fault).unwrap();
                        } else {
                            self.kernel.dispatch_syscall(&mut cpu, &fault).unwrap();
                        }
                    }
                    Err(fault) => panic!("{fault}"),
                }
                if self.dispatch.is_rom()
                    && self
                        .interrupted
                        .as_ref()
                        .is_some_and(|saved| cpu.pc() == saved.cop0().read(14).unwrap())
                {
                    self.finish_interrupt(&cpu);
                }
            }
            if let Some(command) = self.bus.inner.take_cd_command().unwrap() {
                assert!(self.pending.is_none());
                self.commands
                    .push((command.opcode, command.parameters.clone()));
                self.pending = Some(
                    self.bus
                        .inner
                        .apply_cd_drive_command(&mut self.drive, &command)
                        .unwrap()
                        .response,
                );
            }
            // All variants retain explicit response/sector boundaries. Firmware
            // executes the original exception vector and ROM instead of HLE.
            if !handler {
                if self.interrupted.is_none() {
                    if let Some(response) = self.pending.take() {
                        self.bus
                            .inner
                            .respond_cd(response.interrupt, &response.bytes)
                            .unwrap();
                    }
                }
                if self.dispatch != Dispatch::Direct {
                    if cpu.take_interrupt(self.bus.inner.interrupts().pending()) {
                        let mut interrupted = cpu.clone();
                        interrupted.synchronize_load();
                        assert!(self.interrupted.replace(interrupted).is_none());
                        if self.dispatch == Dispatch::Kernel {
                            self.kernel
                                .begin_exception(
                                    &mut cpu,
                                    &mut self.bus,
                                    Environment {
                                        stack_top: 0x80010000,
                                        global_pointer: 0,
                                        missing_hook: MissingHook::Reject,
                                    },
                                )
                                .unwrap();
                        }
                        self.irq_entries += 1;
                    }
                } else if cpu.cop0().status() & 0x401 == 0x401
                    && self.bus.inner.cd_host().unwrap().irq_line()
                {
                    self.call(0x80177264, &[], true);
                    if matches!(
                        self.drive.activity(),
                        Activity::Seeking { .. } | Activity::Pausing { .. } | Activity::Resetting
                    ) {
                        self.pending = self.drive.complete().unwrap();
                    }
                }
            }
        }
        panic!(
            "call {entry:#x} bounded at {:#x}, commands {:?}",
            cpu.pc(),
            self.commands
        );
    }
    pub(crate) fn state(&mut self) -> u32 {
        self.bus.read(0x8014682d, Width::Byte).unwrap()
    }
}

pub(crate) fn reading_fixture(packed_id: u16) -> (Fixture, bof3_audio::catalog::model::XaCue) {
    reading_fixture_with_irq(packed_id, Dispatch::Direct, false, Vsync::Original(0))
}

pub(crate) fn reading_fixture_with_irq(
    packed_id: u16,
    dispatch: Dispatch,
    observed: bool,
    vsync: Vsync,
) -> (Fixture, bof3_audio::catalog::model::XaCue) {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    let cue = cue::read_cues(&exe)
        .unwrap()
        .into_iter()
        .find(|c| c.packed_id == packed_id)
        .unwrap();
    let (bus, boot) = if dispatch.is_rom() {
        let bios =
            Image::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap())
                .unwrap();
        let machine = boot::load_us(bios, &exe, 3_000_000).unwrap();
        (machine.bus, Some(machine.evidence))
    } else {
        (Interconnect::from_executable(&exe), None)
    };
    let mut bus = BusFixture {
        inner: bus,
        vsync,
        gpu_reads: 0,
    };
    if matches!(vsync, Vsync::Candidate) {
        assert_eq!(dispatch, Dispatch::Initialized);
        assert!(
            !observed,
            "original-byte trace cannot describe modified instructions"
        );
        for (address, word) in [(0x80174720, 0x8c500000u32), (0x80174724, 0x8c620000)] {
            assert_eq!(bus.read(address, Width::Word).unwrap(), word);
            bus.write(address, Width::Word, 0).unwrap();
        }
    }
    if dispatch == Dispatch::Initialized {
        bus.inner
            .configure_cd_host_reset(bof3_audio::machine::cd_host::Model::EmulatorReference)
            .unwrap();
    } else {
        bus.inner.configure_cd_host([128, 0, 128, 0]).unwrap();
    }
    bus.inner
        .configure_cd_data(bof3_audio::machine::cd_data::Model::EmulatorReference)
        .unwrap();
    if dispatch != Dispatch::Initialized {
        bus.write(0x1f801800, Width::Byte, 1).unwrap();
        bus.write(0x1f801802, Width::Byte, 0x1f).unwrap();
        bus.write(0x1f801800, Width::Byte, 0).unwrap();
    }
    let mut fixture = Fixture {
        bus,
        drive: Drive::ready(CdPosition::from_lba(0).unwrap(), 0, [0, 0]).unwrap(),
        pending: None,
        commands: Vec::new(),
        transitions: 0,
        kernel: Kernel::default(),
        dispatch,
        irq_entries: 0,
        irq_returns: 0,
        interrupted: None,
        pcs: Vec::new(),
        returns: Vec::new(),
        vsync_calls: 0,
        evidence: observed.then(|| {
            super::Evidence::new(
                exe,
                boot,
                dispatch == Dispatch::Initialized,
                match vsync {
                    Vsync::Original(gpu) => gpu,
                    Vsync::Candidate => unreachable!(),
                },
            )
        }),
    };
    match dispatch {
        Dispatch::Firmware => assert_eq!(fixture.call(0x80175534, &[2], false), 1),
        Dispatch::Initialized => assert_eq!(fixture.call(0x80175534, &[1], false), 1),
        Dispatch::Kernel => {
            fixture.initialize_irq_hook();
            fixture.configure_kernel_fixture();
            fixture.call(0x80174914, &[2, 0x80177264], false);
        }
        Dispatch::Direct => (),
    }
    if dispatch != Dispatch::Initialized {
        // Initial SDK idle state is established by an explicit prior completion.
        fixture.bus.inner.respond_cd(2, &[2]).unwrap();
        if dispatch != Dispatch::Direct {
            fixture.call(0x8017ee1c, &[], false);
        } else {
            fixture.call(0x80177264, &[], true);
        }
    }
    fixture.call(cue::START, &[u32::from(cue.packed_id)], false);
    assert_eq!(fixture.state(), 1);
    for _ in 0..20 {
        fixture.call(cue::TICK, &[], false);
        if fixture.state() == 5 {
            break;
        }
    }
    assert_eq!(
        fixture.state(),
        5,
        "commands {:?}; drive {:?}; pending {:?}; completion {}; callback {:x}; irq {}",
        fixture.commands,
        fixture.drive.activity(),
        fixture.pending,
        fixture.bus.read(0x8014682e, Width::Byte).unwrap(),
        fixture.bus.read(0x80185780, Width::Word).unwrap(),
        fixture.bus.inner.cd_host().unwrap().irq_line()
    );
    assert_eq!(fixture.drive.activity(), Activity::Reading);
    (fixture, cue)
}
