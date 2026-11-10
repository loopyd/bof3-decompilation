#include "config.h"
#include "audio.h"
#include "vocal.h"

#if PLAYER_MODE_VOCAL
#define XA_STOP BYTE(0x8014682c)
#define XA_STATE BYTE(0x8014682d)
#define CD_SYNC_CALLBACK WORD(0x80185780)

volatile struct VocalStatus vocal_status;
static u32 last_service, start_frame;
static int start_pending, stop_pending, allow_repeat;

static int transport_idle(void)
{
    return XA_STATE == 0 && CD_SYNC_CALLBACK == 0;
}

void vocal_initialize(void)
{
    player_status.phase = 1;
    CALL(0x8015cd00, 0, 0, 0, 0);
    if (CALL(0x80175534, 1, 0, 0, 0) != 1 || !transport_idle()) {
        player_status.error = -6;
    }
}

void vocal_update(void)
{
    u32 frame;
    if (player_status.error) return;
    frame = player_status.frames;
    if (player_status.playing) player_status.position_frames = frame - start_frame;
    if (frame - last_service < 2) return;
    last_service = frame;
    if (player_status.playing) {
        /* Match the original foreground VSync(2) cadence and stop pulse. */
        if (stop_pending) XA_STOP = 1;
        CALL(0x80163010, 0, 0, 0, 0);
        if (stop_pending) XA_STOP = 0;
        stop_pending = 0;
    }
    if (transport_idle() &&
        (start_pending || (player_status.playing && vocal_status.repeat && allow_repeat))) {
        CALL(0x801636a0, PLAYER_VOCAL_CUE, 0, 0, 0);
        start_pending = 0;
        start_frame = frame;
        player_status.position_frames = 0;
    }
    vocal_status.state = XA_STATE;
    player_status.playing = !transport_idle();
}

void vocal_play(void)
{
    if (player_status.error) return;
    start_pending = 1;
    allow_repeat = 1;
    if (player_status.playing) stop_pending = 1;
}

void vocal_stop(void)
{
    start_pending = 0;
    allow_repeat = 0;
    if (player_status.playing) stop_pending = 1;
}

void vocal_repeat(void)
{
    vocal_status.repeat = !vocal_status.repeat;
}
#endif
