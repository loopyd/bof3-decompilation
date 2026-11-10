//! Native CD capture ring, independent of callback output admission/consumption.

const CAPACITY: usize = 16384;
const FEEDING_CYCLES: u64 = 33_868_800 * 4 / 75;

pub(super) struct Capture {
    frames: Vec<[i16; 2]>,
    start: usize,
    end: usize,
    last_feed: u64,
}

impl Default for Capture {
    fn default() -> Self {
        Self {
            frames: vec![[0; 2]; CAPACITY],
            start: 0,
            end: 0,
            last_feed: 0,
        }
    }
}

impl Capture {
    pub(super) fn feed(&mut self, frames: &[[i16; 2]], cycle: u64) {
        self.last_feed = cycle;
        for &frame in frames {
            self.frames[self.end] = frame;
            self.end = (self.end + 1) % CAPACITY;
        }
    }

    pub(super) fn preview(&self, cycle: u64) -> Option<[i16; 2]> {
        if self.start != self.end {
            return Some(self.frames[self.start]);
        }
        if self.last_feed != 0 && cycle.wrapping_sub(self.last_feed) < FEEDING_CYCLES {
            return None;
        }
        Some([0; 2])
    }

    pub(super) fn consume(&mut self) {
        if self.start != self.end {
            self.start = (self.start + 1) % CAPACITY;
        }
    }
}
