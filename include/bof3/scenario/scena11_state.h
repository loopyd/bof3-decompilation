#ifndef BOF3_SCENARIO_SCENA11_STATE_H
#define BOF3_SCENARIO_SCENA11_STATE_H

#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

/* Clear this overlay's shared state bytes; leave scenario progress unchanged. */
#define SCENA11_CLEAR_SHARED_STATE() \
    (D_80146865 = 0u, \
     D_80146866 = 0u, \
     D_80146867 = 0u, \
     D_80146875 = 0u, \
     D_80146874 = 0u)

#endif
