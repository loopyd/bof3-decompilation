use bof3_disk_v2::{checksum, metadata, Image};
use std::env;
use std::ffi::OsString;
use std::path::{Path, PathBuf};

#[derive(Default)]
struct Options {
    input: Option<PathBuf>,
    output: Option<PathBuf>,
    extract: Option<PathBuf>,
    project: Option<PathBuf>,
    raw_root: Option<PathBuf>,
    cue: Option<PathBuf>,
    log: Option<PathBuf>,
    quiet: bool,
    verbose: bool,
}

fn main() {
    if let Err(message) = run(env::args_os().skip(1).collect()) {
        eprintln!("bof3-disk: {message}");
        std::process::exit(1);
    }
}

fn run(args: Vec<OsString>) -> Result<(), String> {
    if args.is_empty() || matches!(args[0].to_str(), Some("help" | "-h" | "--help")) {
        print_usage();
        return Ok(());
    }
    let command = args[0].to_str().ok_or("command must be valid UTF-8")?;
    let options = parse_options(&args[1..])?;
    match command {
        "extract" => extract(options),
        "lba-json" => lba_json(options),
        "checksum" => checksum(options),
        "rebuild" => rebuild(options),
        "verify" => verify(options),
        value => Err(format!(
            "unknown command: {value}; use extract, rebuild, lba-json, checksum, or verify"
        )),
    }
}

fn extract(options: Options) -> Result<(), String> {
    reject(
        &options,
        &[
            ('x', options.extract.is_some()),
            ('r', options.raw_root.is_some()),
            ('c', options.cue.is_some()),
        ],
    )?;
    let current = env::current_dir().map_err(|error| error.to_string())?;
    let explicit_input = options.input.is_some();
    let input = match options.input {
        Some(path) => path,
        None => discover_extract_input(&current.join("disk"))?,
    };
    let output = match options.output {
        Some(path) => path,
        None if !explicit_input => current.join("build/extracted"),
        None => return Err("extraction requires -o <output-dir>".into()),
    };
    let project = options.project.unwrap_or_else(|| {
        let file_name = format!("{}.xml", file_stem(&input));
        if explicit_input {
            output.join(file_name)
        } else {
            current.join("build").join(file_name)
        }
    });
    let mut image = Image::open(&input).map_err(display_error(&input))?;
    let entries = image.entries().map_err(display_error(&input))?;
    Image::open(&input)
        .and_then(|image| image.extract(&output))
        .map_err(display_error(&input))?;
    metadata::write_project(
        &project,
        &entries,
        input
            .file_name()
            .and_then(|value| value.to_str())
            .unwrap_or("disc.bin"),
    )
    .map_err(|error| error.to_string())?;
    if !options.quiet {
        eprintln!(
            "extracted {} files to {}",
            entries.iter().filter(|value| !value.is_directory).count(),
            output.display()
        );
        eprintln!("project XML: {}", project.display());
    }
    Ok(())
}

fn discover_extract_input(disk_dir: &Path) -> Result<PathBuf, String> {
    if !disk_dir.is_dir() {
        return Err(format!("directory not found: {}", disk_dir.display()));
    }

    for (extension, label) in [
        ("cue", "cue sheets"),
        ("iso", "ISO images"),
        ("bin", "BIN files"),
    ] {
        let mut matches = std::fs::read_dir(disk_dir)
            .map_err(|error| error.to_string())?
            .filter_map(Result::ok)
            .map(|entry| entry.path())
            .filter(|path| {
                path.is_file()
                    && path
                        .extension()
                        .is_some_and(|value| value.eq_ignore_ascii_case(extension))
            })
            .collect::<Vec<_>>();
        matches.sort();
        match matches.as_slice() {
            [path] => return Ok(path.clone()),
            [] => {}
            _ => {
                return Err(format!(
                    "multiple {label} found in {}; use -i to specify input",
                    disk_dir.display()
                ));
            }
        }
    }

    Err(format!(
        "no supported input image found in {}",
        disk_dir.display()
    ))
}

fn file_stem(path: &Path) -> &str {
    path.file_stem()
        .and_then(|value| value.to_str())
        .unwrap_or("disc")
}

fn rebuild(options: Options) -> Result<(), String> {
    reject(
        &options,
        &[
            ('x', options.extract.is_some()),
            ('p', options.project.is_some()),
            ('r', options.raw_root.is_some()),
            ('c', options.cue.is_some()),
            ('l', options.log.is_some()),
        ],
    )?;
    let input = options.input.ok_or("rebuild requires -i <input-dir>")?;
    let output = options.output.ok_or("rebuild requires -o <output.iso>")?;
    bof3_disk_v2::rebuild::iso(&input, &output).map_err(display_error(&input))?;
    if !options.quiet {
        eprintln!("wrote synthetic ISO: {}", output.display());
    }
    Ok(())
}

