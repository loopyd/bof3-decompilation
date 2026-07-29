#include "internal.h"

s8 func_800A7648(s8 index) {
    index += 2;
    if (index < 0) {
        index = 0;
    }
    if (index >= 5) {
        index = 4;
    }
    return D_800B4E8C[index];
}
