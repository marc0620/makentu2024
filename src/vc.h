#ifndef VC_H
#define VC_H
#include "FreeRTOS.h"
#include "base_types.h"
#include "cybsp.h"
#include "cyhal.h"
#include "queue.h"
#include "stdlib.h"
#define FRAME_SIZE (480u)
#define SAMPLE_RATE_HZ (16000u)
#define DECIMATION_RATE (96u)
#define AUDIO_SYS_CLOCK_HZ (24576000u)
#define PDM_DATA (P10_5)
#define PDM_CLK (P10_4)
#include "cyberon_asr.h"
extern QueueHandle_t voice_command_q;
typedef enum { PINGPONG_SWAP } voice_command_t;
void vc_init();
void vc_task();
void vcsetstate();
void vcfalsestate();
#endif