import json
import struct
from pathlib import Path
from unittest.mock import patch
from harness.common.observation import PathWatch


def event(name):
 raw=name+b'\0'
 raw+=b'\0'*((-len(raw))%4)
 return struct.pack('=iIII',1,2,0,len(raw))+raw


def watch():
 result=object.__new__(PathWatch)
 result._descriptor=123
 result._failure=None
 result._entries={1:{b'input.c'}}
 result._watch_paths={1:{Path('/not-opened')}}
 result._memberships=set()
 return result


def run(chunks):
 obj=watch()
 try:
  with patch('harness.common.observation.os.read',side_effect=chunks): obj._drain()
 except ValueError as error:
  return str(error)
 return 'drained'

other=event(b'other.c')
large=other*(65536//len(other))
results={
 'fragmented_unrelated':run([other]*70+[BlockingIOError()]),
 'selected_after_fragmentation':run([other]*70+[event(b'input.c'),BlockingIOError()]),
 'byte_limit':run([large]*1000),
 'interrupt_limit':run([InterruptedError()]*100),
}
print(json.dumps(results,indent=2))
