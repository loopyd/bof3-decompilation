#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared frame-service sequence func_801527E4, func_8015A758,
 * func_801BDAB8, func_801A0514, func_8019A0E4 and func_8014BA54, then finalizes
 * one shared front-end frame through func_80158C80 and finally runs the
 * func_801D0EA0 slice. No argument, result or state of its own.
 * @source 0x80199398
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_80199398(void)
{
    func_801527E4();
    func_8015A758();
    func_801BDAB8();
    func_801A0514();
    func_8019A0E4();
    func_8014BA54();
    func_80158C80();
    func_801D0EA0();
}
