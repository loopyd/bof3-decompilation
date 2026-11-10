//! PSX executable loading and (eventually) original-runtime execution.
//! Parsing an executable does not establish a supported runtime profile.

pub mod boot;
pub mod bus;
pub mod cd;
mod console;
pub mod cpu;
pub mod dma;
pub mod driver;
pub mod event_calls;
pub mod events;
pub mod exception_calls;
pub mod exception_chains;
pub mod execution;
pub mod gpu_status;
pub mod image;
pub mod interconnect;
pub mod interrupts;
pub mod kernel;
pub mod music;
pub mod music_progress;
pub mod native_queue;
pub mod output_clock;
pub mod reference;
pub mod spu;
pub mod thread_context;
pub mod timers;

pub mod bank;
pub mod cues;
pub mod effects;
