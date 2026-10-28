#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs func_801527E4, func_8015A758, func_801BDAB8,
 * updateActiveGameWorkRecords, func_8019A0E4 and func_8014BA54 in order, then
 * calls func_80158C80 and func_801D0EA0. Takes no arguments and returns no
 * value.
 * @source 0x80199398
 * @status exact
 * @match 100.00
 * @residual none
 */
void runFrameServicesAndFinalize(void)
{
    func_801527E4();
    func_8015A758();
    func_801BDAB8();
    updateActiveGameWorkRecords();
    func_8019A0E4();
    func_8014BA54();
    func_80158C80();
    func_801D0EA0();
}
