#include "Game_Manager.h"
#include "../Entity/Bird/Bird.h"
#include "../font8x8.h"

extern GameState current_state;

// --- Button Flags ---
volatile bool BTN_INTR_FLG;
volatile bool TIMER_INTR_FLG;

// --- Constants ---
int global_score = 0;
int high_score = 0;

void DrawStaticBackdrop() {
    unsigned int *fb = (unsigned int *)FRAMEBUFFER_ADDR;
    // Loop through the rows (1024 of them) of the screen copying 1280 pixels at a time
    for (int y = 0; y < SCREEN_H; y++) {
        memcpy(&fb[y * SCREEN_W], &backdrop[y * SCREEN_W], SCREEN_W * 4);
    }
    Xil_DCacheFlushRange((UINTPTR)FRAMEBUFFER_ADDR, SCREEN_W * SCREEN_H * 4);
}

void DrawImageAlpha(int startX, int startY, int imgW, int imgH, const unsigned int* sourceData) {
    unsigned int *framebuffer = (unsigned int *)FRAMEBUFFER_ADDR;

    // Make sure we are drawing within the bounds of the screen
    if (startY < 0 || (startY + imgH) > SCREEN_H || startX < 0 || (startX + imgW) > SCREEN_W) {
        return;
    }

    // Loop through image pixels row by row
    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {

        	// Current pixel offset
            unsigned int pixel = sourceData[y * imgW + x];

            // Transparency pixel check
            // Shift by 24 bits so we aren't comparing against a large number
            // Ignore if value is 0 (transparent)
            if ((pixel & 0xFF000000) >> 24 > 0) {
                framebuffer[(startY + y) * SCREEN_W + (startX + x)] = pixel;
            }
        }
        // Flush the line to DDR
        Xil_DCacheFlushRange((UINTPTR)&framebuffer[(startY + y) * SCREEN_W + startX], imgW * 4);
    }
}

#if 0 // Test function to see bounds of the screen
// Specific monitors have black bars on the side, not an issue with our software/hardware
#define STRIDE 2048

void TestRedScreen() {
    unsigned int *fb = (unsigned int *)FRAMEBUFFER_ADDR;
    unsigned int red_pixel = 0x00FF0000;

    // make all pixels red to see if black bars dissapear
    for (int y = 0; y < 1024; y++) {
        for (int x = 0; x < STRIDE; x++) {
            fb[y * STRIDE + x] = red_pixel;
        }
    }

    // Flush CPU cache to DDR
    Xil_DCacheFlushRange((UINTPTR)FRAMEBUFFER_ADDR, STRIDE * 1024 * 4);
    printf("Red screen test complete. Stride: %d\n\r", STRIDE);
}

#endif


// --- Interrupt Handlers ---
void BTN_Intr_Handler(void *InstancePtr) {
    XGpio_InterruptClear(&BTNInst, BTN_INT_MASK);
    BTN_INTR_FLG = true;
}

void Timer_Intr_Handler(void *CallBackRef, u8 TmrCtrNumber) {
    TIMER_INTR_FLG = true;
}

void GameOver() {
	COMM_VAL = 2;
    current_state = STATE_DYING;
    // Give the bird negative velocity, causing it to bounce upwards when coliision is detected
    velocity = -8.0f;

}

void PlayPointSound(){
	COMM_VAL = 3;
}

void draw_char(int* frame_buffer, char c, int start_x, int start_y, int color, int scale) {
    if (c < 0 || c > 127) return;

    for (int y = 0; y < 8; y++) {
        unsigned char row = font8x8_basic[(int)c][y];
        for (int x = 0; x < 8; x++) {
            if (row & (1 << x)) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int px = start_x + (x * scale) + sx;
                        int py = start_y + (y * scale) + sy;
                        if (px >= 0 && px < SCREEN_W && py >= 0 && py < SCREEN_H) {
                            frame_buffer[(py * SCREEN_W) + px] = color;
                        }
                    }
                }
            }
        }
    }
}

void draw_text(int x, int y, const char* str, int color, int scale) {
    int current_x = x;
    int* fb_ptr = (int*)FRAMEBUFFER_ADDR;

    // Loop until we hit the end of the string, not just 10 chars
    for (int i = 0; str[i] != '\0'; i++) {
        draw_char(fb_ptr, str[i], current_x, y, color, scale);
        current_x += (8 * scale); // Advance X by character width * scale
    }
}

int Interrupt_Init(u16 DeviceId) {
    XScuGic_Config *GicConfig = XScuGic_LookupConfig(DeviceId);
    XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &InterruptController);
    Xil_ExceptionEnable();

    XScuGic_Connect(&InterruptController, INTC_GPIO_INTERRUPT_ID, (Xil_ExceptionHandler)BTN_Intr_Handler, (void *)&BTNInst);
    XScuGic_Connect(&InterruptController, INTC_TIMER_INTERRUPT_ID, (Xil_InterruptHandler)XTmrCtr_InterruptHandler, (void *)&TimerInst);

    XScuGic_Enable(&InterruptController, INTC_GPIO_INTERRUPT_ID);
    XScuGic_Enable(&InterruptController, INTC_TIMER_INTERRUPT_ID);
    return XST_SUCCESS;
}

