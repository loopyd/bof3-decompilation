extern void func_801F2C04(void);

/* @source 0x801F2D2C
 * @behavior Overlay entry thunk referenced by this payload's trailing pointer
 * word at 0x801F2E14: it calls the overlay body func_801F2C04 with the incoming
 * argument registers left untouched and returns to its caller with that call's
 * result, saving the return address in a 0x18-byte frame so the body runs as a
 * normal non-tail call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void enterArea064Overlay(void) {
  func_801F2C04();
}
