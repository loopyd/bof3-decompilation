#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler D_801FE910[];

/* @source 0x801F986C
 * @behavior Work-byte-1 dispatcher of this overlay: it reads the work-object
 * pointer through the scratchpad pointer slot at 0x1F800044, takes that
 * object's unsigned byte at offset 1, scales it by four and invokes that entry
 * of the overlay-local handler-pointer table D_801FE910 with no arguments.
 * D_801FE910 is entry 31 of the 51-entry in-image code-pointer run that begins
 * at 0x801FE894 (0x801FE894 + 31 * 4 = 0x801FE910) and ends before the zero
 * word at 0x801FE960, so the work byte selects one of that run's entries from
 * index 31 on. The words from that entry to the run's last are 0x801F98B0,
 * 0x801F98E0, 0x801F995C, 0x801F9A10, 0x801F9AC0, 0x801F9B68, 0x801F9C00,
 * 0x801F9C58, 0x801F9CE0, 0x801F9D30, 0x801F9E00, 0x801F9E50, 0x801F9EC4,
 * 0x801F9F2C, 0x801F9F4C, 0x801F9F6C, 0x801FA294, 0x801FE018, 0x801FE170 and
 * 0x801FE2A0. It reads no other state and writes none: the pointer slot is
 * loaded before the 0x18-byte frame is created, and that frame only keeps ra
 * across the indirect call while the callee's result in v0 is discarded. The
 * 0x44-byte body is byte-identical to the same overlay's other work-byte-1
 * dispatchers apart from the loaded table word, which names a different entry
 * of the same run in the same 0x8000 page: 0x801F6F48 (0x801FE894, entry 0),
 * 0x801F707C (0x801FE8A0, entry 3), 0x801F7928 (0x801FE8D0, entry 15),
 * callByte1RunHandler4 (0x801FE908, entry 29)
 * and callByte1RunHandler6 (0x801FE938, entry
 * 41). No word inside this overlay's payload names 0x801F986C; its only
 * reference outside it is the shared handler-pointer word 0x801C7DEC in the
 * concurrently loaded etc/game overlay (BIN/ETC/GAME.EMI#0, load 0x80195800),
 * payload offset 0x325EC, eight words after the word there that names this
 * overlay's 0x801F95D4 dispatcher.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callByte1RunHandler5(void) {
  D_801FE910[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
