#include "bof3/bof3.h"

/* Shared overlay state byte of the main executable, outside this overlay's
 * payload. The sibling world targets call it the overlay state byte and raise
 * it to small values, and this target's map already binds the address. The
 * handler run below selects its work on the value 0x10. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Front-end phase byte of the main executable, outside this overlay's
 * payload. The cue below only fires while the front end holds phase 0. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the main executable, outside this overlay's
 * payload. */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 */

/* @source 0x801F31BC @kind function */
extern void func_801F31BC(void);

/* @source 0x801F36FC
 * @behavior Entry 1 of the state-byte-2 handler run based at 0x801F4AA0 of this
 *           payload, whose dispatcher is 0x801F35E0 and whose entry 0 is
 *           0x801F3624. When the shared overlay state byte D_8014933B holds
 *           0x10 it queues sound cue 0x200 through the main executable
 *           dispatcher 0x8015DF18, but only while the front-end phase byte
 *           D_80143BB0 holds zero, and then increments record byte +0x02
 *           through the scratchpad work record pointer cell 0x1F800044 so the
 *           next dispatch of the same run takes its entry 2. The increment
 *           reads the pointer cell again after the cue call, because that call
 *           may re-publish the record. Calls the payload render helper
 *           0x801F31BC on every path, inside and outside the state gate. Takes
 *           no arguments and returns nothing. The 0x18-byte frame only keeps
 *           $ra.
 *
 * Name evidence: the run based at 0x801F4AA0 is the state-byte-2 handler run of
 * dispatcher 0x801F35E0, whose table word 1 is this span and whose table word 0
 * is the exact sibling 0x801F3624, and that sibling names the state byte +0x02
 * increment it performs the advance that sends the next dispatch to entry 1 at
 * this address. 0x8014933B is the shared overlay state byte the sibling world
 * targets read (setSharedByte1ThenDrawWorld03Area14813_801F3788,
 * drawFullScreenSkyBlueToNavyGradientQuadWhileOverlayStateSetWorld03Area14514_801F532C)
 * and 0x80143BB0 is the front-end phase byte whose zero value gates cue 0x200 in
 * the exact siblings advanceWorkByte2AndPlayCue200WhenFlag67SetThenDrawWorld04Area17013_801F4574
 * and advanceWorkByte2AndPlayCue200WhenCameraInsideRecordCoordinateWindowWorld04Area16413_801F2DB8,
 * whose bodies share this one's gate, cue and increment sequence. 0x8015DF18 is
 * the sound-cue dispatcher those siblings declare, and 0x801F31BC is the payload
 * render helper the same run's entry 0 ends in. The value 0x10 itself is not
 * established beyond this run, so the name states the compared value and leaves
 * the meaning to the gate.
 *
 * Shape evidence: the run's own second handler 0x801F37A8, still an `asm` span
 * of this target, repeats this body word for word from the frame creation to the
 * +0x02 increment and differs only in the compared state value, 0x20 there, and
 * in what follows the increment. No byte-identical twin exists: a scan of every
 * payload under out/binaries for this span's first 0x50 bytes, which already
 * covers the whole body except the final call word and the epilogue, finds only
 * this span, so the clone lever is declined and the body is written from the
 * original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndQueueCue200WhenSharedOverlayStateByteIs10ThenDrawWorld04Area16713_801F36FC(
    void) {
  if (D_8014933B == 0x10) {
    u8* work;

    if (D_80143BB0 == 0) {
      func_8015DF18(0x200);
    }
    work = *(u8**)SPAD_ADDR(u8, 0x44u);
    work[2]++;
  }
  func_801F31BC();
}
