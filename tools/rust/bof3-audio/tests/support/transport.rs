//! Closure evidence for the explicitly supplied XA scheduler fixture.
#[path = "transport/machine.rs"]
pub mod machine;

use bof3_audio::{
    catalog::model::XaCue,
    digest::sha256_hex,
    driver::{
        closure::{self, Root},
        trace::Trace,
    },
    machine::{
        bus::{Bus, Width},
        cpu::Cpu,
        executable::Executable,
        interconnect::Interconnect,
    },
};
use std::collections::{BTreeMap, BTreeSet};

#[derive(Debug, PartialEq, Eq, serde::Serialize)]
pub struct Outcome {
    pub cue: u16,
    pub sectors_per_tick: usize,
    pub commands: Vec<(u8, Vec<u8>)>,
    pub states: Vec<u32>,
    pub sectors: usize,
    pub selected: usize,
    pub data: usize,
    pub frames: usize,
    pub stop_lba: u32,
    pub pcm_sha256: String,
}

type Service = (u32, u32, [u32; 4], u32);

pub struct Evidence {
    full_reset: bool,
    gpu: u32,
    vsync: BTreeMap<(u32, u32), u64>,
    boot: Option<bof3_audio::machine::boot::Evidence>,
    executable: Executable,
    trace: Trace,
    retired: bof3_audio::driver::retirement::Retirement,
    syscall_traps: BTreeMap<u32, u64>,
    entries: BTreeSet<u32>,
    first_lba: Option<u32>,
    sectors: Vec<u8>,
    services: BTreeMap<Service, u64>,
}

impl Evidence {
    pub fn new(
        executable: Executable,
        boot: Option<bof3_audio::machine::boot::Evidence>,
        full_reset: bool,
        gpu: u32,
    ) -> Self {
        Self {
            full_reset,
            gpu,
            vsync: BTreeMap::new(),
            boot,
            executable,
            trace: Trace::default(),
            retired: bof3_audio::driver::retirement::Retirement::default(),
            syscall_traps: BTreeMap::new(),
            entries: BTreeSet::new(),
            first_lba: None,
            sectors: Vec::new(),
            services: BTreeMap::new(),
        }
    }

    pub fn begin_call(&mut self, entry: u32) {
        self.entries.insert(entry);
        self.trace.begin_call();
    }

    pub fn observe_retired(
        &mut self,
        before: &bof3_audio::driver::retirement::Before,
        step: &bof3_audio::machine::cpu::Step,
    ) {
        self.retired
            .observe(&self.executable, before, step)
            .unwrap();
    }

    pub fn observe(&mut self, stage: &str, cpu: &Cpu, bus: &Interconnect) {
        if cpu.pc() == 0x80174700 {
            assert!(
                (cpu.register(4) as i32) < 0,
                "unproven nonnegative audio VSync context"
            );
            *self
                .vsync
                .entry((cpu.register(4), cpu.register(31)))
                .or_default() += 1;
        }
        let vector = cpu.pc() & 0x1fff_ffff;
        if matches!(vector, 0xa0 | 0xb0 | 0xc0) {
            let key = (
                vector,
                cpu.register(9),
                std::array::from_fn(|i| cpu.register(i + 4)),
                cpu.register(31),
            );
            *self.services.entry(key).or_default() += 1;
        }
        self.trace
            .observe_cpu(&self.executable, stage, cpu, bus)
            .unwrap();
    }

    pub fn observe_sector(&mut self, lba: u32, raw: &[u8; 2352]) {
        let first = *self.first_lba.get_or_insert(lba);
        assert_eq!(lba, first + (self.sectors.len() / 2352) as u32);
        self.sectors.extend_from_slice(raw);
    }

    pub fn observe_syscall(&mut self, fault: &bof3_audio::machine::cpu::Fault) {
        assert_eq!(fault.kind, bof3_audio::machine::cpu::FaultKind::Syscall);
        *self.syscall_traps.entry(fault.pc).or_default() += 1;
    }

