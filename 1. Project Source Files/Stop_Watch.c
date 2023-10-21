/*
 ============================================================================
 Name        : Stop_Watch.c
 Author      : Ahmed Shawky
 Description : Stop Watch System Project
 Date        : 28/08/2023
 ============================================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

typedef unsigned char uint8_t ;

/* Variables to hold the clock time. */
uint8_t g_seconds = 0 ;
uint8_t g_minutes = 0 ;
uint8_t g_hours = 0 ;

/* External INT0 Interrupt Service Routine. */
ISR(INT0_vect)
{
	/* The flag is cleared when the interrupt routine is executed.
	 * Alternatively, the flag can be cleared by writing a logical one to it.
	 */
	GIFR |= (1<<INTF0) ;

	/* If a falling edge detected the Stop Watch time should be reset. */
	g_seconds = 0 ;
	g_minutes = 0 ;
	g_hours   = 0 ;
}

/* External INT1 Interrupt Service Routine. */
ISR(INT1_vect)
{
	/* The flag is cleared when the interrupt routine is executed.
	 * Alternatively, the flag can be cleared by writing a logical one to it.
	 */
	GIFR |= (1<<INTF1) ;

	/* If a raising edge detected the Stop Watch time should be paused.
	 * Clear the timer clock bits (CS10=0 CS11=0 CS12=0) to stop the timer clock.
	 */
	TCCR1B &= ~(1<<CS10) ;
	TCCR1B &= ~(1<<CS12) ;

}

/* External INT2 Interrupt Service Routine. */
ISR(INT2_vect)
{
	/* The flag is cleared when the interrupt routine is executed.
	 * Alternatively, the flag can be cleared by writing a logical one to it.
	 */
	GIFR |= (1<<INTF2) ;

	/* If a falling edge detected the Stop Watch time should be resumed.
	 * resume the stop watch by enable the timer through the clock bits.
	 */
	TCCR1B |= (1<<CS10) | (1<<CS12) ;
}

/* Interrupt Service Routine for timer1 compare mode channel A. */
ISR(TIMER1_COMPA_vect)
{
	/* OCF1A is automatically cleared when the Output Compare Match A Interrupt Vector is executed.
	 * Alternatively, OCF1A can be cleared by writing a logic one to its bit location.
	 */
	TIFR |= (1<<OCF1A) ;

	/* Increment the seconds variable. */
	g_seconds += 1 ;
}

/******************************* Functions Prototypes *******************************/
void INT0_Init(void);
void INT1_Init(void);
void INT2_Init(void);
void TIMER1_CTC_Init(void);

void Display_Seconds(void);
void Display_Minutes(void);
void Display_Hours(void);

int main()
{
	/* Connect 7447 decoder 4-pins to the first 4-pins in PORTC. */
	DDRC |= 0x0F ;

	/* Use first 6-pins in PORTA as the enable/disable pins for the six 7-segments */
	DDRA |= 0x3F ;

	/* Setting the Global Interrupt Enable bit(GIE) or I-bit. */
	sei();

	/* Activate external interrupt INT0. */
	INT0_Init();

	/* Activate external interrupt INT1. */
	INT1_Init();

	/* Activate external interrupt INT2. */
	INT2_Init();

	/* Start timer1 to generate compare interrupt every 1000 MiliSeconds(1 Second) */
	TIMER1_CTC_Init();

	while(1)
	{
		if(g_seconds == 60)
		{
			g_seconds = 0 ;
			g_minutes += 1 ;
		}
		if(g_minutes == 60)
		{
			g_minutes = 0 ;
			g_hours += 1 ;
		}
		if(g_hours == 60)
		{
			g_hours = 0 ;
		}

		/* Display the number of seconds. */
		Display_Seconds();

		/* Display the number of minutes. */
		Display_Minutes();

		/* Display the number of hours. */
		Display_Hours();
	}

	return 0 ;
}

