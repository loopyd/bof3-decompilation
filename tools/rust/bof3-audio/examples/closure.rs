//! Development-only direct closure, guest preparation and bounded playback coverage.
use bof3_audio::{
    driver::{
        closure::{self, Root},
        trace::Trace,
    },
    machine::{
        bank::Stage, executable::Executable, firmware::Image, music, output_clock, profile::Profile,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{collections::BTreeSet, fs};
fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 1 && args.len() != 5 {
        return Err("usage: closure US_EXE [BIOS EMI SEQUENCE FRAMES]".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let profile = Profile::identify(&exe)?;
    let inventory = bof3_audio::driver::roots::inventory(&exe)?;
    let mut roots = inventory.roots.clone();
    let mut dynamic = None;
    let mut observed_executable = BTreeSet::new();
    if args.len() == 5 {
        let sequence: usize = args[3].to_str().ok_or("sequence must be UTF-8")?.parse()?;
        let frames: u64 = args[4].to_str().ok_or("frames must be UTF-8")?.parse()?;
        if !(1..=44100 * 600).contains(&frames) {
            return Err("closure probe: frames must be 1..=26460000".into());
        }
        let archive = ArchiveImage::from_bytes(fs::read(&args[2])?)?;
        let mut trace = Trace::default();
        let mut prepared = music::prepare_observed(
            &exe,
            Image::from_bytes(fs::read(&args[1])?)?,
            &archive,
            None,
            sequence,
            &mut |stage, execution| trace.observe(&exe, stage_name(stage), execution),
        )?;
        for call in &prepared.calls {
            roots.push(Root {
                address: call.entry,
                reason: "observed original music preparation call".into(),
            });
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
        // Preparation's observer has no host-call boundary notification. Preserve
        // its raw external transitions, but do not infer re-entry roots from them.
        let preparation_transitions = std::mem::take(&mut trace.reentries);
        trace.begin_call();
        let play = e.call_observed(
            0x8016b9cc,
            [prepared.handle, sequence as u32, 1, 1],
            100000,
            &mut |e| trace.observe(&exe, "play", e),
        )?;
        while e.output().unwrap().frames() < frames {
            trace.begin_call();
            e.call_observed(0x801753dc, [0; 4], 100000, &mut |e| {
                trace.observe(&exe, "idle", e)
            })?;
            e.take_audio_frames()?;
        }
        trace.begin_call();
        let stop = e.call_observed(
            0x8016d534,
            [prepared.handle, sequence as u32, 0, 0],
            100000,
            &mut |e| trace.observe(&exe, "stop", e),
        )?;
        observed_executable.extend(
            trace
                .pcs
                .keys()
                .copied()
                .filter(|&pc| closure::location(&exe, pc).file_offset.is_some()),
        );
        for &(source, target) in trace.reentries.keys() {
            roots.push(Root {address:target,reason:format!("observed re-entry after external PC {source:#010x}; predecessor can be a delay slot, not a callsite")});
        }
        for &(source, target) in trace.indirect.keys() {
            if closure::location(&exe, target).file_offset.is_some() {
                roots.push(Root{address:target,reason:format!("observed register target from {source:#010x}; may be a return continuation, not a function entry")});
            }
        }
        dynamic = Some(serde_json::json!({
            "requested_frames": frames,
            "actual_frames": e.output().unwrap().frames(),
            "boot": prepared.boot,
            "identity": prepared.identity,
            "preparation_calls": prepared.calls,
            "play": play,
            "stop": stop,
            "initialization_observed": true,
            "preparation_host_call_boundaries_observed": false,
            "regions": trace.regions.iter().map(|((stage,region),count)|serde_json::json!({"stage":stage,"region":region,"count":count})).collect::<Vec<_>>(),
            "bios_calls": trace.services.iter().map(|((stage,vector,selector),count)|serde_json::json!({"stage":stage,"vector":vector,"selector":selector,"count":count})).collect::<Vec<_>>(),
            "locations": trace.pcs.iter().chain(&trace.external_pcs).map(|(&address,&count)|serde_json::json!({"location":closure::location(&exe,address),"count":count})).collect::<Vec<_>>(),
            "external_reentries": trace.reentries.iter().map(|(&(source,target),&count)|serde_json::json!({"after_external_pc":source,"target":closure::location(&exe,target),"count":count})).collect::<Vec<_>>(),
            "preparation_external_transitions": preparation_transitions.iter().map(|(&(source,target),&count)|serde_json::json!({"after_external_pc":source,"target":closure::location(&exe,target),"count":count})).collect::<Vec<_>>(),
            "memory_footprints": trace.memory.iter().map(|(&(source,address,width,kind),&count)|serde_json::json!({"source":closure::location(&exe,source),"address":address,"width":width,"kind":kind,"count":count})).collect::<Vec<_>>(),
            "observed_register_targets": trace.indirect.iter().map(|(&(source,target),&count)|serde_json::json!({"source":closure::location(&exe,source),"target":closure::location(&exe,target),"count":count})).collect::<Vec<_>>(),
            "limitations": [
                "One selected sequence and playback interval; other sequences, SFX and voice/XA histories remain incomplete.",
                "Preparation observes guest instructions after BIOS boot. Host staging, device-internal DMA accesses, incoming values/writers and BIOS bootstrap instructions are not observed.",
                "Preparation external transitions can cross host-directed calls and do not seed re-entry roots. Playback calls reset the shared trace at each host boundary.",
                "Register targets and memory footprints are pre-instruction observations for unchanged executable-resident code; they do not establish retirement or complete data closure."
            ]
        }));
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

fn stage_name(stage: Stage) -> &'static str {
    match stage {
        Stage::Initialization => "initialization",
        Stage::Layout => "layout",
        Stage::Header => "header",
        Stage::Samples => "samples",
        Stage::Sequence => "sequence",
        Stage::Callbacks => "callbacks",
    }
}
