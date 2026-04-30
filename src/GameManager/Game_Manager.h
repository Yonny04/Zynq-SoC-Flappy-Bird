#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "xil_types.h"
#include "xtmrctr.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "sleep.h"
#include "xil_cache.h"
#include "sprite_overlay.h"

// --- Hardware Definitions ---
#define BTNS_DEVICE_ID          XPAR_AXI_GPIO_0_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID  XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define INTC_TIMER_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
#define BTN_INT_MASK            XGPIO_IR_CH1_MASK
#define BTN_INT 				XGPIO_IR_CH1_MASK
#define LFSR_BASE               XPAR_LFSR_V2_0_S00_AXI_BASEADDR
#define FRAMEBUFFER_ADDR 0x00900000

// Screen & Backdrop Constants
#define SCREEN_W 1280
#define SCREEN_H 1024
#define X1 0
#define Y1 0

// COMM pointer
#define COMM_VAL (*(volatile unsigned long *)(0xFFFF0000))

// --- Game State Definitions ---
typedef enum {
    STATE_MENU,
	STATE_READY,
    STATE_PLAYING,
    STATE_PAUSED,
	STATE_DYING,
    STATE_GAME_OVER
} GameState;

// --- Game State Definitions ---
extern const unsigned int backdrop[];
extern const unsigned int title[];
extern const unsigned int game_over[];
extern const unsigned int press_to_start[];
extern const unsigned int start[];
extern const unsigned int main_menu[];
extern const unsigned int main_menu_buttons[]; // 64 x 307
extern const unsigned int button_right[]; // 64 x 307
extern const unsigned int button_left[]; // 64 x 307
extern const unsigned int button_down[]; // 64 x 307
extern const unsigned int final_score[];
extern const unsigned int pause_title[]; // 512 x 256
extern const unsigned int pause_menu[]; // 256 x 304
extern const unsigned int play_again[]; //256 x 128

// Image pointer for score drawing
extern int * image_buffer_pointer;

// --- Global Instances ---
XScuGic InterruptController;
XGpio BTNInst;
XTmrCtr TimerInst;

// Button Masks
#define BTN_0_FLAP   0x01 // Start Game / Flap
#define BTN_1_PAUSE  0x02 // Pause / Unpause
#define BTN_2_DIE    0x04 // Simulate Game Over
#define BTN_CENTER 	 0x01 // Start Game / Flap
#define BTN_BOTTOM   0x02 // High Score
#define BTN_LEFT     0x04 // Volume Down
#define BTN_RIGHT    0x08 // Volume Up
#define BTN_UP       0x10 // (Unused for now)


// --- Constants ---
extern int global_score;
extern int high_score;

// --- Methods ---
/**
 * Helps draw a png image raw data on the screen, ignoring transparent pixels
 *
 */
void DrawImageAlpha(int startX, int startY, int imgW, int imgH, const unsigned int* sourceData);

/**
 * Draws the static background and flushes to DDR
 *
 */
void DrawStaticBackdrop();

/**
 * Sets button interrupt flag
 *
 */
void BTN_Intr_Handler(void *InstancePtr);

/**
 * Sets timer interrupt flag based on frame rate
 *
 */
void Timer_Intr_Handler(void *CallBackRef, u8 TmrCtrNumber);

/**
 * Initializes interrupts
 *
 */
int Interrupt_Init(u16 DeviceId);

/**
 * Plays game over collision sounds + updates state + negative velocity for special bounce effect
 *
 */
void GameOver();

/**
 * Sets shared comm value to play point sound
 *
 */
void PlayPointSound();

#endif
