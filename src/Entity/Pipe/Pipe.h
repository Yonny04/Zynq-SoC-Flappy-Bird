#ifndef PIPE_H
#define PIPE_H

#include <stdbool.h>

// Pipe Settings
#define NUM_PIPES 3
#define PIPE_SPEED 4
#define PIPE_SPACING 480 // horizontal pipe gap
#define GAP_SIZE 160 // vertical pipe gap
#define PIPE_WIDTH 128
#define BIRD_SIZE 6

typedef struct {
    int x;
    int gap_y;
    bool passed;
} Pipe;

/**
 * Initializes pipe array with x coord, y gap coord, and passed boolean
 *
 */
void InitPipes();

/**
 * Returns a random number between 100-700
 *
 */
int GetRandomGapY();

/**
 * Returns LSFR value from hardware
 *
 */
int GetHardwareRNG();

/**
 * Updates x and y memory addresses associated to pipes
 *
 */
void UpdateHardwarePipes();

/**
 *
 */
void HidePipes();

/**
 * Collision check, score tracking, and updating new pipe values
 *
 */
void UpdatePipeLogic();

/**
 * Checks if bird sprite bounds are within any of the pipes
 *
 */
int CheckCollision();


#endif
