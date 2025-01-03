# Password-Protected Lock System with Keypad

This project implements a simple password-protected lock system using an ATmega328P microcontroller and a 4x4 matrix keypad. The lock can be opened and closed based on user input through the keypad. It communicates with the user via UART, displaying access status (locked/unlocked, access denied, etc.).

## Features

- 4-digit password entry via a 4x4 matrix keypad.
- UART communication to display the status (lock open, lock closed, or access denied).
- A relay or LED to simulate the lock mechanism.
- Supports password clearing and backspace for error corrections.
- **Dynamic lock toggling**: Allows the system to either open or close the lock based on its current state when the correct password is entered.

## Hardware Requirements

- ATmega328P microcontroller (Arduino Uno or similar).
- 4x4 Matrix Keypad.
- Relay or LED for lock simulation (connected to pin PD6).
- PlatformIO IDE for development and uploading the code.

## Pin Configuration

- **Keypad**:
  - Rows: Connected to pins `C0-C3` (PORTC).
  - Columns: Connected to pins `D2-D5` (PORTD).
  
- **Lock Mechanism (Relay/LED)**:
  - Lock control pin: `PD6` (set as output to control relay/LED).

## Software Requirements

- PlatformIO IDE or compatible editor (VS Code with PlatformIO extension).
- Arduino framework for ATmega328P.
