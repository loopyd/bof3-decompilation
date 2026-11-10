//! Convert an independently supplied original RAM capture to a frozen pitch context.
use bof3_audio::machine::{executable::Executable, reference::tuning};

fn main() -> bof3_audio::Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 2 {
        return Err(
            "usage: context US_EXE ORIGINAL_RAM; writes pitch-context JSON to stdout".into(),
        );
    }
    let exe = Executable::from_bytes(std::fs::read(&args[0])?)?;
    let context = tuning::frozen_context(&exe, &std::fs::read(&args[1])?)?;
    println!("{}", String::from_utf8(context.to_bytes()?)?);
    Ok(())
}
