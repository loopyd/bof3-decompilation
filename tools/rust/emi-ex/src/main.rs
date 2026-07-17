use emi_ex_v2::{
    guess_type, pack, pack_folder, pack_manifest_from, type_extension, Archive, PackEntry,
    PackFolderOptions,
};
use serde::Serialize;
use std::env;
use std::ffi::{OsStr, OsString};
use std::fs;
use std::path::{Path, PathBuf};

fn main() {
    if let Err(message) = run(env::args_os().skip(1).collect()) {
        eprintln!("emi-ex: {message}");
        std::process::exit(1);
    }
}

fn run(mut args: Vec<OsString>) -> Result<(), String> {
    if args
        .first()
        .is_some_and(|arg| arg == "--help" || arg == "-h")
    {
        print_usage();
        return Ok(());
    }
    let mode = match args.first().and_then(|arg| arg.to_str()) {
        Some("extract") => {
            args.remove(0);
            "extract"
        }
        Some("pack") => {
            args.remove(0);
            "pack"
        }
        Some("list") => {
            args.remove(0);
            "list"
        }
        _ => "extract",
    };
    match mode {
        "pack" => run_pack(&args),
        "list" => run_list(&args),
        _ => run_extract(&args),
    }
}

#[derive(Default)]
struct ExtractOptions {
    typed: bool,
    quiet: bool,
    dry_run: bool,
    print_manifest: bool,
    manifest: Option<Option<PathBuf>>,
    output: Option<PathBuf>,
    explicit_index: Option<usize>,
    positionals: Vec<OsString>,
}

fn run_extract(args: &[OsString]) -> Result<(), String> {
    let mut options = ExtractOptions::default();
    let mut index = 0;
    while index < args.len() {
        let value = args[index].to_str();
        match value {
            Some("-e" | "--extensions" | "--typed-extensions") => options.typed = true,
            Some("-q" | "--quiet") => options.quiet = true,
            Some("-d" | "--dry-run") => options.dry_run = true,
            Some("--print-manifest") => options.print_manifest = true,
            Some("-C" | "--no-color") => {}
            Some("-L" | "--log-file") => {
                index += 1;
                require_arg(args, index, value.unwrap())?;
            }
            Some("-o" | "--output") => {
                index += 1;
                options.output = Some(PathBuf::from(require_arg(args, index, value.unwrap())?));
            }
            Some("-n" | "--index") => {
                index += 1;
                options.explicit_index = Some(parse_usize(
                    require_arg(args, index, value.unwrap())?,
                    "extract index",
                )?);
            }
            Some("-J" | "--manifest-json") => {
                let path = optional_option_value(args, &mut index);
                options.manifest = Some(path.map(PathBuf::from));
            }
            Some(value) if value.starts_with('-') => return Err(format!("unknown option {value}")),
            _ => options.positionals.push(args[index].clone()),
        }
        index += 1;
    }
    if options.positionals.is_empty() || options.positionals.len() > 2 {
        return Err("usage: emi-ex extract [OPTIONS] <archive.EMI> [index]".into());
    }
    let source = PathBuf::from(&options.positionals[0]);
    let positional_index = options
        .positionals
        .get(1)
        .map(|value| parse_usize(value, "extract index"))
        .transpose()?;
    if options.explicit_index.is_some() && positional_index.is_some() {
        return Err("extract index was provided twice".into());
    }
    let entry_index = options.explicit_index.or(positional_index);
    if entry_index.is_some() && (options.manifest.is_some() || options.print_manifest) {
        return Err("manifest output is only supported when extracting all entries".into());
    }
    let archive =
        Archive::open(&source).map_err(|error| format!("{}: {error}", source.display()))?;
    let default_output = source.with_extension("");
    if let Some(entry_index) = entry_index {
        let entry = archive.entries().get(entry_index).ok_or_else(|| {
            format!(
                "entry index {entry_index} is out of range (archive has {} entries)",
                archive.entries().len()
            )
        })?;
        let output = match options.output {
            Some(path) if path.is_dir() => {
                path.join(entry_name(entry_index, entry.file_type, options.typed))
            }
            Some(path) => path,
            None => default_output.join(entry_name(entry_index, entry.file_type, options.typed)),
        };
        if !options.dry_run {
            archive
                .extract_entry(entry_index, output)
                .map_err(|error| error.to_string())?;
        }
        return Ok(());
    }

    let output = options.output.unwrap_or(default_output);
    if !options.dry_run {
        archive
            .extract_all(&output, options.typed)
            .map_err(|error| error.to_string())?;
    }
    let manifest = (options.print_manifest || options.manifest.is_some())
        .then(|| archive.manifest_json(options.typed));
    if options.print_manifest {
        println!("{}", manifest.as_deref().unwrap());
    }
    if let Some(requested) = options.manifest {
        let mut path = requested.unwrap_or_else(|| output.join("emi.json"));
        if path.is_dir() {
            path.push("emi.json");
        }
        if !options.dry_run {
            if let Some(parent) = path
                .parent()
                .filter(|parent| !parent.as_os_str().is_empty())
            {
                fs::create_dir_all(parent).map_err(|error| error.to_string())?;
            }
            fs::write(path, manifest.unwrap()).map_err(|error| error.to_string())?;
        }
    }
    if !options.quiet && !options.dry_run {
        eprintln!("extracted {} entries", archive.entries().len());
    }
    Ok(())
}

