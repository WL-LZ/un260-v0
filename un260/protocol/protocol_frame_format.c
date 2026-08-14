#include "protocol_frame_format.h"

static const char g_hex_digits[] = "0123456789ABCDEF";

size_t protocol_frame_format_hex(const uint8_t *data,
                                 size_t len,
                                 char *output,
                                 size_t output_size)
{
    size_t pos = 0;

    if (output == NULL || output_size == 0U) {
        return 0;
    }
    output[0] = '\0';
    if (data == NULL) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        size_t required = i == 0U ? 2U : 3U;

        if (pos + required >= output_size) {
            break;
        }
        if (i > 0U) {
            output[pos++] = ' ';
        }
        output[pos++] = g_hex_digits[data[i] >> 4];
        output[pos++] = g_hex_digits[data[i] & 0x0FU];
    }
    output[pos] = '\0';
    return pos;
}
