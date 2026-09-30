#include "bof3/world/area02414_internal.h"

/* @behavior overlay init step called by func_801F2C58: copies the 27-entry
 * 0x28-byte quad source table into the local target array, averages each quad's
 * four corners into the matching 0x18-byte motion record, hands that average
 * and the record's velocity halfwords to func_80178804, trims each velocity to
 * a signed byte, clears the record's rotation halfwords and recenters all four
 * corners around the average.
 * @source 0x801F3314
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3314(void) {
  World00Area024Quad*       target =
      (World00Area024Quad*)WORLD00_AREA024_TARGET_BASE;
  const World00Area024Quad* source =
      (const World00Area024Quad*)WORLD00_AREA024_SOURCE_TABLE;
  World00Area024Motion* motion =
      (World00Area024Motion*)WORLD00_AREA024_MOTION_BASE;
  VECTOR center;
  u8     i = 0u;

  do {
    *target = *source;

    motion->x = (u16)(((s16)target->vertex[0].x + (s16)target->vertex[1].x +
                       (s16)target->vertex[2].x +
                       (s16)target->vertex[3].x) >>
                      2);
    motion->y = (u16)(((s16)target->vertex[0].y + (s16)target->vertex[1].y +
                       (s16)target->vertex[2].y +
                       (s16)target->vertex[3].y) >>
                      2);
    motion->z = (u16)(((s16)target->vertex[0].z + (s16)target->vertex[1].z +
                       (s16)target->vertex[2].z +
                       (s16)target->vertex[3].z) >>
                      2);

    center.vx = (s16)motion->x;
    center.vy = (s16)motion->y;
    center.vz = (s16)motion->z;
    func_80178804(&center, (SVECTOR*)&motion->vx);

    motion->vx = (u16)((s16)(motion->vx & 0xff00) >> 8);
    motion->vy = (u16)((s16)(motion->vy & 0xff00) >> 8);
    motion->vz = (u16)((s16)(motion->vz & 0xff00) >> 8);
    motion->rx = 0;
    motion->ry = 0;
    motion->rz = 0;

    target->vertex[0].x -= motion->x;
    target->vertex[0].y -= motion->y;
    target->vertex[0].z -= motion->z;
    target->vertex[1].x -= motion->x;
    target->vertex[1].y -= motion->y;
    target->vertex[1].z -= motion->z;
    target->vertex[2].x -= motion->x;
    target->vertex[2].y -= motion->y;
    target->vertex[2].z -= motion->z;
    target->vertex[3].x -= motion->x;
    target->vertex[3].y -= motion->y;
    target->vertex[3].z -= motion->z;

    motion += 1;
    target += 1;
    source += 1;
    i += 1u;
  } while (i < 0x1Bu);
}
