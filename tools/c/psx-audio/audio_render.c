#include <string.h>

#include "audio.h"

AudioStatus audio_render(const AudioRenderRequest *request,
                         AudioRenderResult *result)
{
    if (!request || !result || !request->sep_data || !request->vh_data ||
        !request->vb_data || request->output_rate <= 0)
        return AUDIO_STATUS_INVALID_ARGUMENT;

    memset(result, 0, sizeof(*result));

    if (request->engine == AUDIO_ENGINE_GAME)
        return AUDIO_STATUS_UNSUPPORTED_ENGINE;
    if (request->engine != AUDIO_ENGINE_FAST)
        return AUDIO_STATUS_INVALID_ARGUMENT;

    if (render_bgm(request->sep_data, request->sep_len,
                   request->vh_data, request->vh_len,
                   request->vb_data, request->vb_len,
                   request->sequence, request->output_rate,
                   &result->audio) != 0)
        return AUDIO_STATUS_RENDER_FAILED;

    return AUDIO_STATUS_OK;
}

const char *audio_status_string(AudioStatus status)
{
    switch (status) {
    case AUDIO_STATUS_OK:
        return "ok";
    case AUDIO_STATUS_INVALID_ARGUMENT:
        return "invalid render request";
    case AUDIO_STATUS_UNSUPPORTED_ENGINE:
        return "game engine is not implemented yet";
    case AUDIO_STATUS_RENDER_FAILED:
        return "render failed";
    default:
        return "unknown audio error";
    }
}
