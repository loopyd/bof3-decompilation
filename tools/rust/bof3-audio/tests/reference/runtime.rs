//! Offline reader for independent harness captures; never launches an emulator.
use bof3_audio::digest::sha256_hex;
use serde::Deserialize;
use serde_json::Value;
use std::{io::Read, path::Path};

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Registers {
    pub schema: String,
    pub pc: u32,
    pub registers: [u32; 32],
    pub cycles: u64,
}

fn read(path: &Path, limit: usize) -> Vec<u8> {
    let metadata = path.symlink_metadata().unwrap();
    assert!(
        metadata.is_file(),
        "not a regular capture: {}",
        path.display()
    );
    let mut bytes = Vec::new();
    std::fs::File::open(path)
        .unwrap()
        .take((limit + 1) as u64)
        .read_to_end(&mut bytes)
        .unwrap();
    assert!(
        bytes.len() <= limit,
        "oversized capture: {}",
        path.display()
    );
    bytes
}

pub fn capture(root: &Path, bios: &[u8], executable: &[u8]) -> (Registers, Vec<u8>) {
    let receipt: Value = serde_json::from_slice(&read(&root.join("receipt.json"), 65536)).unwrap();
    assert_eq!(receipt["schema"], "psx.runtime-session/v1");
    assert_eq!(receipt["status"], "passed");
    assert_eq!(receipt["process"]["exit_code"], 0);
    assert!(receipt["process"]["failure"].is_null());
    assert_eq!(
        receipt["emulator"]["revision"],
        "28438546c781fbe372a06399c82bed43ca2c6f4d"
    );
    for (name, bytes) in [("bios", bios), ("executable", executable)] {
        assert_eq!(receipt["inputs"][name]["sha256"], sha256_hex(bytes));
        assert_eq!(receipt["inputs"][name]["bytes"], bytes.len());
    }
    let script = read(&root.join("mission.lua"), 65536);
    assert_eq!(receipt["inputs"]["script"]["sha256"], sha256_hex(&script));
    assert_eq!(receipt["inputs"]["script"]["bytes"], script.len());
    assert_eq!(receipt["completion"]["schema"], "psx.runtime-completion/v1");
    let captures = receipt["completion"]["captures"].as_array().unwrap();
    assert_eq!(captures.len(), 2);
    let capture = |name: &str, limit| {
        let descriptors: Vec<_> = captures.iter().filter(|row| row["path"] == name).collect();
        assert_eq!(descriptors.len(), 1);
        let bytes = read(&root.join(name), limit);
        assert_eq!(descriptors[0]["bytes"], bytes.len());
        assert_eq!(descriptors[0]["sha256"], sha256_hex(&bytes));
        bytes
    };
    let registers: Registers = serde_json::from_slice(&capture("registers.json", 65536)).unwrap();
    assert_eq!(registers.schema, "psx.runtime-registers/v1");
    assert_eq!(registers.registers[0], 0);
    let ram = capture("ram.bin", 2 * 1024 * 1024);
    assert_eq!(ram.len(), 2 * 1024 * 1024);
    (registers, ram)
}

pub fn compare(
    cpu: &bof3_audio::machine::cpu::Cpu,
    ram: &[u8],
    expected: &Registers,
    expected_ram: &[u8],
    boundary: &str,
) {
    let registers: Vec<_> = (0..32)
        .filter(|&r| cpu.register(r) != expected.registers[r])
        .map(|r| serde_json::json!({"index": r, "rust": cpu.register(r), "redux": expected.registers[r]}))
        .collect();
    let differences = ram
        .iter()
        .zip(expected_ram)
        .enumerate()
        .filter(|(_, (actual, expected))| actual != expected);
    let count = differences.clone().count();
    let first: Vec<_> = differences.take(64)
        .map(|(offset, (actual, expected))| serde_json::json!({"offset": offset, "rust": actual, "redux": expected}))
        .collect();
    println!(
        "{}",
        serde_json::json!({
            "schema": "bof3.runtime-comparison/v1", "boundary": boundary,
            "register_differences": registers, "ram_differing_bytes": count,
            "first_differences": first, "rust_ram_sha256": sha256_hex(ram), "redux_ram_sha256": sha256_hex(expected_ram),
            "redux_cycles": expected.cycles, "rust_instructions": cpu.instructions(),
            "limits": "RAM, GPR and PC only; no timing equality, devices, complete CPU state or audio acceptance"
        })
    );
    assert_eq!(ram.len(), expected_ram.len());
    assert_eq!(cpu.pc(), expected.pc);
    assert!(
        registers.is_empty(),
        "integer register divergence at {boundary}"
    );
    assert_eq!(count, 0, "RAM divergence at {boundary}");
}
