#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801BE920
 * @behavior Swap two indexed records, optionally preserving their secondary
 * fields, then invalidate both records' byte at offset 0x43.
 */
void func_801BE920(u8 first, u8 second, u8 preserve_secondary)
{
    u8 byte_value;
    u32 word_value;

    byte_value = D_80145E98[first].field_71;
    D_80145E98[first].field_71 = D_80145E98[second].field_71;
    D_80145E98[second].field_71 = byte_value;

    if (preserve_secondary == 0) {
        byte_value = D_80145E98[first].field_00;
        D_80145E98[first].field_00 = D_80145E98[second].field_00;
        D_80145E98[second].field_00 = byte_value;
        byte_value = D_80145E98[first].field_21;
        D_80145E98[first].field_21 = D_80145E98[second].field_21;
        D_80145E98[second].field_21 = byte_value;
        byte_value = D_80145E98[first].field_124;
        D_80145E98[first].field_124 = D_80145E98[second].field_124;
        D_80145E98[second].field_124 = byte_value;
        word_value = D_80145E98[first].field_2C;
        D_80145E98[first].field_2C = D_80145E98[second].field_2C;
        D_80145E98[second].field_2C = word_value;
        word_value = D_80145E98[first].field_30;
        D_80145E98[first].field_30 = D_80145E98[second].field_30;
        D_80145E98[second].field_30 = word_value;
        word_value = D_80145E98[first].field_34;
        D_80145E98[first].field_34 = D_80145E98[second].field_34;
        D_80145E98[second].field_34 = word_value;
    }

    word_value = D_80145E98[first].field_04;
    D_80145E98[first].field_04 = D_80145E98[second].field_04;
    D_80145E98[second].field_04 = word_value;
    word_value = D_80145E98[first].field_08;
    D_80145E98[first].field_08 = D_80145E98[second].field_08;
    D_80145E98[second].field_08 = word_value;
    word_value = D_80145E98[first].field_0C;
    D_80145E98[first].field_0C = D_80145E98[second].field_0C;
    D_80145E98[second].field_0C = word_value;
    byte_value = D_80145E98[first].field_111;
    D_80145E98[first].field_111 = D_80145E98[second].field_111;
    D_80145E98[second].field_111 = byte_value;
    byte_value = D_80145E98[first].field_110;
    D_80145E98[first].field_110 = D_80145E98[second].field_110;
    D_80145E98[second].field_110 = byte_value;
    D_80145E98[first].field_43 = 0xFF;
    D_80145E98[second].field_43 = 0xFF;
}
