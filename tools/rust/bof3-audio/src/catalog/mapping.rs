//! Validated owned inputs for US catalog associations.
use super::model::{Layout, SchedulerEvidence, XaCue};
use crate::{
    digest::sha256_hex,
    machine::{
        bus::{Bus, Width},
        driver::Image,
        image::profile::Profile,
    },
    Result,
};

pub struct Mapping {
    pub(crate) profile: Profile,
    pub(crate) source_load_address: u32,
    pub(crate) layouts: Vec<Layout>,
    pub(crate) auxiliary_end: u32,
    pub(crate) lookup: [u8; 256],
    pub(crate) music: [u8; 662],
    pub(crate) xa_cues: Vec<XaCue>,
    pub(crate) xa_scheduler: SchedulerEvidence,
}

// Original US SLUS_004.22 source addresses, not relocated execution targets.
pub(crate) const XA_SOURCE_HANDLERS: [u32; 9] = [
    0x801639b8, 0x801639d0, 0x80163a54, 0x80163af0, 0x80163b74, 0x80163c58, 0x80163d20, 0x80163d80,
    0x80163e3c,
];

impl Mapping {
    pub fn read(image: &Image) -> Result<Self> {
        let mut ram = image.ram()?;
        let mut music = [0; 662];
        let start = image.data("audio_music_cue_table", music.len() as u32)?;
        if image.symbol("audio_music_cue_table_end")? != start + music.len() as u32 {
            return Err("owned music cue table has an invalid extent".into());
        }
        for (index, byte) in music.iter_mut().enumerate() {
            *byte = ram.read(start + index as u32, Width::Byte)? as u8;
        }
        if sha256_hex(&music) != "a5f56ee8ee693d1cdabb5fc544abe8219dcfbca90b5b7755b3cefcfb7870b230"
        {
            return Err("owned music cue table disagrees with reviewed US records".into());
        }
        let mut lookup = [0; 256];
        let start = image.data("audio_cue_lookup", lookup.len() as u32)?;
        for (index, byte) in lookup.iter_mut().enumerate() {
            *byte = ram.read(start + index as u32, Width::Byte)? as u8;
        }
        if sha256_hex(&lookup) != "b2969a94e5a91ea0858a8de4496dffb0dbbdf7dd6c40872063ef264e4ddd0569"
        {
            return Err("owned SFX lookup disagrees with reviewed US records".into());
        }
        let mut bases = [0; 3];
        let start = image.data("audio_xa_disc_lbas", 12)?;
        for (index, base) in bases.iter_mut().enumerate() {
            *base = ram.read(start + index as u32 * 4, Width::Word)?;
        }
        if bases != [65798, 20081, 106214] {
            return Err("owned XA LBAs disagree with verified US ISO directory extents".into());
        }
        let start = image.data("audio_xa_handlers", 36)?;
        for state in 0..9 {
            if ram.read(start + state * 4, Width::Word)?
                != image.entry(&format!("audio_xa_state_{state}"))?
            {
                return Err("owned XA scheduler has an invalid handler binding".into());
            }
        }
        let mut profile = Profile::us();
        profile.bootstrap = "owned_driver_catalog_mapping_playback_unverified";
        Ok(Self {
            profile,
            source_load_address: 0x80096800,
            layouts: (0..3)
                .map(|selector| crate::machine::audio::bank::layout(image, selector))
                .collect::<Result<_>>()?,
            auxiliary_end: image.audio()?.auxiliary_end,
            lookup,
            music,
            xa_cues: crate::xa::cue::read_owned_cues(image, bases)?,
            xa_scheduler: scheduler(),
        })
    }
}

pub(crate) fn scheduler() -> SchedulerEvidence {
    use crate::xa::cue;
    SchedulerEvidence {
        start_entry: cue::START,
        completion_callback: cue::CALLBACK,
        tick_entry: cue::TICK,
        state_table: cue::STATE_TABLE,
        state_entries: XA_SOURCE_HANDLERS,
        position_poll_period_ticks: 2,
        watchdog_threshold_ticks: 181,
        stop_condition: "state 5 enters state 6 when a completed GetlocL response converts to LBA >= runtime_stop_threshold_lba; state 6 reduces the CD mix to zero and requests Pause",
        timing_evidence: "original initialization, callback, position comparison, cancellation and watchdog paths tested with injected responses; drive/IRQ latency, tick frequency, buffered PCM and audible endpoints remain unverified",
    }
}
