#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define F_CPU 16000000UL

// Keypad mapping
char keypad[4][4] = {
    {'1', '2', '3', '4'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Predefined password (4-digit PIN)
char correctPassword[] = "1234";  // You can change this to any code
char enteredPassword[4];
uint8_t passwordIndex = 0;
uint8_t lockState = 0;  // 0 for locked, 1 for unlocked

// Pin configurations for the LED or Relay
#define LOCK_PIN PD6  // Assume the lock (LED/Relay) is connected to PD6 (pin 6 of Port D)

void UART_Init(void) {
    // Set baud rate to 9600
    unsigned int ubrr = 103;  // UBRR = (F_CPU / (16 * BAUD)) - 1
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Enable transmitter
    UCSR0B = (1 << TXEN0);

    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_Transmit(char data) {
    // Wait for the transmit buffer to be empty
    while (!(UCSR0A & (1 << UDRE0)));
    // Send the data
    UDR0 = data;
}

void UART_SendString(const char *str) {
    while (*str) {
        UART_Transmit(*str++);
    }
}

void Keypad_Init(void) {
    // Set Row pins (C0-C3) as input with pull-up
    DDRC &= ~(0x0F);    // Set the first 4 pins of PORTC as input
    PORTC |= 0x0F;      // Enable pull-up resistors on these pins

    // Set Column pins (D2-D5) as output
    DDRD |= 0x3C;       // Set pins D2, D3, D4, D5 as output
    PORTD |= 0x3C;      // Set columns to HIGH initially (idle state)
}

char Keypad_Scan(void) {
    for (uint8_t col = 0; col < 4; col++) {
        // Set one column LOW
        PORTD &= ~(1 << (col + 2));  // D2 to D5 are pins for columns
        _delay_us(50);               // Small delay to stabilize

        for (uint8_t row = 0; row < 4; row++) {
            // Check if any row is pulled LOW (button pressed)
            if (!(PINC & (1 << row))) { // If row is LOW, key is pressed
                _delay_ms(20);         // Debounce delay
                while (!(PINC & (1 << row)));  // Wait for key release
                PORTD |= (1 << (col + 2));    // Reset column to HIGH
                return keypad[row][col];  // Return the pressed key
            }
        }

        // Reset column to HIGH after scanning
        PORTD |= (1 << (col + 2));
    }
    return '\0';  // No key pressed
}

void Lock_Open(void) {
    PORTD |= (1 << LOCK_PIN);  // Set the LOCK_PIN HIGH (turn on LED or activate relay)
    UART_SendString("\nLOCK OPEN");  // Send the lock open message via UART
    lockState = 1;  // Set lock state to open
}

void Lock_Close(void) {
    PORTD &= ~(1 << LOCK_PIN);  // Set the LOCK_PIN LOW (turn off LED or deactivate relay)
    UART_SendString("\nLOCK CLOSED");  // Send the lock closed message via UART
    lockState = 0;  // Set lock state to closed
}

void Access_Denied(void) {
    UART_SendString("\nACCESS DENIED");  // Send access denied message via UART
}

int main(void) {
    UART_Init();     // Initialize UART for communication
    Keypad_Init();   // Initialize keypad I/O
    DDRD |= (1 << LOCK_PIN);  // Set LOCK_PIN (PD6) as output for relay/LED

    char key;
    passwordIndex = 0;  // Reset password index
    memset(enteredPassword, 0, 4);  // Clear entered password

    UART_SendString("Enter password: ");

    while (1) {
        key = Keypad_Scan();  // Scan the keypad

        if (key != '\0') {
            // If '*' is pressed, delete the last entered character
            if (key == '*') {
                if (passwordIndex > 0) {
                    passwordIndex--;  // Move the index back to remove last character
                    UART_SendString("\b \b");  // Use backspace to erase last '*' in UART
                }
                UART_SendString("\nEnter password: ");  // Move to next line after '*' or '#'
            }
            // If '#' is pressed, clear the entire password
            else if (key == '#') {
                memset(enteredPassword, 0, 4);  // Clear entered password
                passwordIndex = 0;  // Reset password index
                UART_SendString("\nEnter password: ");  // Prompt again on the next line after '*' or '#'
            }
            // Otherwise, handle normal character entry
            else {
                if (passwordIndex < 4) {
                    enteredPassword[passwordIndex] = key;
                    passwordIndex++;
                    UART_Transmit('*');  // Display '' for each entered key
                }
            }

            if (passwordIndex == 4) {
                // Password entered, check if it matches
                if (strncmp(enteredPassword, correctPassword, 4) == 0) {
                    // Correct password entered
                    if (lockState == 0) {
                        Lock_Open();  // Unlock the system if the lock is closed
                    } else {
                        Lock_Close(); // Lock the system if it's already open
                    }
                } else {
                    Access_Denied();  // Send "ACCESS DENIED" message if the password is incorrect
                }
                passwordIndex = 0;  // Reset password index for the next entry attempt
                memset(enteredPassword, 0, 4);  // Clear entered password
                UART_SendString("\nEnter password: ");  // Prompt again on the next line
            }
        }
    }

    return 0;
}