//! Verified extraction publication shared by bank and XA exporters.

use crate::Result;
use std::{
    fs::{self, OpenOptions},
    io::Write,
    path::{Path, PathBuf},
    sync::atomic::{AtomicU64, Ordering},
};

pub(crate) fn write_new(path: &Path, bytes: &[u8]) -> Result<()> {
    let mut file = OpenOptions::new().write(true).create_new(true).open(path)?;
    file.write_all(bytes)?;
    file.sync_all()?;
    Ok(())
}

pub(crate) fn require_absent(path: &Path) -> Result<()> {
    match fs::symlink_metadata(path) {
        Ok(_) => Err(format!("output already exists: {}", path.display()).into()),
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => Ok(()),
        Err(error) => Err(error.into()),
    }
}

pub(crate) struct Publication {
    pub(crate) staging: PathBuf,
    output: PathBuf,
}

impl Publication {
    pub(crate) fn new(output: &Path) -> Result<Self> {
        let name = output.file_name().ok_or("output needs a directory name")?;
        let parent = output
            .parent()
            .filter(|p| !p.as_os_str().is_empty())
            .unwrap_or(Path::new("."));
        if !parent.is_dir() {
            return Err("output parent directory must exist".into());
        }
        static NEXT: AtomicU64 = AtomicU64::new(0);
        for _ in 0..100 {
            let staging = parent.join(format!(
                ".bof3-extract-{}-{}",
                std::process::id(),
                NEXT.fetch_add(1, Ordering::Relaxed)
            ));
            let mut builder = fs::DirBuilder::new();
            #[cfg(unix)]
            {
                use std::os::unix::fs::DirBuilderExt;
                builder.mode(0o700);
            }
            match builder.create(&staging) {
                Ok(()) => {
                    return Ok(Self {
                        staging,
                        output: parent.join(name),
                    })
                }
                Err(e) if e.kind() == std::io::ErrorKind::AlreadyExists => continue,
                Err(e) => return Err(e.into()),
            }
        }
        Err("unable to reserve extraction staging directory".into())
    }

    pub(crate) fn publish(self) -> Result<()> {
        require_absent(&self.output)?;
        fs::rename(&self.staging, &self.output)?;
        Ok(())
    }
}

impl Drop for Publication {
    fn drop(&mut self) {
        let _ = fs::remove_dir_all(&self.staging);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn publication_conflict_preserves_destination_and_removes_owned_stage() {
        let parent = std::env::temp_dir();
        let target = parent.join(format!("bof3-publish-test-{}", std::process::id()));
        let transaction = Publication::new(&target).unwrap();
        let stage = transaction.staging.clone();
        write_new(&stage.join("data"), b"verified payload").unwrap();
        fs::create_dir(&target).unwrap();
        write_new(&target.join("sentinel"), b"existing user data").unwrap();
        assert!(transaction
            .publish()
            .unwrap_err()
            .to_string()
            .contains("already exists"));
        assert_eq!(
            fs::read(target.join("sentinel")).unwrap(),
            b"existing user data"
        );
        assert!(!stage.exists());
        fs::remove_file(target.join("sentinel")).unwrap();
        fs::remove_dir(target).unwrap();
    }
}
