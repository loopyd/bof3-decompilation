//! Trace original note/pitch calls and SPU writes for an edited zero-step fixture.
use bof3_audio::{
    bank::Bank,
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
        return Err("usage: zero_runtime_probe US_EXE BIOS BGM000_EMI".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let source = ArchiveImage::from_bytes(fs::read(&args[2])?)?;
    let mut header = source.entry(0)?.to_vec();
    for p in &Bank::parse(&header)?.programs {
        for t in &p.tones {
            let at = 0x820 + p.tone_block * 512 + t.index * 32;
            header[at + 4..at + 8].copy_from_slice(&[60, 0, 60, 60]);
        }
    }
    header[0x826..0x828].copy_from_slice(&[108, 108]);
    header[0x82c..0x82e].copy_from_slice(&[12, 12]);
    header[0x830..0x834].copy_from_slice(&[0x0f, 0, 0xc0, 0x1f]);
    let events = [
        0, 0xc0, 0, 0, 0x90, 108, 127, 48, 0xe0, 0, 0, 96, 0x90, 108, 0, 0, 0xff, 0x2f,
    ];
    let mut sep = source.entry(1)?[..19].to_vec();
    sep[8..10].copy_from_slice(&96u16.to_be_bytes());
    sep[10..13].copy_from_slice(&[7, 0xa1, 0x20]);
    sep[15..19].copy_from_slice(&(events.len() as u32).to_be_bytes());
    sep.extend(events);
    sep.resize(source.entry(1)?.len(), 0);
    let archive = source.replace_entries(&[(0, header.as_slice()), (1, sep.as_slice())])?;
    let mut prepared = music::prepare(
        &exe,
        Image::from_bytes(fs::read(&args[1])?)?,
        &archive,
        None,
        0,
    )?;
    let e = &mut prepared.execution;
    e.enable_output(output_clock::Model::PcsxReduxNtsc)?;
    let mut log = Vec::new();
    let mut returns = Vec::new();
    let mut observe = |e: &Execution| -> Result<()> {
        let pc = e.cpu.pc();
        let frame = e.output().unwrap().frames();
        if matches!(pc, 0x8017102c | 0x80171b20) {
            log.push(serde_json::json!({"kind":"entry","pc":pc,"frame":frame,
                "args":(4..8).map(|r|e.cpu.register(r)).collect::<Vec<_>>() }));
            returns.push(e.cpu.register(31));
        }
        if returns.last() == Some(&pc) {
            log.push(
                serde_json::json!({"kind":"return","pc":pc,"frame":frame,"v0":e.cpu.register(2)}),
            );
            returns.pop();
        }
        let ram = e.bus.ram().bytes();
        let at = ram_offset(pc, 4)?;
        let op = u32::from_le_bytes(ram[at..at + 4].try_into()?);
        if matches!(op >> 26, 0x29 | 0x2b) {
            let address = e
                .cpu
                .register(((op >> 21) & 31) as usize)
                .wrapping_add((op as i16 as i32) as u32)
                & 0x1fffffff;
            if ((0x1f801c00..0x1f801d80).contains(&address) && address & 15 == 4)
                || matches!(address, 0x1f801d88 | 0x1f801d8a)
            {
                log.push(serde_json::json!({"kind":"spu_write","pc":pc,"frame":frame,
                    "address":address,"value":e.cpu.register(((op >> 16) & 31) as usize)}));
            }
        }
        Ok(())
    };
    e.call_observed(0x8016b9cc, [prepared.handle, 0, 1, 1], 100000, &mut observe)?;
    let mut peak = 0;
    while e.output().unwrap().frames() < 44100 {
        e.call_observed(0x801753dc, [0; 4], 100000, &mut observe)?;
        for frame in e.take_audio_frames()? {
            for sample in frame {
                peak = peak.max(sample.unsigned_abs());
            }
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({"schema":"bof3.zero-runtime-probe/v1",
        "boot":prepared.boot,"identity":prepared.identity,"peak":peak,"events":log}))?
    );
    Ok(())
}
