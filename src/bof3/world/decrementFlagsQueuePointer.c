#include "bof3/world/area03004_internal.h"

/* @behavior Decrements work countdown byte 9 and reloads the work pointer. A
 * zero reloaded countdown calls the shared clearWorkFlags helper;
 * otherwise, bit 3 of that reloaded countdown gates a call to the
 * bounded work-pointer queue appender.
 * @source 0x801D64E8
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementFlagsQueuePointer(void) {
    u8 *work;
    u8 *reload;

    work = D_1F800044;
    work[9] = work[9] - 1;
    reload = D_1F800044;
    if (reload[9] == 0) {
        func_80196070();
    } else if (reload[9] & 8) {
        queueUnprojectedActiveWork();
    }
}
