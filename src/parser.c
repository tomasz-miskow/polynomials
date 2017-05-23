/** @file
   TODO

   @author Tomasz Miśków <tm385898@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date TODO
*/

#include "poly.h"
#include "parser.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/** TODO */
#define MAX_COMMAND_LENGTH 9

/** TODO */
typedef unsigned long parser_coeff_t;

/** TODO */
typedef long parser_exp_t;

/* DEKLARACJE FUNKCJI POMOCNICZYCH */

/** TODO */
static void ParserSkipLine();

/** TODO */
static bool ParserCoeffIsInRange(parser_coeff_t parser_coeff, bool is_negative);

/** TODO */
static bool ParserExpIsInRange(parser_exp_t parser_exp);

/** TODO */
static bool ParserVarIndexIsInRange(unsigned long parser_var_index);

/** TODO */
static bool CharIsStartOfCoeff(char c);

/** TODO */
static bool CharIsStartOfCommand(char c);

/** TODO */
static parser_coeff_t CharToCoeff(char c);

/** TODO */
static parser_exp_t CharToExp(char c);

/** TODO */
static char CharPeek();

/** TODO */
static char ParseChar(int *col);

/** TODO */
static ParserResult ParseCoeff(int *col, poly_coeff_t *coeff);

/** TODO */
static ParserResult ParseMonoExp(int *col, poly_exp_t *exp);

/** TODO */
static ParserResult ParseMono(int *col, Mono *m);

/** TODO */
static ParserResult ParsePoly(int *col, Poly *p);

/** TODO */
static CalcCommand ParseCommandFromArray(char char_array[]);

/** TODO */
static ParserResult ParseVarIdx(unsigned *var_idx);

/** TODO */
static ParserResult ParseValue(poly_coeff_t *coeff);

/* IMPLEMENTACJA FUNKCJI POMOCNICZYCH */

static void ParserSkipLine()
{
	// TODO
	while (getchar() != '\n');
}

static bool ParserCoeffIsInRange(parser_coeff_t parser_coeff, bool is_negative)
{
	if (is_negative)
	{
		parser_coeff_t max = ((parser_coeff_t) LONG_MAX) + 1;
		return parser_coeff <= max;
	}
	else
	{
		return parser_coeff <= LONG_MAX;
	}
}

static bool ParserExpIsInRange(parser_exp_t parser_exp)
{
	return (0 <= parser_exp && parser_exp <= INT_MAX);
}

static bool ParserVarIndexIsInRange(unsigned long parser_var_index)
{
	return parser_var_index <= INT_MAX;
}

static inline bool CharIsStartOfCoeff(char c)
{
	return isdigit(c) || c == '-';
}

