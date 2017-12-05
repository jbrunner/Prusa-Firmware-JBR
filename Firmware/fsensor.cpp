#include "Marlin.h"

#ifdef PAT9125

#include "fsensor.h"
#include "pat9125.h"
#include "planner.h"
#include "fastio.h"

//#include "LiquidCrystal.h"
//extern LiquidCrystal lcd;


#define FSENSOR_ERR_MAX          5  //filament sensor max error count
#define FSENSOR_INT_PIN         63  //filament sensor interrupt pin PK1
#define FSENSOR_INT_PIN_MSK   0x02  //filament sensor interrupt pin mask (bit1)
#define FSENSOR_CHUNK_LEN      560  //filament sensor chunk length in steps

extern void stop_and_save_print_to_ram(float z_move, float e_move);
extern void restore_print_from_ram_and_continue(float e_move);
extern int8_t FSensorStateMenu;

void fsensor_stop_and_save_print()
{
	stop_and_save_print_to_ram(0, 0); //XYZE - no change	
}

void fsensor_restore_print_and_continue()
{
	restore_print_from_ram_and_continue(0); //XYZ = orig, E - no change
}

//uint8_t fsensor_int_pin = FSENSOR_INT_PIN;
uint8_t fsensor_int_pin_old = 0;
int16_t fsensor_chunk_len = FSENSOR_CHUNK_LEN;
bool fsensor_enabled = true;
//bool fsensor_ignore_error = true;
bool fsensor_M600 = false;
uint8_t fsensor_err_cnt = 0;
int16_t fsensor_st_cnt = 0;
uint8_t fsensor_log = 1;



bool fsensor_enable()
{
	puts_P(PSTR("fsensor_enable\n"));
	int pat9125 = pat9125_init(PAT9125_XRES, PAT9125_YRES);
    printf_P(PSTR("PAT9125_init:%d\n"), pat9125);
	fsensor_enabled = pat9125?true:false;
//	fsensor_ignore_error = true;
	fsensor_M600 = false;
	fsensor_err_cnt = 0;
	eeprom_update_byte((uint8_t*)EEPROM_FSENSOR, fsensor_enabled?0x01:0x00); 
	FSensorStateMenu = fsensor_enabled?1:0;
	return fsensor_enabled;
}

void fsensor_disable()
{
	puts_P(PSTR("fsensor_disable\n"));
	fsensor_enabled = false;
	eeprom_update_byte((uint8_t*)EEPROM_FSENSOR, 0x00); 
	FSensorStateMenu = 0;
}

void pciSetup(byte pin)
{
	*digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin)); // enable pin
	PCIFR |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
	PCICR |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group 
}

void fsensor_setup_interrupt()
{
//	uint8_t fsensor_int_pin = FSENSOR_INT_PIN;
//	uint8_t fsensor_int_pcmsk = digitalPinToPCMSKbit(pin);
//	uint8_t fsensor_int_pcicr = digitalPinToPCICRbit(pin);

	pinMode(FSENSOR_INT_PIN, OUTPUT);
	digitalWrite(FSENSOR_INT_PIN, LOW);
	fsensor_int_pin_old = 0;

	pciSetup(FSENSOR_INT_PIN);
}

ISR(PCINT2_vect)
{
//	return;
	if (!((fsensor_int_pin_old ^ PINK) & FSENSOR_INT_PIN_MSK)) return;
//	puts("PCINT2\n");
//	return;

	int st_cnt = fsensor_st_cnt;
	fsensor_st_cnt = 0;
	sei();
/*	*digitalPinToPCMSK(fsensor_int_pin) &= ~bit(digitalPinToPCMSKbit(fsensor_int_pin));
	digitalWrite(fsensor_int_pin, HIGH);
	*digitalPinToPCMSK(fsensor_int_pin) |= bit(digitalPinToPCMSKbit(fsensor_int_pin));*/
	pat9125_update_y();
	if (st_cnt != 0)
	{
#ifdef DEBUG_FSENSOR_LOG
		if (fsensor_log)
		{
			MYSERIAL.print("cnt=");
			MYSERIAL.print(st_cnt, DEC);
			MYSERIAL.print(" dy=");
			MYSERIAL.print(pat9125_y, DEC);
		}
#endif //DEBUG_FSENSOR_LOG
		if (st_cnt != 0)
		{
			if( (pat9125_y == 0) || ((pat9125_y > 0) && (st_cnt < 0)) || ((pat9125_y < 0) && (st_cnt > 0)))
			{ //invalid movement
				if (st_cnt > 0) //only positive movements
					fsensor_err_cnt++;
#ifdef DEBUG_FSENSOR_LOG
			if (fsensor_log)
			{
				MYSERIAL.print("\tNG ! err=");
				MYSERIAL.println(fsensor_err_cnt, DEC);
			}
#endif //DEBUG_FSENSOR_LOG
			}
			else
			{ //propper movement
				if (fsensor_err_cnt > 0)
					fsensor_err_cnt--;
//					fsensor_err_cnt = 0;
#ifdef DEBUG_FSENSOR_LOG
				if (fsensor_log)
				{
					MYSERIAL.print("\tOK    err=");
					MYSERIAL.println(fsensor_err_cnt, DEC);
				}
#endif //DEBUG_FSENSOR_LOG
			}
		}
		else
		{ //no movement
#ifdef DEBUG_FSENSOR_LOG
		if (fsensor_log)
			MYSERIAL.println("\tOK 0");
#endif //DEBUG_FSENSOR_LOG
		}
	}
	pat9125_y = 0;
	return;
}

void fsensor_st_block_begin(block_t* bl)
{
	if (!fsensor_enabled) return;
	if (((fsensor_st_cnt > 0) && (bl->direction_bits & 0x8)) || 
		((fsensor_st_cnt < 0) && !(bl->direction_bits & 0x8)))
	{
		if (_READ(63)) _WRITE(63, LOW);
		else _WRITE(63, HIGH);
	}
//		PINK |= FSENSOR_INT_PIN_MSK; //toggle pin
//		_WRITE(fsensor_int_pin, LOW);
}

void fsensor_st_block_chunk(block_t* bl, int cnt)
{
	if (!fsensor_enabled) return;
	fsensor_st_cnt += (bl->direction_bits & 0x8)?-cnt:cnt;
	if ((fsensor_st_cnt >= fsensor_chunk_len) || (fsensor_st_cnt <= -fsensor_chunk_len))
	{
		if (_READ(63)) _WRITE(63, LOW);
		else _WRITE(63, HIGH);
	}
//		PINK |= FSENSOR_INT_PIN_MSK; //toggle pin
//		_WRITE(fsensor_int_pin, LOW);
}

void fsensor_update()
{
	if (!fsensor_enabled) return;
	if (fsensor_err_cnt > FSENSOR_ERR_MAX)
	{
		MYSERIAL.println("fsensor_update (fsensor_err_cnt > FSENSOR_ERR_MAX)");
/*		if (fsensor_ignore_error)
		{
			MYSERIAL.println("fsensor_update - error ignored)");
			fsensor_ignore_error = false;
		}
		else*/
		{
			MYSERIAL.println("fsensor_update - ERROR!!!");
			fsensor_stop_and_save_print();
			enquecommand_front_P((PSTR("M600")));
			fsensor_M600 = true;
			fsensor_enabled = false;
		}
	}
}

#endif //PAT9125
