#include "Entity/Bird/Bird.h"
#include "GameManager/Game_Manager.h"
#include "Entity/Pipe/Pipe.h"
#include "font8x8.h"

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

#define sev() __asm__("sev")
#define ARM1_STARTADR 0xFFFFFFF0
#define ARM1_BASEADDR 0x10080000
#define COMM_VAL (*(volatile unsigned long *)(0xFFFF0000))
#define VOLUME_VAL  (*(volatile unsigned long *)(0xFFFF0004)) // 4 byte offset

int menu();

// Pointer to image buffer memory address
int * image_buffer_pointer = (int *)0x00900000;

// Initialize game state
GameState current_state = STATE_MENU;
uint32_t prev_btn_state = 0;

// Interrupt flags for push buttons and timer
bool BTN_INTR_FLG = false;
bool TIMER_INTR_FLG = false;

int main() {
    XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
    XGpio_InterruptEnable(&BTNInst, BTN_INT_MASK);
    XGpio_InterruptGlobalEnable(&BTNInst);

    XTmrCtr_Initialize(&TimerInst, XPAR_AXI_TIMER_0_DEVICE_ID);
    XTmrCtr_SetHandler(&TimerInst, (XTmrCtr_Handler)Timer_Intr_Handler, &TimerInst);
    XTmrCtr_SetResetValue(&TimerInst, 0, 0xFFE67698);
    XTmrCtr_SetOptions(&TimerInst, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

    Interrupt_Init(XPAR_PS7_SCUGIC_0_DEVICE_ID);

    // Initial Screen Setup
    // Clear the frame buffer before drawing the backdrop
    memset((void*)FRAMEBUFFER_ADDR, 0, SCREEN_W * SCREEN_H * 4);
    printf("Drawing Full-Screen Backdrop...\n\r");
    DrawStaticBackdrop();

    // Hide bird and pipes off screen for the main menu
    HideHardwareSprites();

    COMM_VAL = 0;

    // Disable cache on OCM
    // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0
    Xil_SetTlbAttributes(0xFFFF0000,0x14de2);
    Xil_Out32(ARM1_STARTADR, ARM1_BASEADDR);
    dmb(); //waits until write has finished

    // Wake up cores
    sev();

    // Start timer
    XTmrCtr_Start(&TimerInst, 0);
    menu();

    return 1;
}

int menu(){
	bool state_init = true; // Flag to execute specific actions once upon state change
	global_score = 0;
	high_score = 0;

	uint32_t menu_drawn_btns = 0;
	char high_score_str[32];

	// Variables to help with score colour change
	int prev_global_score = 0;
	int score_color_timer = 0;

	// Variables to help with high score flashing
	int flash_timer = 0;
	bool game_over_ui_drawn = false;

	// Volume control
	VOLUME_VAL = 50;

	while(1) {
		if(TIMER_INTR_FLG == true) {
			TIMER_INTR_FLG = false;

			// Read Buttons & Detect Rising Edges (low to high)
			u32 current_btn_state = XGpio_DiscreteRead(&BTNInst, 1);
			u32 btn_presses = current_btn_state & ~prev_btn_state;
			prev_btn_state = current_btn_state;
			BTN_INTR_FLG = false;

			// Game loop state machine
			switch (current_state) {

				case STATE_MENU:
					// Draw main menu elements (Runs Once)
					if (state_init) {
						DrawStaticBackdrop();
						DrawImageAlpha((SCREEN_W / 2) - 192, 256, 384, 128, title);
						DrawImageAlpha((SCREEN_W / 2) - 96, 448, 256, 307, main_menu);

						// Draw buttons showcasing main menu options
						DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, main_menu_buttons);

						// Set init to false to only draw once
						state_init = false;

						// Reset button tracker for button highlights
						menu_drawn_btns = 0;
					}

					// Bitmask to handle button press highlights when selecting volume and high score options
					u32 held_dpad = current_btn_state & (BTN_LEFT | BTN_RIGHT | BTN_BOTTOM);

					// Held makes sure we only enter loop once rather than actively looping
					if (held_dpad != menu_drawn_btns) {

						// Draw default buttons (all white/orange)
						DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, main_menu_buttons);

						// Overwrite with specific button (green) to visually indicate which button is pressed
						if (held_dpad & BTN_RIGHT) {
							DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, button_right);
						}
						if (held_dpad & BTN_LEFT) {
							DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, button_left);
						}
						if (held_dpad & BTN_BOTTOM) {
							DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, button_down);
						}

						// Save this state so we don't redraw it again next frame
						menu_drawn_btns = held_dpad;
					}

					// Center button starts game
					if (btn_presses & BTN_CENTER) {
						current_state = STATE_READY;
						state_init = true;
						printf("Get Ready!\n\r");
					}

					// Right button increases volume + plays sound to indicate new change
					if (btn_presses & BTN_RIGHT) {
						// Increase volume flag for Core 1
						if(VOLUME_VAL < 100) {
							VOLUME_VAL += 10;
						}
						COMM_VAL = 3;

					}

					// Left button decreases volume + plays sound to indicate new change
					if (btn_presses & BTN_LEFT) {
						// Decrease volume flag for Core 1
						if(VOLUME_VAL > 0) {
							VOLUME_VAL -= 10;
						}
						COMM_VAL = 3;
					}

					// Right button display high score (hidden if not clicked, stays on screen when clicked)
					if (btn_presses & BTN_BOTTOM) {
						sprintf(high_score_str, "%d", high_score);
						draw_text((SCREEN_W / 2) + 256 - 96, 710, high_score_str, 0xFFFFFF, 6);
						Xil_DCacheFlushRange((UINTPTR)FRAMEBUFFER_ADDR, SCREEN_W * SCREEN_H * 4);
					}
					break;


				case STATE_READY:
					// Only draw the ready graphics ONCE
					// Simulates the original games "tap to start screen" - "press btnc to start"
					if (state_init) {
						DrawStaticBackdrop(); // Clear the main menu elements by drawing backdrop again
						DrawImageAlpha((SCREEN_W / 2) - 128, 256, 256, 128, press_to_start);

						// Initial bird X/Y values
						bird_curr_y = BIRD_Y1;
						UpdateHardwareBird(BIRD_X1, bird_curr_y);

						// Set init to false since we only need to draw once
						state_init = false;
					}

					// BTN_0_FLAP = BTNC
					// Initializes game when player clicks button
					if (btn_presses & BTN_0_FLAP) {
						InitPipes();

						velocity = 0;
						global_score = 0;

						// Variables to help with dynamic score colour
						prev_global_score = 0;
						score_color_timer = 0;

						// Clear "press btnc to start" elements and transition to playing
						DrawStaticBackdrop();
						current_state = STATE_PLAYING;
						state_init = true;
					}
					break;

				case STATE_PLAYING:

					// Check for flap and increase bird velocity + play flap sound clip
					if (btn_presses & BTN_0_FLAP) {
						velocity = flap_power;
						COMM_VAL = 1;
					}

					// Check for pause menu
					if (btn_presses & BTN_1_PAUSE) {
						current_state = STATE_PAUSED;
						state_init = true;
						printf("Game Paused\n\r");
					}

					// Score board display
					char score_buf[32];
					sprintf(score_buf, "SCORE:%d", global_score);

					// Score board flash logic
					// Check if score increased this frame
					if (global_score > prev_global_score) {
						// Flash colour for 30 frames and update tracker
						score_color_timer = 30;
						prev_global_score = global_score;
					}

					// Default score clour = white
					uint32_t text_color = 0xFFFFFF;

					if (score_color_timer > 0) {
						text_color = 0x00F00; // Flash color: Black
						score_color_timer--;   // Count down 1 tick per frame
					}

					// Cover score section with backdrop when updating score to prevent overlapping numbers
					int startRow = 120;
					int endRow = 170;
					int startCol = 1000;
					int endCol = 1300;

					for(int row = startRow; row < endRow; row++) {
						for(int col = startCol; col < endCol; col++) {
							// Calculate index for the backdrop array
							int pixel_index = row * SCREEN_W + col;

							// Overwrite the framebuffer with the original backdrop pixel
							*((int*)(FRAMEBUFFER_ADDR + (pixel_index * 4))) = backdrop[pixel_index];
						}
					}

					// Update the score with the text color based on logic above
					draw_text(1030, 120, score_buf, text_color, 3);

					// Flush the old score
					UINTPTR flush_start = (UINTPTR)FRAMEBUFFER_ADDR + (startRow * SCREEN_W * 4);
					u32 flush_size = ((endRow - startRow) * SCREEN_W * 4);
					Xil_DCacheFlushRange(flush_start, flush_size);

					// Physics & Logic Updates every frame
					velocity += gravity;
					bird_curr_y += (int)velocity;

					// Check bird bounds
					if(bird_curr_y > 855) { bird_curr_y = 855; velocity = 0; }
					if(bird_curr_y < 0)   { bird_curr_y = 0;   velocity = 0; }

					UpdatePipeLogic();

					// Hardware Updates
					UpdateHardwareBird(BIRD_X1, bird_curr_y);
					UpdateHardwarePipes();
					break;

				case STATE_PAUSED:
					// Initialize pause menu once
					if (state_init) {
						// Black out the screen and push to DDR
						memset((void*)FRAMEBUFFER_ADDR, 0, SCREEN_W * SCREEN_H * 4);
						Xil_DCacheFlushRange((UINTPTR)FRAMEBUFFER_ADDR, SCREEN_W * SCREEN_H * 4);

						// Hide the hardware sprites
						HideHardwareSprites();

						// Draw pause title "game paused" (Centered at top)
						DrawImageAlpha((SCREEN_W / 2) - 256, 128, 512, 256, pause_title);

						// Draw pause menu options "resume, volume up/down, reset"
						DrawImageAlpha((SCREEN_W / 2) - 128 + 32, 448, 256, 304, pause_menu);

						// Draw buttons
						DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, main_menu_buttons);

						// Reset button tracker for pause menu
						state_init = false;
						menu_drawn_btns = 0;
					}

					// Same code as main menu to handle button selection and highlights
					u32 held_dpad_pause = current_btn_state & (BTN_LEFT | BTN_RIGHT | BTN_BOTTOM);

					if (held_dpad_pause != menu_drawn_btns) {
						DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, main_menu_buttons);

						if (held_dpad_pause & BTN_RIGHT) {
							DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, button_right);
						}
						if (held_dpad_pause & BTN_LEFT) {
							DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, button_left);
						}
						if (held_dpad_pause & BTN_BOTTOM) {
							DrawImageAlpha((SCREEN_W / 2) - 192, 448, 64, 307, button_down);
						}

						menu_drawn_btns = held_dpad_pause;
					}

					if (btn_presses & BTN_CENTER) {
						DrawStaticBackdrop();
						current_state = STATE_PLAYING;
						printf("Game Resumed\n\r");
					}
					if (btn_presses & BTN_BOTTOM) {
						current_state = STATE_MENU;
						state_init = true;
						global_score = 0;
						printf("Resetting to Main Menu\n\r");
					}
					if (btn_presses & BTN_RIGHT) {
						if(VOLUME_VAL < 100) {
							VOLUME_VAL += 10;
						}
						COMM_VAL = 3;
					}
					if (btn_presses & BTN_LEFT) {
						if(VOLUME_VAL > 0) {
							VOLUME_VAL -= 10;
						}
						COMM_VAL = 3;
					}
					break;

				// Special effect state
				case STATE_DYING:
					// Apply gravity (no flapping allowed here)
					velocity += gravity;
					bird_curr_y += (int)velocity;

					// Check if the bird has hit the floor
					if(bird_curr_y >= 855) {
						bird_curr_y = 855;

						// Force the hardware to draw the bird exactly on the floor
						UpdateHardwareBird(BIRD_X1, bird_curr_y);

						// Wait 0.5 seconds (500,000 microseconds)
						usleep(500000);

						// Transition to actual game over screen
						current_state = STATE_GAME_OVER;
						state_init = true;
					} else {
						// Keep updating birds position until it hits the floor
						UpdateHardwareBird(BIRD_X1, bird_curr_y);
					}
					break;

				case STATE_GAME_OVER:
					// Initialize game end state once (high score sound)
					if (state_init) {
						game_over_ui_drawn = false; // Reset the UI flag

						// Check for a new high score
						if(global_score > high_score && global_score > 0) {
							high_score = global_score;
							flash_timer = 60; // 60 frames
						} else {
							flash_timer = 0; // no flash since no new high score
						}

						state_init = false;
					}

					// Flashing logic
					if (flash_timer > 0) {
						// Flash every 10 frames
						if (flash_timer % 10 == 0) {
#if 0 // Flash screen black and white (sound alone seems better so commenting out)
							// Alternate between White and Black
							uint32_t flash_color = ((flash_timer / 10) % 2 == 0) ? 0xFFFFFFFF : 0xFF000000;

							memset((void*)FRAMEBUFFER_ADDR, flash_color, SCREEN_W * SCREEN_H * 4);

							// Push the color to the screen
							Xil_DCacheFlushRange((UINTPTR)FRAMEBUFFER_ADDR, SCREEN_W * SCREEN_H * 4);
#endif

							COMM_VAL = 3;
						}

						flash_timer--; // Count down 1 tick per frame
					}

					// Draw the normal Game Over Screen just once
					if (flash_timer == 0 && !game_over_ui_drawn) {

						// Clear the screen to Black
						memset((void*)FRAMEBUFFER_ADDR, 0, SCREEN_W * SCREEN_H * 4);
						Xil_DCacheFlushRange((UINTPTR)FRAMEBUFFER_ADDR, SCREEN_W * SCREEN_H * 4);

						// Hide sprites off screen since the are drawn overtop of the vga signal
						HideHardwareSprites();

						// Game over menu elements (title, score letters, play again)
						DrawImageAlpha((SCREEN_W / 2) - 256, 256, 512, 67, game_over);
						DrawImageAlpha((SCREEN_W / 2) - 128 - 64 - 32, 448, 256, 128, final_score);
						DrawImageAlpha((SCREEN_W / 2) - 128, 512 + 256, 256, 128, play_again);

						// Display current and high scores
						char current_score_str[32];
						char high_score_str[32];

						sprintf(current_score_str, "%d", global_score);
						sprintf(high_score_str, "%d", high_score);

						draw_text((SCREEN_W / 2) + 50, 480 - 16, current_score_str, 0xFFFFFF, 3);
						draw_text((SCREEN_W / 2) + 50, 530 + 12, high_score_str, 0xFFFFFF, 3);

						Xil_DCacheFlushRange((UINTPTR)FRAMEBUFFER_ADDR, SCREEN_W * SCREEN_H * 4);

						// Mark the UI as drawn so we don't redraw it every frame
						game_over_ui_drawn = true;
					}

					// Input Handling (Locked out until the screen finishes flashing and draws the UI)
					if (game_over_ui_drawn) {
						if (btn_presses & BTN_0_FLAP) {
							current_state = STATE_MENU;
							state_init = true;
							global_score = 0;
							printf("Returned to Menu\n\r");
						}
					}
					break;
			}
		}
	}
    return 0;

}
