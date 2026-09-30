use bof3_audio::machine::executable::{ram_offset, Executable, RAM_BYTES};

fn image() -> Vec<u8> {
    let mut bytes = vec![0; 0x808];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    for (offset, value) in [
        (0x10, 0x8001_0000u32),
        (0x18, 0x8001_0000),
        (0x1c, 8),
        (0x30, 0x801f_ff00),
        (0x34, 0xf0),
    ] {
        bytes[offset..offset + 4].copy_from_slice(&value.to_le_bytes());
    }
    bytes[0x800..].copy_from_slice(&[8, 0, 0xe0, 3, 0, 0, 0, 0]); // jr ra; nop
    bytes
}

#[test]
fn loads_only_declared_payload_at_verified_ram_offset() {
    let mut bytes = image();
    bytes.extend_from_slice(b"opaque tail");
    let exe = Executable::from_bytes(bytes.clone()).unwrap();
    assert_eq!(exe.bytes(), bytes);
    assert_eq!(exe.stack_pointer(), Some(0x801f_fff0));
    let ram = exe.load_ram();
    assert_eq!(ram.len(), RAM_BYTES);
    assert_eq!(&ram[0x10000..0x10008], &bytes[0x800..0x808]);
    assert!(ram[..0x10000].iter().all(|&b| b == 0));
    assert!(ram[0x10008..].iter().all(|&b| b == 0));
}

#[test]
fn aliases_are_explicit_and_ranges_cannot_wrap() {
    for address in [0x10000, 0x8001_0000, 0xa001_0000] {
        assert_eq!(ram_offset(address, 4).unwrap(), 0x10000);
    }
    for address in [
        0x0020_0000,
        0x1f80_0000,
        0x1fc0_0000,
        0x8020_0000,
        0xc001_0000,
    ] {
        assert!(ram_offset(address, 4).is_err());
    }
    assert!(ram_offset(0x801f_fff0, 17).is_err());
    assert!(ram_offset(0x8001_0000, usize::MAX).is_err());
}

#[test]
fn malformed_images_are_rejected_before_loading() {
    let bytes = image();
    for length in [0, 7, 0x7ff, 0x800, 0x807] {
        assert!(Executable::from_bytes(bytes[..length].to_vec()).is_err());
    }
    for (offset, value) in [
        (0x10, 0x8001_0001u32),
        (0x18, 0x801f_fffc),
        (0x1c, u32::MAX),
        (0x30, 0xffff_fff0),
    ] {
        let mut bad = bytes.clone();
        bad[offset..offset + 4].copy_from_slice(&value.to_le_bytes());
        assert!(Executable::from_bytes(bad).is_err());
    }
}

#[test]
fn valid_but_unknown_executable_is_not_a_supported_profile() {
    let exe = Executable::from_bytes(image()).unwrap();
    let error = bof3_audio::machine::profile::Profile::identify(&exe)
        .unwrap_err()
        .to_string();
    assert!(error.contains("unsupported runtime profile"));
    assert!(error.contains("SHA-256"));
}

#[test]
#[ignore = "requires user-supplied US executable via BOF3_AUDIO_EXE"]
fn local_us_executable_loads_at_the_manifest_address() {
    let path = std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE");
    let exe = Executable::from_bytes(std::fs::read(path).unwrap()).unwrap();
    assert_eq!(exe.header().load_address, 0x8009_6800);
    assert_eq!(exe.header().entry_pc, 0x8014_aa0c);
    assert_eq!(exe.header().text_size, 0x160800);
    assert_eq!(exe.stack_pointer(), Some(0x801f_fff0));
    assert_eq!(&exe.load_ram()[0x96800..0x1f7000], exe.text());
}
