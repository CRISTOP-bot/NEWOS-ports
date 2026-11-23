#include "voxel/keymap.h"
#include "voxel/app.h"

void voxel_key_decoder_init(struct voxel_key_decoder *decoder)
{
    if (!decoder)
        return;
    decoder->esc = 0;
    decoder->bracket = 0;
    decoder->delete_seq = 0;
    decoder->pending = 0;
}

static uint32_t plain(unsigned char byte)
{
    if (byte == 0x7fu)
        return VOXEL_KEY_BACKSPACE;
    if (byte == '\r' || byte == '\n')
        return VOXEL_KEY_ENTER;
    if (byte == '\t')
        return VOXEL_KEY_TAB;
    if (byte >= 1u && byte <= 26u)
        return VOXEL_KEY_CTRL | (uint32_t)('a' + byte - 1u);
    return (uint32_t)byte;
}

uint32_t voxel_key_decode(struct voxel_key_decoder *decoder,
                          unsigned char byte)
{
    uint32_t result;

    if (!decoder)
        return 0;

    if (decoder->pending) {
        result = plain((unsigned char)decoder->pending);
        decoder->pending = 0;
        return result;
    }

    if (decoder->delete_seq) {
        decoder->delete_seq = 0;
        decoder->bracket = 0;
        return byte == '~' ? VOXEL_KEY_DELETE : 0;
    }

    if (decoder->bracket) {
        decoder->bracket = 0;
        switch (byte) {
        case 'A': return VOXEL_KEY_UP;
        case 'B': return VOXEL_KEY_DOWN;
        case 'C': return VOXEL_KEY_RIGHT;
        case 'D': return VOXEL_KEY_LEFT;
        case '3': decoder->delete_seq = 1; return 0;
        default: return 0;
        }
    }

    if (decoder->esc) {
        decoder->esc = 0;
        if (byte == '[') {
            decoder->bracket = 1;
            return 0;
        }
        /* A lone ESC is a key of its own; deliver the next byte afterwards. */
        decoder->pending = byte;
        return VOXEL_KEY_ESCAPE;
    }

    if (byte == 0x1bu) {
        decoder->esc = 1;
        return 0;
    }
    return plain(byte);
}
