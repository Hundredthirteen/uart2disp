#include	<xc.h>


//************************************
//**
//**
//**
//**    o       o       o       o       o
//**    |       |        ______/       /
//**    |       |       /             /
//**     \      |  A4A4A   __________/
//**      \     \_A     C_/
//**       \      5     5
//**        \------C4C4C
//**         _____C     C___
//**        /     0     2   \
//**       /       C3C3C     \ C1----\
//**      /       /           \       \
//**     /       /             \       \
//**    o       o       o       o       o
//**
//**                  PORTA - - 5 4  - - - -
//**                  PORTC - - 5 4  3 2 1 0
//**
//**    0 = A4A5            - - 1 0  - - - -    0x20
//**    0 = C0C2C3C5        - - 1 0  1 1 0 1    0x2D
//**
//**    1 =                 - - 0 1  - - - -    0x10
//**    1 = C2C5            - - 1 0  0 1 0 0    0x24
//**
//**    2 = A4              - - 0 0  - - - -    0x00
//**    2 = C5C4C0C3        - - 1 1  1 0 0 1    0x39
//**
//**    3 = A4              - - 0 0  - - - -    0x00
//**    3 = C5C4C2C3        - - 1 1  1 1 0 0    0x3C
//**
//**    4 = A5              - - 1 1  - - - -    0x30
//**    4 = C5C4C2          - - 1 1  0 1 0 0    0x34
//**
//**    5 = A4A5            - - 1 0  - - - -    0x20
//**    5 = C4C2C3          - - 0 1  1 1 0 0    0x1C
//**
//**    6 = A4A5            - - 1 0  - - - -    0x20
//**    6 = C4C0C2C3        - - 0 1  1 1 0 1    0x1D
//**
//**    7 = A4              - - 0 0  - - - -    0x00
//**    7 = C5C2            - - 1 0  0 1 0 0    0x24
//**
//**    8 = A4A5            - - 1 0  - - - -    0x20
//**    8 = C5C4C0C2C3      - - 1 1  1 1 0 1    0x3D
//**
//**    9 = A4A5            - - 1 0  - - - -    0x20
//**    9 = C5C4C2C3        - - 1 1  1 1 0 0    0x3C
//**
//**    - =                 - - 0 1  - - - -    0x10
//**    - = C4              - - 0 1  0 0 0 0    0x10
//**
//**    ' '=                - - 0 1  - - - -    0x10
//**    ' '= C4             - - 0 0  0 0 0 0    0x00
//**
//**    -u= A4              - - 0 0  - - - -    0x00
//**    -u=                 - - 0 0  0 0 0 0    0x00
//**
//**    _ =                 - - 0 1  - - - -    0x10
//**    _ = C3              - - 0 0  1 0 0 0    0x08
//**
//**  |lu = A5              - - 1 1  - - - -    0x30
//**  |lu =                 - - 0 0  0 0 0 0    0x00
//**
//**  |ld =                 - - 0 1  - - - -    0x10
//**  |ld = C0              - - 0 0  0 0 0 1    0x01
//**
//**  |ru =                 - - 0 1  - - - -    0x10
//**  |ru = C5              - - 1 0  0 0 0 0    0x20
//**
//**  |rd =                 - - 0 1  - - - -    0x10
//**  |rd = C2              - - 0 0  0 1 0 0    0x04
//**
//*********************************

//*********************************
//**
//**            /---------|  |---------\
//**            |  B3 -- / __ \ -- B5  |
//**            |   /  /        \  \   |
//**            | /  /            \  \ |
//**            |/ |                | \|
//**            |\ |      B6        | /|
//**            | \  \            /  / |
//**            |   \  \        /  /   |
//**            |  B2 -- \ __ / -- B4  |
//**            /---------|  |---------\
//**
//*********************************




__CONFIG(WDTDIS & HS & UNPROTECT & BORDIS & LVPDIS);
//#pragma config WDTDIS=ON,


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

