//! One linked-US sequence scheduling call, not a hardware clock or IRQ model.

use crate::Result;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Clock {
    pub delay: i32,
    pub quantum: i16,
    pub slow_counter: i16,
}

impl Clock {
    /// Execute the arithmetic/event order at US 0x8016CE0C. Each event callback
    /// supplies the next delay and may change the quantum (e.g. a tempo event).
    /// An exhausted budget fails after the already executed callbacks; callers
    /// must not publish a partial render as success.
    pub fn tick(
        &mut self,
        max_events: usize,
        mut event: impl FnMut(&mut Self) -> Result<()>,
    ) -> Result<usize> {
        let difference = self.delay.wrapping_sub(i32::from(self.quantum));
        if difference > 0 {
            if self.slow_counter > 0 {
                self.slow_counter -= 1;
            } else if self.slow_counter == 0 {
                self.slow_counter = self.quantum;
                self.delay = self.delay.wrapping_sub(1);
            } else {
                self.delay = difference;
            }
            return Ok(0);
        }
        // The original repeats this signed comparison after subtraction; it
        // differs from testing `difference` alone at overflow boundaries.
        if i32::from(self.quantum) < self.delay {
            return Ok(0);
        }
        let mut accumulated = self.delay;
        let mut events = 0;
        loop {
            if events == max_events {
                return Err(
                    format!("SEP scheduler exceeded {max_events} events in one tick").into(),
                );
            }
            event(self)?;
            events += 1;
            if self.delay == 0 {
                continue;
            }
            accumulated = accumulated.wrapping_add(self.delay);
            if accumulated >= i32::from(self.quantum) {
                self.delay = accumulated.wrapping_sub(i32::from(self.quantum));
                return Ok(events);
            }
        }
    }
}
