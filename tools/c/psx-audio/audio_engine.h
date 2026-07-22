#ifndef PSX_AUDIO_ENGINE_H
#define PSX_AUDIO_ENGINE_H

#include "audio.h"

int audio_engine_render(const uint8_t* sep_data, size_t sep_len,
                        const uint8_t* vh_data, size_t vh_len,
                        const uint8_t* vb_data, size_t vb_len, int seq_index,
                        int output_rate, RenderOutput* out);

#endif
