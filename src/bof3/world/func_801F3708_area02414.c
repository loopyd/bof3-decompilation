#include "bof3/world/area02414_internal.h"

/* @behavior seeds the motion record's three rotation halfwords from masked
 * rand() values, builds the rotation matrix from them with a zeroed
 * translation, publishes that matrix as both the current rotation and the
 * current translation matrix, then rotates and translates the source quad's
 * four 6-byte corners and stores each transformed corner plus the matching
 * motion-record position halfword into the destination quad.
 * @source 0x801F3708
 * @status partial
 * @match 73.43
 * @residual non-exact live audit: 105/143 instructions, 572 -> 536 bytes; the non-nop instruction multiset is identical (130/130), so the residual is instruction order only. This build hoists the constant-offset corner loads above the preceding stores and fills the load delay slots with them, where the original keeps program order and fills those slots with the local address setup, leaving three unfilled delays per corner. Measured shapes: reversed and alternate declaration order of vertex/projected/flag (84/97/97, the reversed order also moves the frame), halfword-pointer corner copies (97), explicit corner temps (105, retained, also restoring the original s5/s4/s3 allocation). Profile and permuter rungs are opt-in and untried.
 */
void func_801F3708(void* arg0, void* arg1, s16* arg2) {
  const World00Area024Quad* src;
  World00Area024Quad*       dst;
  World00Area024Motion*     motion;
  MATRIX                    matrix;
  SVECTOR                   vertex;
  VECTOR                    projected;
  long                      flag;
  u16                       vx;
  u16                       vy;
  u16                       vz;

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

  vx = src->vertex[0].x;
  vertex.vx = vx;
  vy = src->vertex[0].y;
  vertex.vy = vy;
  vz = src->vertex[0].z;
  vertex.vz = vz;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[0].x = (u16)(motion->x + projected.vx);
  dst->vertex[0].y = (u16)(motion->y + projected.vy);
  dst->vertex[0].z = (u16)(motion->z + projected.vz);

  vx = src->vertex[1].x;
  vertex.vx = vx;
  vy = src->vertex[1].y;
  vertex.vy = vy;
  vz = src->vertex[1].z;
  vertex.vz = vz;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[1].x = (u16)(motion->x + projected.vx);
  dst->vertex[1].y = (u16)(motion->y + projected.vy);
  dst->vertex[1].z = (u16)(motion->z + projected.vz);

  vx = src->vertex[2].x;
  vertex.vx = vx;
  vy = src->vertex[2].y;
  vertex.vy = vy;
  vz = src->vertex[2].z;
  vertex.vz = vz;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[2].x = (u16)(motion->x + projected.vx);
  dst->vertex[2].y = (u16)(motion->y + projected.vy);
  dst->vertex[2].z = (u16)(motion->z + projected.vz);

  vx = src->vertex[3].x;
  vertex.vx = vx;
  vy = src->vertex[3].y;
  vertex.vy = vy;
  vz = src->vertex[3].z;
  vertex.vz = vz;
  RotTrans(&vertex, &projected, &flag);
  dst->vertex[3].x = (u16)(motion->x + projected.vx);
  dst->vertex[3].y = (u16)(motion->y + projected.vy);
  dst->vertex[3].z = (u16)(motion->z + projected.vz);
}
