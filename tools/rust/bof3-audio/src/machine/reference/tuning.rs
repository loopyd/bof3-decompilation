//! Original EXE/RAM pitch evaluation and frozen-context conversion for development.
use crate::{
    machine::{bus::Ram, executable::Executable, profile::Profile},
    voice::tuning::{validate_tables, Bindings, FrozenPitchContext, Reference},
    Result,
};

pub fn from_executable(exe: &Executable) -> Result<Reference> {
    let profile = Profile::identify(exe)?;
    let ram = Ram::from_executable(exe);
    validate_tables(&ram, [profile.spu_pitch_table, profile.pitch_table])?;
    let bindings = Bindings {
        entry: profile.pitch_entry,
        tone_block: 0x8018e7df,
        tone_index: 0x8018e7e4,
        tone_pointer: 0x8018e25c,
        lookup_range: exe.header().load_address
            ..exe.header().load_address + exe.header().text_size as u32,
        source_base: exe.header().load_address,
        state: "isolated_executable",
    };
    Reference::new(profile, ram, bindings)
}

/// Check original code/table identity before exporting only the 86-byte gap.
pub fn frozen_context(exe: &Executable, runtime_ram: &[u8]) -> Result<FrozenPitchContext> {
    let profile = Profile::identify(exe)?;
    if runtime_ram.len() != 2 * 1024 * 1024 {
        return Err("tuning context: expected exactly 2 MiB of PSX RAM".into());
    }
    let original = Ram::from_executable(exe);
    for (address, size) in [
        (profile.pitch_entry, 252),
        (profile.pitch_table, 386),
        (profile.spu_pitch_table, 386),
    ] {
        let start = crate::machine::executable::ram_offset(address, size)?;
        if runtime_ram[start..start + size] != original.bytes()[start..start + size] {
            return Err(format!(
                "tuning context: original routine/table bytes differ at 0x{address:08X}"
            )
            .into());
        }
    }
    FrozenPitchContext::from_capture(runtime_ram)
}

pub fn from_runtime_ram(exe: &Executable, runtime_ram: &[u8]) -> Result<Reference> {
    let context = frozen_context(exe, runtime_ram)?;
    let mut reference = from_executable(exe)?;
    reference.apply_context(&context)?;
    Ok(reference)
}
