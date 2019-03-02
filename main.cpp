#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <avr/sleep.h>

const uint8_t On_C = 0b00011000;
#define Plus 0b00010001
#define Sub  0b00100001
#define Did  0b10000001
#define Mul  0b01000001
#define Equ  0b00010010

bool pressKey = false; //ничего не нажато
const uint8_t keyDelay = 2;//60;   //кол-во итераций в ISR которое нужно для срабатывания нажатия клавиши
uint8_t keyCounter = 0;  //счётчик кол-ва нажатия в ISR

uint16_t powerCounter = 0;
const uint16_t powerStop = 10000;//244*120;// 2минуты
bool power = false;

const uint8_t delaySign = 250;// задержка на индикацию нажатого действия (* / - + =)

const bool NumOne = true;
const bool NumTwo = false;
bool writeTo = NumOne;

bool inf = false;
int32_t numberOne = 0;
int32_t numberTwo = 0;
int32_t belowZero = 0 ;

#define firstZnakoMesto	 0b00000001
#define secondZnakoMesto 0b00000010
#define thirdZnakoMesto	 0b00000100
#define fourthZnakoMesto 0b00001000

inline void go2PreviusZnakoMesto(){	PORTB = PORTB >> 1;} 

#define currentZnakoMesto PORTB

#define i 0b00000100
#define n 0b01010100
#define f 0b01110001
#define p 0b01110011
#define e 0b01111001
#define q 0b01100111
#define u 0b00111110
#define m 0b00110111
#define l 0b00111000
#define d 0b01011110
#define s 0b01101101
#define b 0b01111100
#define minus 0b01000000

uint8_t dispVal[4] = {0,0,0,0b00111111};

int32_t mod(int32_t num)
{
	return num < 0 ? num * (-1) : num;
}


uint8_t getLength(int32_t num)
{
	uint8_t result = 0;
	while( num != 0)
	{	
		num /= 10;
		result++;
	}
	return result;
}

inline bool isKeyPres()
{
	return (PINB > 0b00001111);
} 

bool isKeyPres(uint8_t key)
{
	return (PINB == key);
}

uint8_t setPortD(uint8_t num)
{
	switch (num)
	{
		case 0:
			return	0b00111111;
		case 1:
			return	0b00000110;
		case 2:
			return	0b01011011;
		case 3:
			return	0b01001111;
		case 4:
			return	0b01100110;
		case 5:
			return	0b01101101;
		case 6:
			return	0b01111101;
		case 7:
			return	0b00100111;
		case 8:
			return	0b01111111;
		case 9:
			return	0b01101111;
		default:
			return	0b00000001;
	}
}

int32_t pow_dec(int32_t num, uint8_t pw)
{
	uint8_t counter = 1;
	while(pw != counter)
	{
		num *= 10;
		counter++;
	}
	return pw == 0 ? 1 : num;
}

void makeDisplayValue()
{
	dispVal[0] = 0;
	dispVal[1] = 0;
	dispVal[2] = 0;
	dispVal[3] = 0b00111111;
	
	if(writeTo)//NumOne
	{
		if(inf)
		{
			dispVal[0] = 0;
			dispVal[1] = i;
			dispVal[2] = n;
			dispVal[3] = f;
		}
		else
		{
			if(numberOne < 0)
			{
				if(getLength(numberOne) < 4)
				{
					for(uint8_t j = 0; j < getLength(numberOne); j++)
					{
						dispVal[3-j] = setPortD(mod(numberOne/pow_dec(10,j)%10));
					}
					dispVal[3-getLength(numberOne)] = minus;
				}
				else
				{
					dispVal[3] = setPortD(getLength(numberOne)-1);
					dispVal[2] = e;
					dispVal[1] = setPortD(mod(numberOne/pow_dec(10,getLength(numberOne)-1)%10));
					dispVal[0] = minus;
				}
			}//OK
			else
			{
			
				if(getLength(numberOne) <= 4)
				{
				
					for(uint8_t j = 0; j < getLength(numberOne); j++)
					{
						dispVal[3-j] = setPortD(numberOne/pow_dec(10,j)%10);
					}
				
				}
				else
				{	
					dispVal[3] = setPortD(getLength(numberOne)-1);
					dispVal[2] = e;
					dispVal[1] = setPortD(numberOne/pow_dec(10,getLength(numberOne)-1)%10);
					dispVal[0] = 0;
				}
			}//OK
		}
	}
	else//NumTwo
	{
		for(uint8_t j = 0; j < getLength(numberTwo); j++)
		{
			dispVal[3-j] = setPortD(numberTwo/pow_dec(10,j)%10);
		}
	}


	if(belowZero != 0)
	{
		//for(uint8_t j = 0; j < getLength(belowZero); j++)
		//{
			//dispVal[3-j] = setPortD(belowZero/pow_dec(10,j)%10);
		//}
		dispVal[0] = setPortD(belowZero);
	}
}

uint8_t coder(uint8_t code)
{
	code -=1;
	code = code -((code >> 1) & 0b01010101);
	return (code & 0b00110011) + ((code >> 2) & 0b00110011);
}

