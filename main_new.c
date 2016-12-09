// PIC16F873A Configuration Bit Settings
// 'C' source line config statements

#include <xc.h>

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


#define TX_BUF_LEN  20
#define RX_BUF_LEN  20

#define START_DIST_FLAG (1<<0)
#define STOP_DIST_FLAG (1<<1)
#define EOL_FLAG (1<<2)
#define DOT0 (1<<3)
#define DOT1 (1<<4)
#define DOT2 (1<<5)
#define DOT3 (1<<6)
#define AUTOSHUT_FLAG (1<<7)
#define STOP_MARK_FLAG (1<<8)
#define MARK_FLAG (1<<9)
#define CR_FLAG (1<<10)
#define PWRON_FLAG (1<<11)

#define COM_ULC RB6
#define COM_RD  RB5
#define B_C     RB4
#define B_RU    RB3
#define B_DL    RB2

#define LRF_BUT RB1
#define LRF_PWR_EN  RB7

#define ON_BUT  RA1
#define PWR_ON  RA2
#define OLED_RST    RA3

const unsigned char PRELOAD = 0x9C; //1ms T0
char dig_num = 0x00;
volatile unsigned int delay_val = 0;

int rx_cur = 0;
int rx_start = 0;
int rx_end = 0;
int rx_cnt;
int tx_cnt;
int tx_cur;
int imp = 0;
volatile unsigned char tx_buf[TX_BUF_LEN];
volatile unsigned char rx_buf[RX_BUF_LEN];
volatile unsigned char tmp_buf[RX_BUF_LEN];

char one_ms_tick = 0;
signed char anim_cnt = 0;
char anim_step = 0;
char tmp = 0;
unsigned char continues_button_press = 0;
char soft_repeater_state = 0;
unsigned char i = 0;
unsigned char k = 0;
volatile unsigned char j = 0;
volatile int flags = 0;
unsigned int B_RU_cnt;

void Delay_ms(unsigned int timeout)
{
    delay_val = timeout;
    while(delay_val)
        asm("nop");
}

void tx_str(const char* str)
{
    unsigned char i=0;
    while (TXEN) ;
    while( (str[i]!=0) && (i < TX_BUF_LEN) )
    {
	tx_buf[i]=str[i];
	i++;
    }

    tx_cnt = i;
    tx_cur=0;
    TXEN = 1;
}

void interrupt isr(void)
{
if(T0IF) 	//=========Timer0===========
{
    one_ms_tick = 1;
    if(delay_val > 0)
        delay_val--;

	TMR0 -= PRELOAD;	// re-load timer
	T0IF = 0;		// Clear Timer 0 Interrupt Flag.
} 	//END_=========Timer0===========
//else
if(RCIF)	//============RX============
{
    while(RCIF)	//RCREG is 2 bytes FIFO
    {
	rx_buf[rx_end] = RCREG;
        if(rx_end < RX_BUF_LEN-1)
            rx_end++;
        else
            rx_end = 0;

    }
    if(OERR)
    {
	rx_buf[rx_end] = RCREG;
        if(rx_end < RX_BUF_LEN-1)
            rx_end++;
        else
            rx_end = 0;

        rx_buf[rx_end] = RCREG;
        if(rx_end < RX_BUF_LEN-1)
            rx_end++;
        else
            rx_end = 0;
	CREN=0;
	CREN=1;
    }
//RCIF=0; by hardware
}	//END_============RX============
//else
if(TXIF && TRMT)	//=========TX===========
{
    if(TXEN)
    {
	if(tx_cnt>tx_cur)
	{
		TXREG = tx_buf[tx_cur];
		tx_cur++;
	}
	else
	{
		TXEN = 0;
	}
    }
    TXIF=0;
}//END_=========TX===========
}