/******************************* Functions Definitions *******************************/
void INT0_Init(void)
{
	/* Connect a push button to pin PD2 with the internal pull-up resistor. */
	DDRD &= ~(1<<PD2) ;
	PORTD |= (1<<PD2) ;

	/* Configure External Interrupt INT0 with falling edge.
	 * The falling edge of INT0 generates an interrupt request.
	 */
	MCUCR |= (1<<ISC01) ;

	/* When the INT0 bit is set (one) and the I-bit in the Status Register (SREG) is set (one),
	 * the external pin interrupt is enabled.
	 */
	GICR |= (1<<INT0) ;
}

void INT1_Init(void)
{
	/* Connect a push button to pin PD3 with the external pull-down resistor. */
	DDRD &= ~(1<<PD3) ;

	/* Configure External Interrupt INT1 with raising edge.
	 * The rising edge of INT1 generates an interrupt request.
	 */
	MCUCR |= (1<<ISC10) | (1<<ISC11) ;

	/* When the INT1 bit is set (one) and the I-bit in the Status Register (SREG) is set (one),
	 * the external pin interrupt is enabled.
	 */
	GICR |= (1<<INT1) ;
}

void INT2_Init(void)
{
	/* Connect a push button to pin PB2 with the internal pull-up resistor. */
	DDRB &= ~(1<<PB2) ;
	PORTB |= (1<<PB2) ;

	/* Configure External Interrupt INT2 with falling edge.
	 * If ISC2 is written to zero, a falling edge on INT2 activates the interrupt.
	 */
	MCUCSR &= ~(1<<ISC2) ;

	/* When the INT2 bit is set (one) and the I-bit in the Status Register (SREG) is set (one),
	 * the external pin interrupt is enabled.
	 */
	GICR |= (1<<INT2) ;
}

void TIMER1_CTC_Init(void)
{
	/* Configure timer1 control registers
	 * 1. Non PWM mode FOC1A=1.
	 * 2. No need for OC1A & OC1B in this example so COM1A0=0 & COM1A1=0 & COM1B0=0 & COM1B1=0.
	 * 3. CTC Mode and compare value in OCR1A WGM10=0 & WGM11=0 & WGM12=1 & WGM13=0.
	 * 4. Clock = F_CPU/1024 CS10=1 CS11=0 CS12=1.
	 */
	TCCR1A |= (1<<FOC1A) ;
	TCCR1B |= (1<<WGM12) | (1<<CS10) | (1<<CS12) ;

	/* Timer initial value. */
	TCNT1 = 0 ;

	/* Timer compare value. */
	OCR1A = 976 ;

	/* Enable compare interrupt for channel A. */
	TIMSK |= (1<<OCIE1A) ;
}

void Display_Seconds(void)
{
	/* out the number of seconds. */
	PORTA = ( PORTA & 0xC0 ) | 0x01 ;
	PORTC = ( PORTC & 0xF0 ) |  ( g_seconds % 10 ) ;

	/* make small delay to see the changes in the 7-segment.
	 * 10Microseconds delay will not effect the seconds count.
	 */
	_delay_us(10);

	PORTA = ( PORTA & 0xC0 ) | 0x02 ;
	PORTC = ( PORTC & 0xF0 ) |  ( g_seconds / 10 ) ;

	_delay_us(10);
}

void Display_Minutes(void)
{
	/* out the number of minutes. */
	PORTA = ( PORTA & 0xC0 ) | 0x04 ;
	PORTC = ( PORTC & 0xF0 ) |  ( g_minutes % 10 ) ;

	_delay_us(10);

	PORTA = ( PORTA & 0xC0 ) | 0x08 ;
	PORTC = ( PORTC & 0xF0 ) |  ( g_minutes / 10 ) ;

	_delay_us(10);
}

void Display_Hours(void)
{
	/* out the number of hours. */
	PORTA = ( PORTA & 0xC0 ) | 0x10 ;
	PORTC = ( PORTC & 0xF0 ) |  ( g_hours % 10 ) ;

	_delay_us(10);

	PORTA = ( PORTA & 0xC0 ) | 0x20 ;
	PORTC = ( PORTC & 0xF0 ) |  ( g_hours / 10 ) ;

	_delay_us(10);
}