const unsigned char PRELOAD = 0x9C; //1ms T0
//               0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F  0x10  0x11
//                0     1     2     3     4     5     6     7     8     9     -    ' '    -u    _     |lu   |ld  |ru   |rd
char BSA[18] = { 0x20, 0x10, 0x00, 0x00, 0x30, 0x20, 0x20, 0x00, 0x20, 0x20, 0x10, 0x10, 0x00, 0x10, 0x30, 0x10, 0x10, 0x10};
char BSC[18] = { 0x2D, 0x24, 0x39, 0x3C, 0x34, 0x1C, 0x1D, 0x24, 0x3D, 0x3C, 0x10, 0x00, 0x00, 0x08, 0x00, 0x01, 0x20, 0x04};
char dig_num = 0x00;
volatile unsigned char display[4] = {0x08, 0x08, 0x08, 0x08 };
unsigned int VAL = 0x0000;
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

void Parse_Str(void)
{
    char buf_len = rx_cnt;
    i = 0;
    flags &= ~CR_FLAG;
    rx_cur = rx_start;
    GIE = 0;
    while(rx_cur!=rx_end)
    {
        tmp_buf[i++] = rx_buf[rx_cur];
        if(rx_buf[rx_cur] == '\r')
        {
            flags |= CR_FLAG;
            break;
        }

        if(rx_cur < RX_BUF_LEN-1)
            rx_cur++;
        else
            rx_cur = 0;
    }
    GIE = 1;
    if(flags & CR_FLAG)
    {
        tmp_buf[i-1] = 0x0B; //'r';
        tmp_buf[i] = 0x0B; //'n';
        tmp_buf[i+1] = 0x0B;
        rx_start = rx_cur + 1;
        for(j = 0; j < i; j++)
        {

            if(tmp_buf[j] == 'T')
            {
                anim_cnt = -1;
                for(k = 0; k < 4; k++)
                {
                    if( (tmp_buf[j + 2 + k] >= '0') && (tmp_buf[j + 2 + k] <= '9') )
                        display[k] = tmp_buf[j + 2 + k] - 48;
                    else
                        if(tmp_buf[j + 2 + k] == ',')
                        {
                            if(k == 1) {display[1] = 0x0D; display[2] = 0x0D; display[3] = 0x0D;}
                            else
                                if(k == 2) {display[2] = 0x0D; display[3] = 0x0D;}
                                else
                                    display[3] = 0x0D;
                            break;
                        }
                        else
                            if(tmp_buf[j + 2 + k] == '-')
                                display[k] = 0x0A;
                            else
                                display[k] = 0x0D;
                }
                
                if(display[1] == 0x0B)
                {
                    display[3] = display[0];
                    display[2] = 0x0B;
                    display[1] = 0x0B;
                    display[0] = 0x0B;
                }
                else
                    if(display[2] == 0x0B)
                    {
                            display[3] = display[1];
                            display[2] = display[0];
                            display[1] = 0x0B;
                            display[0] = 0x0B;
                    }
                    else
                        if(display[3] == 0x0B)
                        {
                            display[3] = display[2];
                            display[2] = display[1];
                            display[1] = display[0];
                            display[0] = 0x0B;
                        }
                break;
            }

            if(tmp_buf[j] == 'K')
            {
                if(tmp_buf[j+2] == '1')
                    flags |= MARK_FLAG;
                else
                    flags &= ~MARK_FLAG;
                break;
            }
        }
    }

}

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

void tx_str_INT(const char* str, signed int val)
{
    unsigned char i=0;
    unsigned char j=0;
    unsigned char k=0;
    unsigned char tmp;
    unsigned char minus = 0;

    if(val < 0)
    {
        val = val * -1;
        minus++;
    }
    while (TXEN) ;
    while( (str[i]!=0) && (i < TX_BUF_LEN) )
    {
        if(str[i] == '$')
        {
            if(minus)
            {
                tx_buf[i+j] = '-';
                i++;
            }

            while(val >= 10)
            {
                tx_buf[i+j] = (val % 10) + 48;
                val = val / 10;
                j++;
            }
            tx_buf[i+j] = val + 48;

            for(k = 0;k <= ((j)/2);k++)
            {
        	tmp = tx_buf[k+i];
                tx_buf[k+i] = tx_buf[j-k+i];
                tx_buf[j-k+i] = tmp;
            }

            if(minus) i--;
        }
        else
            tx_buf[i+j]=str[i];
	i++;
    }

    tx_cnt = i+j;
    tx_cur=0;
    TXEN = 1;
}