main()
{
        PORTA = 0x00;
        PORTB = 0x00;
        PORTC = 0x00;
        ADCON1 = 0x07;//all digital
	TRISA = 0xFF;
    TRISA2 = 0;
    TRISA3 = 0; //RA2, RA3 - outputs
	//TRISB = 0x1D; //RB7,RB6,RB5,RB1 - out, other inputs
    TRISB = 0x7D; //RB7,RB1 - out, other inputs
    nRBPU = 0;   //PortB pulled-up
	TRISC = 0xBF; //RC6 - output




/*
000		1 : 2 //500us
001		1 : 4
010		1 : 8
011		1 : 16
100		1 : 32 //8ms
101		1 : 64 //16ms
110		1 : 128
111		1 : 256
*/

PSA = 0;
PS2 = 1;
PS1 = 0;
PS0 = 0;
T0CS = 0;			// select internal clock
TMR0 = -PRELOAD;	// preload timer
T0IE = 1;			// enable timer interrupt


//uart
rx_cur=0;
rx_cnt=0;
tx_cnt=0;
tx_cur=0;

//== init UART

SPEN = 1; //SerialPortEn
BRGH = 1; // HighSpeed /16
SPBRG = 10;//113636.4 for 115.200, err=1.36% at 20MHz
TXIE = 1;
RCIE = 1;
TX9 = 0;
RX9 = 0;
SYNC = 0; //Async mode;
CREN = 0;


//== Interrupts
//PIE1 = 1;
PEIE = 1; //Enable peripheral interrupts
GIE = 1; //Global interrupt enable

OLED_RST = 1;
Delay_ms(5);
OLED_RST = 0;

delay_val = 500;
while(delay_val && (!ON_BUT))
if(!ON_BUT)
    PWR_ON = 1;

while(!ON_BUT);


Delay_ms(100);

B_RU_cnt = 0;

    while(1)
    {

        TRISB5 = 1;//COM_RD input
        TRISB6 = 0;//COM_ULC output
        COM_ULC = 0;
        
        if( !(B_C) )   //center
        {
            if( !(continues_button_press & 0x06) )
            {
                continues_button_press |= 0x06;
                LRF_BUT = 1;
            }
            
            if( !(B_RU) )   //up
            {
                if(B_RU_cnt)
                {
                    tx_str("IMP,2500\r\n");
                    Delay_ms(500);
                    tx_str("COR,25,3\r\n");
                    Delay_ms(500);
                    tx_str("MND,1\r\n");
                    Delay_ms(500);
                    tx_str("LOOP,100\r\n");
                    Delay_ms(500);
                    tx_str("MOD,1\r\n");                    
                    B_RU_cnt = 0;
                }
                else
                {
                    tx_str("IMP,500\r\n");
                    Delay_ms(500);
                    tx_str("COR,25,3\r\n");
                    Delay_ms(500);
                    tx_str("MND,1\r\n");
                    Delay_ms(500);
                    tx_str("LOOP,100\r\n");
                    Delay_ms(500);
                    tx_str("MOD,2\r\n");                    
                    B_RU_cnt = 1;
                }
                while( !(B_RU) ) ;
            }
/*
            if( !(B_RU) )   //up
            {
                if( !(flags & PWRON_FLAG))
                {
                    //LRF_BUT = 1;
                    LRF_PWR_EN = 1;
                    LRF_BUT = 0;
                    flags |= PWRON_FLAG;
                }
                else
                {
                    //LRF_BUT = 0;
                    LRF_BUT = 1;
                    LRF_PWR_EN = 0;
                    flags &= ~PWRON_FLAG;
                }
                while( !(B_RU) ) ;
            }
 */
        }
        else
        {
            if(continues_button_press & 0x06)
            {
                continues_button_press &= ~0x06;
//                if(!(flags & PWRON_FLAG))
                    LRF_BUT = 0;
            }
        }

        if( !(B_RU) )   //up
        {
            if( !(flags & AUTOSHUT_FLAG))
            {
                LRF_PWR_EN = 1;
                Delay_ms(500);
                flags |= AUTOSHUT_FLAG;
                tx_str("START,0,0\r\n");
            }
            else
            {
                flags &= ~AUTOSHUT_FLAG;
                tx_str("STOP\r\n");
                if(!(flags & PWRON_FLAG))
                    LRF_PWR_EN = 0;
            }
            while( !(B_RU) ) ;
        }
        
        if( !(B_DL) )   //left
        {
            if(flags & DOT0)
            {
                tx_str("LPD,0\r\n");
                flags &= ~DOT0;
                if( !(flags & 0x78) )
                {
                    Delay_ms(500);
                    if(!(flags & PWRON_FLAG))
                        LRF_PWR_EN = 0;
                }
            }
            else
            {
                if( !(LRF_PWR_EN) )
                {
                    LRF_PWR_EN = 1;
                    Delay_ms(500);
                }
                tx_str("LPD,1\r\n");
                flags |= DOT0;
            }
            while( !(B_DL) ) ;
        }

        TRISB6 = 1;//COM_ULC input
        TRISB5 = 0;//COM_RD output
        COM_RD = 0;
        
        if( !(B_DL) )   //down
        {

            if(flags & DOT3)
            {
                tx_str("APD,0\r\n");
                flags &= ~DOT3;
                if( !(flags & 0x78) )
                {
                    Delay_ms(500);
                    if(!(flags & PWRON_FLAG))
                        LRF_PWR_EN = 0;
                }
            }
            else
            {
                if( !(PORTB & 0x80) )
                {
                    LRF_PWR_EN = 1;
                    Delay_ms(500);
                }
                tx_str("APD,1\r\n");
                flags |= DOT3;
            }

            while( !(B_DL) ) ;
        }

        if( !(B_RU) )   //right
        {
            if( !(flags & PWRON_FLAG))
            {
                LRF_PWR_EN = 1;
                flags |= PWRON_FLAG;
            }
            else
            {
                LRF_PWR_EN = 0;
                flags &= ~PWRON_FLAG;
            }
            
            while( !(B_RU) ) ;
        }
/*        
        if( !(B_RU) )   //right
        {
            tx_str("IMP,500\r\n");
            Delay_ms(500);
            tx_str("MND,1\r\n");
            Delay_ms(500);
            tx_str("MOD,2\r\n");
//            Delay_ms(500);
//            tx_str("COR,25,0\r\n");
//            Delay_ms(500);
//            tx_str("IMP,5000\r\n");
            while( !(B_RU) ) ;
        }
*/

        if(!ON_BUT)
        {
//            delay_val = 2000;
//            while(delay_val && (!ON_BUT))
//            if(delay_val == 0)
            PWR_ON = 0;
            while(!ON_BUT);
        }

        continue;		// let interrupt do its job
    }
}
