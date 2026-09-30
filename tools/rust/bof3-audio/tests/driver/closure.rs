use bof3_audio::{
    driver::closure::{audit, location, Root},
    machine::executable::Executable,
};
const BASE: u32 = 0x80010000;
fn executable(words: &[u32]) -> Executable {
    let mut bytes = vec![0; 0x800];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    bytes[0x10..0x14].copy_from_slice(&BASE.to_le_bytes());
    bytes[0x18..0x1c].copy_from_slice(&BASE.to_le_bytes());
    bytes[0x1c..0x20].copy_from_slice(&((words.len() * 4) as u32).to_le_bytes());
    for word in words {
        bytes.extend(word.to_le_bytes());
    }
    Executable::from_bytes(bytes).unwrap()
}
fn roots(address: u32) -> Vec<Root> {
    vec![Root {
        address,
        reason: "synthetic entry".into(),
    }]
}
#[test]
fn calls_and_proven_unconditional_branches_keep_delay_slots_without_linear_overreach() {
    let image = executable(&[
        0x0c000000 | ((BASE + 32) >> 2 & 0x03ffffff),
        0xaf820000,
        0x10000003,
        0,
        0xffffffff,
        0xffffffff,
        0x03e00008,
        0,
        0x03e00008,
        0x8f820004,
    ]);
    let report = audit(&image, &roots(BASE), 100).unwrap();
    assert_eq!(
        report
            .instructions
            .iter()
            .map(|p| p.address - BASE)
            .collect::<Vec<_>>(),
        [0, 4, 8, 12, 24, 28, 32, 36]
    );
    assert_eq!(
        report
            .delay_slots
            .iter()
            .map(|p| p.address - BASE)
            .collect::<Vec<_>>(),
        [4, 12, 28, 36]
    );
    assert_eq!(report.memory_instructions.len(), 2);
    assert_eq!(report.call_entries[0].address, BASE + 32);
    assert_eq!(report.call_entries[0].file_offset, Some(0x820));
    assert_eq!(report.unresolved.len(), 2);
    assert!(!report.data_closure_complete && !report.pruning_authorized);
}
#[test]
fn indirect_calls_exceptions_and_unknown_instructions_remain_explicit() {
    let image = executable(&[0x0320f809, 0, 0x0000000c, 0xffffffff]);
    let report = audit(&image, &roots(BASE), 100).unwrap();
    assert_eq!(report.instructions.len(), 4);
    assert!(report.unresolved.iter().any(|u| u.register == Some(25)));
    assert!(report
        .unresolved
        .iter()
        .any(|u| u.reason.contains("syscall")));
    assert!(report
        .unresolved
        .iter()
        .any(|u| u.reason.contains("unclassified")));
    assert!(report
        .edges
        .iter()
        .any(|e| e.kind == "indirect call continuation" && e.target.address == BASE + 8));
}
#[test]
fn delay_control_flow_and_external_targets_cannot_be_accepted_as_closed() {
    let image = executable(&[0x08000000 | ((BASE + 0x100) >> 2 & 0x03ffffff), 0x03e00008]);
    let report = audit(&image, &roots(BASE), 100).unwrap();
    assert!(report
        .unresolved
        .iter()
        .any(|u| u.reason.contains("delay slot")));
    assert!(report
        .unresolved
        .iter()
        .any(|u| u.location.address == BASE + 0x100 && u.location.file_offset.is_none()));
    assert!(!report.pruning_authorized);
}
#[test]
fn alias_offsets_and_bounds_are_checked_without_truncated_success() {
    let image = executable(&[0x03e00008, 0]);
    assert_eq!(location(&image, BASE + 0x20000000).file_offset, Some(0x800));
    assert_eq!(
        audit(&image, &roots(BASE + 0x20000000), 2)
            .unwrap()
            .instructions
            .len(),
        2
    );
    assert!(audit(&image, &roots(BASE), 1)
        .unwrap_err()
        .to_string()
        .contains("safety limit"));
    assert!(audit(&image, &roots(BASE + 1), 100).is_err());
    assert!(audit(&image, &roots(BASE + 8), 100).is_err());
    assert!(audit(&image, &[], 100).is_err());
}

#[test]
fn root_inventory_rejects_unidentified_executables() {
    assert!(bof3_audio::driver::roots::inventory(&executable(&[0x03e00008, 0])).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original SFX dispatcher table snapshot"]
fn original_dispatch_table_keeps_all_possible_reads_and_only_executable_candidates() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let inventory = bof3_audio::driver::roots::inventory(&exe).unwrap();
    assert_eq!(inventory.cue_table.len(), 16);
    let candidates: Vec<_> = inventory
        .cue_table
        .iter()
        .filter(|r| r.target.file_offset.is_some())
        .map(|r| r.target.address)
        .collect();
    assert_eq!(
        candidates,
        [0x8015e994, 0x8015efac, 0x8015f5c8, 0x8015fbe4, 0x80160200, 0x8016081c, 0x80160e38]
    );
    for row in &inventory.cue_table {
        assert_eq!(
            row.pointer.file_offset,
            Some((row.pointer.address - 0x80096000) as usize)
        );
        if row.target.file_offset.is_none() {
            assert!(!inventory
                .roots
                .iter()
                .any(|r| r.address == row.target.address));
        }
    }
    assert!(inventory
        .roots
        .iter()
        .any(|r| r.address == bof3_audio::xa::cue::CALLBACK));
}