void byte2seg(char val)
{
//    PORTC |= 0x3F;
//    PORTC &= ~BSC[val];
    PORTC = ((PORTC & 0xC0) | (~BSC[val]));
//    PORTA |= 0x30;
//    PORTA &= ~BSA[val];
    PORTA = ((PORTA & 0x0F) | (~BSA[val]));
}

void int2str(int val)
{
    int val_1000 = val % 1000;
    if(val > 999)
        display[0] = val / 1000;
    else
        display[0] = 16; //empty

    if(val > 99)
        display[1] = val_1000 / 100;
    else
        display[1] = 16; //empty

    if(val > 9)
        display[2] = (val_1000 % 100) / 10;
    else
        display[2] = 16; //empty

    display[3] = (val_1000 % 100) % 10;
}


void out()
{
    unsigned char i = 0;

    if(flags & MARK_FLAG)
        flags |= (DOT1 + DOT2);
    else
        flags &= ~(DOT1 + DOT2);

        PORTA |= 0x0F;
        if(dig_num > 3)
            dig_num = 0x00;


        byte2seg(display[dig_num]);
//        PORTA &= ~(unsigned char)(1 << dig_num);
        switch(dig_num)
        {
            case 0: PORTA &= ~0x01; if(flags & DOT0) PORTC &= ~0x02; break;
            case 1: PORTA &= ~0x02; if(flags & DOT1) PORTC &= ~0x02; break;
            case 2: PORTA &= ~0x04; if(flags & DOT2) PORTC &= ~0x02; break;
            case 3: PORTA &= ~0x08; if(flags & DOT3) PORTC &= ~0x02; break;
        }

        for(i=0; i<255; i++)
            NOP();

        dig_num++;
}


