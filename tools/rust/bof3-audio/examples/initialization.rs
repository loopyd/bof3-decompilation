//! Development evidence for BIOS services used by original audio preparation.
use bof3_audio::{
    driver::closure::{self, Root},
    machine::{
        bank::Stage,
        executable::{ram_offset, Executable},
        firmware::Image,
        music,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use std::{
    collections::{BTreeMap, BTreeSet},
    fs,
};
#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
struct Service {
    stage: Stage,
    vector: u32,
    selector: u32,
    return_pc: u32,
    arguments: [u32; 4],
    jump_buffer: Option<[u32; 12]>,
}
fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 3 {
        return Err("usage: initialization US_EXE BIOS EMI".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let image = ArchiveImage::from_bytes(fs::read(&args[2])?)?;
    let mut services = BTreeMap::<Service, u64>::new();
    let mut counts = BTreeMap::<Stage, u64>::new();
    let mut pcs = BTreeSet::new();
    let mut regions = BTreeMap::<(Stage, &str), u64>::new();
    let mut observer = |stage, e: &bof3_audio::machine::execution::Execution| -> Result<()> {
        let pc = e.cpu.pc();
        *counts.entry(stage).or_default() += 1;
        let region = if closure::location(&exe, pc).file_offset.is_some() {
            pcs.insert(pc);
            "executable"
        } else if Image::offset(pc).is_some() {
            "bios_rom"
        } else {
            "other_ram"
        };
        *regions.entry((stage, region)).or_default() += 1;
        if let Ok(vector) = ram_offset(pc, 4) {
            if matches!(vector, 0xa0 | 0xb0 | 0xc0) {
                let selector = e.cpu.register(9);
                let arguments = std::array::from_fn(|i| e.cpu.register(i + 4));
                let jump_buffer = if vector == 0xb0 && selector == 0x19 {
                    let at = ram_offset(arguments[0], 48)?;
                    Some(std::array::from_fn(|i| {
                        u32::from_le_bytes(
                            e.bus.ram().bytes()[at + i * 4..at + i * 4 + 4]
                                .try_into()
                                .unwrap(),
                        )
                    }))
                } else {
                    None
                };
                *services
                    .entry(Service {
                        stage,
                        vector: vector as u32,
                        selector,
                        return_pc: e.cpu.register(31),
                        arguments,
                        jump_buffer,
                    })
                    .or_default() += 1;
            }
        }
        Ok(())
    };
    let prepared = music::prepare_observed(
        &exe,
        Image::from_bytes(fs::read(&args[1])?)?,
        &image,
        None,
        0,
        &mut observer,
    )?;
    let mut roots = bof3_audio::driver::roots::inventory(&exe)?.roots;
    for call in &prepared.calls {
        roots.push(Root {
            address: call.entry,
            reason: "observed host-directed original preparation call".into(),
        });
    }
    // Every observed original PC is retained as evidence, not labeled a function.
    for address in &pcs {
        roots.push(Root {address:*address,reason:"original instruction observed during initialization/loading; not a function boundary".into()});
    }
    let mut seen = BTreeSet::new();
    roots.retain(|r| seen.insert(r.address));
    let audit = closure::audit(&exe, &roots, 200000)?;
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.audio.driver-initialization/v1","boot":prepared.boot,"identity":prepared.identity,
            "calls":prepared.calls,"stage_instructions":counts,"regions":regions.iter().map(|(&(stage,region),&count)|serde_json::json!({"stage":stage,"region":region,"count":count})).collect::<Vec<_>>(),
            "services":services.iter().map(|(service,count)|serde_json::json!({"service":service,"count":count})).collect::<Vec<_>>(),
            "observed_original_instructions":pcs.len(),"closure":audit,
            "limitations":["BIOS boot, host archive staging and layout scouting are not observed; these are guest preparation calls after boot.","Service arguments and HookEntryInt jump buffers are runtime evidence, not proof of all initialization inputs or a portable BIOS RAM image.","Observed PCs do not imply function boundaries; pruning and BIOS-free startup remain unaccepted."]
        }))?
    );
    Ok(())
}