ISR(TIMER0_COMPA_vect)//Прерывание по сравнению
{
	if(power)
	{	
		go2PreviusZnakoMesto();
		if(currentZnakoMesto < firstZnakoMesto) currentZnakoMesto = fourthZnakoMesto; // поочередное открывание ключей
		
		PORTD = dispVal[3-coder(PORTB)];
		
		if(isKeyPres())
		{
			keyCounter++;
			powerCounter = 0;
		}
		else
		{
			powerCounter++;
			if(powerCounter > powerStop)
			{
				powerCounter = 0;
				keyCounter = 0;
				power = false;
				PORTD = 0;
				PORTB = 0b00001000;
			}
		}

		if(keyCounter > keyDelay)
		{
			pressKey = true; 
			keyCounter = 0;
		}
	}
	else
	{
		if(isKeyPres(On_C))
		{
			keyCounter++;
		}
		if(keyCounter > keyDelay)
		{
			keyCounter = 0;
			power = true;
		}
	
	}
}

void key_disp_timer_init()
{
	TCCR0A  = _BV(WGM01);	//включили CTC
	TCCR0B |= _BV(CS00);
	TCCR0B |= _BV(CS02);	// Установил делитель на 1024 // 8 MHz / 1024 = 7'812Hz
	OCR0A   = 80;   		// Установил делитель на 32 // 8 MHz / 32 = 244Hz ~ 60 меганий на 1 секцию диодов
	TIMSK0  = _BV(OCIE0A);	// Включил работу с TCCR0B
	sei();					// разрешил прерывания
}

void key0(uint8_t* sign,int32_t* number)
{
		*number *= 10;	
}

void key1(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 1;
}

void key2(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 2;	
}

void key3(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 3;
}

void key4(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 4;
}

void key5(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 5;
}

void key6(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 6;
}

void key7(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 7;
}

void key8(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 8;
}

void key9(uint8_t* sign,int32_t* number)
{
		*number = *number*10 + 9;
}

void On_C_f(uint8_t* sign,int32_t* number)
{
	if((*sign) == Equ)
	{
		inf	= false;
		numberTwo = 0;
		numberOne = 0;
		writeTo = NumOne;
		(*sign) = 0;
	}

	(*number) /= 10;
}

void Plus_f(uint8_t* sign,int32_t* number)
{
	writeTo = !writeTo;
	*sign = Plus;
	dispVal[0] = p;
	dispVal[1] = l;
	dispVal[2] = u;
	dispVal[3] = s;
	pressKey = false;
	_delay_ms(delaySign);
}

void Sub_f(uint8_t* sign,int32_t* number)
{
	writeTo = !writeTo;
	*sign = Sub;
	dispVal[0] = 0;
	dispVal[1] = s;
	dispVal[2] = u;
	dispVal[3] = b;
	pressKey = false;
	_delay_ms(delaySign);
}

void Did_f(uint8_t* sign,int32_t* number)
{
	writeTo = !writeTo;
	*sign = Did;
	dispVal[0] = 0;
	dispVal[1] = d;
	dispVal[2] = i;
	dispVal[3] = d;
	pressKey = false;
	_delay_ms(delaySign);
}

void Mul_f(uint8_t* sign,int32_t* number)
{
	writeTo = !writeTo;
	*sign = Mul;
	dispVal[0] = m;
	dispVal[1] = m;
	dispVal[2] = u;
	dispVal[3] = l;
	pressKey = false;
	_delay_ms(delaySign);
}

void Equ_f(uint8_t* sign,int32_t* number)
{
	writeTo = NumOne;
	dispVal[0] = 0;
	dispVal[1] = e;
	dispVal[2] = q;
	dispVal[3] = u;
	pressKey = false;
	_delay_ms(delaySign);
	switch (*sign)
	{
		case Plus:
		numberOne += numberTwo;
		break;

		case Sub:
		numberOne -= numberTwo;
		break;

		case Mul:
		numberOne *= numberTwo;
		break;

		case Did:
		if(numberTwo == 0)
		{
			inf = true;
			numberOne = 0;
			numberTwo = 0;
		}
		else 
		{
			belowZero = 5 % 2 ;numberTwo;
			numberOne /= numberTwo;
		}
		break;

		default:
		break;
	}
	*sign = Equ;
	numberTwo = 0;
}

typedef void ( *VoidFunPtr )(uint8_t* ,int32_t* );
const VoidFunPtr keyBoard[4][4] = {{Plus_f,Sub_f,Mul_f,Did_f},
									{Equ_f,key3,key6,key9},
									{key0,key2,key5,key8},
									{On_C_f,key1,key4,key7}};

uint8_t getColumn()
{
	return coder(PINB & 0b00001111);
}

uint8_t getRow()
{
	return coder((PINB & 0b11110000) >> 4);
}

int main()
{
	ADCSRA &= ~_BV(ADEN); // Отключаем АЦП
	DDRD  = 0b11111111;// порты D 0-7  в режиме вывода (для дисплея)
	PORTD = 0b00000000;
	DDRB  = 0b11111111;// порты B 0-3 в режиме вывода|порты B 4-7 в режиме ввода(для клавы) 
	PORTB = 0b00001000;
		
	uint8_t sign = 0; // запоминает нужное действие (+-=:*)
		
	key_disp_timer_init();
	
	while(1)
	{
		if(power)
		{
			if(pressKey)
			{
				if(writeTo)//NumOne
				{
					keyBoard[getColumn()][getRow()]( &sign, &numberOne);
				}
				else//NumTwo
				{
					keyBoard[getColumn()][getRow()]( &sign, &numberTwo);
				}
				pressKey = false;
				if(numberTwo > 9999)numberTwo %= 10000;
				makeDisplayValue();
				
				if(numberOne > 9999)
				{
					if(sign == Equ)
					{
						makeDisplayValue();
						numberOne = 0;
					}
					else
					{
						numberOne %= 10000;
						makeDisplayValue();
					}
				}
			}//if pressKey
		}	
	}	
}

