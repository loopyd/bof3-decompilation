//! Bounded original-code data-dependency evidence, not production pruning.
use bof3_audio::machine::{
    bus::{Bus, BusError, Ram, Width},
    cpu::Cpu,
    executable::{ram_offset, Executable},
    profile::Profile,
};

const ENTRY: u32 = 0x80174700;
const GPU_LOAD: u32 = 0x80174720;
const TIMER_LOAD: u32 = 0x80174724;
const RETURN: u32 = 0x80001000;

struct Inputs {
    ram: Ram,
    gpu: Option<u32>,
    gpu_reads: usize,
    timer_address: u32,
    timer: Option<u32>,
    timer_reads: usize,
    writes: Vec<(u32, usize, u32)>,
}

impl Bus for Inputs {
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        if address == 0x1f801814 && width == Width::Word {
            self.gpu_reads += 1;
            return self.gpu.ok_or(BusError {
                address,
                detail: "GPU input deliberately unavailable".into(),
            });
        }
        if address == self.timer_address && width == Width::Word {
            self.timer_reads += 1;
            return self.timer.ok_or(BusError {
                address,
                detail: "timer input deliberately unavailable".into(),
            });
        }
        self.ram.read(address, width)
    }
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        self.writes.push((address, width.bytes(), value));
        self.ram.write(address, width, value)
    }
    fn write_masked(&mut self, address: u32, _: u32, _: u8) -> Result<(), BusError> {
        Err(BusError {
            address,
            detail: "masked writes are outside the bounded VSync fixture".into(),
        })
    }
}

#[derive(Debug, PartialEq, Eq)]
struct Outcome {
    registers: [u32; 32],
    hilo: (u32, u32),
    pcs: Vec<u32>,
    writes: Vec<(u32, usize, u32)>,
    ram_hash: String,
}

fn execute(
    exe: &Executable,
    argument: u32,
    gpu: Option<u32>,
    timer: Option<u32>,
    omit: bool,
    counter: u32,
) -> (Outcome, [usize; 2]) {
    let mut ram = Ram::from_executable(exe);
    assert_eq!(ram.read(0x801845f8, Width::Word).unwrap(), 0x1f801814);
    let timer_address = ram.read(0x801845fc, Width::Word).unwrap();
    assert_eq!(timer_address, 0x1f801110);
    let instruction = ram.read(GPU_LOAD, Width::Word).unwrap();
    assert_eq!(instruction, 0x8c500000); // lw s0,0(v0)
    let timer_instruction = ram.read(TIMER_LOAD, Width::Word).unwrap();
    assert_eq!(timer_instruction, 0x8c620000); // lw v0,0(v1)
    if omit {
        ram.write(GPU_LOAD, Width::Word, 0).unwrap();
        ram.write(TIMER_LOAD, Width::Word, 0).unwrap();
    } // Fixture-only NOP candidate.
    ram.write(0x80184600, Width::Word, 0xfffffff0).unwrap();
    ram.write(0x801856c4, Width::Word, counter).unwrap();
    let mut inputs = Inputs {
        ram,
        gpu,
        gpu_reads: 0,
        timer_address,
        timer,
        timer_reads: 0,
        writes: Vec::new(),
    };
    let mut cpu = Cpu::new(ENTRY);
    for register in 1..32 {
        cpu.set_register(register, 0xabc00000 + register as u32);
    }
    cpu.set_register(4, argument);
    cpu.set_register(29, 0x801ff000);
    cpu.set_register(31, RETURN);
    let mut pcs = Vec::new();
    for _ in 0..100 {
        if cpu.pc() == RETURN {
            break;
        }
        pcs.push(cpu.pc());
        cpu.step(&mut inputs).unwrap();
    }
    assert_eq!(cpu.pc(), RETURN);
    assert_eq!(cpu.register(2), counter);
    let expected: Vec<_> = (ENTRY..=0x8017474c)
        .step_by(4)
        .chain((0x80174830..=0x80174844).step_by(4))
        .collect();
    assert_eq!(
        pcs, expected,
        "negative branch must avoid waiting and positive-mode GPU users"
    );
    // Compare all guest data, excluding only the explicitly substituted code words.
    let mut normalized = inputs.ram.bytes().to_vec();
    let at = ram_offset(GPU_LOAD, 4).unwrap();
    normalized[at..at + 4].copy_from_slice(&instruction.to_le_bytes());
    normalized[at + 4..at + 8].copy_from_slice(&timer_instruction.to_le_bytes());
    (
        Outcome {
            registers: std::array::from_fn(|i| cpu.register(i)),
            hilo: (cpu.hi(), cpu.lo()),
            pcs,
            writes: inputs.writes,
            ram_hash: bof3_audio::digest::sha256_hex(&normalized),
        },
        [inputs.gpu_reads, inputs.timer_reads],
    )
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original negative VSync peripheral dependencies and fixture-only NOP candidate"]
fn negative_vsync_does_not_use_gpu_or_timer_inputs() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    for argument in [0x80000000, 0xffff0000, u32::MAX] {
        let (reference, reads) = execute(&exe, argument, Some(0), Some(0), false, 0x12345678);
        assert_eq!(reads, [1, 1]);
        for timer in [0, 0xffff, u32::MAX] {
            for gpu in [0, u32::MAX, 0x14802000, 0xa5a55a5a] {
                let (observed, reads) =
                    execute(&exe, argument, Some(gpu), Some(timer), false, 0x12345678);
                assert_eq!(observed, reference);
                assert_eq!(reads, [1, 1]);
            }
        }
        let (candidate, reads) = execute(&exe, argument, None, None, true, 0x12345678);
        assert_eq!(candidate, reference);
        assert_eq!(reads, [0, 0]);
        for counter in [0, u32::MAX] {
            let (original, reads) = execute(
                &exe,
                argument,
                Some(u32::MAX),
                Some(u32::MAX),
                false,
                counter,
            );
            assert_eq!(reads, [1, 1]);
            let (candidate, reads) = execute(&exe, argument, None, None, true, counter);
            assert_eq!(candidate, original);
            assert_eq!(reads, [0, 0]);
        }
    }
}
