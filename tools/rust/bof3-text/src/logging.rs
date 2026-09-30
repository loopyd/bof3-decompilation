//! Logging setup and small colour helpers for CLI output.
//!
//! Verbosity maps `-v`..`-vvvv` onto log levels, `--no-color`/`-nc` (or the
//! `NO_COLOR` environment variable, or a non-terminal stream) disables colour,
//! and `--json` switches both the primary output and the log records to JSON.

use std::io::{self, IsTerminal, Write};
use std::sync::atomic::{AtomicBool, Ordering};

use env_logger::{Builder, Env};
use log::LevelFilter;
use owo_colors::OwoColorize;

static COLOR: AtomicBool = AtomicBool::new(false);

/// Log level implied by a repeat count of `-v`.
pub fn level_for(verbosity: u8) -> LevelFilter {
    match verbosity {
        0 => LevelFilter::Warn,
        1 => LevelFilter::Info,
        2 => LevelFilter::Debug,
        _ => LevelFilter::Trace,
    }
}

fn filter_name(level: LevelFilter) -> &'static str {
    match level {
        LevelFilter::Off => "off",
        LevelFilter::Error => "error",
        LevelFilter::Warn => "warn",
        LevelFilter::Info => "info",
        LevelFilter::Debug => "debug",
        LevelFilter::Trace => "trace",
    }
}

/// Configure the logger. Returns the effective level so callers can report it.
pub fn init(verbosity: u8, no_color: bool, json: bool) -> LevelFilter {
    let level = level_for(verbosity);
    let color = !no_color && std::env::var_os("NO_COLOR").is_none() && io::stderr().is_terminal();
    COLOR.store(color, Ordering::Relaxed);
    let mut builder = Builder::from_env(Env::default().default_filter_or(filter_name(level)));
    builder.format(move |buffer, record| {
        let message = record.args().to_string();
        if json {
            let value = serde_json::json!({
                "level": record.level().as_str().to_lowercase(),
                "target": record.target(),
                "message": message,
            });
            writeln!(buffer, "{value}")
        } else if color {
            writeln!(buffer, "{} {message}", paint_level(record.level()))
        } else {
            writeln!(
                buffer,
                "{:<5} {message}",
                record.level().as_str().to_lowercase()
            )
        }
    });
    // A failed init only matters when a logger already exists (for example under a
    // test harness); the CLI must not abort for it.
    let _ = builder.try_init();
    level
}

fn paint_level(level: log::Level) -> String {
    let label = level.as_str().to_lowercase();
    match level {
        log::Level::Error => label.red().bold().to_string(),
        log::Level::Warn => label.yellow().bold().to_string(),
        log::Level::Info => label.green().to_string(),
        log::Level::Debug => label.cyan().to_string(),
        log::Level::Trace => label.dimmed().to_string(),
    }
}

/// Whether colour is currently enabled for CLI output.
pub fn colors_enabled() -> bool {
    COLOR.load(Ordering::Relaxed)
}

/// Colour `text` only when colour output is enabled.
pub fn paint(text: &str, style: fn(&str) -> String) -> String {
    if colors_enabled() {
        style(text)
    } else {
        text.to_string()
    }
}

/// Style helpers used by the command output.
pub fn success(text: &str) -> String {
    paint(text, |value| value.green().bold().to_string())
}

pub fn failure(text: &str) -> String {
    paint(text, |value| value.red().bold().to_string())
}

pub fn heading(text: &str) -> String {
    paint(text, |value| value.bold().to_string())
}
