#include "internal.h"

/* does: scans the newly exposed COMMU00 table range and queues one type-4
 * and/or type-5 notification code when those tags appear for the first time in
 * the current pass.
 * @source: 0x801eedf8 FUN_801eedf8
 */
void func_801eedf8(void) {
  u16 next_row;
  u8  notification_flags;
  u8  notification_type;

  notification_flags = 0u;
  if (BOF3_COMMU00_LAST_NOTIFICATION_ROW < BOF3_COMMU00_WORLD_STATE) {
    next_row = (u16)(BOF3_COMMU00_LAST_NOTIFICATION_ROW + 1u);
    if (next_row <= BOF3_COMMU00_WORLD_STATE) {
      do {
        notification_type = BOF3_COMMU00_TYPE45_NOTIFICATION_TABLE[next_row];
        if ((notification_type == 4u) && ((notification_flags & 1u) == 0u)) {
          *commu00_notification_queue_slot(
              BOF3_COMMU00_NOTIFICATION_QUEUE_COUNT) = notification_type;
          BOF3_COMMU00_NOTIFICATION_QUEUE_COUNT += 1u;
          notification_flags |= 1u;
        } else if ((notification_type == 5u) &&
                   ((notification_flags & 2u) == 0u)) {
          *commu00_notification_queue_slot(
              BOF3_COMMU00_NOTIFICATION_QUEUE_COUNT) = 5u;
          BOF3_COMMU00_NOTIFICATION_QUEUE_COUNT += 1u;
          notification_flags |= 2u;
        }

        next_row += 1u;
      } while (next_row <= BOF3_COMMU00_WORLD_STATE);
    }

    BOF3_COMMU00_LAST_NOTIFICATION_ROW = BOF3_COMMU00_WORLD_STATE;
  }
}
