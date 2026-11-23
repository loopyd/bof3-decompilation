#include "bof3/emi/dispatchers.h"

/* This overlay's handler table at 0x801F37F8: two code pointers - 0x801F2C48 and
 * 0x801F2D0C. The Splat `textbin` row D_801F30AC places the words, so the table
 * is only referenced here, at its first entry. */
extern void (*workByte4HandlersWorld01Area04313[])(void); /* @source 0x801F37F8 @kind data */

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the work-record pointer published at
 *   the scratchpad pointer cell 0x1F800044 once, before the frame is created,
 *   takes its byte at offset 4, scales that byte by four and calls the matching
 *   entry of this overlay's two-entry handler table at 0x801F37F8 with no
 *   arguments, discarding any result. The original does not bound the index
 *   before the scaled table load. It reads no other state and writes none; the
 *   0x18-byte frame only keeps $ra at 0x10 across the indirect call.
 *
 *   Name evidence: the only word of this payload that holds 0x801F2C04 is entry
 *   0 of the four-word overlay entry run at 0x801F37A8, whose base the cells at
 *   0x801F37BC and 0x801F37F0 both hold. The index byte at +4 selects inside
 *   this overlay's own handler table, so the name describes that local table and
 *   claims no engine-wide role.
 *
 *   Shape evidence: this 17-word, 68-byte body differs from the dispatcher at
 *   the same address 0x801F2C04 in world01 area039 by one word only - the %lo of
 *   the load that reads its own table (0x37F8 here, 0x4C10 there, payload
 *   compare at offset 0x28). That near-twin, and the hundred-odd copies the
 *   shared dispatcher header records, are shape evidence and not clone
 *   eligibility, so every binding and literal here is written from this span's
 *   own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4,
                      callWorkByte4Table37,
                      workByte4HandlersWorld01Area04313);
