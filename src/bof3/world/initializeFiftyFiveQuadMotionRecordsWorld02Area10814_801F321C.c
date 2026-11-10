#include "bof3/bof3.h"

typedef struct Area10814QuadVertex {
  u16 x;
  u16 y;
  u16 z;
} Area10814QuadVertex;

typedef struct Area10814Quad {
  u16 unk_00;
  Area10814QuadVertex vertex[4];
  u8 unk_1a[0x0e];
} Area10814Quad;

typedef struct Area10814Motion {
  u16 x;
  u16 y;
  u16 z;
  u16 unk_06;
  u16 vx;
  u16 vy;
  u16 vz;
  u16 unk_0e;
  s16 rx;
  s16 ry;
  s16 rz;
  s16 unk_16;
} Area10814Motion;

/* @source 0x80147B40 @kind data */
extern Area10814Quad* D_80147B40;
/* @source 0x80178804 @kind function */
extern void func_80178804(VECTOR* center, SVECTOR* result);

/* @source 0x801F321C
 * @behavior Copies 55 source quad records from the pointer at 0x80147B40
 * into the 0x28-byte table at 0x800E4FA8. Averages each quad's four signed
 * halfword corners into the matching 0x18-byte record at 0x800E4A80, calls
 * func_80178804 with that center and the velocity halfwords, shifts the
 * resulting signed velocities right by seven, clears the rotation halfwords,
 * and subtracts the center from all four corners. Called by func_801F2C88
 * without arguments; its result is not consumed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeFiftyFiveQuadMotionRecordsWorld02Area10814_801F321C(void) {
  Area10814Quad* target = PSX_PTR(Area10814Quad, 0x800E4FA8u);
  const Area10814Quad* source = D_80147B40;
  Area10814Motion* motion = PSX_PTR(Area10814Motion, 0x800E4A80u);
  VECTOR center;
  u8 i = 0u;
  u8 j;
  u16* corner;

  do {
    *target = *source;
    motion->x = (u16)(((s16)target->vertex[0].x + (s16)target->vertex[1].x +
                       (s16)target->vertex[2].x + (s16)target->vertex[3].x) >> 2);
    motion->y = (u16)(((s16)target->vertex[0].y + (s16)target->vertex[1].y +
                       (s16)target->vertex[2].y + (s16)target->vertex[3].y) >> 2);
    motion->z = (u16)(((s16)target->vertex[0].z + (s16)target->vertex[1].z +
                       (s16)target->vertex[2].z + (s16)target->vertex[3].z) >> 2);
    center.vx = (s16)motion->x;
    center.vy = (s16)motion->y;
    center.vz = (s16)motion->z;
    func_80178804(&center, (SVECTOR*)&motion->vx);
    motion->vx = (u16)((s16)motion->vx >> 7);
    motion->vy = (u16)((s16)motion->vy >> 7);
    motion->vz = (u16)((s16)motion->vz >> 7);
    motion->rx = 0;
    motion->ry = 0;
    motion->rz = 0;
    corner = &target->vertex[0].x;
    j = 0u;
    do {
      *corner -= motion->x;
      corner++;
      *corner -= motion->y;
      corner++;
      *corner -= motion->z;
      corner++;
      j += 1u;
    } while (j < 4u);
    motion += 1;
    target += 1;
    source += 1;
    i += 1u;
  } while (i < 0x37u);
}
