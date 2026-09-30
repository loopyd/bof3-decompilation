//! Original game SFX dispatcher probe with source-qualified auxiliary data.
use bof3_audio::driver::trace::Trace;
use bof3_audio::{
    digest::sha256_hex,
    driver::closure::{self, Root},
    interchange::wave::Wave,
    machine::{
        bank, cues, effects,
        executable::{ram_offset, Executable},
        firmware::Image,
        output_clock,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{collections::BTreeSet, fs};
#[path = "support/events.rs"]
mod events;
use events::Action;

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 10 {
        return Err("usage: cues US_EXE BIOS EMI VH VB AUX EVENTS FRAMES TAIL_FRAMES WAV".into());
    }
    let number =
        |i: usize| -> Result<u32> { Ok(args[i].to_str().ok_or("non-UTF8 number")?.parse()?) };
    let frames = number(7)?;
    let tail = number(8)?;
    if frames == 0 || frames > 441_000 || tail > 441_000 {
        return Err("probe body must be 1..441000 frames and tail 0..441000".into());
    }
    let schedule = events::parse(
        args[6].to_str().ok_or("non-UTF8 events")?,
        u64::from(frames),
    )?;
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let archive = ArchiveImage::from_bytes(fs::read(&args[2])?)?;
    let mut trace = Trace::default();
    let mut prepared = bank::prepare(
        &exe,
        Image::from_bytes(fs::read(&args[1])?)?,
        &archive,
        &bank::Options {
            header_entry: number(3)? as usize,
            body_entry: number(4)? as usize,
            sequence_entry: None,
            layout: None,
        },
        &mut |stage, e| trace.observe(&exe, &format!("{stage:?}"), e),
    )?;
    let table = cues::stage(&mut prepared, &archive, number(5)? as usize)?;
    let callbacks = prepared
        .execution
        .call_observed(0x8015ce70, [0; 4], 100_000, &mut |e| {
            trace.observe(&exe, "Callbacks", e)
        })?;
    prepared
        .execution
        .enable_output(output_clock::Model::PcsxReduxNtsc)?;
    let mut event_reports = Vec::new();
    let mut event_entries = Vec::new();
    let mut voices = BTreeSet::new();
    let mut pcm = Vec::new();
    for event in &schedule {
        while prepared.execution.output().unwrap().frames() < event.frame {
            prepared
                .execution
                .call_observed(0x801753dc, [0; 4], 100_000, &mut |e| {
                    trace.observe(&exe, "Playback", e)
                })?;
            pcm.extend(
                prepared
                    .execution
                    .take_audio_frames()?
                    .into_iter()
                    .flatten(),
            );
        }
        let actual_frame = prepared.execution.output().unwrap().frames();
        let before = cues::state(&prepared)?;
        let mut notes = Vec::<[u32; 8]>::new();
        let mut volumes = Vec::<[u32; 3]>::new();
        let mut calls = Vec::new();
        match event.action {
            Action::Cue { row } => {
                calls.push(cues::dispatch(&mut prepared, &table, row, &mut |e| {
                    trace.observe(&exe, "Dispatch", e)?;
                    if e.cpu.pc() == effects::KEY_ON {
                        let at = ram_offset(e.cpu.register(29), 32)?;
                        notes.push(std::array::from_fn(|i| {
                            if i < 4 {
                                e.cpu.register(i + 4)
                            } else {
                                let off = at + i * 4;
                                u32::from_le_bytes(
                                    e.bus.ram().bytes()[off..off + 4].try_into().unwrap(),
                                )
                            }
                        }));
                    }
                    if e.cpu.pc() == 0x8016f8f8 {
                        volumes.push(std::array::from_fn(|i| e.cpu.register(i + 4)));
                    }
                    Ok(())
                })?);
                for note in &notes {
                    voices.insert(u8::try_from(note[0])?);
                }
            }
            Action::Poll => calls.push(cues::refresh(&mut prepared, &mut |e| {
                trace.observe(&exe, "Poll", e)
            })?),
            Action::Release => {
                for &voice in &voices {
                    calls.push(effects::key_off(&mut prepared, voice, &mut |e| {
                        trace.observe(&exe, "Release", e)
                    })?);
                }
                voices.clear();
            }
        }
        event_entries.extend(calls.iter().map(|c| c.entry));
        event_reports.push(
            serde_json::json!({"event":event,"actual_frame":actual_frame,"before":before,
            "after":cues::state(&prepared)?,"calls":calls,"notes":notes,"volume_calls":volumes}),
        );
    }
    while prepared.execution.output().unwrap().frames() < u64::from(frames) {
        prepared
            .execution
            .call_observed(0x801753dc, [0; 4], 100_000, &mut |e| {
                trace.observe(&exe, "Playback", e)
            })?;
        pcm.extend(
            prepared
                .execution
                .take_audio_frames()?
                .into_iter()
                .flatten(),
        );
    }
    let body_frames = prepared.execution.output().unwrap().frames();
    let mut releases = Vec::new();
    for voice in voices {
        releases.push(effects::key_off(&mut prepared, voice, &mut |e| {
            trace.observe(&exe, "Release", e)
        })?);
    }
    while prepared.execution.output().unwrap().frames() < body_frames + u64::from(tail) {
        prepared
            .execution
            .call_observed(0x801753dc, [0; 4], 100_000, &mut |e| {
                trace.observe(&exe, "Tail", e)
            })?;
        pcm.extend(
            prepared
                .execution
                .take_audio_frames()?
                .into_iter()
                .flatten(),
        );
    }
    pcm.extend(
        prepared
            .execution
            .take_audio_frames()?
            .into_iter()
            .flatten(),
    );
    let peak = pcm.iter().map(|&v| i32::from(v).abs()).max().unwrap_or(0);
    let nonzero = pcm.iter().filter(|&&v| v != 0).count();
    let wave = Wave::new(2, 44_100, pcm)?;
    let bytes = wave.to_bytes()?;
    let mut roots = prepared
        .calls
        .iter()
        .chain([&callbacks])
        .chain(&releases)
        .map(|c| Root {
            address: c.entry,
            reason: "host-directed original call".into(),
        })
        .collect::<Vec<_>>();
    roots.extend(event_entries.into_iter().map(|address| Root {
        address,
        reason: "explicit scheduled original cue/poll/release call".into(),
    }));
    roots.push(Root {
        address: 0x801753dc,
        reason: "guest idle service boundary".into(),
    });
    for &(source, target) in trace.indirect.keys().chain(trace.reentries.keys()) {
        if closure::location(&exe, target).file_offset.is_some() {
            roots.push(Root {address:target,reason:format!("observed register target or external re-entry after {source:#x}; not a function boundary")});
        }
    }
    let mut seen = BTreeSet::new();
    roots.retain(|r| seen.insert(r.address));
    let audit = closure::audit(&exe, &roots, 200_000)?;
    let covered: BTreeSet<_> = audit.instructions.iter().map(|i| i.address).collect();
    let missing: Vec<_> = trace
        .pcs
        .keys()
        .filter(|pc| !covered.contains(pc))
        .collect();
    let report = serde_json::json!({
        "schema":"bof3.audio.driver-cues/v2", "boot":prepared.boot,"identity":prepared.identity,"table":table,"schedule":schedule,
        "preparation":prepared.calls,"callbacks":callbacks,"events":event_reports,"releases":releases,
        "body_frames":body_frames,"tail_frames":tail,"frames":wave.frames(),"peak":peak,"nonzero_samples":nonzero,"wav_sha256":sha256_hex(&bytes),
        "observed_original_instructions":trace.pcs.len(),"closure":audit,"observed_not_in_closure":missing,
        "memory_footprints":trace.memory.iter().map(|((source,address,width,kind),count)|serde_json::json!({"source":source,"address":address,"width":width,"kind":kind,"count":count})).collect::<Vec<_>>(),
        "regions":trace.regions.iter().map(|((stage,region),count)|serde_json::json!({"stage":stage,"region":region,"count":count})).collect::<Vec<_>>(),
        "services":trace.services.iter().map(|((stage,vector,selector),count)|serde_json::json!({"stage":stage,"vector":vector,"selector":selector,"count":count})).collect::<Vec<_>>(),
        "indirect":trace.indirect.iter().map(|((source,target),count)|serde_json::json!({"source":source,"target":target,"count":count})).collect::<Vec<_>>(),
        "external_reentries":trace.reentries.iter().map(|((source,target),count)|serde_json::json!({"after_external_pc":source,"target":target,"count":count})).collect::<Vec<_>>(),
        "limitations":["Single bank, local records; cross-bank redirection and CD transport remain unsupported.","Explicit probe schedule; original main-loop poll cadence and broader arbitration remain unproven.","Memory footprints decode executable-resident instructions only; merged accesses report the aligned word, not exact byte enables. BIOS/kernel data and DMA accesses remain outside this trace.","BIOS boot excluded from observation; reference CPU clock, no independent fidelity or pruning acceptance."]
    });
    fs::write(&args[9], bytes)?;
    println!("{}", serde_json::to_string_pretty(&report)?);
    Ok(())
}
