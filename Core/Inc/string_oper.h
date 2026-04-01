/*
 * string_oper.h
 *
 *  Created on: Mar 31, 2026
 *      Author: RafałMarkielowski
 */

#ifndef INC_STRING_OPER_H_
#define INC_STRING_OPER_H_

typedef enum{
	Sign_plusMinus,
	Sign_minus,
	Sign_plus,
	Sign_none
}Int2Str_plusminus;

typedef enum{
	None,
	Space = ' ',
	Tabu = '	',
	Zero = '0'
}Int2Str_freeSign;

char* Int2Str	  (int value, char freeSign, int maxDigits, int plusMinus);
void  float2stri  (char *buffer, float value, unsigned int dec_digits);

#endif /* INC_STRING_OPER_H_ */
