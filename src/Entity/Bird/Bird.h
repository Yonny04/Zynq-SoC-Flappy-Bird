#ifndef BIRD_H
#define BIRD_H

// IP Base Addresses
#define SPRITE_BASE             XPAR_SPRITE_OVERLAY_0_S00_AXI_BASEADDR

// Bird Start
#define BIRD_X1 (200)    		// Fixed X position for the bird
#define BIRD_Y1 (SCREEN_H/2)	// Bird starting position

// --- Bird Constants ---
extern int bird_curr_y;
extern float velocity;
extern float gravity;
extern float flap_power;

/**
 * Update memory addresses for bird x and y position
 *
 */
void UpdateHardwareBird(int x, int y);

/**
 * Update memory addresses for bird x, bird y, and pipe x to hide off screen
 *
 */
void HideHardwareSprites();

#endif
