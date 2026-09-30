fn main() -> std::process::ExitCode {
    match bof3_audio::cli::run(std::env::args_os().skip(1)) {
        Ok(code) => std::process::ExitCode::from(code),
        Err(error) => {
            eprintln!("bof3-audio: {error}");
            std::process::ExitCode::from(2)
        }
    }
}
