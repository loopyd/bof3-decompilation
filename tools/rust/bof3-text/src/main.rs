//! `bof3-text`: BOF3 area dialogue text blocks.
//!
//! Module layout: [`cli`] owns the command line, [`logging`] the log/colour
//! setup, [`command`] the editable-text grammar, [`syntax`] the document codecs
//! and linter, [`extract`] the EMI/bank parsing, [`pack`] the repacker,
//! [`query`] indexing and queries, and [`models`] the shared dialogue, bank and
//! command models.

mod cli;
mod command;
mod extract;
mod filters;
mod lexagraph;
mod logging;
mod models;
mod pack;
mod query;
mod search;
mod segmentmap;
mod syntax;
mod textindex;
mod windows;

fn main() {
    // clap expects argv[0] to be the program name, so the full vector is passed.
    std::process::exit(cli::main(std::env::args_os().collect()));
}