#[derive(Default)]
struct PackOptions {
    output: Option<PathBuf>,
    dry_run: bool,
    keep: bool,
    default_type: Option<u16>,
    include: Vec<String>,
    exclude: Vec<String>,
    manifest: Option<Option<PathBuf>>,
    folder: Option<PathBuf>,
    inputs: Vec<PackEntry>,
}

fn run_pack(args: &[OsString]) -> Result<(), String> {
    let mut options = PackOptions {
        keep: true,
        ..PackOptions::default()
    };
    let mut index = 0;
    while index < args.len() {
        let value = args[index]
            .to_str()
            .ok_or("pack arguments must be valid UTF-8")?;
        match value {
            "-o" | "--output" => {
                index += 1;
                options.output = Some(PathBuf::from(require_arg(args, index, value)?));
            }
            "-d" | "--dry-run" => options.dry_run = true,
            "-q" | "--quiet" | "-C" | "--no-color" => {}
            "-L" | "--log-file" => {
                index += 1;
                require_arg(args, index, value)?;
            }
            "-k" | "--keep-original" => options.keep = true,
            "-K" | "--no-keep" => options.keep = false,
            "-t" | "--type" => {
                index += 1;
                options.default_type = Some(parse_u16(require_arg(args, index, value)?, "type")?);
            }
            "-I" | "--include" | "-X" | "--exclude" => {
                index += 1;
                let pattern = require_arg(args, index, value)?
                    .to_string_lossy()
                    .into_owned();
                if value == "-I" || value == "--include" {
                    options.include.push(pattern);
                } else {
                    options.exclude.push(pattern);
                }
            }
            "-J" | "--manifest-json" => {
                options.manifest = Some(optional_option_value(args, &mut index).map(PathBuf::from));
            }
            "-i" | "--input" => {
                let start = index + 1;
                index = start;
                while index < args.len() && !is_option(&args[index]) {
                    let path = PathBuf::from(&args[index]);
                    options.inputs.push(PackEntry {
                        file_type: guess_type(&path).or(options.default_type).unwrap_or(255),
                        path,
                        ram_ptr: 0,
                    });
                    index += 1;
                }
                if index == start {
                    return Err("--input requires at least one file".into());
                }
                index -= 1;
            }
            value if value.starts_with('-') => return Err(format!("unknown option {value}")),
            _ => {
                if options.folder.is_some() || !options.inputs.is_empty() {
                    return Err(format!("unexpected positional in pack mode: {value}"));
                }
                options.folder = Some(PathBuf::from(&args[index]));
            }
        }
        index += 1;
    }
    let output = options.output.ok_or("pack requires -o <archive.EMI>")?;
    if options.folder.is_some() && !options.inputs.is_empty() {
        return Err("pack accepts either a folder or -i inputs, not both".into());
    }
    if options.manifest.is_some() && options.folder.is_none() {
        return Err("manifest input is only supported when packing a folder".into());
    }

    let packed_paths = if let Some(folder) = options.folder {
        if !folder.is_dir() {
            return Err(format!("input folder does not exist: {}", folder.display()));
        }
        if let Some(requested) = options.manifest {
            let manifest = requested.unwrap_or_else(|| folder.join("emi.json"));
            if options.dry_run {
                fs::metadata(&manifest).map_err(|error| error.to_string())?;
                Vec::new()
            } else {
                pack_manifest_from(&output, &folder, &manifest)
                    .map_err(|error| error.to_string())?;
                manifest_names(&folder, &manifest)?
            }
        } else {
            let mut exclude = options.exclude;
            if !exclude.iter().any(|pattern| pattern == "emi.json") {
                exclude.push("emi.json".into());
            }
            let folder_options = PackFolderOptions {
                default_type: options.default_type,
                include_patterns: options.include,
                exclude_patterns: exclude,
            };
            let paths = enumerate_paths(&folder, &folder_options)?;
            if !options.dry_run {
                pack_folder(&output, &folder, &folder_options)
                    .map_err(|error| error.to_string())?;
            }
            paths
        }
    } else {
        if options.inputs.is_empty() {
            return Err("no input files or folder provided".into());
        }
        for input in &options.inputs {
            fs::metadata(&input.path).map_err(|error| error.to_string())?;
        }
        let paths = options
            .inputs
            .iter()
            .map(|entry| entry.path.clone())
            .collect();
        if !options.dry_run {
            pack(&output, &options.inputs, 0).map_err(|error| error.to_string())?;
        }
        paths
    };
    if !options.keep && !options.dry_run {
        for path in packed_paths {
            let _ = fs::remove_file(path);
        }
    }
    Ok(())
}

