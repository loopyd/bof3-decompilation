//! Development evidence for an explicit original-runtime SFX tone.
use bof3_audio::{
    digest::sha256_hex,
    driver::closure::{self, Root},
    interchange::wave::Wave,
    machine::{
        bank,
        bus::{Bus, Width},
        effects::{self, Note},
        executable::Executable,
        execution::Execution,
        firmware::Image,
        output_clock,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{collections::BTreeSet, fs};

use bof3_audio::driver::trace::Trace;

fn registers(e: &mut Execution, voice: u8) -> Result<Vec<u32>> {
    (0..8)
        .map(|i| {
            e.bus
                .read(0x1f801c00 + u32::from(voice) * 16 + i * 2, Width::Half)
                .map_err(Into::into)
        })
        .collect()
}

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 15 {
        return Err("usage: effects US_EXE BIOS EMI VH VB VOICE PROGRAM TONE KEY FINE LEFT RIGHT FRAMES TAIL_FRAMES WAV".into());
    }
    let number = |i: usize| -> Result<u32> {
        Ok(args[i]
            .to_str()
            .ok_or("non-UTF8 numeric argument")?
            .parse()?)
    };
    let note = Note {
        voice: number(5)?.try_into()?,
        program: number(6)?.try_into()?,
        tone: number(7)?.try_into()?,
        key: number(8)?.try_into()?,
        fine: number(9)?.try_into()?,
        left: number(10)?.try_into()?,
        right: number(11)?.try_into()?,
    };
    let frames = number(12)?;
    let tail = number(13)?;
    if frames == 0 || frames > 441_000 || tail > 441_000 {
        return Err("probe body must be 1..441000 frames and tail 0..441000".into());
    }
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
    let callbacks = prepared
        .execution
        .call_observed(0x8015ce70, [0; 4], 100_000, &mut |e| {
            trace.observe(&exe, "Callbacks", e)
        })?;
    prepared
        .execution
        .enable_output(output_clock::Model::PcsxReduxNtsc)?;
    let key_on = effects::key_on(&mut prepared, note, &mut |e| {
        trace.observe(&exe, "KeyOn", e)
    })?;
    let e = &mut prepared.execution;
    let mut pcm = Vec::new();
    while e.output().unwrap().frames() < u64::from(frames) {
        e.call_observed(0x801753dc, [0; 4], 100_000, &mut |e| {
            trace.observe(&exe, "Playback", e)
        })?;
        pcm.extend(e.take_audio_frames()?.into_iter().flatten());
    }
    let body_frames = e.output().unwrap().frames();
    let playing_registers = registers(e, note.voice)?;
    let release = effects::key_off(&mut prepared, note.voice, &mut |e| {
        trace.observe(&exe, "Release", e)
    })?;
    let e = &mut prepared.execution;
    while e.output().unwrap().frames() < body_frames + u64::from(tail) {
        e.call_observed(0x801753dc, [0; 4], 100_000, &mut |e| {
            trace.observe(&exe, "Tail", e)
        })?;
        pcm.extend(e.take_audio_frames()?.into_iter().flatten());
    }
    pcm.extend(e.take_audio_frames()?.into_iter().flatten());
    let released_registers = registers(e, note.voice)?;
    let peak = pcm.iter().map(|&v| i32::from(v).abs()).max().unwrap_or(0);
    let nonzero = pcm.iter().filter(|&&v| v != 0).count();
    let wave = Wave::new(2, 44_100, pcm)?;
    let bytes = wave.to_bytes()?;
    let mut roots = prepared
        .calls
        .iter()
        .chain([&callbacks, &key_on, &release])
        .map(|call| Root {
            address: call.entry,
            reason: "host-directed original runtime call".into(),
        })
        .collect::<Vec<_>>();
    roots.push(Root {
        address: 0x801753dc,
        reason: "guest idle service boundary".into(),
    });
    for &(source, target) in trace.indirect.keys().chain(trace.reentries.keys()) {
        if closure::location(&exe, target).file_offset.is_some() {
            roots.push(Root { address: target, reason: format!("observed register target or external re-entry after {source:#x}; not a proven function entry") });
        }
    }
    let mut seen = BTreeSet::new();
    roots.retain(|root| seen.insert(root.address));
    let audit = closure::audit(&exe, &roots, 200_000)?;
    let covered: BTreeSet<_> = audit.instructions.iter().map(|at| at.address).collect();
    let missing: Vec<_> = trace
        .pcs
        .keys()
        .filter(|pc| !covered.contains(pc))
        .copied()
        .collect();
    let report = serde_json::json!({
        "schema":"bof3.audio.driver-effects/v1", "boot":prepared.boot,
        "identity":prepared.identity,"note":note,"calls":prepared.calls,
        "callbacks":callbacks,"key_on":key_on,"release":release,
        "requested_body_frames":frames,"body_frames":body_frames,"tail_frames":tail,
        "frames":wave.frames(),"peak":peak,"nonzero_samples":nonzero,"wav_sha256":sha256_hex(&bytes),
        "playing_registers":playing_registers,"released_registers":released_registers,
        "register_order":["left","right","pitch","start","adsr1","adsr2","envelope","repeat"],
        "observed_original_instructions":trace.pcs.len(),"closure":audit,
        "memory_footprints":trace.memory.iter().map(|((source,address,width,kind),count)|serde_json::json!({"source":source,"address":address,"width":width,"kind":kind,"count":count})).collect::<Vec<_>>(),
        "observed_not_in_closure":missing,
        "external_reentries":trace.reentries.iter().map(|((source,target),count)|serde_json::json!({"after_external_pc":source,"target":target,"count":count})).collect::<Vec<_>>(),
        "regions":trace.regions.iter().map(|((stage,region),count)|serde_json::json!({"stage":stage,"region":region,"count":count})).collect::<Vec<_>>(),
        "services":trace.services.iter().map(|((stage,vector,selector),count)|serde_json::json!({"stage":stage,"vector":vector,"selector":selector,"count":count})).collect::<Vec<_>>(),
        "indirect":trace.indirect.iter().map(|((source,target),count)|serde_json::json!({"source":source,"target":target,"count":count})).collect::<Vec<_>>(),
        "limitations":["Explicit SDK tone, not game cue dispatch or automatic voice allocation.","BIOS boot excluded from observation; BIOS and original EXE still required.","Memory footprints decode executable-resident instructions only; merged accesses report the aligned word, not exact byte enables. BIOS/kernel data and DMA accesses remain outside this trace.","Fixed body and release tail, reference instruction clock; not independent fidelity or pruning acceptance."]
    });
    fs::write(&args[14], bytes)?;
    println!("{}", serde_json::to_string_pretty(&report)?);
    Ok(())
}
