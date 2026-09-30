//! Bounded direct control-flow closure with explicit unresolved boundaries.
//! This is instruction reachability, not recovered function or data ownership.
use super::flow::{decode, Flow};
use crate::{
    digest::sha256_hex,
    machine::executable::{ram_offset, Executable},
    Result,
};
use serde::Serialize;
use std::collections::{BTreeMap, BTreeSet, VecDeque};

#[derive(Clone, Debug, Serialize)]
pub struct Root {
    pub address: u32,
    pub reason: String,
}
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
pub struct Location {
    pub address: u32,
    pub file_offset: Option<usize>,
}
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
pub struct Edge {
    pub source: Location,
    pub target: Location,
    pub kind: &'static str,
}
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
pub struct Unresolved {
    pub location: Location,
    pub reason: &'static str,
    pub register: Option<u8>,
}
#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub executable_sha256: String,
    pub roots: Vec<Root>,
    pub instructions: Vec<Location>,
    pub call_entries: Vec<Location>,
    pub delay_slots: Vec<Location>,
    pub edges: Vec<Edge>,
    pub unresolved: Vec<Unresolved>,
    pub memory_instructions: Vec<Location>,
    pub system_instructions: Vec<Location>,
    pub data_closure_complete: bool,
    pub pruning_authorized: bool,
    pub limitations: Vec<&'static str>,
}

pub fn location(executable: &Executable, address: u32) -> Location {
    let file_offset = ram_offset(address, 4).ok().and_then(|physical| {
        let load = ram_offset(executable.header().load_address, 4).ok()?;
        let offset = physical.checked_sub(load)?;
        (offset.checked_add(4)? <= executable.text().len()).then_some(offset + 0x800)
    });
    Location {
        address,
        file_offset,
    }
}

pub fn audit(executable: &Executable, roots: &[Root], limit: usize) -> Result<Report> {
    if roots.is_empty() || roots.len() > 4096 || !(1..=1_000_000).contains(&limit) {
        return Err(
            "driver closure: require 1..=4096 roots and an instruction limit 1..=1000000".into(),
        );
    }
    if roots.iter().any(|root| {
        root.address & 3 != 0
            || root.reason.trim().is_empty()
            || location(executable, root.address).file_offset.is_none()
    }) {
        return Err(
            "driver closure: each root needs an aligned executable address and evidence reason"
                .into(),
        );
    }
    let mut queue: VecDeque<_> = roots.iter().map(|r| (r.address, false)).collect();
    let mut visited = BTreeSet::new();
    let mut nodes = BTreeMap::new();
    let mut edges = BTreeSet::new();
    let mut entries = BTreeSet::new();
    let mut delays = BTreeSet::new();
    let mut unresolved = BTreeSet::new();
    let mut memory = BTreeSet::new();
    let mut system = BTreeSet::new();
    while let Some((pc, delay)) = queue.pop_front() {
        if !visited.insert((pc, delay)) {
            continue;
        }
        let at = location(executable, pc);
        let Some(offset) = at.file_offset.filter(|_| pc & 3 == 0) else {
            unresolved.insert(Unresolved {
                location: at,
                reason: "target outside aligned executable payload",
                register: None,
            });
            continue;
        };
        if !nodes.contains_key(&pc) && nodes.len() >= limit {
            return Err(
                "driver closure: instruction safety limit reached; no complete report produced"
                    .into(),
            );
        }
        nodes.insert(pc, at.clone());
        let word = u32::from_le_bytes(executable.bytes()[offset..offset + 4].try_into()?);
        if matches!(word >> 26, 0x20..=0x26 | 0x28..=0x2b | 0x2e) {
            memory.insert(at.clone());
        }
        if word >> 26 == 0x10 {
            system.insert(at.clone());
        }
        let flow = decode(pc, word);
        if delay {
            delays.insert(at.clone());
            if !matches!(flow, Flow::Next) {
                unresolved.insert(Unresolved {
                    location: at,
                    reason: "control flow, trap or unknown instruction in delay slot",
                    register: None,
                });
            }
            continue;
        }
        let mut edge = |target, kind, is_delay| {
            let to = location(executable, target);
            edges.insert(Edge {
                source: at.clone(),
                target: to,
                kind,
            });
            queue.push_back((target, is_delay));
        };
        match flow {
            Flow::Next => edge(pc.wrapping_add(4), "next", false),
            Flow::Jump { target, link } => {
                edge(pc.wrapping_add(4), "delay", true);
                edge(target, if link { "call" } else { "jump" }, false);
                if link {
                    entries.insert(location(executable, target));
                    edge(pc.wrapping_add(8), "call continuation", false);
                }
            }
            Flow::Branch {
                target,
                taken,
                untaken,
                link,
            } => {
                edge(pc.wrapping_add(4), "delay", true);
                if taken {
                    edge(
                        target,
                        if link { "conditional call" } else { "branch" },
                        false,
                    );
                }
                if untaken || link {
                    edge(pc.wrapping_add(8), "branch continuation", false);
                }
                if link && taken {
                    entries.insert(location(executable, target));
                }
            }
            Flow::Indirect { register, link } => {
                edge(pc.wrapping_add(4), "delay", true);
                if link {
                    edge(pc.wrapping_add(8), "indirect call continuation", false);
                }
                unresolved.insert(Unresolved {
                    location: at,
                    reason: if register == 31 && !link {
                        "return register target requires call/context evidence"
                    } else {
                        "indirect target requires dispatch/context evidence"
                    },
                    register: Some(register),
                });
            }
            Flow::Trap(reason) => {
                edge(pc.wrapping_add(4), "possible exception return", false);
                unresolved.insert(Unresolved {
                    location: at,
                    reason,
                    register: None,
                });
            }
            Flow::Unknown => {
                unresolved.insert(Unresolved {
                    location: at,
                    reason: "unsupported or unclassified instruction",
                    register: None,
                });
            }
        }
    }
    Ok(Report {
        schema:"bof3.audio.driver-closure/v1", executable_sha256:sha256_hex(executable.bytes()), roots:roots.to_vec(),
        instructions:nodes.into_values().collect(), call_entries:entries.into_iter().collect(), delay_slots:delays.into_iter().collect(),
        edges:edges.into_iter().collect(), unresolved:unresolved.into_iter().collect(), memory_instructions:memory.into_iter().collect(), system_instructions:system.into_iter().collect(),
        data_closure_complete:false, pruning_authorized:false,
        limitations:vec!["Conservative direct instruction closure only; roots and call entries are not recovered function boundaries.", "Both conditional paths are included unless register identity proves one impossible; caller continuations do not prove callees return.", "Indirect calls, return targets, exceptions, memory references, runtime initialization and unclassified opcodes require further evidence.", "No code or data removal is authorized by this report, including instructions absent from dynamic coverage."],
    })
}
