# Teensy 100x100 Matrix Inversion Challenge

This project implements a non-blocking Gaussian-Jordan matrix inversion of a 100x100 matrix on a Teensy 4.x, while maintaining a user-configurable LED blink rate without the use of interrupts.

## Project Structure
- **/firmware**: Teensy C++ source code.
- **/tools**: Python utility for streaming CSV data and saving results.
- **requirements.txt**: Python dependencies.

## Technical Highlights
1. **Cooperative Multitasking**: The matrix math is "sliced" into 100 steps (one per column) to prevent CPU starvation of the LED and Serial tasks.
2. **Memory Optimization**: Uses a contiguous data pool with a pointer-based row-swapping system ($O(1)$ pivoting complexity).
3. **Gated Serial Protocol**: A custom handshake protocol between Python and Firmware ensures data integrity during high-speed transfers.

## How to Run
1. **Firmware**: Load the `.ino` file onto the Teensy.
2. **Python**:
   ```bash
   pip install -r requirements.txt
   python tools/loader.py