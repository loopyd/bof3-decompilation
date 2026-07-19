#include "bof3/bof3.h"

s32 func_801DD7D8(s32 arg0, s32 arg1, s32 arg2) {
  s32 ret;

  if (arg2 < arg1) {
    return arg0;
  }
  ret = arg1;
  if (arg0 >= arg2) {
    ret = arg2;
  }
  return ret;
}
