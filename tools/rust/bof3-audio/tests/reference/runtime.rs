//! Offline v2 reader for fresh-EXE harness captures; never launches an emulator.
//! Select the expected tool with BOF3_AUDIO_REDUX_REVISION and
//! BOF3_AUDIO_REDUX_BINARY_SHA256. Restored-state origins are outside this reader.
use bof3_audio::digest::sha256_hex;
use serde::Deserialize;
use serde_json::Value;
use std::{collections::BTreeSet, io::Read, path::Path};

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Registers {
    pub schema: String,
    pub pc: u32,
    pub registers: [u32; 32],
    pub cycles: u64,
}

pub struct Capture {
    pub registers: Registers,
    pub ram: Vec<u8>,
    pub provenance: Value,
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

pub fn capture(root: &Path, bios: &[u8], executable: &[u8]) -> Capture {
    let receipt_bytes = read(&root.join("receipt.json"), 1024 * 1024);
    let receipt: Value = serde_json::from_slice(&receipt_bytes).unwrap();
    assert_eq!(receipt["schema"], "psx.runtime-session/v2");
    assert_eq!(receipt["status"], "passed");
    assert_eq!(receipt["process"]["exit_code"], 0);
    assert!(receipt["process"]["failure"].is_null());
    assert!(
        receipt["inputs"]["origin"].is_null(),
        "restored-state origins are outside this fresh-EXE reference reader"
    );
    assert!(
        receipt["inputs"]["executable"].is_object(),
        "fresh-EXE reference requires a captured executable identity"
    );
    for (field, variable, length) in [
        ("revision", "BOF3_AUDIO_REDUX_REVISION", 40),
        ("binary_sha256", "BOF3_AUDIO_REDUX_BINARY_SHA256", 64),
    ] {
        let expected = std::env::var(variable)
            .unwrap_or_else(|_| panic!("set {variable} to the selected reference identity"));
        assert_eq!(expected.len(), length);
        assert!(expected
            .bytes()
            .all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(&b)));
        assert_eq!(receipt["emulator"][field], expected);
    }
    for (name, bytes) in [("bios", bios), ("executable", executable)] {
        assert_eq!(receipt["inputs"][name]["sha256"], sha256_hex(bytes));
        assert_eq!(receipt["inputs"][name]["bytes"], bytes.len());
    }
    assert_eq!(
        receipt["emulator"]["resources"]["schema"],
        "psx.mission-bundle/v1"
    );
    assert_eq!(receipt["emulator"]["resources"]["state"]["version"], 6);
    assert_eq!(receipt["settings"]["schema"], "psx.runtime-settings/v1");
    match receipt["mission"]["kind"].as_str().unwrap() {
        "external" => {
            let script = read(&root.join("mission.lua"), 1024 * 1024);
            assert_eq!(receipt["inputs"]["script"]["sha256"], sha256_hex(&script));
            assert_eq!(receipt["inputs"]["script"]["bytes"], script.len());
        }
        "builtin" => {
            assert!(matches!(
                receipt["mission"]["name"].as_str(),
                Some("target" | "call")
            ));
            assert_eq!(
                receipt["mission"]["resources"],
                receipt["emulator"]["resources"]
            );
        }
        _ => panic!("unsupported reference mission kind"),
    }
    assert_eq!(receipt["completion"]["schema"], "psx.runtime-completion/v1");
    let captures = receipt["completion"]["captures"].as_array().unwrap();
    assert!((2..=128).contains(&captures.len()));
    let mut names = BTreeSet::new();
    let mut total = 0usize;
    for descriptor in captures {
        let name = descriptor["path"].as_str().unwrap();
        assert!(!name.is_empty() && name != "." && name != "..");
        assert!(name
            .bytes()
            .all(|b| b.is_ascii_alphanumeric() || b"_.-".contains(&b)));
        assert!(names.insert(name));
        let bytes = read(&root.join(name), 64 * 1024 * 1024);
        assert_eq!(descriptor["bytes"], bytes.len());
        assert_eq!(descriptor["sha256"], sha256_hex(&bytes));
        total += bytes.len();
        assert!(total <= 128 * 1024 * 1024);
    }
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
    let provenance = serde_json::json!({
        "receipt_sha256": sha256_hex(&receipt_bytes),
        "revision": receipt["emulator"]["revision"],
        "binary_sha256": receipt["emulator"]["binary_sha256"],
        "source_manifest_sha256": receipt["emulator"]["source_manifest_sha256"],
        "mission_bundle_sha256": receipt["emulator"]["resources"]["bundle_sha256"],
        "script_sha256": receipt["inputs"]["script"]["sha256"],
        "settings": receipt["settings"], "mission_kind": receipt["mission"]["kind"],
        "mission_name": receipt["mission"]["name"], "arguments": receipt["arguments"]
    });
    Capture {
        registers,
        ram,
        provenance,
    }
}

pub fn compare(cpu: &bof3_audio::machine::cpu::Cpu, ram: &[u8], capture: &Capture, boundary: &str) {
    let expected = &capture.registers;
    let expected_ram = &capture.ram;
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
            "schema": "bof3.runtime-comparison/v2", "boundary": boundary,
            "reference": capture.provenance,
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
