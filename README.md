<h4 align="center">
    <br>
    <img src= "img/System_Blk_Diagram.png" width="800">
</h4>

<h4 align="center">
    Flappy Bird Zynq-7000 SoC Development Project
</h4>

<p align="center">
    <a href="#project-overview">Overview</a> •
    <a href="#key-features">Key Features</a> •
    <a href="#system-architecture">Architecture</a> •
    <a href="#development-milestones">Milestones</a>
    <br>
    <a href="#tech-stack--tools">Tech Stack</a> •
    <a href="#getting-started">Setup</a> •
    <a href="#future-improvements">Future Work</a>
</p>


## Zynq-7000 Flappy Bird SoC Implementation

A high-performance, hardware-accelerated version of Flappy Bird implemented on the **Xilinx Zynq-7000 All Programmable SoC**. This project demonstrates a complete Hardware/Software (HW/SW) co-design, utilizing the ARM Cortex-A9 processor for game logic and custom FPGA logic for real-time video generation.

---

##  System Architecture

The project is split into two main domains to maximize the efficiency of the Zynq architecture:

### 1. Processing System (PS) - "The Brain"
* **Application:** C/C++ code running on the ARM Cortex-A9 core.
* **Responsibilities:** * Calculating bird physics (gravity, lift).
  * Collision detection logic.
  * Score management and game state machine.
  * Writing coordinate data to the PL via AXI-Lite bus.

### 2. Programmable Logic (PL) - "The Muscle"
* **Video Engine:** Custom RTL (Verilog/VHDL) rendering the game at 60 FPS.
* **VGA/HDMI Controller:** Generates timing signals and pixel data.
* **Sprite Hardware:** Specialized logic for drawing the bird, pipes, and background layers with zero CPU overhead.

---

##  Tech Stack & Tools
* **Hardware:** Zynq-7000 SoC (Zybo, ZedBoard, or Pynq)
* **Development:** Vivado Design Suite (Hardware design & Bitstream)
* **Software:** Vitis Unified Software Platform (C/C++ firmware)
* **Protocol:** AXI4-Lite for PS-to-PL communication

---

##  Getting Started

### Prerequisites
* Xilinx Vivado & Vitis (2020.1 or newer recommended).
* A compatible micro-USB cable for programming and UART debugging.

### Installation & Deployment
1. **Clone the Repo**
   ```bash
   git clone [https://github.com/Yonny04/Zynq-SoC-Flappy-Bird.git](https://github.com/Yonny04/Zynq-SoC-Flappy-Bird.git)
