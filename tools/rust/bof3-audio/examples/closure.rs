//! Development-only direct closure and bounded post-bootstrap playback coverage.
use bof3_audio::{
    driver::closure::{self, Root},
    machine::{
        executable::{ram_offset, Executable},
        firmware::Image,
        music, output_clock,
        profile::Profile,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{
    collections::{BTreeMap, BTreeSet},
    fs,
};
fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 1 && args.len() != 4 {
        return Err("usage: closure US_EXE [BIOS EMI FRAMES]".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let profile = Profile::identify(&exe)?;
    let inventory = bof3_audio::driver::roots::inventory(&exe)?;
    let mut roots = inventory.roots.clone();
    let mut dynamic = None;
    let mut observed_executable = BTreeSet::new();
    if args.len() == 4 {
        let frames: u64 = args[3].to_str().ok_or("frames must be UTF-8")?.parse()?;
        if !(1..=44100 * 600).contains(&frames) {
            return Err("closure probe: frames must be 1..=26460000".into());
        }
        let archive = ArchiveImage::from_bytes(fs::read(&args[2])?)?;
        let mut prepared = music::prepare(
            &exe,
            Image::from_bytes(fs::read(&args[1])?)?,
            &archive,
            None,
            0,
        )?;
        for call in &prepared.calls {
            roots.push(Root{address:call.entry,reason:"original music preparation call; initialization itself is not dynamically observed".into()});
        }
        for (address, reason) in [
            (0x8016b9cc, "original sequence play"),
            (0x8016d534, "original sequence stop"),
            (0x801753dc, "guest idle service boundary"),
        ] {
            roots.push(Root {
                address,
                reason: reason.into(),
            });
        }
        let e = &mut prepared.execution;
        e.enable_output(output_clock::Model::PcsxReduxNtsc)?;
        let mut hits = BTreeMap::<u32, u64>::new();
        let mut indirect = BTreeMap::<(u32, u32), u64>::new();
        let mut regions = BTreeMap::<&str, u64>::new();
        let mut bios_calls = BTreeMap::<(u32, u32), u64>::new();
        let mut memory = BTreeMap::<(u32, u32, u8, &str), u64>::new();
        let mut previous = None;
        let mut reentries = BTreeMap::<(u32, u32), u64>::new();
        let mut observer = |e: &bof3_audio::machine::execution::Execution| -> Result<()> {
            let pc = e.cpu.pc();
            if matches!(pc, 0xa0 | 0xb0 | 0xc0) {
                *bios_calls.entry((pc, e.cpu.register(9))).or_default() += 1;
            }
            let location = closure::location(&exe, pc);
            if location.file_offset.is_some() {
                if let Some(before) = previous {
                    if closure::location(&exe, before).file_offset.is_none() {
                        *reentries.entry((before, pc)).or_default() += 1;
                    }
                }
            }
            previous = Some(pc);
            let region = if location.file_offset.is_some() {
                "executable"
            } else if Image::offset(pc).is_some() {
                "bios"
            } else {
                "other RAM"
            };
            *hits.entry(pc).or_default() += 1;
            *regions.entry(region).or_default() += 1;
            if let Some(offset) = location.file_offset {
                let ram = ram_offset(pc, 4)?;
                if e.bus.ram().bytes()[ram..ram + 4] != exe.bytes()[offset..offset + 4] {
                    return Err(
                        format!("closure probe: executable bytes modified at {pc:#010x}").into(),
                    );
                }
                let word = u32::from_le_bytes(exe.bytes()[offset..offset + 4].try_into()?);
                let access = match word >> 26 {
                    0x20 | 0x24 => Some((1, "read")),
                    0x21 | 0x25 => Some((2, "read")),
                    0x23 => Some((4, "read")),
                    0x22 | 0x26 => Some((4, "merge read")),
                    0x28 => Some((1, "write")),
                    0x29 => Some((2, "write")),
                    0x2b => Some((4, "write")),
                    0x2a | 0x2e => Some((4, "merge read/write")),
                    _ => None,
                };
                if let Some((width, kind)) = access {
                    let mut address = e
                        .cpu
                        .register(((word >> 21) & 31) as usize)
                        .wrapping_add(word as i16 as i32 as u32);
                    if kind.starts_with("merge") {
                        address &= !3;
                    }
                    *memory.entry((pc, address, width, kind)).or_default() += 1;
                }
                if word >> 26 == 0 && matches!(word & 63, 8 | 9) {
                    let register = ((word >> 21) & 31) as usize;
                    *indirect.entry((pc, e.cpu.register(register))).or_default() += 1;
                }
            }
            Ok(())
        };
        let play = e.call_observed(
            0x8016b9cc,
            [prepared.handle, 0, 1, 1],
            100000,
            &mut observer,
        )?;
        while e.output().unwrap().frames() < frames {
            e.call_observed(0x801753dc, [0; 4], 100000, &mut observer)?;
            e.take_audio_frames()?;
        }
        let stop = e.call_observed(
            0x8016d534,
            [prepared.handle, 0, 0, 0],
            100000,
            &mut observer,
        )?;
        observed_executable.extend(
            hits.keys()
                .copied()
                .filter(|&pc| closure::location(&exe, pc).file_offset.is_some()),
        );
        for &(source, target) in reentries.keys() {
            roots.push(Root {address:target,reason:format!("observed re-entry after external PC {source:#010x}; predecessor can be a delay slot, not a callsite")});
        }
        for &(source, target) in indirect.keys() {
            if closure::location(&exe, target).file_offset.is_some() {
                roots.push(Root{address:target,reason:format!("observed register target from {source:#010x}; may be a return continuation, not a function entry")});
            }
        }
        dynamic = Some(
            serde_json::json!({"requested_frames":frames,"actual_frames":e.output().unwrap().frames(),"boot":prepared.boot,"identity":prepared.identity,"play":play,"stop":stop,"initialization_observed":false,"regions":regions,
            "bios_calls":bios_calls.iter().map(|(&(vector,selector),&count)|serde_json::json!({"vector":vector,"selector":selector,"count":count})).collect::<Vec<_>>(),
            "locations":hits.iter().map(|(&address,&count)|serde_json::json!({"location":closure::location(&exe,address),"count":count})).collect::<Vec<_>>(),
            "external_reentries":reentries.iter().map(|(&(source,target),&count)|serde_json::json!({"after_external_pc":source,"target":closure::location(&exe,target),"count":count})).collect::<Vec<_>>(),
            "memory_footprints":memory.iter().map(|(&(source,address,width,kind),&count)|serde_json::json!({"source":closure::location(&exe,source),"address":address,"width":width,"kind":kind,"count":count})).collect::<Vec<_>>(),
            "observed_register_targets":indirect.iter().map(|(&(source,target),&count)|serde_json::json!({"source":closure::location(&exe,source),"target":closure::location(&exe,target),"count":count})).collect::<Vec<_>>(),
            "limitations":["One sequence, post-bootstrap reference-clock interval only; initialization, other sequences, SFX and voice/XA coverage are incomplete.","Register targets and memory addresses are pre-instruction observations for unchanged executable-resident code; other RAM/BIOS memory accesses and full instruction-cache provenance remain unobserved."]}),
        );
    }
    let mut seen = BTreeSet::new();
    roots.retain(|r| seen.insert(r.address));
    let report = closure::audit(&exe, &roots, 200000)?;
    let included: BTreeSet<_> = report.instructions.iter().map(|p| p.address).collect();
    let observed_not_in_closure: Vec<_> = observed_executable
        .difference(&included)
        .map(|&p| closure::location(&exe, p))
        .collect();
    println!(
        "{}",
        serde_json::to_string_pretty(
            &serde_json::json!({"profile":profile,"inventory":inventory,"static":report,"dynamic":dynamic,"observed_not_in_closure":observed_not_in_closure})
        )?
    );
    Ok(())
}
