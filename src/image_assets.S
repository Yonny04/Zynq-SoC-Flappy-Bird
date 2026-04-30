.section .rodata
.global backdrop
.global title
.global game_over
.global press_to_start
.global main_menu
.global main_menu_buttons
.global button_right
.global button_left
.global button_down
.global final_score
.global pause_title
.global pause_menu
.global play_again

.align 4 // 1280x1024
backdrop:
	.incbin "../src/Sprites/backdrop.data"

.align 4 // 384x128
title:
	.incbin "../src/Sprites/flappy_title.data"

.align 4 // 512x67
game_over:
	.incbin "../src/Sprites/game_over.data"

.align 4 // 256x128
press_to_start:
	.incbin "../src/Sprites/press_to_start.data"

.align 4 // 256x307
main_menu:
	.incbin "../src/Sprites/main_menu.data"

.align 4 // 64x307
main_menu_buttons:
	.incbin "../src/Sprites/main_menu_buttons.data"

.align 4 // 64x307
button_right:
	.incbin "../src/Sprites/mm_volume_up.data"

.align 4 // 64x307
button_left:
	.incbin "../src/Sprites/mm_volume_down.data"

.align 4 // 64x307
button_down:
	.incbin "../src/Sprites/mm_high_score.data"

.align 4 // 256x128
final_score:
	.incbin "../src/Sprites/final_score.data"

.align 4 // 512x256
pause_title:
	.incbin "../src/Sprites/pause_title.data"

.align 4 // 256 x 304
pause_menu:
	.incbin "../src/Sprites/pause_menu.data"

.align 4 // 256 x 304
play_again:
	.incbin "../src/Sprites/play_again.data"
