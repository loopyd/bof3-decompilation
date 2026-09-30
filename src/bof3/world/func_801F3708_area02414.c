#include "bof3/world/area02414_internal.h"

/* @behavior seeds the motion record's three rotation halfwords from masked
 * rand() values, builds the rotation matrix from them with a zeroed
 * translation, publishes that matrix as both the current rotation and the
 * current translation matrix, then rotates and translates the source quad's
 * four 6-byte corners and stores each transformed corner plus the matching
 * motion-record position halfword into the destination quad.
 * @source 0x801F3708
 * @status partial
 * @match 67.83
 * @residual non-exact live audit: 97/143 instructions, 572 -> 536 bytes; the non-nop instruction multiset is identical (130/130), so the residual is instruction order only.
 */
void func_801F3708(void* arg0, void* arg1, s16* arg2) {
  const World00Area024Quad* src;
  World00Area024Quad*       dst;
  World00Area024Motion*     motion;
  MATRIX                    matrix;
  SVECTOR                   vertex;
  VECTOR                    projected;
  long                      flag;

  src = (const World00Area024Quad*)arg0;
  dst = (World00Area024Quad*)arg1;
  motion = (World00Area024Motion*)arg2;

  motion->rx = rand() & 0xFC0;
  motion->ry = rand() & 0xFC0;
  motion->rz = rand() & 0xFC0;
  RotMatrix((SVECTOR*)&motion->rx, &matrix);
  matrix.t[0] = 0;
  matrix.t[1] = 0;
  matrix.t[2] = 0;
  SetTransMatrix(&matrix);
  SetRotMatrix(&matrix);

  vertex.vx = src->vertex[0].x;
  vertex.vy = src->vertex[0].y;
  vertex.vz = src->vertex[0].z;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[0].x = (u16)(motion->x + projected.vx);
  dst->vertex[0].y = (u16)(motion->y + projected.vy);
  dst->vertex[0].z = (u16)(motion->z + projected.vz);

  vertex.vx = src->vertex[1].x;
  vertex.vy = src->vertex[1].y;
  vertex.vz = src->vertex[1].z;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[1].x = (u16)(motion->x + projected.vx);
  dst->vertex[1].y = (u16)(motion->y + projected.vy);
  dst->vertex[1].z = (u16)(motion->z + projected.vz);

  vertex.vx = src->vertex[2].x;
  vertex.vy = src->vertex[2].y;
  vertex.vz = src->vertex[2].z;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[2].x = (u16)(motion->x + projected.vx);
  dst->vertex[2].y = (u16)(motion->y + projected.vy);
  dst->vertex[2].z = (u16)(motion->z + projected.vz);

  vertex.vx = src->vertex[3].x;
  vertex.vy = src->vertex[3].y;
  vertex.vz = src->vertex[3].z;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[3].x = (u16)(motion->x + projected.vx);
  dst->vertex[3].y = (u16)(motion->y + projected.vy);
  dst->vertex[3].z = (u16)(motion->z + projected.vz);
}
