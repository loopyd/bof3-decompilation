//! Original-US end-marker state changes, separate from controller loop counts.
//! Voice release and linked-sequence activation remain the execution owner's job.

use crate::sequence::loops::LoopState;
use serde::Serialize;

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct State {
    pub cursor: usize,
    pub restart_cursor: usize,
    pub play_count: i16,
    pub completed: u16,
    pub elapsed_units: u32,
    pub delay_units: i32,
    pub tick_quantum: i16,
    pub flags: u32,
    pub active: bool,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Action {
    Restart,
    /// The runtime additionally activates a configured successor and releases
    /// the current sequence's voices. This model does not emulate those calls.
    Stop,
}

impl State {
    /// Match the linked routine's wrapping halfword increment and signed compare.
    /// User-facing play counts still need representability validation; these raw
    /// fields deliberately expose the runtime's behavior at counter overflow.
    pub fn end(&mut self, loops: &mut LoopState) -> Action {
        self.completed = self.completed.wrapping_add(1);
        if self.play_count == 0 || (self.completed as i16) < self.play_count {
            self.cursor = self.restart_cursor;
            self.elapsed_units = 0;
            self.delay_units = 0;
            loops.count_pending = false;
            if self.play_count != 0 {
                loops.start_cursor = self.restart_cursor;
            }
            Action::Restart
        } else {
            self.flags = (self.flags & !0x0b) | 0x204;
            self.active = false;
            loops.start_cursor = self.restart_cursor;
            self.delay_units = i32::from(self.tick_quantum);
            Action::Stop
        }
    }
}
