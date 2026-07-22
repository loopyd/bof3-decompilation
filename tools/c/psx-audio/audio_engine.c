#include "audio_engine.h"

int audio_engine_render(const uint8_t* sep_data, size_t sep_len,
                        const uint8_t* vh_data, size_t vh_len,
                        const uint8_t* vb_data, size_t vb_len, int seq_index,
                        int output_rate, RenderOutput* out) {
  if (!sep_data || !vh_data || !vb_data || !out || output_rate <= 0)
    return -1;
  return render_bgm(sep_data, sep_len, vh_data, vh_len, vb_data, vb_len,
                    seq_index, output_rate, out);
}
