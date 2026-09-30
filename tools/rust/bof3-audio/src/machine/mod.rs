//! PSX executable loading and (eventually) original-runtime execution.
//! Parsing an executable does not establish a supported runtime profile.

pub mod adsr;
pub mod boot;
pub mod bus;
pub mod cd_audio;
pub mod cd_data;
pub mod cd_dma;
pub mod cd_drive;
pub mod cd_host;
pub mod cd_position;
pub mod cd_queue;
pub mod cop0;
pub mod cpu;
pub mod dma;
mod event_calls;
pub mod events;
pub mod exception_calls;
pub mod exception_chains;
pub mod executable;
pub mod execution;
pub mod firmware;
mod gaussian;
pub mod interconnect;
pub mod interrupts;
pub mod kernel;
pub mod music;
pub mod music_progress;
pub mod output_clock;
pub mod profile;
pub mod spu_clock;
pub mod spu_dma;
pub mod spu_mixer;
pub mod spu_noise;
pub mod spu_reverb;
pub mod spu_sample;
pub mod spu_transfer;
pub mod spu_voice_ports;
pub mod spu_voices;
pub mod spu_volume;
pub mod thread_context;
pub mod timers;
mod xa_filter;

pub mod bank;
pub mod cues;
pub mod effects;
