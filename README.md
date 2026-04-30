<h4 align="center">
    <br> <img src="img/system-blk-diagram.png" width="800" alt="System Block Diagram">
</h4>

<h4 align="center">
    Flappy Bird Zynq-7000 SoC Implementation
</h4>

<p align="center">
    <a href="#description">Description</a> •
    <a href="#hw-sw-architecture">Architecture</a> •
    <a href="#technical-specifications">Specifications</a> •
    <a href="#project-structure">Structure</a>
    <br>
    <a href="#getting-started">Setup</a> •
    <a href="#controls">Controls</a> •
    <a href="#future-work">Future Work</a>
</p>

## Description

A high-performance hardware-accelerated implementation of Flappy Bird for the **Xilinx Zynq-7000 SoC**. This project demonstrates a complete HW/SW co-design: the ARM Cortex-A9 Processing System (PS) manages game state and physics, while the Programmable Logic (PL) handles real-time video generation and sprite rendering.

## HW-SW Architecture

The system utilizes the AXI4-Lite protocol to bridge the PS and PL, ensuring low-latency communication between the game engine and the display controller.

### Logic Partitioning
* **Processing System (PS):** Runs C code to calculate bird trajectories, gravity, and collision logic. It writes coordinates to the PL through memory-mapped registers.
* **Programmable Logic (PL):** A custom RTL video engine that renders sprites and background layers at a constant 60 FPS without taxing the CPU.

## Technical Specifications

### Physics & Dynamics
The bird's vertical position $y$ is updated in the PS using a simple Euler integration of gravity:
$$y_{t+1} = y_t + v_t \Delta t$$
$$v_{t+1} = v_t + g \Delta t$$

### Video Pipeline
The VGA controller generates pixel-perfect timing for 640x480 resolution:
* **Pixel Clock:** 25.175 MHz
* **Refresh Rate:** 60 Hz
* **Interface:** AXI4-Lite for coordinate updates.

## Project Structure

```text
Zynq-SoC-Flappy-Bird/
├── src/
│   ├── core_0.c             # Main C application (Game Logic)
│   ├── GameManager/         # Game state & physics handlers
│   ├── Entity/              # Object definitions (Bird, Pipes)
│   └── Sprites/             # Sprite bitmaps and rendering logic
├── img/
│   └── system-blk-diagram.png
├── font8x8.c                # Hardware font renderer
└── lscript.ld               # Linker script for ARM PS
