//! Bounded execution-order SEP traversal, preserving original loop cursor rules.
//! Ticks are logical deltas, not quantized game scheduler calls or audio frames.

use crate::{
    sequence::events::{self, Events, Kind},
    sequence::loops::LoopState,
    Result,
};
use serde::Serialize;

#[derive(Clone, Debug)]
pub struct Limits {
    pub infinite_traversals: u32,
    pub max_events: usize,
    pub max_ticks: u64,
}

impl Default for Limits {
    fn default() -> Self {
        Self {
            infinite_traversals: 2,
            max_events: 1_000_000,
            max_ticks: 1_000_000_000,
        }
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct Step {
    /// Relative to the start of the selected sequence's data, AFTER its delta.
    pub source_cursor: usize,
    pub message_bytes: usize,
    pub tick: u64,
    pub status: u8,
    pub explicit_status: bool,
    pub kind: Kind,
    /// Original dispatcher result, even when policy stops an infinite loop.
    pub next_cursor: usize,
    pub consumed_delta: Option<u32>,
    pub next_delay: u32,
    pub jumped: bool,
    pub loops: LoopState,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
#[serde(tag = "reason", rename_all = "snake_case")]
pub enum Stop {
    EndMarker,
    InfiniteLoopLimit {
        source_cursor: usize,
        traversals: u32,
    },
}

#[derive(Clone, Debug, Serialize)]
pub struct Timeline {
    pub steps: Vec<Step>,
    pub stop: Stop,
    pub final_tick: u64,
    pub source_trailing_offset: usize,
    pub source_trailing_bytes: usize,
}

pub fn trace(bytes: &[u8], limits: &Limits) -> Result<Timeline> {
    if limits.infinite_traversals == 0 || limits.max_events == 0 {
        return Err("SEP timeline: positive loop traversals and event budget required".into());
    }
    let physical = Events::parse(bytes)?;
    // Opaque bytes after the physical EOT are preserved, never invented events.
    let stream = &bytes[..physical.trailing_offset];
    let (first_delta, mut cursor) = events::read_delta(stream, 0)?;
    validate_delay(first_delta)?;
    let mut tick = u64::from(first_delta);
    let mut loops = LoopState::new(cursor);
    let mut running = None;
    let mut traversals = 0u32;
    let mut steps = Vec::new();
    loop {
        if steps.len() == limits.max_events {
            return Err(format!(
                "SEP timeline: exceeded {} events at source byte {cursor}",
                limits.max_events
            )
            .into());
        }
        if tick > limits.max_ticks {
            return Err(format!(
                "SEP timeline: tick {tick} exceeds limit {}",
                limits.max_ticks
            )
            .into());
        }
        let source_cursor = cursor;
        let message = events::read_message(stream, cursor, running)?;
        running = Some(message.status);
        let mut next_cursor = message.end;
        let mut consumed_delta = None;
        let mut next_delay = 0;
        let mut jumped = false;
        let mut stop = None;
        if message.kind == Kind::End {
            // Whole-sequence play-count/restart and successor activation are
            // separate from controller loops; this trace ends at the first EOT.
            stop = Some(Stop::EndMarker);
        } else {
            let (delta, after_delta) = events::read_delta(stream, message.end)?;
            validate_delay(delta)?;
            consumed_delta = Some(delta);
            next_cursor = after_delta;
            next_delay = delta;
            if let Kind::Controller { controller, value } = message.kind {
                match controller {
                    6 | 98 | 99 => {
                        let transition = loops
                            .apply(controller, value, after_delta, delta)
                            .map_err(|e| format!("SEP timeline at source byte {cursor}: {e}"))?;
                        next_cursor = transition.cursor;
                        next_delay = transition.delay;
                        jumped = transition.jumped;
                        if controller == 99 && value == 20 {
                            traversals = 0;
                        }
                        if controller == 99 && value == 30 && loops.remaining == 127 {
                            traversals = traversals
                                .checked_add(1)
                                .ok_or("SEP timeline: traversal overflow")?;
                            if traversals == limits.infinite_traversals {
                                stop = Some(Stop::InfiniteLoopLimit {
                                    source_cursor: cursor,
                                    traversals,
                                });
                            }
                        }
                    }
                    // These callbacks have no loop cursor effects; their gain/
                    // pan transfer functions are not established by this trace.
                    7 | 10 => (),
                    _ => {
                        return Err(format!(
                        "SEP timeline at source byte {cursor}: unsupported controller {controller}"
                    )
                        .into())
                    }
                }
            }
        }
        steps.push(Step {
            source_cursor,
            message_bytes: message.end - source_cursor,
            tick,
            status: message.status,
            explicit_status: message.explicit_status,
            kind: message.kind,
            next_cursor,
            consumed_delta,
            next_delay,
            jumped,
            loops: loops.clone(),
        });
        if let Some(stop) = stop {
            return Ok(Timeline {
                steps,
                stop,
                final_tick: tick,
                source_trailing_offset: physical.trailing_offset,
                source_trailing_bytes: physical.trailing_bytes,
            });
        }
        tick = tick
            .checked_add(u64::from(next_delay))
            .ok_or("SEP timeline: tick overflow")?;
        cursor = next_cursor;
    }
}

fn validate_delay(delta: u32) -> Result<()> {
    if delta > i32::MAX as u32 / 10 {
        return Err(format!("SEP timeline: delta {delta} overflows the positive signed runtime delay after scaling by ten").into());
    }
    Ok(())
}
