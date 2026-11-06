#include "bof3/bof3.h"

extern u8 D_80148330; /* @source 0x80148330 @kind data */
extern u8 D_80148331; /* @source 0x80148331 @kind data */
extern u8 D_80148332; /* @source 0x80148332 @kind data */
extern u8 D_80148333; /* @source 0x80148333 @kind data */
extern s16 D_80148334; /* @source 0x80148334 @kind data */
extern s16 D_80148336; /* @source 0x80148336 @kind data */
extern u8 D_80148354; /* @source 0x80148354 @kind data */
extern u8 D_80148355; /* @source 0x80148355 @kind data */
extern u8 D_80148356; /* @source 0x80148356 @kind data */
extern u8 D_80148357; /* @source 0x80148357 @kind data */
extern s16 D_80148358; /* @source 0x80148358 @kind data */
extern s16 D_8014835A; /* @source 0x8014835A @kind data */
extern u8 D_80148378; /* @source 0x80148378 @kind data */
extern u8 D_80148379; /* @source 0x80148379 @kind data */
extern u8 D_8014837A; /* @source 0x8014837A @kind data */
extern u8 D_8014837B; /* @source 0x8014837B @kind data */
extern s16 D_8014837C; /* @source 0x8014837C @kind data */
extern s16 D_8014837E; /* @source 0x8014837E @kind data */
extern u8 D_80148382; /* @source 0x80148382 @kind data */
extern u8 D_80148383; /* @source 0x80148383 @kind data */
extern u8 D_8014839C; /* @source 0x8014839C @kind data */
extern u8 D_8014839D; /* @source 0x8014839D @kind data */
extern u8 D_8014839E; /* @source 0x8014839E @kind data */
extern u8 D_8014839F; /* @source 0x8014839F @kind data */
extern s16 D_801483A0; /* @source 0x801483A0 @kind data */
extern s16 D_801483A2; /* @source 0x801483A2 @kind data */
extern u8 D_801483A6; /* @source 0x801483A6 @kind data */
extern u8* D_801483BC; /* @source 0x801483BC @kind data */
extern u8 D_801DA028[]; /* @source 0x801DA028 @kind data */

/* @source 0x801D1D1C
 * @behavior Initializes the SHISU (masters) system-mode work state: the two
 *           panel origin/offset halfword pairs D_80148358/D_8014835A and
 *           D_8014837C/D_8014837E, the panel extent halfwords
 *           D_801483A0/D_801483A2, the per-panel mode bytes at D_80148354,
 *           D_80148378 and D_8014839C, and the remaining cursor/mask bytes of
 *           the shared UI work block, then publishes the four-byte handle table
 *           D_801DA028 as the block's record pointer D_801483BC. The offset and
 *           mask bytes are written through one reused s16 local, which is the
 *           form that gives both literals the original's a1 register.
 * @status partial
 * @match 91.43
 * @residual One contiguous six-instruction block is scheduled at the block head
 *           instead of in source position: the s16 local's -100 definition plus
 *           the D_8014835A/D_8014837E stores and its 0xFF redefinition occupy
 *           offsets 0x00-0x14, while the original places the -100 definition at
 *           0x0C, the two stores at 0x28/0x34 and the 0xFF definition at 0x38.
 *           The local's definition and its dependent stores schedule as one
 *           chain whenever they come from one quantity; spelling the same
 *           literals inline keeps them in place but then the allocator uses v0
 *           for both and the 0x80148383/0x801483A6 stores pull up with their
 *           definition, which measured 60/70.
 */
void initializeShisuUiState(void)
{
    s16 value;

    D_80148330 = 0;
    D_80148331 = 1;
    D_80148332 = 0;
    D_80148333 = 2;
    D_80148334 = 0;
    D_80148336 = 0;
    D_80148358 = 0x14;
    D_8014837C = 0x70;
    value = -100;
    D_8014835A = value;
    D_8014837E = value;
    value = 0xFF;
    D_80148382 = 8;
    D_8014839E = 3;
    D_801483A0 = -0x78;
    D_801483A2 = 0x3F;
    D_80148354 = 1;
    D_80148355 = 1;
    D_80148356 = 1;
    D_80148357 = 2;
    D_80148378 = 1;
    D_80148379 = 1;
    D_8014837A = 2;
    D_8014837B = 2;
    D_80148383 = value;
    D_8014839C = 1;
    D_8014839D = 1;
    D_8014839F = 2;
    D_801483A6 = value;
    D_801483BC = D_801DA028;
}
