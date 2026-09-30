//! BIOS ExCB links in guest RAM. BIOS boot allocation is separate.
//! Only head removal is supported: original non-head removal reads an
//! uninitialized stack word, so implementing ordinary list removal is incorrect.

use super::{
    bus::{Bus, Width},
    executable::ram_offset,
};
use crate::Result;
use std::collections::BTreeSet;

const TABLE_BYTES: usize = 32;
const NODE_BYTES: usize = 16;
const MAX_NODES: usize = 4096;

#[derive(Clone, Copy, Debug)]
pub struct Table {
    base: u32,
    physical: usize,
}

impl Table {
    /// Consume the BIOS-owned descriptor; an EXE alone does not initialize it.
    pub fn read(bus: &mut impl Bus) -> Result<Self> {
        let base = bus.read(0x100, Width::Word)?;
        let size = bus.read(0x104, Width::Word)?;
        if base == 0 || base & 3 != 0 || size != TABLE_BYTES as u32 {
            return Err("BIOS exception chains: missing or malformed ExCB descriptor".into());
        }
        let physical = ram_offset(base, TABLE_BYTES)?;
        if overlaps(physical, TABLE_BYTES, 0x100, 8) {
            return Err("BIOS exception chains: table overlaps its descriptor".into());
        }
        Ok(Self { base, physical })
    }

    fn entry(&self, priority: u32) -> Result<u32> {
        if priority >= 4 {
            return Err("BIOS exception-chain priority outside 0..4".into());
        }
        Ok(self.base + priority * 8)
    }

    pub(crate) fn node(&self, address: u32) -> Result<usize> {
        if address == 0 || address & 3 != 0 {
            return Err("BIOS exception chain: null or unaligned node".into());
        }
        let physical = ram_offset(address, NODE_BYTES)?;
        if overlaps(physical, NODE_BYTES, self.physical, TABLE_BYTES)
            || overlaps(physical, NODE_BYTES, 0x100, 8)
        {
            return Err("BIOS exception-chain node overlaps table or descriptor".into());
        }
        Ok(physical)
    }

    pub(crate) fn head(&self, bus: &mut impl Bus, priority: u32) -> Result<u32> {
        Ok(bus.read(self.entry(priority)?, Width::Word)?)
    }

    /// Reject cyclic, overlapping or excessively long chains before mutation.
    /// Addresses retain their original alias bits; physical offsets serve only
    /// integrity checks, never pointer-equality substitution for BIOS removal.
    fn chain(&self, bus: &mut impl Bus, mut head: u32) -> Result<BTreeSet<usize>> {
        let mut nodes = BTreeSet::new();
        while head != 0 {
            let physical = self.node(head)?;
            if nodes.len() == MAX_NODES {
                return Err("BIOS exception chain exceeds 4096-node safety limit".into());
            }
            if nodes
                .range(physical.saturating_sub(NODE_BYTES - 1)..physical + NODE_BYTES)
                .next()
                .is_some()
            {
                return Err("BIOS exception chain contains a cycle or overlapping nodes".into());
            }
            nodes.insert(physical);
            head = bus.read(head, Width::Word)?;
        }
        Ok(nodes)
    }

    pub fn enqueue(&self, bus: &mut impl Bus, priority: u32, node: u32) -> Result<()> {
        let entry = self.entry(priority)?;
        let physical = self.node(node)?;
        let head = bus.read(entry, Width::Word)?;
        let nodes = self.chain(bus, head)?;
        if nodes.len() == MAX_NODES {
            return Err("BIOS SysEnqIntRP would exceed the 4096-node safety limit".into());
        }
        if nodes
            .range(physical.saturating_sub(NODE_BYTES - 1)..physical + NODE_BYTES)
            .next()
            .is_some()
        {
            return Err("BIOS SysEnqIntRP: node is already linked or overlaps the chain".into());
        }
        // Store ordering follows the inspected BIOS comparison. Bus faults are
        // terminal; preflight validates RAM bounds, not transactional bus writes.
        bus.write(entry, Width::Word, node)?;
        bus.write(node, Width::Word, head)?;
        Ok(())
    }

    pub fn dequeue_head(&self, bus: &mut impl Bus, priority: u32, node: u32) -> Result<u32> {
        let entry = self.entry(priority)?;
        let head = bus.read(entry, Width::Word)?;
        if head == 0 || head != node {
            return Err("BIOS SysDeqIntRP requires the exact non-null head pointer; absent/non-head removal has unverified original stack-dependent behavior".into());
        }
        self.chain(bus, head)?;
        let next = bus.read(head, Width::Word)?;
        bus.write(entry, Width::Word, next)?;
        // The detached node retains its old next link and other three words.
        Ok(head)
    }
}

fn overlaps(a: usize, a_len: usize, b: usize, b_len: usize) -> bool {
    a < b + b_len && b < a + a_len
}
