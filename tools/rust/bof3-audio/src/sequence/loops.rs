//! Linked-US controller loop transitions, independent of sample synthesis/timing.
//!
//! This models the single sequence-wide cursor/count context, not a loop stack.
//! Other NRPN and instrument edits remain unsupported by this owner.

use crate::Result;
use serde::Serialize;

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct LoopState {
    pub start_cursor: usize,
    pub selector: Option<u8>,
    pub count_pending: bool,
    pub count_active: bool,
    pub remaining: u8,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct Transition {
    pub cursor: usize,
    /// Encoded delta units; the runtime accumulator uses ten times this value.
    pub delay: u32,
    pub jumped: bool,
}

impl LoopState {
    /// Callers supply the initial saved cursor (runtime record +0x0C).
    /// No archive offset or sequence-restart position is inferred here.
    pub fn new(start_cursor: usize) -> Self {
        Self {
            start_cursor,
            selector: None,
            count_pending: false,
            count_active: false,
            remaining: 0,
        }
    }

    /// `next_cursor` points AFTER the following delta, just as the runtime's
    /// reader leaves it. A jump uses that delta, not the delta at its target.
    /// The original delta is consumed even when an infinite jump forces zero.
    pub fn apply(
        &mut self,
        controller: u8,
        value: u8,
        next_cursor: usize,
        following_delta: u32,
    ) -> Result<Transition> {
        if value > 127 {
            return Err("SEP loop controller: value exceeds seven bits".into());
        }
        let mut result = Transition {
            cursor: next_cursor,
            delay: following_delta,
            jumped: false,
        };
        match (controller, value) {
            (99, 20) => {
                self.selector = Some(20);
                self.count_pending = true;
                self.start_cursor = next_cursor;
            }
            (99, 30) => {
                self.selector = Some(30);
                match self.remaining {
                    0 => self.count_active = false,
                    1..=126 => {
                        self.remaining -= 1;
                        if self.remaining == 0 { self.count_active = false; }
                        else { result.cursor = self.start_cursor; result.jumped = true; }
                    }
                    _ => {
                        result.cursor = self.start_cursor;
                        result.delay = 0;
                        result.jumped = true;
                    }
                }
            }
            (6 | 98, _) if matches!(self.selector, Some(20 | 30)) => {
                if self.count_pending && !self.count_active {
                    self.remaining = value;
                    self.count_pending = false;
                    self.count_active = true;
                }
            }
            _ => return Err(format!("SEP controller {controller} value {value}: unsupported outside verified loop context").into()),
        }
        Ok(result)
    }
}
