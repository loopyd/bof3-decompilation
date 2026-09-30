//! Diagnostic comparison of isolated pitch calls in EXE and initialized audio RAM.
//! This does not establish stable scheduling contexts or independent PCM fidelity.
use bof3_audio::{
    catalog::model::AssetData, catalog::model::Catalog, digest::sha256_hex, machine::bus::Bus,
    machine::bus::Width, machine::executable::Executable, machine::firmware::Image, machine::music,
    machine::output_clock, voice::tuning::KeyTuning, voice::tuning::Reference,
};
use emi_ex_v2::image::ArchiveImage;
use std::{
    collections::{BTreeMap, BTreeSet},
    fs,
    path::PathBuf,
};

fn main() -> bof3_audio::Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if !(4..=5).contains(&args.len()) {
        return Err(
            "usage: tuning_init_probe US_EXE BIOS BGM_EMI CORPUS_ROOT [PLAYBACK_FRAMES]".into(),
        );
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let bios_bytes = fs::read(&args[1])?;
    let firmware = Image::from_bytes(bios_bytes.clone())?;
    let archive = ArchiveImage::from_bytes(fs::read(&args[2])?)?;
    let mut prepared = music::prepare(&exe, firmware, &archive, None, 0)?;
    let mut reference = Reference::from_executable(&exe)?;
    let profile = reference.profile().clone();
    let execution = &mut prepared.execution;
    let playback_frames = args
        .get(4)
        .map(|v| v.to_string_lossy().parse::<u64>())
        .transpose()?
        .unwrap_or(0);
    if playback_frames > 44100 * 600 {
        return Err("probe playback exceeds 600-second bound".into());
    }
    let ranges = [profile.pitch_table, profile.spu_pitch_table]
        .map(|a| (a & 0x1fffffff)..((a & 0x1fffffff) + 386));
    let mut table_reads = BTreeMap::<(u32, u32), u64>::new();
    let mut table_writes = Vec::new();
    if playback_frames != 0 {
        execution.enable_output(output_clock::Model::PcsxReduxNtsc)?;
        let mut observer =
            |e: &bof3_audio::machine::execution::Execution| -> bof3_audio::Result<()> {
                let pc = e.cpu.pc();
                let code = match bof3_audio::machine::executable::ram_offset(pc, 4) {
                    Ok(at) => &e.bus.ram().bytes()[at..at + 4],
                    Err(_) => {
                        let address = pc & 0x1fff_ffff;
                        if !(0x1fc0_0000..=0x1fc7_fffc).contains(&address) {
                            return Err(
                                format!("probe cannot observe instruction at {pc:#010x}").into()
                            );
                        }
                        let at = (address - 0x1fc0_0000) as usize;
                        &bios_bytes[at..at + 4]
                    }
                };
                let op = u32::from_le_bytes(code.try_into()?);
                let address = e
                    .cpu
                    .register(((op >> 21) & 31) as usize)
                    .wrapping_add(op as i16 as i32 as u32)
                    & 0x1fffffff;
                let word = address & !3;
                if !ranges.iter().any(|r| word < r.end && word + 4 > r.start) {
                    return Ok(());
                }
                if op >> 26 == 0x25 {
                    *table_reads.entry((pc, address)).or_default() += 1;
                }
                if matches!(op >> 26, 0x28..=0x2b | 0x2e | 0x3a) {
                    table_writes.push(serde_json::json!({"pc":pc,"address":address,"opcode":op,
                    "frame":e.output().unwrap().frames()}));
                    return Err("original playback attempted a pitch-table store".into());
                }
                Ok(())
            };
        execution.call_observed(
            0x8016b9cc,
            [prepared.handle, 0, 1, 1],
            100000,
            &mut observer,
        )?;
        while execution.output().unwrap().frames() < playback_frames {
            execution.call_observed(0x801753dc, [0; 4], 100000, &mut observer)?;
            execution.take_audio_frames()?;
        }
    }
    let mut table_hashes = Vec::new();
    for address in [profile.pitch_table, profile.spu_pitch_table] {
        let offset = (address & 0x1fffffff) as usize;
        let hash = sha256_hex(&execution.bus.ram().bytes()[offset..offset + 386]);
        if hash != bof3_audio::machine::profile::US_PITCH_SHA256 {
            return Err("initialized pitch table changed".into());
        }
        table_hashes.push(serde_json::json!({"address":address,"sha256":hash}));
    }
    // Freeze interrupt-driven scheduling before synthetic tone probes. This is
    // a snapshot of the completed playback interval, not concurrent gameplay.
    let status = execution.cpu.cop0().status();
    execution.cpu.cop0_mut().write(12, status & !1)?;
    // Same isolated note-on inputs as Reference, applied after original initialization.
    execution.bus.write(0x8018e7df, Width::Byte, 0)?;
    execution.bus.write(0x8018e7e4, Width::Byte, 0)?;
    execution.bus.write(0x8018e25c, Width::Word, 0x80010000)?;
    let catalog = Catalog::read(Some(&PathBuf::from(&args[3])), &[])?;
    let mut keys = BTreeSet::new();
    for asset in &catalog.assets {
        if let AssetData::Bank { metadata, .. } = &asset.data {
            for program in &metadata.programs {
                for tone in &program.tones {
                    for key in tone.key_min..=tone.key_max {
                        keys.insert((key, tone.center, tone.shift));
                    }
                }
            }
        }
    }
    let mut changes = Vec::new();
    let mut range_failures = Vec::new();
    let mut alternate_roots = Vec::new();
    let mut changed_addresses = BTreeMap::<u32, (u16, u16, usize)>::new();
    for &(key, center, shift) in &keys {
        let before = reference.key(key, center, shift, 44100)?;
        let lookup = before.pitch_lookup.unwrap();
        execution
            .bus
            .write(0x80010004, Width::Byte, u32::from(center))?;
        execution
            .bus
            .write(0x80010005, Width::Byte, u32::from(shift))?;
        let value = execution.bus.read(lookup.runtime_address, Width::Half)? as u16;
        let mut reads = Vec::new();
        let call = execution.call_observed(
            profile.pitch_entry,
            [u32::from(key), 0, 0, 0],
            100,
            &mut |e| {
                let ram = e.bus.ram().bytes();
                let pc = bof3_audio::machine::executable::ram_offset(e.cpu.pc(), 4)?;
                let opcode = u32::from_le_bytes(ram[pc..pc + 4].try_into().unwrap());
                if opcode >> 26 == 0x25 {
                    let address = e
                        .cpu
                        .register(((opcode >> 21) & 31) as usize)
                        .wrapping_add((opcode as i16 as i32) as u32);
                    let at = bof3_audio::machine::executable::ram_offset(address, 2)?;
                    reads.push((
                        address,
                        u16::from_le_bytes(ram[at..at + 2].try_into().unwrap()),
                    ));
                }
                Ok(())
            },
        )?;
        if reads != [(lookup.runtime_address, value)] {
            return Err(
                format!("initialized lookup differs from EXE-address witness: {reads:?}").into(),
            );
        }
        if execution.output().is_some() {
            execution.take_audio_frames()?;
        }
        let after = KeyTuning::from_register(key, center, shift, call.result as u16, 44100)?;
        if value != lookup.value {
            changed_addresses
                .entry(lookup.runtime_address)
                .or_insert((lookup.value, value, 0))
                .2 += 1;
        }
        if before.pitch_register != after.pitch_register {
            changes.push(serde_json::json!({"key":key,"center":center,"shift":shift,
                "address":lookup.runtime_address,"before":before.pitch_register,"after":after.pitch_register}));
        }
        if before
            .sf2
            .as_ref()
            .is_some_and(|fit| fit.root_key != before.center.min(127))
        {
            alternate_roots.push(before.clone());
        }
        if before
            .unsupported
            .is_some_and(|reason| reason.contains("generator range"))
        {
            range_failures.push(before);
        }
    }
    let mut alternate_root_sources = Vec::new();
    for row in &alternate_roots {
        for asset in &catalog.assets {
            if let AssetData::Bank { metadata, .. } = &asset.data {
                for program in &metadata.programs {
                    for tone in &program.tones {
                        if tone.center == row.center
                            && tone.shift == row.shift
                            && (tone.key_min..=tone.key_max).contains(&row.key)
                        {
                            alternate_root_sources.push(serde_json::json!({"bank":asset.id,
                                "program":program.program,"tone":tone.index,"key":row.key}));
                        }
                    }
                }
            }
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.initialized-pitch-probe/v1", "executable_sha256":profile.exe_sha256,
            "boot":prepared.boot,"bank":prepared.identity,"initialization_calls":prepared.calls,"table_hashes":table_hashes,
            "playback_frames":playback_frames,"pitch_table_stores":table_writes,
            "instruction_observer":"RAM and immutable BIOS ROM; other execution regions reject",
            "synthetic_probe_state":"interrupt admission disabled after playback snapshot",
            "playback_table_reads":table_reads.iter().map(|(&(pc,address),&count)|serde_json::json!({"pc":pc,"address":address,"count":count})).collect::<Vec<_>>(),
            "distinct_keys":keys.len(),"changed_addresses":changed_addresses,
            "changed_returns":changes,"sf2_range_failures":range_failures,"alternate_root_tuning":alternate_roots,"alternate_root_sources":alternate_root_sources,
            "limitations":["Original initialization followed by isolated synthetic tone-context calls; no active scheduling.",
                "State-dependent lookups remain unsuitable for an unconditional static SoundFont."]
        }))?
    );
    Ok(())
}
