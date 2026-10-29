//! Explicit original EXE/BIOS development execution. Production callers use driver::Image.
use super::{
    adsr, bank, boot, driver::Audio, executable::Executable, execution::Execution, firmware::Image,
    profile::Profile, spu_clock, spu_reverb, spu_sample, spu_voice_ports::DisableModel,
};
use crate::{catalog::loader, Result};
use emi_ex_v2::image::ArchiveImage;

pub const KEY_ON: u32 = 0x8016e400;
pub const DISPATCH: u32 = 0x8015df18;
pub const REFRESH: u32 = 0x8015d044;

fn audio(executable: &Executable) -> Result<Audio> {
    let profile = Profile::identify(executable)?;
    Ok(Audio {
        initialize: profile.sound_initialize,
        layout: 0x80161808,
        slot_table: 0x8014677c,
        header_capacities: 0x8018239c,
        sequence_capacities: 0x801823f0,
        open_head: 0x80173c50,
        transfer: 0x80174354,
        completed: 0x80174598,
        sequence_open: 0x8016b38c,
        callbacks: 0x8015ce70,
        sequence_handles: 0x80190308,
        play: 0x8016b9cc,
        stop: 0x8016d534,
        idle: 0x801753dc,
        key_on: KEY_ON,
        key_off: 0x8016e794,
        cue_dispatch: DISPATCH,
        cue_refresh: 0x8015d044,
        auxiliary_end: 0x80148a04,
        cue_gate: 0x8018232a,
        cue_work: 0x8018b308,
        voice_status: 0x8018e140,
        cue_lookup: 0x801821e0,
        tone_read: 0x8016e5dc,
        loop_start: 0x8016a308,
        infinite_loop: 0x8016a374,
        sequence_end: 0x8016cf1c,
    })
}

pub fn plan_bank<'a>(
    executable: &Executable,
    archive: &'a ArchiveImage,
    options: &bank::Options,
) -> Result<bank::Plan<'a>> {
    Profile::identify(executable)?;
    bank::plan_with_layout(archive, options, |selector| {
        loader::initialize_layout(executable, selector)
    })
}

pub fn prepare_bank(
    executable: &Executable,
    firmware: Image,
    archive: &ArchiveImage,
    options: &bank::Options,
    observer: &mut impl FnMut(bank::Stage, &Execution) -> Result<()>,
) -> Result<bank::Prepared<boot::Evidence>> {
    let audio = audio(executable)?;
    let plan = plan_bank(executable, archive, options)?;
    let machine = boot::load_us(firmware, executable, 3_000_000)?;
    let mut execution = Execution::new(
        machine.cpu,
        machine.bus,
        spu_clock::Model::EmulatorReference,
    );
    execution.bus.configure_spu_voices(
        spu_sample::Model::EmulatorReference,
        adsr::Model::EmulatorReference,
    )?;
    execution
        .bus
        .configure_spu_disable(DisableModel::EmulatorReference)?;
    execution
        .bus
        .configure_spu_reverb(spu_reverb::Model::EmulatorReference)?;
    bank::load(
        execution,
        machine.evidence,
        Vec::new(),
        audio,
        plan,
        observer,
    )
}

pub fn prepare_music(
    executable: &Executable,
    firmware: Image,
    archive: &ArchiveImage,
    layout: Option<u8>,
    sequence: usize,
) -> Result<super::music::Prepared<boot::Evidence>> {
    prepare_music_observed(
        executable,
        firmware,
        archive,
        layout,
        sequence,
        &mut |_, _| Ok(()),
    )
}

pub fn prepare_music_observed(
    executable: &Executable,
    firmware: Image,
    archive: &ArchiveImage,
    layout: Option<u8>,
    sequence: usize,
    observer: &mut impl FnMut(bank::Stage, &Execution) -> Result<()>,
) -> Result<super::music::Prepared<boot::Evidence>> {
    Profile::identify(executable)?;
    super::music::prepare_with(archive, layout, sequence, observer, |options, observer| {
        prepare_bank(executable, firmware, archive, options, observer)
    })
}

pub fn progress_instructions() -> super::music_progress::Instructions {
    super::music_progress::Instructions {
        loop_start: 0x8016a308,
        infinite_loop: 0x8016a374,
        end: 0x8016cf1c,
    }
}

pub fn render_music(
    executable: &Executable,
    firmware: Image,
    archive: &ArchiveImage,
    options: &crate::psx_render::Options,
) -> Result<crate::psx_render::Rendered<boot::Evidence>> {
    options.validate()?;
    let runtime = prepare_music(
        executable,
        firmware,
        archive,
        options.layout,
        options.sequence,
    )?;
    crate::psx_render::render_prepared(runtime, options, "original_us_runtime", vec![
        "Development original EXE/BIOS execution with host-fed archive buffers and explicit functional clocks.",
        "Output starts after initialization; independent PCM/native timing acceptance remains separate.",
    ])
}