    pub fn report(&self, cue: &XaCue, outcome: &Outcome, gpu_reads: usize) {
        assert!(!self.vsync.is_empty());
        assert_eq!(
            self.vsync.values().sum::<u64>(),
            gpu_reads as u64,
            "every GPU read must be explained by the observed negative VSync calls"
        );
        let mut roots: Vec<_> = self
            .entries
            .iter()
            .map(|&address| Root {
                address,
                reason: "explicit XA fixture guest call".into(),
            })
            .collect();
        for &(source, target) in self.trace.indirect.keys() {
            if closure::location(&self.executable, target)
                .file_offset
                .is_some()
            {
                roots.push(Root {
                    address: target,
                    reason: format!(
                        "XA observed indirect target from {source:#x}; not a function boundary"
                    ),
                });
            }
        }
        for &(source, target) in self.trace.reentries.keys() {
            roots.push(Root {
                address: target,
                reason: format!("XA re-entry after supplied kernel boundary {source:#x}"),
            });
        }
        let mut seen = BTreeSet::new();
        roots.retain(|root| seen.insert(root.address));
        let audit = closure::audit(&self.executable, &roots, 200_000).unwrap();
        let retained: BTreeSet<_> = audit.instructions.iter().map(|at| at.address).collect();
        let missing: Vec<_> = self
            .trace
            .pcs
            .keys()
            .filter(|pc| !retained.contains(pc))
            .collect();
        assert!(
            missing.is_empty(),
            "unaccounted original XA PCs: {missing:x?}"
        );
        for address in [
            bof3_audio::xa::cue::START,
            bof3_audio::xa::cue::SELECTOR,
            bof3_audio::xa::cue::TICK,
            bof3_audio::xa::cue::CALLBACK,
            0x80177264,
        ] {
            assert!(
                self.trace.pcs.contains_key(&address),
                "unobserved XA root {address:#x}"
            );
        }
        assert!(!audit.data_closure_complete && !audit.pruning_authorized);
        if self.boot.is_some() {
            assert!(self
                .trace
                .regions
                .iter()
                .any(|((_, region), &count)| *region == "bios_rom" && count > 0));
            for (vector, selector, count) in [(0xa0, 0x72, 1), (0xb0, 9, 5), (0xc0, 3, 2)] {
                assert_eq!(
                    self.services
                        .iter()
                        .filter(|((v, s, _, _), _)| *v == vector && *s == selector)
                        .map(|(_, n)| n)
                        .sum::<u64>(),
                    count
                );
            }
        }
        let trace = &self.trace;
        if self.boot.is_some() {
            assert_eq!(
                trace.regions.values().sum::<u64>(),
                self.retired.instructions.values().sum::<u64>() + self.syscall_traps.values().sum::<u64>(),
                "every original ROM-path pre-step observation must retire or raise its recorded syscall"
            );
        }
        let report = serde_json::json!({
            "schema": "bof3.audio.driver-transport/v3",
            "kernel_model": if self.boot.is_some() {"original_us_rom"} else {"supplied_kernel"},
            "boot": self.boot,
            "initialization": if self.full_reset {"device_reset"} else {"callbacks"},
            "gpu_input": {"value": self.gpu, "reads":gpu_reads},
            "vsync_calls": self.vsync.iter().map(|((argument,return_pc),count)| serde_json::json!({"argument":argument,"return_pc":closure::location(&self.executable,*return_pc),"count":count})).collect::<Vec<_>>(),
            "target": "exe/slus_004_22",
            "packed_id": cue.packed_id, "cue": cue, "sectors_per_tick": outcome.sectors_per_tick,
            "outcome": outcome,
            "media": {"source":cue.disc_path,"first_lba":self.first_lba,
                "sector_count":self.sectors.len()/2352,"raw_sector_bytes":2352,
                "consumed_sectors_sha256":sha256_hex(&self.sectors)},
            "observed_original_instructions": trace.pcs.len(),
            "pcs": trace.pcs.iter().map(|(&pc,&count)| serde_json::json!({"location":closure::location(&self.executable,pc),"count":count})).collect::<Vec<_>>(),
            "regions": trace.regions.iter().map(|((stage,region),count)| serde_json::json!({"stage":stage,"region":region,"count":count})).collect::<Vec<_>>(),
            "services": trace.services.iter().map(|((stage,vector,selector),count)| serde_json::json!({"stage":stage,"vector":vector,"selector":selector,"count":count})).collect::<Vec<_>>(),
            "service_calls": self.services.iter().map(|((vector,selector,arguments,return_pc),count)| serde_json::json!({"vector":vector,"selector":selector,"arguments":arguments,"return_pc":return_pc,"count":count})).collect::<Vec<_>>(),
            "indirect": trace.indirect.iter().map(|((source,target),count)| serde_json::json!({"source":closure::location(&self.executable,*source),"target":closure::location(&self.executable,*target),"count":count})).collect::<Vec<_>>(),
            "reentries": trace.reentries.iter().map(|((source,target),count)| serde_json::json!({"source":source,"target":closure::location(&self.executable,*target),"count":count})).collect::<Vec<_>>(),
            "memory": trace.memory.iter().map(|((pc,address,width,kind),count)| serde_json::json!({"instruction":closure::location(&self.executable,*pc),"address":address,"width":width,"kind":kind,"count":count})).collect::<Vec<_>>(),
            "retirement":self.retired.report(),
            "syscall_traps":self.syscall_traps.iter().map(|(pc,count)|serde_json::json!({"pc":pc,"count":count})).collect::<Vec<_>>(),
            "closure": audit,
            "limitations": [
                "Original EXE instructions checked against guest RAM; no independent emulator or hardware reference.",
                if self.full_reset { "Kernel RAM and exception vector derive from original US BIOS shell handoff; original CD reset mode 1 establishes SDK readiness without HLE service interception." } else if self.boot.is_some() { "Kernel RAM and exception vector derive from original US BIOS shell handoff; original CD reset mode 2 completes without HLE service interception. Full device reset is not executed." } else { "Explicit kernel RAM; hook initialization stops at unsupported A0:72. Pre-dispatch services are handled by the fixture kernel, not observed BIOS implementations." },
                if self.full_reset { "Original reset establishes SDK completion state; GPU status, initially spinning/authenticated drive, emulator-reference host reset state, dry SPU context, CD responses and tick/sector schedules remain supplied inputs." } else { "GPU status, ready drive/host, prior SDK completion, dry SPU context, CD responses and tick/sector schedules are supplied inputs." },
                "The memory field is a pre-step EXE-only footprint. Retirement records actual successful EXE, ROM and other-RAM steps with byte enables; synthetic kernel, host and DMA accesses remain excluded.",
                "Three first-cue cases do not establish full cue/history coverage, data ownership, physical timing, BIOS independence or pruning authority."
            ]
        });
        println!("{}", serde_json::to_string(&report).unwrap());
    }
}

pub fn configure_spu(bus: &mut Interconnect) {
    use bof3_audio::machine::{adsr, spu_sample};
    // Explicit dry SPU input, not BIOS initialization or game mixer acceptance.
    bus.configure_spu_voices(spu_sample::Model::Published, adsr::Model::Published)
        .unwrap();
    for (address, value) in [
        (0x1f801dac, 4),
        (0x1f801daa, 0x8011),
        (0x1f801db0, 0x4000),
        (0x1f801db2, 0x4000),
        (0x1f801d80, 0x2000),
        (0x1f801d82, 0x2000),
    ] {
        bus.write(address, Width::Half, value).unwrap();
    }
}
