#include "Bird.h"
#include "../Pipe/Pipe.h"
#include "../../GameManager/Game_Manager.h"

int bird_curr_y = BIRD_Y1;
float velocity = 0.0f;
float gravity = 0.3f;
float flap_power = -7.0f;

void UpdateHardwareBird(int x, int y) {
    Xil_Out32(SPRITE_BASE + 0, x);
    Xil_Out32(SPRITE_BASE + 4, y);
}

void HideHardwareSprites() {
    // Push the bird and all pipes way off-screen (Y = 2000)
    Xil_Out32(SPRITE_BASE + 0, 2000);  // Bird X
    Xil_Out32(SPRITE_BASE + 4, 2000);  // Bird Y

    Xil_Out32(SPRITE_BASE + 8,  2000); // P1 X
    Xil_Out32(SPRITE_BASE + 12, 2000); // P1 Gap
    Xil_Out32(SPRITE_BASE + 16, 2000); // P2 X
    Xil_Out32(SPRITE_BASE + 20, 2000); // P2 Gap
    Xil_Out32(SPRITE_BASE + 24, 2000); // P3 X
    Xil_Out32(SPRITE_BASE + 28, 2000); // P3 Gap
}
