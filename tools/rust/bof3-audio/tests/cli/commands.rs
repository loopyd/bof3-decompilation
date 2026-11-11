use std::process::Command;

#[test]
fn exposes_surface_and_refuses_incomplete_operations() {
    let help = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .arg("--help")
        .output()
        .unwrap();
    assert!(help.status.success());
    let help = String::from_utf8(help.stdout).unwrap();
    for operation in [
        "index", "query", "map", "extract", "render", "pack", "verify",
    ] {
        assert!(help.contains(operation));
    }
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["render", "--mode", "music"])
        .output()
        .unwrap();
    assert_eq!(result.status.code(), Some(2));
    assert!(String::from_utf8(result.stderr)
        .unwrap()
        .contains("not implemented"));
}

#[test]
fn rejects_ambiguous_cli_input() {
    for args in [
        vec!["verify"],
        vec!["verify", "--mode", "music", "--mode", "audio"],
        vec!["verify", "--mode", "vocals"],
        vec!["verify", "--mode", "audio", "--archive"],
        vec!["bogus"],
    ] {
        let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(&args)
            .output()
            .unwrap();
        assert_eq!(result.status.code(), Some(2), "{args:?}");
    }
}
