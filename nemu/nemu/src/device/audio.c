#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#include <stdio.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static void audio_play(void *userdata, uint8_t *stream, int len) {
  int nread = len;
  int count = audio_base[reg_count];

  if (count < len) nread = count;
  memcpy(stream, sbuf, nread);
  if (len > nread) memset(stream + nread, 0, len - nread);

  int after = count - nread;
  audio_base[reg_count] = after;

  for (int i = 0; i < count - nread; ++i)
    sbuf[i] = sbuf[i + nread];
    
  memset(sbuf + count - nread, 0, nread);

  printf("output: %d %d\n", count, after);
}

static void init_audio_sdl() {
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS;  // 假设系统中音频数据的格式总是使用 16 位有符号数来表示
  s.userdata = NULL;        // 不使用

  s.freq = audio_base[reg_freq];
  s.channels = audio_base[reg_channels];
  s.samples = audio_base[reg_samples];
  s.callback = audio_play;

  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&s, NULL);
  SDL_PauseAudio(0);

  // puts("init_audio_sdl");
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if (audio_base[reg_init] == false) {
    init_audio_sdl();
    audio_base[reg_init] = true;
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
