#include "keypad.h"
#include "lcd.h"
#include "uart.h"
#include <string.h>

static sint8 count = 0;
static uint8 sec = 30;

ISR (TIMER0_COMP_vect) {
	if(++count == 4) {
		count = 0;
		sec--;
		LCD_goToRowColumn(1, 6);
		if(sec / 10)
			LCD_intgerToString(sec);
		else {
			LCD_intgerToString(0);
			LCD_intgerToString(sec);
		}
	}
}

int main(void) {
	TCCR0 |= (1 << FOC0) | (1 << WGM01) | (1 << CS02) | (1 << CS00);
	OCR0 = 250;
	SREG |= (1 << 7); // Global interrupt

	UART_init();
	LCD_init();
	LCD_sendCommand(CURSOR_BLINKING);
	LCD_displayString("Enter Password:");
	LCD_goToRowColumn(1, 0);
	uint8 key, change, i = 0;
	uint8 pass[9];
	while(1) {
		key = KeyPad_getPressedKey();
		if(key == '=' || key == 'C') {
			if(key == '=' && i < 10) {
				pass[i] = '=';
				UART_sendString(pass);
				if(change == FALSE) {
					key = UART_recieveByte();
					if(key == TRUE) { // Password is right
						count = 0;
						LCD_sendCommand(CLEAR_COMMAND);
						LCD_displayString("1-Open");
						LCD_displayStringRowColumn(1, 0, "2-Reset Password");
						key = KeyPad_getPressedKey();
						UART_sendByte(key);
						if(key == '1') { // Opening the door
							LCD_sendCommand(CLEAR_COMMAND);
							LCD_sendCommand(CURSOR_OFF);
							LCD_displayStringRowColumn(0, 4, "Opened");
							LCD_displayStringRowColumn(1, 1, "Successfully");
							_delay_ms(1500);
						} else if(key == '2') { // Changing the password
							LCD_sendCommand(CLEAR_COMMAND);
							LCD_displayString("Max 8 numbers");
							_delay_ms(500);
							LCD_sendCommand(CLEAR_COMMAND);
							LCD_sendCommand(CURSOR_ON);
							change = TRUE; // To change the password in the next iteration
						} else { // Entering invalid input
							LCD_sendCommand(CLEAR_COMMAND);
							LCD_sendCommand(CURSOR_OFF);
							LCD_displayStringRowColumn(1, 1, "Invalid input!");
							i = key = 0;
							_delay_ms(1000);
						}
					} else { // Password is wrong
						if(count == -3) { // Too many attempts
							count = 0;
							LCD_sendCommand(CLEAR_COMMAND);
							LCD_displayString("Too many wrong  attempts");
							_delay_ms(500);
							LCD_sendCommand(CLEAR_COMMAND);
							LCD_displayString(" Wait for timer");
							LCD_sendCommand(CURSOR_OFF);
							LCD_goToRowColumn(1, 6);
							LCD_intgerToString(sec);
							TIMSK |= (1 << OCIE0); // Open timer
							while(sec) {}
							sec = 30;
							TIMSK ^= (1 << OCIE0); // Close timer
						} else { // didn't exceed 3 attempts
							LCD_sendCommand(CLEAR_COMMAND);
							LCD_sendCommand(CURSOR_OFF);
							LCD_displayStringRowColumn(1, 0, "Wrong  Password!");
							count--;
							_delay_ms(1000);
						}
					}
				} else { // Sending password after getting the number
					LCD_sendCommand(CLEAR_COMMAND);
					LCD_sendCommand(CURSOR_OFF);
					LCD_displayStringRowColumn(0, 0, "Password changed");
					LCD_displayStringRowColumn(1, 1, "Successfully");
					change = FALSE;
					_delay_ms(1000);
				}
			} else if (key == '=' && i >= 10) {
				LCD_sendCommand(CLEAR_COMMAND);
				LCD_displayString("Max 8 numbers");
				_delay_ms(500);
			}
			i = 0;
			memset(pass, '\0', 8*9);
			LCD_sendCommand(CLEAR_COMMAND);
			LCD_displayString("Enter password:");
			LCD_goToRowColumn(1, 0);
			LCD_sendCommand(CURSOR_ON);
			LCD_sendCommand(CURSOR_BLINKING);
		} else {
			LCD_displayCharacter(key);
			pass[i++] = key;
		}
		_delay_ms(200);
	}
}
