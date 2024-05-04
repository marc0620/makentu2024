// clang-format off
#include "vc.h"
#include "capsense_task.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>
// clang-format on
int vcstate = 0;
const cyhal_pdm_pcm_cfg_t pdm_pcm_cfg = {
    .sample_rate = SAMPLE_RATE_HZ,
    .decimation_rate = DECIMATION_RATE,
    .mode = CYHAL_PDM_PCM_MODE_LEFT,
    .word_length = 16,                    /* bits */
    .left_gain = CYHAL_PDM_PCM_MAX_GAIN,  /* dB */
    .right_gain = CYHAL_PDM_PCM_MAX_GAIN, /* dB */
};
QueueHandle_t voice_command_q;

volatile bool pdm_pcm_flag = false;
int16_t pdm_pcm_ping[FRAME_SIZE] = {0};
int16_t pdm_pcm_pong[FRAME_SIZE] = {0};
int16_t *pdm_pcm_buffer = &pdm_pcm_ping[0];
cyhal_pdm_pcm_t pdm_pcm;
cyhal_clock_t audio_clock;
cyhal_clock_t pll_clock;
void asr_callback(const char *function, char *message, char *parameter);
void pdm_pcm_isr_handler(void *arg, cyhal_pdm_pcm_event_t event);
void clock_init(void);
void asr_callback(const char *function, char *message, char *parameter);
void vc_init() {
  uint64_t uid;
  clock_init();

  cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
  cyhal_pdm_pcm_init(&pdm_pcm, PDM_DATA, PDM_CLK, &audio_clock, &pdm_pcm_cfg);
  cyhal_pdm_pcm_register_callback(&pdm_pcm, pdm_pcm_isr_handler, NULL);
  cyhal_pdm_pcm_enable_event(&pdm_pcm, CYHAL_PDM_PCM_ASYNC_COMPLETE, CYHAL_ISR_PRIORITY_DEFAULT, true);
  cyhal_pdm_pcm_start(&pdm_pcm);
  cyhal_pdm_pcm_read_async(&pdm_pcm, pdm_pcm_buffer, FRAME_SIZE);
  printf("\x1b[2J\x1b[;H");
  printf("===== Cyberon DSpotter Demo =====\r\n");
  uid = Cy_SysLib_GetUniqueId();
  printf("uniqueIdHi: 0x%08lX, uniqueIdLo: 0x%08lX\r\n", (uint32_t)(uid >> 32), (uint32_t)(uid << 32 >> 32));
  if (!cyberon_asr_init(asr_callback)) {
    printf("Cyberon ASR init failed\r\n");
    while (1)
      ;
  }
  printf("\r\nAwaiting voice input trigger command (\"Hello CyberVoice\"):\r\n");
}
void pdm_pcm_isr_handler(void *arg, cyhal_pdm_pcm_event_t event) {
  if (vcstate == 0) {
    return;
  }
  BaseType_t xYieldRequired;
  static bool ping_pong = false;
  (void)arg;
  (void)event;
  if (ping_pong) {
    cyhal_pdm_pcm_read_async(&pdm_pcm, pdm_pcm_ping, FRAME_SIZE);
    pdm_pcm_buffer = &pdm_pcm_pong[0];
  } else {
    cyhal_pdm_pcm_read_async(&pdm_pcm, pdm_pcm_pong, FRAME_SIZE);
    pdm_pcm_buffer = &pdm_pcm_ping[0];
  }

  ping_pong = !ping_pong;
  voice_command_t commmand = PINGPONG_SWAP;
  xYieldRequired = xQueueSendToBackFromISR(voice_command_q, &commmand, 0u);
  portYIELD_FROM_ISR(xYieldRequired);
}
void clock_init(void) {
  cyhal_clock_reserve(&pll_clock, &CYHAL_CLOCK_PLL[0]);
  cyhal_clock_set_frequency(&pll_clock, AUDIO_SYS_CLOCK_HZ, NULL);
  cyhal_clock_set_enabled(&pll_clock, true, true);

  cyhal_clock_reserve(&audio_clock, &CYHAL_CLOCK_HF[1]);
  cyhal_clock_set_source(&audio_clock, &pll_clock);
  cyhal_clock_set_enabled(&audio_clock, true, true);
}
void vc_task() {
  BaseType_t rtos_api_result;
  cy_status status;
  voice_command_t vc_cmd;

  while (1) {
    rtos_api_result = xQueueReceive(voice_command_q, &vc_cmd, portMAX_DELAY);
    if (rtos_api_result == pdPASS) {
      switch (vc_cmd) {
      case PINGPONG_SWAP:
        if (vcstate == 1)
          cyberon_asr_process(pdm_pcm_buffer, FRAME_SIZE);
        break;
      default:
        break;
      }
    }
  }
}
void vcsetstate() {
  if (vcstate == 0)
    cyhal_pdm_pcm_read_async(&pdm_pcm, pdm_pcm_ping, FRAME_SIZE);
  vcstate = 1;
  printf("setstate: %d\r\n", vcstate);
}
void vcfalsestate() {
  vcstate = 0;
  printf("falsestate: %d\r\n", vcstate);
}
void asr_callback(const char *function, char *message, char *parameter) { printf("[%s]%s(%s)\r\n", function, message, parameter); }