static bool CharIsStartOfCommand(char c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static parser_exp_t CharToExp(char c)
{
	assert(isdigit(c));
	return (poly_coeff_t) c - '0';
}

static parser_coeff_t CharToCoeff(char c)
{
	assert(isdigit(c));
	return (poly_coeff_t) c - '0';
}

static char CharPeek()
{
	char c = getchar();
	ungetc(c, stdin);
	return c;
}

static char ParseChar(int *col)
{
	if (col)
	{
		(*col)++;
	}

	return getchar();
}

static ParserResult ParseCoeff(int *col, poly_coeff_t *coeff)
{
	if (CharIsStartOfCoeff(CharPeek()))
	{
		parser_coeff_t parser_coeff = 0;
		bool is_negative = false;

		if (CharPeek() == '-')
		{
			is_negative = true;
			ParseChar(col);

			if (!isdigit(CharPeek()))
			{
				return PARSER_ERROR;
			}
		}

		while (isdigit(CharPeek()))
		{
			char c = ParseChar(col);
			parser_coeff = 10 * (parser_coeff) + CharToCoeff(c);

			if (!ParserCoeffIsInRange(parser_coeff, is_negative))
			{
				return PARSER_ERROR;
			}
		}

		*coeff = (poly_coeff_t) parser_coeff;

		if (is_negative)
		{
			*coeff *= -1;
		}

		return PARSER_SUCCESS;
	}
	else
	{
		return PARSER_ERROR;
	}
}

static ParserResult ParseMonoExp(int *col, poly_exp_t *exp)
{
	if (!isdigit(CharPeek()))
	{
		return PARSER_ERROR;
	}
	else
	{
		parser_exp_t parser_exp = 0;

		while (isdigit(CharPeek()))
		{
			char c = ParseChar(col);
			parser_exp = 10 * parser_exp + CharToExp(c);

			if (!ParserExpIsInRange(parser_exp))
			{
				return PARSER_ERROR;
			}
		}

		*exp = parser_exp;
		return PARSER_SUCCESS;
	}
}

static ParserResult ParseMono(int *col, Mono *m)
{
	if (CharPeek() == '(')
	{
		ParseChar(col);
		Poly p;
		if (ParsePoly(col, &p) != PARSER_ERROR)
		{
			if (CharPeek() == ',')
			{
				ParseChar(col);
				poly_exp_t exp;

				if (ParseMonoExp(col, &exp) != PARSER_ERROR)
				{
					*m = MonoFromPoly(&p, exp);

					if (CharPeek() == ')')
					{
						ParseChar(col);
						return PARSER_SUCCESS;
					}
					else
					{
						return PARSER_ERROR;
					}
				}
				else
				{
					return PARSER_ERROR;
				}
			}
			else
			{
				return PARSER_ERROR;
			}
		}
		else
		{
			return PARSER_ERROR;
		}
	}
	else
	{
		return PARSER_ERROR;
	}
}

static ParserResult ParsePoly(int *col, Poly *p)
{
	if (CharIsStartOfCoeff(CharPeek()))
	{
		poly_coeff_t coeff;

		if (ParseCoeff(col, &coeff) == PARSER_ERROR)
		{
			return PARSER_ERROR;
		}
		else
		{
			*p = PolyFromCoeff(coeff);
			return PARSER_SUCCESS;
		}
	}
	else if (CharPeek() == '(')
	{
		unsigned size = 10;
		Mono* monos = (Mono*) calloc(size, sizeof(Mono));
		assert(monos);

		unsigned count = 0;

		ParserResult parser_result = PARSER_SUCCESS;
		do
		{
			if (CharPeek() == '+')
			{
				ParseChar(col);
			}

			parser_result = ParseMono(col, &(monos[count]));
			count++;

			if (count == size - 1)
			{
				size *= 2;
				monos = (Mono*) realloc(monos, size * sizeof(Mono));
				assert(monos);
			}
		} while (parser_result == PARSER_SUCCESS && CharPeek() == '+');

		*p = PolyAddMonos(count, monos);
		free(monos);
		return parser_result;
	}
	else
	{
		*p = PolyZero();
		return PARSER_ERROR;
	}
}

static CalcCommand ParseCommandFromArray(char char_array[])
{
	if (strcmp(char_array, "ZERO") == 0)
	{
		return CALC_ZERO;
	}
	else if (strcmp(char_array, "IS_COEFF") == 0)
	{
		return CALC_IS_COEFF;
	}
	else if (strcmp(char_array, "IS_ZERO") == 0)
	{
		return CALC_IS_ZERO;
	}
	else if (strcmp(char_array, "CLONE") == 0)
	{
		return CALC_CLONE;
	}
	else if (strcmp(char_array, "ADD") == 0)
	{
		return CALC_ADD;
	}
	else if (strcmp(char_array, "MUL") == 0)
	{
		return CALC_MUL;
	}
	else if (strcmp(char_array, "NEG") == 0)
	{
		return CALC_NEG;
	}
	else if (strcmp(char_array, "SUB") == 0)
	{
		return CALC_SUB;
	}
	else if (strcmp(char_array, "IS_EQ") == 0)
	{
		return CALC_IS_EQ;
	}
	else if (strcmp(char_array, "DEG") == 0)
	{
		return CALC_DEG;
	}
	else if (strcmp(char_array, "DEG_BY") == 0)
	{
		return CALC_DEG_BY;
	}
	else if (strcmp(char_array, "AT") == 0)
	{
		return CALC_AT;
	}
	else if (strcmp(char_array, "PRINT") == 0)
	{
		return CALC_PRINT;
	}
	else if (strcmp(char_array, "POP") == 0)
	{
		return CALC_POP;
	}
	else
	{
		return CALC_WRONG_COMMAND;
	}
}

static ParserResult ParseVarIdx(unsigned *var_idx)
{
	if (!isdigit(CharPeek()))
	{
		return PARSER_ERROR;
	}
	else
	{
		unsigned long parser_var_idx = 0;

		while (isdigit(CharPeek()))
		{
			char c = ParseChar(NULL);
			parser_var_idx = 10 * parser_var_idx + CharToExp(c);

			if (!ParserVarIndexIsInRange(parser_var_idx))
			{
				return PARSER_ERROR;
			}
		}

		*var_idx = parser_var_idx;
		return PARSER_SUCCESS;
	}
}

static ParserResult ParseValue(poly_coeff_t *value)
{
	if (CharIsStartOfCoeff(CharPeek()))
	{
		parser_coeff_t parser_coeff = 0;
		bool is_negative = false;

		if (CharPeek() == '-')
		{
			is_negative = true;
			ParseChar(NULL);

			if (!isdigit(CharPeek()))
			{
				return PARSER_ERROR;
			}
		}

		while (isdigit(CharPeek()))
		{
			char c = ParseChar(NULL);
			parser_coeff = 10 * (parser_coeff) + CharToCoeff(c);

			if (!ParserCoeffIsInRange(parser_coeff, is_negative))
			{
				return PARSER_ERROR;
			}
		}

		*value = (poly_coeff_t) parser_coeff;

		if (is_negative)
		{
			*value *= -1;
		}

		return PARSER_SUCCESS;
	}
	else
	{
		return PARSER_ERROR;
	}
}

/* IMPLEMENTACJA FUNKCJI GŁÓWNYCH */

bool ParserIsCommand()
{
	return CharIsStartOfCommand(CharPeek());
}

bool ParserIsEndOfFile()
{
	return CharPeek() == EOF;
}

ParserResult ParseLinePoly(Poly *p, int row)
{
	int col = 1;

	ParserResult parser_result = ParsePoly(&col, p);

	if (parser_result == PARSER_ERROR || CharPeek() != '\n')
	{
		PolyDestroy(p);
		ErrorParserPoly(row, col);
		ParserSkipLine();
		return PARSER_ERROR;
	}
	else
	{
		ParserSkipLine();
		return parser_result;
	}
}

ParserResult ParseLineCommand(CalcCommand *command, poly_coeff_t *parameter, int row)
{
	char char_array[MAX_COMMAND_LENGTH] = "";

	unsigned i = 0;

	while (CharPeek() != ' ' && CharPeek() != '\n' && i < MAX_COMMAND_LENGTH - 1)
	{
		char_array[i] = ParseChar(NULL);
		i++;
	}

	*command = ParseCommandFromArray(char_array);

	if (*command == CALC_WRONG_COMMAND)
	{
		ParserSkipLine();
		ErrorParserCommand(1);
		return PARSER_ERROR;
	}
	else if (*command == CALC_DEG_BY || *command == CALC_AT)
	{
		if (CharPeek() == ' ')
		{
			ParseChar(NULL);

			unsigned int var_idx;

			switch (*command)
			{
				case CALC_DEG_BY:
					if (ParseVarIdx(&var_idx) == PARSER_SUCCESS)
					{
						*parameter = (poly_coeff_t) var_idx;
					}
					else
					{
						ParserSkipLine();
						ErrorParserVariable(row);
						return PARSER_ERROR;
					}
					break;

				case CALC_AT:
					if (ParseValue(parameter) == PARSER_ERROR)
					{
						ParserSkipLine();
						ErrorParserValue(row);
						return PARSER_ERROR;
					}
					break;
				default:
					assert(false);
					return PARSER_ERROR;
					break;
			}

			if (CharPeek() != '\n')
			{
				ParserSkipLine();
				ErrorParserCommand(row);
				return PARSER_ERROR;
			}
			else
			{
				return PARSER_SUCCESS;
			}
		}
		else
		{
			ParserSkipLine();
			ErrorParserCommand(1);
			return PARSER_ERROR;
		}
	}
	else if (CharPeek() != '\n')
	{
		ErrorParserCommand(1);
		ParserSkipLine();
		return PARSER_ERROR;
	}
	else
	{
		ParseChar(NULL);
		return PARSER_SUCCESS;
	}
}