use std::io::{self, Write};
use std::path::PathBuf;
use std::process::ExitCode;

use bof3_textures::{
    Error, Result,
    catalog::{self, Filter, Index},
    package, sources,
};
use clap::{Parser, Subcommand};

#[derive(Parser)]
#[command(
    version,
    about = "Index, query, unpack, pack, and verify qualified BOF3 graphics"
)]
struct Cli {
    #[command(subcommand)]
    command: Command,
}

#[derive(Subcommand)]
enum Command {
    /// Export Rust sources, required local crate sources, lockfiles, and build instructions.
    Package {
        output: PathBuf,
        #[arg(long)]
        source_root: Option<PathBuf>,
    },
    /// Export captured snapshots, raw resources, decoded banks, and qualified editable views.
    Unpack {
        input: PathBuf,
        #[arg(long)]
        output: PathBuf,
        #[arg(long)]
        source: Option<String>,
    },
    /// Validate a self-contained package and publish a new archive tree.
    Pack {
        input: PathBuf,
        #[arg(long)]
        output: PathBuf,
        #[arg(long)]
        json: bool,
    },
    /// List game categories independently of current source support.
    Categories {
        #[arg(long)]
        json: bool,
    },
    /// Index an original US EMI or the qualified captured extracted-disc root.
    Index {
        input: PathBuf,
        #[arg(long)]
        output: PathBuf,
        #[arg(long)]
        source: Option<String>,
    },
    /// Query a fresh catalog. Source filters match substrings; other filters match exactly.
    Query {
        index: PathBuf,
        #[arg(long)]
        category: Option<String>,
        #[arg(long)]
        source: Option<String>,
        #[arg(long)]
        asset: Option<String>,
        #[arg(long)]
        storage: Option<String>,
        #[arg(long)]
        kind: Option<String>,
        #[arg(long)]
        palette: Option<String>,
        #[arg(long)]
        animation: Option<String>,
        #[arg(long)]
        dependency: Option<String>,
        #[arg(long)]
        evidence: Option<String>,
        /// Query logical bank records instead of physical files and EMI entries.
        #[arg(long)]
        records: bool,
        #[arg(long)]
        json: bool,
    },
    /// Report physical and logical coverage. Strict coverage fails while any required evidence is missing.
    Coverage {
        input: PathBuf,
        #[arg(long)]
        source: Option<String>,
        #[arg(long)]
        strict: bool,
        #[arg(long)]
        json: bool,
    },
    /// Report independent verification properties; incomplete verification returns exit 1.
    Verify {
        input: PathBuf,
        #[arg(long, conflicts_with = "package")]
        source: Option<String>,
        /// Compare this output directory with intent reconstructed from an edit package.
        #[arg(long)]
        package: Option<PathBuf>,
        #[arg(long)]
        json: bool,
    },
}

fn print_json(value: &impl serde::Serialize) -> Result<()> {
    let mut stdout = io::stdout().lock();
    serde_json::to_writer_pretty(&mut stdout, value)?;
    writeln!(stdout)?;
    Ok(())
}

fn run(cli: Cli) -> Result<bool> {
    match cli.command {
        Command::Package {
            output,
            source_root,
        } => {
            let default_root = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../../..");
            let files =
                package::source_package(source_root.as_deref().unwrap_or(&default_root), &output)?;
            print_json(&serde_json::json!({"output": output, "source_files": files}))?;
        }
        Command::Unpack {
            input,
            output,
            source,
        } => {
            let manifest = package::unpack(&sources::qualify(&input, source.as_deref())?, &output)?;
            print_json(
                &serde_json::json!({"schema": manifest.schema, "output": output, "files": manifest.snapshots.len(), "resources": manifest.exports.len(), "full_support": false}),
            )?;
        }
        Command::Pack { input, output, .. } => {
            print_json(&package::pack(&input, &output)?)?;
        }
        Command::Categories { .. } => {
            print_json(&catalog::categories())?;
        }
        Command::Index {
            input,
            output,
            source,
        } => {
            let index = catalog::build(&sources::qualify(&input, source.as_deref())?)?;
            package::publish_json(&output, &input, &index)?;
        }
        Command::Query {
            index,
            category,
            source,
            asset,
            storage,
            kind,
            palette,
            animation,
            dependency,
            evidence,
            records,
            ..
        } => {
            let index = Index::load(&index)?;
            if let Some(category) = &category
                && !index.categories.iter().any(|known| known.id == *category)
            {
                return Err(Error::Invalid(format!(
                    "unknown category {category}; use textures categories"
                )));
            }
            let filter = Filter {
                category: category.as_deref(),
                source: source.as_deref(),
                asset: asset.as_deref(),
                storage: storage.as_deref(),
                kind: kind.as_deref(),
                palette: palette.as_deref(),
                animation: animation.as_deref(),
                dependency: dependency.as_deref(),
                evidence: evidence.as_deref(),
            };
            if records {
                print_json(
                    &index
                        .records
                        .iter()
                        .filter(|record| filter.matches(*record))
                        .collect::<Vec<_>>(),
                )?;
            } else {
                print_json(
                    &index
                        .resources
                        .iter()
                        .filter(|resource| filter.matches(*resource))
                        .collect::<Vec<_>>(),
                )?;
            }
        }
        Command::Coverage {
            input,
            source,
            strict,
            ..
        } => {
            let index = catalog::build(&sources::qualify(&input, source.as_deref())?)?;
            print_json(&catalog::coverage(&index))?;
            return Ok(!strict);
        }
        Command::Verify {
            input,
            source,
            package,
            ..
        } => {
            let report = if let Some(package) = package {
                bof3_textures::verification::output(&input, &package)?
            } else {
                bof3_textures::verification::original(&sources::qualify(
                    &input,
                    source.as_deref(),
                )?)?
            };
            print_json(&report)?;
            return Ok(false);
        }
    }
    Ok(true)
}

fn main() -> ExitCode {
    match run(Cli::parse()) {
        Ok(true) => ExitCode::SUCCESS,
        Ok(false) => ExitCode::from(1),
        Err(error) => {
            eprintln!(
                "{}",
                serde_json::json!({"error": {"code": error.code(), "message": error.to_string()}})
            );
            ExitCode::from(2)
        }
    }
}
