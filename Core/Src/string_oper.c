/*
 * string_oper.c
 *
 *  Created on: Mar 31, 2026
 *      Author: RafałMarkielowski
 */

#include "string_oper.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SIZE_STRBUFF	50
#define SIZE_HEXBUFF	50

static int idx=0;
static char strBuff[SIZE_STRBUFF]={0};
static char hexBuff[SIZE_HEXBUFF]={0};

static unsigned char hex_to_ASCII (unsigned char val)
{
	unsigned char hex=val&0xF;

    if(hex==0x01) return '1';
    else if(hex==0x02) return '2';
	else if(hex==0x03) return '3';
	else if(hex==0x04) return '4';
	else if(hex==0x05) return '5';
	else if(hex==0x06) return '6';
	else if(hex==0x07) return '7';
	else if(hex==0x08) return '8';
	else if(hex==0x09) return '9';
	else if(hex==0x00) return '0';
	else if(hex==0x0A) return 'A';
	else if(hex==0x0B) return 'B';
	else if(hex==0x0C) return 'C';
	else if(hex==0x0D) return 'D';
	else if(hex==0x0E) return 'E';
	else if(hex==0x0F) return 'F';
	return 0;
}

char* Int2Str(int value, char freeSign, int maxDigits, int plusMinus)
{
	int i=10,k=1,j,idx_copy;
	int absolutValue;
	char sign;

	idx_copy=idx;

	int _IsSign(void)
	{
		if(value<0)
		{
			switch(plusMinus)
			{
			case Sign_plusMinus:
			case Sign_minus:
				return 1;
			}
		}
		else if(value>0)
		{
			switch(plusMinus)
			{
			case Sign_plusMinus:
			case Sign_plus:
				return 1;
			}
		}
		return 0;
	}

	if(value<0){
		absolutValue=-value;
		sign='-';
	}
	else{
		absolutValue=value;
		sign='+';
	}

	while(1)
	{
		if(absolutValue<i)
		{
		   if(k<maxDigits)
			{
		   	if(idx+maxDigits >= SIZE_STRBUFF){
		   		idx=0;
		   		idx_copy=0;
		   	}

		   	if(Sign_none!=plusMinus)
		   	{
			   	if(Space==freeSign)
			   		strBuff[idx++]=Space;
			   	else
			   	{
			   		if(1==_IsSign())
			   			strBuff[idx++]=sign;
			   		else
			   			strBuff[idx++]=Space;
			   	}
		   	}

		   	if(None!=freeSign)
		   	{
			   	j=maxDigits-k;
			   	memset(&strBuff[idx],freeSign,j);
			   	idx+=j;
		   	}

		   	if(Space==freeSign)
		   	{
		   		if(1==_IsSign())
		   			strBuff[idx-1]=sign;
		   	}
			}
		   else
		   {
		   	if(idx+k >= SIZE_STRBUFF){
		   		idx=0;
		   		idx_copy=0;
		   	}

		   	if(Sign_none!=plusMinus)
		   	{
		   		if(1==_IsSign())
		   			strBuff[idx++]=sign;
		   		else
		   			strBuff[idx++]=Space;
		   	}
		   }
			itoa(absolutValue,&strBuff[idx],10);
			idx+=k;
			break;
		}
		else
		{
			i*=10;
			k++;
		}
	}
	strBuff[idx++]=0;
	return strBuff+idx_copy;
}

const double round_nums[8] =
{ 0.5, 0.05, 0.005, 0.0005, 0.00005,0.000005,0.0000005,0.00000005};

void float2stri(char *buffer, float value, unsigned int dec_digits)
{
	int idx;
	int64_t dbl_int, dbl_frac;
	int64_t mult = 1;
	char *output = buffer;
	char tbfr[40];

	if (isfinite(value))
	{
		if ((value <= -99999999999999) || (value >= 99999999999999))
		{
			*output++ = '-';
			*output++ = '-';
			*output++ = '-';
			*output++ = '-';
			*output++ = '-';
		}
		else
		{
			if (value < 0.0)
			{
				*output++ = '-';
				value *= -1.0;
			}

			if (dec_digits <8)
			{
				value += round_nums[dec_digits];
				for (idx = 0; idx < dec_digits; idx++)
					mult *= 10;
			}
			else
			{
				dec_digits = 7;
				value += round_nums[7];
				mult = 10000000;
			}

			dbl_int = (int64_t) value;
			dbl_frac = (int64_t) ((value - (float) dbl_int) * (float) mult);

			idx = 0;
			while (dbl_int != 0)
			{
				tbfr[idx++] = '0' + (dbl_int % 10);
				dbl_int /= 10;
			}

			if (idx == 0)
				*output++ = '0';
			else
			{
				while (idx > 0)
				{
					*output++ = tbfr[idx - 1];
					idx--;
				}
			}

			if (dec_digits > 0)
			{
				*output++ = '.';

				idx = 0;
				while (dbl_frac != 0)
				{
					tbfr[idx++] = '0' + (dbl_frac % 10);
					dbl_frac /= 10;
				}
				while (idx < dec_digits)
					tbfr[idx++] = '0';

				if (idx == 0)
					*output++ = '0';
				else
				{
					while (idx > 0)
					{
						*output++ = tbfr[idx - 1];
						idx--;
					}
				}
			}
		}
	}
	else
	{
		*output++ = '-';
		*output++ = '-';
		*output++ = '-';
		*output++ = '-';
		*output++ = '-';
	}
	*output = 0;
}

char* HexToAscii(uint8_t val,int mem){
	int i=mem;
	hexBuff[i++]=hex_to_ASCII(val>>4);
	hexBuff[i++]=hex_to_ASCII(val);
	hexBuff[i++]=0;
	return &hexBuff[mem];
}
char* Hex2ToAscii(uint16_t val,int mem){
	int i=mem;
	hexBuff[i++]=hex_to_ASCII(val>>12);
	hexBuff[i++]=hex_to_ASCII(val>>8);
	hexBuff[i++]=hex_to_ASCII(val>>4);
	hexBuff[i++]=hex_to_ASCII(val);
	hexBuff[i++]=0;
	return &hexBuff[mem];
}
