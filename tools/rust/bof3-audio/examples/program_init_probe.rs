//! Original SEP-open channel programs and subsequent note-on arguments.
use bof3_audio::{
    machine::{
        executable::{ram_offset, Executable},
        execution::Execution,
        firmware::Image,
        music, output_clock,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::fs;

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 3 {
        return Err("usage: program_init_probe US_EXE BIOS EMI".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let archive = ArchiveImage::from_bytes(fs::read(&args[2])?)?;
    let mut prepared = music::prepare(
        &exe,
        Image::from_bytes(fs::read(&args[1])?)?,
        &archive,
        None,
        0,
    )?;
    let record = ram_offset(prepared.record, 0xac)?;
    let initial = prepared.execution.bus.ram().bytes()[record..record + 0xac].to_vec();
    let e = &mut prepared.execution;
    e.enable_output(output_clock::Model::PcsxReduxNtsc)?;
    let mut notes = Vec::new();
    let mut returns = Vec::new();
    let mut observer = |e: &Execution| -> Result<()> {
        let pc = e.cpu.pc();
        if pc == 0x8017102c {
            let sp = ram_offset(e.cpu.register(29), 24)?;
            let ram = e.bus.ram().bytes();
            notes.push(serde_json::json!({"frame":e.output().unwrap().frames(),
                "arguments":(4..8).map(|r|e.cpu.register(r)).collect::<Vec<_>>(),
                "velocity":u32::from_le_bytes(ram[sp+16..sp+20].try_into()?),
                "pan":u32::from_le_bytes(ram[sp+20..sp+24].try_into()?),
                "channel_programs":&ram[record+0x2c..record+0x3c]}));
            returns.push((e.cpu.register(31), notes.len() - 1));
        } else if returns.last().is_some_and(|&(address, _)| pc == address) {
            let (_, index) = returns.pop().unwrap();
            notes[index]["return"] = serde_json::json!(e.cpu.register(2));
        }
        Ok(())
    };
    e.call_observed(
        0x8016b9cc,
        [prepared.handle, 0, 1, 1],
        100000,
        &mut observer,
    )?;
    let mut peak = 0;
    while e.output().unwrap().frames() < 88200 {
        e.call_observed(0x801753dc, [0; 4], 100000, &mut observer)?;
        for frame in e.take_audio_frames()? {
            for v in frame {
                peak = peak.max(v.unsigned_abs());
            }
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.initial-program-probe/v1","boot":prepared.boot,"identity":prepared.identity,
            "initialization_calls":prepared.calls,"record_address":prepared.record,
            "initial_record_hex":initial.iter().map(|b|format!("{b:02x}")).collect::<String>(),
            "initial_programs":&initial[0x2c..0x3c],"peak":peak,"notes":notes
        }))?
    );
    Ok(())
}
