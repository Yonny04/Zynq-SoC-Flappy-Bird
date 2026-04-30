#include <stdio.h>
#include <stdlib.h>

#include "Pipe.h"
#include "../Bird/Bird.h"
#include "../../GameManager/Game_Manager.h"


extern int current_state;
Pipe pipes[NUM_PIPES];

void InitPipes() {
    for (int i = 0; i < NUM_PIPES; i++) {
        // Start pipes off-screen to the right, staggered by spacing
        pipes[i].x = SCREEN_W + (i * PIPE_SPACING);
        pipes[i].gap_y = GetRandomGapY();
        pipes[i].passed = false;
    }
}

int GetRandomGapY() {
    return (GetHardwareRNG() % 600) + 100;
}

int GetHardwareRNG() {
    return (int)(Xil_In32(LFSR_BASE) & 0x7FFFFFFF);
}

void UpdateHardwarePipes() {
    // Pipe 1
    Xil_Out32(SPRITE_BASE + 8,  pipes[0].x);
    Xil_Out32(SPRITE_BASE + 12, pipes[0].gap_y);
    // Pipe 2
    Xil_Out32(SPRITE_BASE + 16, pipes[1].x);
    Xil_Out32(SPRITE_BASE + 20, pipes[1].gap_y);
    // Pipe 3
    Xil_Out32(SPRITE_BASE + 24, pipes[2].x);
    Xil_Out32(SPRITE_BASE + 28, pipes[2].gap_y);
}

void HidePipes() {
    Xil_Out32(SPRITE_BASE + 8,  2000); // P1 X
    Xil_Out32(SPRITE_BASE + 12, 2000); // P1 Gap
    Xil_Out32(SPRITE_BASE + 16, 2000); // P2 X
    Xil_Out32(SPRITE_BASE + 20, 2000); // P2 Gap
    Xil_Out32(SPRITE_BASE + 24, 2000); // P3 X
    Xil_Out32(SPRITE_BASE + 28, 2000); // P3 Gap
}


void UpdatePipeLogic() {
    int backdrop_left = X1;
    int backdrop_right = X1 + SCREEN_W;
    int pipe_width = 128;

    if(current_state != STATE_PLAYING) return;

    for (int i = 0; i < NUM_PIPES; i++) {
        pipes[i].x -= PIPE_SPEED;

        // 1. Collision Check (If this doesn't trigger, the bird is safe)
        if (CheckCollision(i)) {
            GameOver();
            return; // Exit early if game is over
        }

        // 2. Score Tracking
        // If bird passed the X-threshold and we haven't counted this pipe yet
        if (pipes[i].passed == 0 && BIRD_X1 > (pipes[i].x - (pipe_width / 3))) {
            PlayPointSound();
            global_score++;
            pipes[i].passed = 1;
        }

        // 3. Reset Pipe
        if (pipes[i].x < (backdrop_left - pipe_width)) {
            pipes[i].x = backdrop_right;
            pipes[i].gap_y = GetRandomGapY();
            pipes[i].passed = 0;
        }
    }
}

int CheckCollision(int i) {
    int pipe_visual_width = GAP_SIZE;
    int bird_visual_h = BIRD_SIZE;

    int vertical_forgiveness = 25;

    int hitbox_left = pipes[i].x - pipe_visual_width/2;
    int hitbox_right = pipes[i].x + pipe_visual_width/2;

    int actual_gap_top = pipes[i].gap_y - (GAP_SIZE / 2) + vertical_forgiveness;
    int actual_gap_bottom = pipes[i].gap_y + (GAP_SIZE / 2) + vertical_forgiveness;

    // Horizontal Collision
    if (BIRD_X1 + 16 > hitbox_left && BIRD_X1 < hitbox_right) {
        // Vertical Collision
        if (bird_curr_y < actual_gap_top || (bird_curr_y + bird_visual_h) > actual_gap_bottom) {
            return 1;
        }
    }

    if(bird_curr_y >= 855) return 1; // Floor collision
    return 0;
}
