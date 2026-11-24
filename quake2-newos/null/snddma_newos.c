/*
 * snddma_newos.c - Quake 2 sound DMA backend for NEWOS.
 * Exposes an 8-bit mono DMA buffer to the portable Quake mixer; on Submit,
 * the buffer is expanded to 16-bit stereo PCM and handed to SYS_AUDIO_WRITE
 * (the HDA backend added to NEWOS). This is what gives Quake 2 real sound.
 */
#include "../client/client.h"
#include "../client/snd_loc.h"
#include "newos_platform.h"
#include <stdlib.h>
#include <string.h>

#define Q2_SPEED      48000
#define Q2_SAMPLES    (Q2_SPEED / 4)          /* 0.25 s per submission */
#define Q2_CHUNK      256

dma_t  dma;

static long  audio_h = -1;
static byte  q2_dma_buffer[Q2_SAMPLES];       /* 8-bit mono */
static short q2_pcm[Q2_SAMPLES * 2];          /* 16-bit stereo */

qboolean SNDDMA_Init(void)
{
    audio_h = sys_audio_open();
    if (audio_h < 0)
        return false;               /* no codec: Quake runs silent */

    memset(&dma, 0, sizeof(dma));
    dma.channels = 1;
    dma.samples = Q2_SAMPLES;
    dma.submission_chunk = Q2_CHUNK;
    dma.samplebits = 8;
    dma.speed = Q2_SPEED;
    dma.buffer = q2_dma_buffer;
    dma.samplepos = 0;
    return true;
}

int SNDDMA_GetDMAPos(void)
{
    /* The mixer reads this to know where to write. With one-shot playback
     * we advance a full buffer per Submit; return 0 while painting. */
    return dma.samplepos;
}

void SNDDMA_Shutdown(void)
{
    if (audio_h >= 0)
        sys_audio_close(audio_h);
    audio_h = -1;
}

void SNDDMA_BeginPainting(void)
{
}

void SNDDMA_Submit(void)
{
    int i;
    if (audio_h < 0)
        return;

    /* 8-bit unsigned mono -> 16-bit signed stereo (center at 128). */
    for (i = 0; i < Q2_SAMPLES; i++) {
        short v = (short)(((int)q2_dma_buffer[i] - 128) << 8);
        q2_pcm[2 * i] = v;
        q2_pcm[2 * i + 1] = v;
    }
    sys_audio_write(audio_h, q2_pcm, (u32)(Q2_SAMPLES * 4));
    dma.samplepos = 0;
}