fn lba_json(options: Options) -> Result<(), String> {
    reject(&options, &[('c', options.cue.is_some())])?;
    let input = options.input.ok_or("lba-json requires -i <input-image>")?;
    let output = options.output.ok_or("lba-json requires -o <output-json>")?;
    let extract = options
        .extract
        .ok_or("lba-json requires -x <extract-output-dir>")?;
    let project = options
        .project
        .ok_or("lba-json requires -p <project-xml>")?;
    let raw_root = options.raw_root;
    let mut image = Image::open(&input).map_err(display_error(&input))?;
    let entries = image.entries().map_err(display_error(&input))?;
    Image::open(&input)
        .and_then(|image| image.extract(&extract))
        .map_err(display_error(&input))?;
    metadata::write_project(
        &project,
        &entries,
        input
            .file_name()
            .and_then(|value| value.to_str())
            .unwrap_or("disc.bin"),
    )
    .map_err(|error| error.to_string())?;
    metadata::write_lba_json(
        &output,
        &entries,
        raw_root
            .as_deref()
            .unwrap_or(Path::new("processed/emi_raw")),
        &extract,
    )
    .map_err(|error| error.to_string())?;
    if !options.quiet {
        eprintln!("wrote LBA JSON: {}", output.display());
    }
    Ok(())
}

fn checksum(options: Options) -> Result<(), String> {
    reject(
        &options,
        &[
            ('p', options.project.is_some()),
            ('x', options.extract.is_some()),
            ('r', options.raw_root.is_some()),
            ('c', options.cue.is_some()),
        ],
    )?;
    let input = options.input.ok_or("checksum requires -i <input-dir>")?;
    let output = options.output.ok_or("checksum requires -o <output-json>")?;
    let current = env::current_dir().map_err(|error| error.to_string())?;
    let records = checksum::scan(&input, &current).map_err(|error| error.to_string())?;
    checksum::write(&output, &records).map_err(|error| error.to_string())?;
    if !options.quiet {
        eprintln!("wrote disk checksums: {}", output.display());
    }
    Ok(())
}

fn verify(options: Options) -> Result<(), String> {
    reject(
        &options,
        &[
            ('p', options.project.is_some()),
            ('x', options.extract.is_some()),
            ('r', options.raw_root.is_some()),
            ('c', options.cue.is_some()),
        ],
    )?;
    let input = options.input.ok_or("verify requires -i <input-dir>")?;
    let checksums = options
        .output
        .ok_or("verify requires -o <checksums-json>")?;
    let current = env::current_dir().map_err(|error| error.to_string())?;
    let expected = checksum::read(&checksums).map_err(|error| error.to_string())?;
    let mismatches =
        checksum::verify(&input, &current, &expected).map_err(|error| error.to_string())?;
    if !mismatches.is_empty() {
        for mismatch in &mismatches {
            eprintln!("{mismatch}");
        }
        return Err(format!(
            "verify failed with {} mismatch(es)",
            mismatches.len()
        ));
    }
    if !options.quiet {
        eprintln!("verify passed: {} file(s) matched", expected.len());
    }
    Ok(())
}

fn parse_options(args: &[OsString]) -> Result<Options, String> {
    let mut options = Options::default();
    let mut index = 0;
    while index < args.len() {
        let flag = args[index].to_str().ok_or("options must be valid UTF-8")?;
        match flag {
            "-q" => options.quiet = true,
            "-v" => options.verbose = true,
            "-h" | "--help" => return Err("use `bof3-disk help`".into()),
            "-i" | "-o" | "-x" | "-p" | "-r" | "-c" | "-l" => {
                index += 1;
                let value = PathBuf::from(
                    args.get(index)
                        .ok_or_else(|| format!("option {flag} requires an argument"))?,
                );
                match flag {
                    "-i" => options.input = Some(value),
                    "-o" => options.output = Some(value),
                    "-x" => options.extract = Some(value),
                    "-p" => options.project = Some(value),
                    "-r" => options.raw_root = Some(value),
                    "-c" => options.cue = Some(value),
                    "-l" => options.log = Some(value),
                    _ => unreachable!(),
                }
            }
            value => return Err(format!("unknown option: {value}")),
        }
        index += 1;
    }
    Ok(options)
}

fn reject(_options: &Options, invalid: &[(char, bool)]) -> Result<(), String> {
    if let Some((flag, _)) = invalid.iter().find(|(_, present)| *present) {
        return Err(format!("-{flag} is not valid with this command"));
    }
    Ok(())
}

fn display_error(path: &Path) -> impl FnOnce(bof3_disk_v2::Error) -> String + '_ {
    move |error| format!("{}: {error}", path.display())
}

fn print_usage() {
    println!("Usage: bof3-disk <extract|rebuild|lba-json|checksum|verify> [options]");
    println!("  -i input  -o output  -x extract-dir  -p project.xml  -r raw-root");
    println!("  -c cue    -l log     -q quiet        -v verbose");
}
