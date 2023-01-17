#include <am.h>
#include <nemu.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

#define CONFIG_SB_SIZE 0x10000

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = false;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = 0;
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  Area buf = ctl->buf;
  int len = buf.end - buf.start;
  int count = inl(AUDIO_COUNT_ADDR);

  if (len + count > CONFIG_SB_SIZE) {
    printf("wait!!!\n");
    return; // wait
  }

  uint8_t *base = (uint8_t *)AUDIO_SBUF_ADDR;
  base += count;
  memcpy(base, buf.start, len);

  int after = count + len;
  outl(AUDIO_COUNT_ADDR, after);

  printf("input: %d %d\n", count, after);
}