void anim()
{
    anim_cnt++;
    if(anim_cnt > 30)
    {
        anim_cnt = 0;
        switch(anim_step)
        {
/*
            case 0:display[0]=0x0D;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
            case 1:display[0]=0x0B;display[1]=0x0D;display[2]=0x0B;display[3]=0x0B;break;
            case 2:display[0]=0x0B;display[1]=0x0B;display[2]=0x0D;display[3]=0x0B;break;
            case 3:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x0D;break;
            case 4:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x11;break;
            case 5:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x10;break;
            case 6:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x0C;break;
            case 7:display[0]=0x0B;display[1]=0x0B;display[2]=0x0C;display[3]=0x0B;break;
            case 8:display[0]=0x0B;display[1]=0x0C;display[2]=0x0B;display[3]=0x0B;break;
            case 9:display[0]=0x0C;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
            case 10:display[0]=0x0E;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
            case 11:display[0]=0x0F;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
*/
            case 0:display[0]=0x0D;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
            case 1:display[0]=0x0B;display[1]=0x0D;display[2]=0x0B;display[3]=0x0B;break;
            case 2:display[0]=0x0B;display[1]=0x11;display[2]=0x0B;display[3]=0x0B;break;
            case 3:display[0]=0x0B;display[1]=0x0B;display[2]=0x0E;display[3]=0x0B;break;
            case 4:display[0]=0x0B;display[1]=0x0B;display[2]=0x0C;display[3]=0x0B;break;
            case 5:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x0C;break;
            case 6:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x10;break;
            case 7:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x11;break;
            case 8:display[0]=0x0B;display[1]=0x0B;display[2]=0x0B;display[3]=0x0D;break;
            case 9:display[0]=0x0B;display[1]=0x0B;display[2]=0x0D;display[3]=0x0B;break;
            case 10:display[0]=0x0B;display[1]=0x0B;display[2]=0x0F;display[3]=0x0B;break;
            case 11:display[0]=0x0B;display[1]=0x10;display[2]=0x0B;display[3]=0x0B;break;
            case 12:display[0]=0x0B;display[1]=0x0C;display[2]=0x0B;display[3]=0x0B;break;
            case 13:display[0]=0x0C;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
            case 14:display[0]=0x0E;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
            case 15:display[0]=0x0F;display[1]=0x0B;display[2]=0x0B;display[3]=0x0B;break;
        }
        anim_step++;
        if(anim_step > 15)//11
            anim_step = 0;
    }
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
        ADCON1 = 0x07;
	TRISA = 0xC0; //RA[5..0] - outputs
	TRISB = 0x7D; //RB7, RB1 - out, other inputs
        nRBPU = 0;   //PortB pulled-up
	TRISC = 0x80; //all outputs, RC7 - input




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
CREN = 1;


//== Interrupts
//PIE1 = 1;
PEIE = 1; //Enable peripheral interrupts
GIE = 1; //Global interrupt enable

Delay_ms(100);


    while(1)
    {

        if( !(PORTB & 0x40) )   //B6
        {
            if( !(continues_button_press & 0x06) )
            {
                continues_button_press |= 0x06;
                PORTB |= 0x80;
            }

            if( !(PORTB & 0x20) )   //B5
            {
                if( !(flags & PWRON_FLAG))
                {
                    PORTB |= 0x80;
                    flags |= PWRON_FLAG;
                }
                else
                {
                    flags &= ~PWRON_FLAG;
                    PORTB &= ~0x80;
                }
                while( !(PORTB & 0x20) ) ;
            }

            if( !(PORTB & 0x08) )   //B3
            {
                if(imp <= 4500)
                    imp += 500;
                tx_str_INT("IMP,$\r\n", imp);
                Delay_ms(500);
                while( !(PORTB & 0x08) ) ;
            } //END B3

            if( !(PORTB & 0x04) )   //B2
            {
                if(imp >= 1000)
                    imp -= 500;
                tx_str_INT("IMP,$\r\n", imp);
                Delay_ms(500);
                while( !(PORTB & 0x04) ) ;
            }  //END B2
        }
        else
        {
            if(continues_button_press & 0x06)
            {
                continues_button_press &= ~0x06;
                if(!(flags & PWRON_FLAG))
                    PORTB &= ~0x80;
            }
        }

        if( !(PORTB & 0x08) )   //B3
        {
            if(flags & DOT0)
            {
                tx_str("LPD,0\r\n");
                flags &= ~DOT0;
                if( !(flags & 0x78) )
                {
                    Delay_ms(500);
                    if(!(flags & PWRON_FLAG))
                        PORTB &= ~0x80;
                }
            }
            else
            {
                if( !(PORTB & 0x80) )
                {
                    PORTB |= 0x80;
                    Delay_ms(500);
                }
                tx_str("LPD,1\r\n");
                flags |= DOT0;
            }
            while( !(PORTB & 0x08) ) ;
        }

        if( !(PORTB & 0x04) )   //B2
        {

            if(flags & DOT3)
            {
                tx_str("APD,0\r\n");
                flags &= ~DOT3;
                if( !(flags & 0x78) )
                {
                    Delay_ms(500);
                    if(!(flags & PWRON_FLAG))
                        PORTB &= ~0x80;
                }
            }
            else
            {
                if( !(PORTB & 0x80) )
                {
                    PORTB |= 0x80;
                    Delay_ms(500);
                }
                tx_str("APD,1\r\n");
                flags |= DOT3;
            }

            while( !(PORTB & 0x04) ) ;
        }

        if( !(PORTB & 0x10) )   //B4
        {
            tx_str("COR,25,0\r\n");
            Delay_ms(500);
            tx_str("IMP,5000\r\n");
/*
            PORTB |= 0x80;
            Delay_ms(300);
            PORTB &= ~0x80;
            Delay_ms(200);
 */
            while( !(PORTB & 0x10) ) ;
        }



        if( !(PORTB & 0x20) )   //B5
        {
            if( !(flags & AUTOSHUT_FLAG))
            {
                PORTB |= 0x80;
                Delay_ms(500);
                flags |= AUTOSHUT_FLAG;
                tx_str("START,0,0\r\n");
            }
            else
            {
                flags &= ~AUTOSHUT_FLAG;
                tx_str("STOP\r\n");
                if(!(flags & PWRON_FLAG))
                    PORTB &= ~0x80;
            }
            while( !(PORTB & 0x20) ) ;
        }
/*        else
        {
            if(flags & AUTOSHUT_FLAG)
            {
                flags &= ~AUTOSHUT_FLAG;
                tx_str("STOP\r\n");
                if(!(flags & PWRON_FLAG))
                    PORTB &= ~0x80;
            }
        }
*/

        
        if(rx_start != rx_end)
	{
                Parse_Str();
        }

        if(one_ms_tick)
        {
            //PORTB ^= 0x80;
            if(anim_cnt >= 0)
                anim();
            out();
            one_ms_tick = 0;
        }

        continue;		// let interrupt do its job
    }
}
