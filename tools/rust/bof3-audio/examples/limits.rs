//! Investigate rejected source cues by observing the bounded original runtime.
//! Compares direct dispatch with cue preflight; it is not a render fallback.
use bof3_audio::{
    digest::sha256_hex,
    machine::{
        bank, cues, effects,
        executable::{ram_offset, Executable},
        firmware::Image,
        output_clock,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde_json::{json, Value};
use std::{collections::BTreeSet, fs, path::Path};

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 4 {
        return Err("usage: limits US_EXE BIOS CORPUS_ROOT COVERAGE_JSON".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let bios = fs::read(&args[1])?;
    let coverage: Value = serde_json::from_slice(&fs::read(&args[3])?)?;
    let mut reports = Vec::new();
    for table in coverage["tables"].as_array().ok_or("missing tables")? {
        let Some(rows) = table["report"]["rows"].as_array() else {
            continue;
        };
        for row in rows.iter().filter(|r| !r["error"].is_null()) {
            for bypass in [true, false] {
                let path = table["path"].as_str().ok_or("missing path")?;
                let archive = ArchiveImage::from_bytes(fs::read(Path::new(&args[2]).join(path))?)?;
                if Some(sha256_hex(archive.bytes()).as_str()) != table["sha256"].as_str() {
                    return Err("archive changed since coverage audit".into());
                }
                let mut p = bank::prepare(
                    &exe,
                    Image::from_bytes(bios.clone())?,
                    &archive,
                    &bank::Options {
                        header_entry: table["report"]["header"].as_u64().ok_or("missing header")?
                            as usize,
                        body_entry: table["report"]["body"].as_u64().ok_or("missing body")?
                            as usize,
                        sequence_entry: None,
                        layout: None,
                    },
                    &mut |_, _| Ok(()),
                )?;
                let aux = cues::stage(
                    &mut p,
                    &archive,
                    table["aux"].as_u64().ok_or("missing aux")? as usize,
                )?;
                let index = row["row"].as_u64().ok_or("missing row")? as usize;
                let record = aux.records.get(index).ok_or("missing record")?;
                p.execution.call(0x8015ce70, [0; 4], 100_000)?;
                p.execution
                    .enable_output(output_clock::Model::PcsxReduxNtsc)?;
                let mut calls = Vec::<Value>::new();
                let mut return_pc = None;
                let mut voices = BTreeSet::new();
                let mut observe = |e: &bof3_audio::machine::execution::Execution| -> Result<()> {
                    let pc = e.cpu.pc();
                    if pc == effects::KEY_ON {
                        let at = ram_offset(e.cpu.register(29), 32)?;
                        let arguments = std::array::from_fn::<_, 8, _>(|i| {
                            if i < 4 {
                                e.cpu.register(i + 4)
                            } else {
                                u32::from_le_bytes(
                                    e.bus.ram().bytes()[at + i * 4..at + i * 4 + 4]
                                        .try_into()
                                        .unwrap(),
                                )
                            }
                        });
                        voices.insert(arguments[0]);
                        calls.push(json!({"arguments": arguments}));
                        return_pc = Some(e.cpu.register(31));
                    } else if pc == 0x8016e5dc {
                        let address = e.cpu.register(2);
                        let at = ram_offset(address, 32)?;
                        calls.last_mut().ok_or("tone read without key-on")?["tone"] = json!({
                            "address": address,
                            "bytes": &e.bus.ram().bytes()[at..at + 32],
                        });
                    } else if Some(pc) == return_pc {
                        calls.last_mut().ok_or("return without key-on")?["result"] =
                            json!(e.cpu.register(2));
                        return_pc = None;
                    }
                    Ok(())
                };
                let result = if bypass {
                    p.execution.call_observed(
                        cues::DISPATCH,
                        [(u32::from(aux.game_bank_id) << 8) | index as u32, 0, 0, 0],
                        100_000,
                        &mut observe,
                    )
                } else {
                    cues::dispatch(&mut p, &aux, index, &mut observe)
                };
                let error = result.err().map(|e| e.to_string());
                let mut pcm = Vec::new();
                let rendering = (|| -> Result<()> {
                    pcm.extend(p.execution.take_audio_frames()?);
                    if error.is_none() {
                        while p.execution.output().unwrap().frames() < 1024 {
                            p.execution.call(0x801753dc, [0; 4], 100_000)?;
                            pcm.extend(p.execution.take_audio_frames()?);
                        }
                        for voice in voices {
                            effects::key_off(&mut p, voice.try_into()?, &mut |_| Ok(()))?;
                        }
                        while p.execution.output().unwrap().frames() < 45124 {
                            p.execution.call(0x801753dc, [0; 4], 100_000)?;
                            pcm.extend(p.execution.take_audio_frames()?);
                        }
                    }
                    Ok(())
                })();
                reports.push(json!({
                "adapter_preflight_bypassed": bypass,
                "path": path, "identity": p.identity, "row": index, "record": record,
                "declared_tones": p.bank.programs.iter().find(|p| p.program == record[1] & 127).map(|p| p.tones.len()),
                "calls": calls, "error": error,
                "render_error": rendering.err().map(|e| e.to_string()),
                "pcm_frames": pcm.len(), "peak": pcm.iter().flatten().map(|v| i32::from(*v).abs()).max(),
                "pcm_sha256": sha256_hex(&pcm.iter().flatten().flat_map(|s| s.to_le_bytes()).collect::<Vec<_>>()),
            }));
            }
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&json!({
        "schema": "bof3.audio.cue-limits/v2",
            "pruning_authorized": false, "reports": reports,
        }))?
    );
    Ok(())
}