fn enumerate_paths(folder: &Path, options: &PackFolderOptions) -> Result<Vec<PathBuf>, String> {
    let mut paths = Vec::new();
    for entry in fs::read_dir(folder).map_err(|error| error.to_string())? {
        let entry = entry.map_err(|error| error.to_string())?;
        if !entry
            .file_type()
            .map_err(|error| error.to_string())?
            .is_file()
        {
            continue;
        }
        let name = entry.file_name().to_string_lossy().into_owned();
        let included = options.include_patterns.is_empty()
            || options
                .include_patterns
                .iter()
                .any(|pattern| emi_ex_v2::glob_match(pattern, &name));
        let excluded = options
            .exclude_patterns
            .iter()
            .any(|pattern| emi_ex_v2::glob_match(pattern, &name));
        if included && !excluded {
            paths.push(entry.path());
        }
    }
    paths.sort();
    if paths.is_empty() {
        return Err("folder contains no selected regular files".into());
    }
    Ok(paths)
}

fn manifest_names(folder: &Path, manifest: &Path) -> Result<Vec<PathBuf>, String> {
    let value: serde_json::Value =
        serde_json::from_slice(&fs::read(manifest).map_err(|error| error.to_string())?)
            .map_err(|error| error.to_string())?;
    Ok(value
        .get("entries")
        .and_then(|entries| entries.as_array())
        .into_iter()
        .flatten()
        .filter_map(|entry| {
            let name = entry.get("name")?.as_str()?;
            (!name.is_empty()).then(|| folder.join(name))
        })
        .collect())
}

fn entry_name(index: usize, file_type: u16, typed: bool) -> String {
    format!(
        "{index}.{}",
        if typed {
            type_extension(file_type)
        } else {
            "bin"
        }
    )
}

fn optional_option_value(args: &[OsString], index: &mut usize) -> Option<OsString> {
    let next = args.get(*index + 1)?;
    if is_option(next) {
        return None;
    }
    *index += 1;
    Some(next.clone())
}

fn is_option(value: &OsStr) -> bool {
    value.to_string_lossy().starts_with('-')
}

fn require_arg<'a>(args: &'a [OsString], index: usize, option: &str) -> Result<&'a OsStr, String> {
    args.get(index)
        .map(OsString::as_os_str)
        .ok_or_else(|| format!("{option} requires an argument"))
}

fn parse_usize(value: &OsStr, label: &str) -> Result<usize, String> {
    value
        .to_str()
        .ok_or_else(|| format!("{label} must be valid UTF-8"))?
        .parse()
        .map_err(|_| format!("invalid {label}"))
}

