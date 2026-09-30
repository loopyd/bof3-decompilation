//! Read-only observations of verified US sequence loop/end instructions.
//! Selection and executable identity must be validated by music::prepare.
use super::{executable::ram_offset, execution::Execution};
use crate::Result;
use serde::Serialize;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Stop {
    InfiniteLoopLimit,
    EndMarker,
}

#[derive(Clone, Debug, Serialize)]
pub struct Boundary {
    pub reason: Stop,
    pub frame: u64,
    pub instruction: u64,
    pub pc: u32,
    pub cursor: u32,
}

#[derive(Debug, Serialize)]
pub struct Progress {
    pub handle: u32,
    pub sequence: u32,
    pub record: u32,
    pub requested_infinite_traversals: Option<u32>,
    pub loop_starts: u64,
    pub infinite_traversals: u64,
    pub total_infinite_traversals: u64,
    pub end_markers: u64,
    pub saved_loop_cursor: Option<u32>,
    pub boundary: Option<Boundary>,
}

impl Progress {
    pub fn new(handle: u32, sequence: u32, record: u32, traversals: Option<u32>) -> Result<Self> {
        if handle >= 2 || sequence >= 4 || traversals == Some(0) {
            return Err("music progress: invalid handle, sequence or traversal limit".into());
        }
        ram_offset(record, 0xac)?;
        Ok(Self {
            handle,
            sequence,
            record,
            requested_infinite_traversals: traversals,
            loop_starts: 0,
            infinite_traversals: 0,
            total_infinite_traversals: 0,
            end_markers: 0,
            saved_loop_cursor: None,
            boundary: None,
        })
    }

    pub fn observe(&mut self, execution: &Execution) -> Result<()> {
        let pc = execution.cpu.pc();
        if !matches!(pc, 0x8016a308 | 0x8016a374 | 0x8016cf1c) {
            return Ok(());
        }
        let cpu = &execution.cpu;
        if pc == 0x8016cf1c {
            if cpu.register(4) & 0xffff != self.handle || cpu.register(5) & 0xffff != self.sequence
            {
                return Ok(());
            }
        } else if cpu.register(16) != self.record {
            return Ok(());
        }
        let at = ram_offset(self.record, 0xac)?;
        let record = &execution.bus.ram().bytes()[at..at + 0xac];
        let word = |offset| u32::from_le_bytes(record[offset..offset + 4].try_into().unwrap());
        let reason = match pc {
            // Delay-slot store commits the saved cursor after the start delta.
            0x8016a308 => {
                self.loop_starts += 1;
                self.infinite_traversals = 0;
                self.saved_loop_cursor = Some(cpu.register(3));
                None
            }
            // This store is reached only on the 127-count infinite branch.
            0x8016a374 => {
                if record[0x28] != 127 || cpu.register(2) != word(0x0c) {
                    return Err("music progress: infinite-loop instruction state disagrees with verified US handler".into());
                }
                self.infinite_traversals += 1;
                self.total_infinite_traversals += 1;
                self.saved_loop_cursor = Some(cpu.register(2));
                (self
                    .requested_infinite_traversals
                    .is_some_and(|limit| self.infinite_traversals >= u64::from(limit)))
                .then_some(Stop::InfiniteLoopLimit)
            }
            _ => {
                self.end_markers += 1;
                Some(Stop::EndMarker)
            }
        };
        if self.boundary.is_none() {
            if let Some(reason) = reason {
                self.boundary = Some(Boundary {
                    reason,
                    frame: execution.output().map_or(0, |c| c.frames()),
                    instruction: cpu.instructions(),
                    pc,
                    cursor: word(4),
                });
            }
        }
        Ok(())
    }
}
