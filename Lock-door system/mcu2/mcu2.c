#include "external_eeprom.h"
#include "uart.h"
#include <string.h>

#define EEPROM_Adress 0x0020

int main(void) {
	uint8 pass[8] = "1111";
	uint8 check[8];
	UART_init();
	DDRC |= 0x03;
	DDRD |= 0b0001111100;
	EEPROM_init();
	EEPROM_writeString(EEPROM_Adress, 8, pass);
	_delay_ms(10);
	while(1) {
		UART_receiveString(check);
		EEPROM_readString(EEPROM_Adress, 8, pass);
		if(!strcmp(pass, check)){ // Right password
			UART_sendByte(TRUE);
			check[0] = UART_recieveByte();
			if(check[0] == '1') { // Open the door
				SET_BIT(PORTD, PD5);
				SET_BIT(PORTD, PD3);
				_delay_ms(500);
				CLEAR_BIT(PORTD, PD5);

				_delay_ms(500);
				SET_BIT(PORTD, PD6);
				_delay_ms(500);
				CLEAR_BIT(PORTD, PD6);
				CLEAR_BIT(PORTD, PD3);
			} else if(check[0] == '2') { // Changing the password
				memset(pass, '\0', 8*8);
				UART_receiveString(pass);
				EEPROM_writeString(EEPROM_Adress, 8, pass);
			}
		} else { // Wrong password
			UART_sendByte(FALSE);
			SET_BIT(PORTD, PD2);
			SET_BIT(PORTD, PD4);
			_delay_ms(700);
			CLEAR_BIT(PORTD, PD2);
			CLEAR_BIT(PORTD, PD4);
		}
		memset(check, '\0', 8*8);
		EEPROM_readString(EEPROM_Adress, 8, pass);
	}
}