fn parse_u16(value: &OsStr, label: &str) -> Result<u16, String> {
    value
        .to_str()
        .ok_or_else(|| format!("{label} must be valid UTF-8"))?
        .parse()
        .map_err(|_| format!("invalid {label}"))
}

fn print_usage() {
    println!("Usage: emi-ex [extract] [OPTIONS] <archive.EMI> [index]");
    println!("       emi-ex pack -o <archive.EMI> [-J [manifest]] <folder>");
    println!("       emi-ex pack -o <archive.EMI> [-t N] -i <files...>");
    println!("       emi-ex list <archive.EMI>");
}

/// Read a 4-byte little-endian word from an entry payload at `offset`.
fn entry_word(archive: &Path, entry: &emi_ex_v2::Entry, offset: u64) -> Option<u32> {
    use std::fs::File;
    use std::io::{Read, Seek, SeekFrom};
    let mut file = File::open(archive).ok()?;
    file.seek(SeekFrom::Start(entry.offset + offset)).ok()?;
    let mut buf = [0u8; 4];
    file.read_exact(&mut buf).ok()?;
    Some(u32::from_le_bytes(buf))
}

/// For a code-bearing entry, the payload header stores the text/code base at
/// offset 0x18 (`t_addr`). The code begins at file offset `t_addr - ram_ptr`
/// within the entry. Returns that offset when `t_addr` looks like a RAM vram.
fn entry_code_start(archive: &Path, entry: &emi_ex_v2::Entry) -> Option<u64> {
    let t_addr = entry_word(archive, entry, 0x18)?;
    // Plausible RAM vram: kernel/data range 0x80000000+ or low RAM 0x00000000.
    let looks_like_vram = (t_addr & 0x80000000 != 0) || t_addr < 0x00800000;
    if !looks_like_vram {
        return None;
    }
    let start = t_addr as i64 - entry.ram_ptr as i64;
    if start < 0 || start as u64 > entry.size as u64 {
        None
    } else {
        Some(start as u64)
    }
}

#[derive(Serialize)]
struct ListEntry {
    index: usize,
    offset: u64,
    ram_ptr: u32,
    size: u32,
    file_type: u16,
    code_off: Option<u64>,
    first4: u32,
}

fn run_list(args: &[OsString]) -> Result<(), String> {
    let mut json = false;
    let mut positionals: Vec<&OsStr> = Vec::new();
    for arg in args {
        let value = arg.to_str().unwrap_or("");
        if value == "--json" {
            json = true;
            continue;
        }
        if value.starts_with('-') {
            return Err(format!("unknown option {value}"));
        }
        positionals.push(arg);
    }
    if positionals.len() != 1 {
        return Err("usage: emi-ex list [--json] <archive.EMI>".into());
    }
    let source = PathBuf::from(positionals[0]);
    let archive =
        Archive::open(&source).map_err(|error| format!("{}: {error}", source.display()))?;

    if json {
        let entries: Vec<ListEntry> = archive
            .entries()
            .iter()
            .enumerate()
            .map(|(index, entry)| ListEntry {
                index,
                offset: entry.offset,
                ram_ptr: entry.ram_ptr,
                size: entry.size,
                file_type: entry.file_type,
                code_off: entry_code_start(&source, entry),
                first4: entry.first4,
            })
            .collect();
        println!(
            "{}",
            serde_json::to_string_pretty(&entries).map_err(|e| e.to_string())?
        );
        return Ok(());
    }

    println!(
        "{:<5} {:<10} {:<12} {:<10} {:<6} {:<10} {}",
        "IDX", "OFFSET", "RAM_PTR", "SIZE", "TYPE", "CODE_OFF", "FIRST4"
    );
    for (index, entry) in archive.entries().iter().enumerate() {
        let code_off = entry_code_start(&source, entry)
            .map(|off| format!("{off:#x}"))
            .unwrap_or_else(|| "-".to_string());
        println!(
            "{:<5} {:<10} {:<#12x} {:<#10x} {:<6} {:<10} {:#x}",
            index,
            format!("{:#x}", entry.offset),
            entry.ram_ptr,
            entry.size,
            entry.file_type,
            code_off,
            entry.first4,
        );
    }
    Ok(())
}
