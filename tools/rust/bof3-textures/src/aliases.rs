use std::collections::{BTreeMap, BTreeSet};

use crate::{Error, Result};

pub(crate) struct Aliases<'a, T: ?Sized> {
    records: BTreeMap<usize, Vec<(&'a T, String)>>,
}

impl<'a, T: PartialEq + ?Sized> Aliases<'a, T> {
    pub(crate) fn new() -> Self {
        Self {
            records: BTreeMap::new(),
        }
    }

    pub(crate) fn add(&mut self, offset: usize, value: &'a T, view: String) {
        self.records.entry(offset).or_default().push((value, view));
    }

    pub(crate) fn finish(self, context: &str) -> Result<BTreeMap<usize, &'a T>> {
        let mut conflicts = Vec::new();
        for (offset, views) in &self.records {
            if views.iter().any(|(value, _)| *value != views[0].0) {
                let names = views
                    .iter()
                    .map(|(_, view)| view.as_str())
                    .collect::<BTreeSet<_>>()
                    .into_iter()
                    .collect::<Vec<_>>()
                    .join(", ");
                conflicts.push(format!("[{names}] at byte {offset}"));
            }
        }
        if !conflicts.is_empty() {
            return Err(Error::Conflict(format!(
                "{context} aliases disagree: {}",
                conflicts.join("; ")
            )));
        }
        Ok(self
            .records
            .into_iter()
            .map(|(offset, views)| (offset, views[0].0))
            .collect())
    }
}
