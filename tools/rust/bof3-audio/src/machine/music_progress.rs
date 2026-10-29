//! Read-only observations at explicitly bound sequence loop/end instructions.
use super::{executable::ram_offset, execution::Execution};
use crate::Result;
use serde::Serialize;

#[derive(Clone, Copy, Debug, Serialize)]
pub struct Instructions {
    pub loop_start: u32,
    pub infinite_loop: u32,
    pub end: u32,
}

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
    pub instructions: Instructions,
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
    pub fn new(
        instructions: Instructions,
        handle: u32,
        sequence: u32,
        record: u32,
        traversals: Option<u32>,
    ) -> Result<Self> {
        if handle >= 2 || sequence >= 4 || traversals == Some(0) {
            return Err("music progress: invalid handle, sequence or traversal limit".into());
        }
        ram_offset(record, 0xac)?;
        let points = [
            instructions.loop_start,
            instructions.infinite_loop,
            instructions.end,
        ];
        for &point in &points {
            ram_offset(point, 4)?;
            if point & 3 != 0 {
                return Err("music progress: unaligned observation instruction".into());
            }
        }
        if points[0] == points[1] || points[0] == points[2] || points[1] == points[2] {
            return Err("music progress: observation instructions overlap".into());
        }
        Ok(Self {
            instructions,
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
        if pc != self.instructions.loop_start
            && pc != self.instructions.infinite_loop
            && pc != self.instructions.end
        {
            return Ok(());
        }
        let cpu = &execution.cpu;
        if pc == self.instructions.end {
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
            pc if pc == self.instructions.loop_start => {
                self.loop_starts += 1;
                self.infinite_traversals = 0;
                self.saved_loop_cursor = Some(cpu.register(3));
                None
            }
            // This store is reached only on the 127-count infinite branch.
            pc if pc == self.instructions.infinite_loop => {
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
