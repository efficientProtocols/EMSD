/*+
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef SCCS_VER	/*{*/
static char sccs[] = "@(#)saynum.c	1.1    Released: 1/28/88";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "pf.h"

#include "int2english.h"

/*
**              Integer to Readable ASCII Conversion Routine.
**
** Synopsis:
**
**      say_cardinal(value)
**      	long int     value;          -- The number to output
**
**	The number is translated into a string of phonemes
**
*/

static String cardinals0[] = {
	"zero ",	"one ",	       	"two ",		"three ",
	"four ",	"five ",	"six ",		"seven ",
	"eight ",	"nine ",		
	"ten ",		"eleven ",	"twelve ",	"thirteen ",
	"fourteen ",	"fifteen ", 	"sixteen ",	"seventeen ",
	"eighteen ",	"nineteen "
} ;

static String cardinals20[] = {
	"twenty ",	"thirty ",	"fourty ",	"fifty ",
	"sixty ",	"seventy ",	"eighty ",	"ninety "
} ;

static String ordinals0[] = {
	"zeroth ",	"first ",	"second ",	"third ",
	"fourth ",	"fifth ",	"sixth ",	"seventh ",
	"eigth ",	"nineth ",		
	"tenth ",	"eleventh ",	"twelveth ",	"thirteenth ",
	"fourteenth ",	"fifteenth ", 	"sixteenth ",	"seventeenth ",
	"eighteenth ",	"nineteenth "
} ;

static String ordinals20[] = {
	"twentieth ", "thirtieth ",	"fourtieth ",	"fiftieth ",
	"sixtieth ",  "seventyieth ",   "eightieth ",	"ninetyth "
} ;


/*
**	 Note: this is recursive.
*/
Char *
PF_intToCardinalEnglish(strBegin, strEnd, value)
Char *strBegin;
Char *strEnd;
LgInt value;
{
    Char *str;

    str = strBegin;
    *str = '\0';

    if (value < 0) {
	if (!(str = PF_strmcat(str, "minus ", strEnd))) {
	    return (Char *) 0;
	}
	value = (-value);
	if (value < 0) {	/* Over flow */
	    return ("infinity");
	}
    }

    if (value >= 1000000000L) {	/* Billions */
	if (!(str = PF_intToCardinalEnglish(str, strEnd, value/1000000000L))) {
	    return (Char *) 0;
	}
	if (!(str = PF_strmcat(str, "bilion ", strEnd))) {
	    return (Char *) 0;
	}
	value = value % 1000000000;
	if (value == 0) {
	    return (str);		/* Even billion */
	}
	if (value < 100) {	/* as in THREE BILLION AND FIVE */
	    if (!(str = PF_strmcat(str, "and ", strEnd))) {
		return (Char *) 0;
	    }
	}
    }

    if (value >= 1000000L) {	/* Millions */
	if (!(str = PF_intToCardinalEnglish(str, strEnd, value/1000000L))) {
	    return (Char *) 0;
	}
	if (!(str = PF_strmcat(str, "million ", strEnd))) {
	    return (Char *) 0;
	}
	value = value % 1000000L;
	if (value == 0) {
	    return (str);		/* Even million */
	}
	if (value < 100) {	/* as in THREE MILLION AND FIVE */
	    if (!(str = PF_strmcat(str, "and ", strEnd))) {
		return (Char *) 0;
	    }
	}
    }

    /* Thousands 1000..1099 2000..99999 */
    /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
    if ((value >= 1000L && value <= 1099L) || value >= 2000L) {
	if (!(str = PF_intToCardinalEnglish(str, strEnd, value/1000L))) {
	    return (Char *) 0;
	}
	if (!(str = PF_strmcat(str, "thousand ", strEnd))) {
	    return (Char *) 0;
	}
	value = value % 1000L;
	if (value == 0) {
	    return (str);		/* Even thousand */
	}
	if (value < 100) {	/* as in THREE thousand AND FIVE */
	    if (!(str = PF_strmcat(str, "and ", strEnd))) {
		return (Char *) 0;
	    }
	}
    }

    if (value >= 100L) {
	if (!(str = PF_strmcat(str, cardinals0[value/100], strEnd))) {
	    return (Char *) 0;
	}
	if (!(str = PF_strmcat(str, "hundred ", strEnd))) {
	    return (Char *) 0;
	}
	value = value % 100;
	if (value == 0) {
	    return (str);		/* Even hundred */
	}
    }

    if (value >= 20) {
	if (!(str = PF_strmcat(str, cardinals20[(value-20)/10], strEnd))) {
	    return (Char *) 0;
	}
	value = value % 10;
	if (value == 0) {
	    return (str);		/* Even ten */
	}
    }

    if (!(str = PF_strmcat(str, cardinals0[value], strEnd))) {
	return (Char *) 0;
    }
    return (str);
} 


/*
 *	Ordinals
 */
Char *
PF_intToOrdinalEnglish(strBegin, strEnd, value)
Char *strBegin;
Char *strEnd;
LgInt value;
{
    Char *str;

    str = strBegin;
    *str = '\0';

    if (value < 0) {
	if (!(str = PF_strmcat(str, "minus ", strEnd))) {
	    return (Char *) 0;
	}
	value = (-value);
	if (value < 0) {	/* Over flow */
	    return ("infinity");
	}
    }

    if (value >= 1000000000L) {	/* Billions */
	if (!(str = PF_intToCardinalEnglish(str, strEnd, value/1000000000L))) {
	    return (Char *) 0;
	}
	value = value % 1000000000;
	if (value == 0) {
	    if (!(str = PF_strmcat(str, "billionth ", strEnd))) {
		return (Char *) 0;
	    }
	    return (str);		/* Even billion */
	}
	if (!(str = PF_strmcat(str, "bilion ", strEnd))) {
	    return (Char *) 0;
	}
	if (value < 100) {	/* as in THREE BILLION AND FIVE */
	    if (!(str = PF_strmcat(str, "and ", strEnd))) {
		return (Char *) 0;
	    }
	}
    }

    if (value >= 1000000L) {	/* Millions */
	if (!(str = PF_intToCardinalEnglish(str, strEnd, value/1000000L))) {
	    return (Char *) 0;
	}
	value = value % 1000000L;
	if (value == 0) {
	    if (!(str = PF_strmcat(str, "millionth ", strEnd))) {
		return (Char *) 0;
	    }
	    return (str);		/* Even million */
	}
	if (!(str = PF_strmcat(str, "million ", strEnd))) {
	    return (Char *) 0;
	}
	if (value < 100) {	/* as in THREE MILLION AND FIVE */
	    if (!(str = PF_strmcat(str, "and ", strEnd))) {
		return (Char *) 0;
	    }
	}
    }

    /* Thousands 1000..1099 2000..99999 */
    /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
    if ((value >= 1000L && value <= 1099L) || value >= 2000L) {
	if (!(str = PF_intToCardinalEnglish(str, strEnd, value/1000L))) {
	    return (Char *) 0;
	}
	value = value % 1000L;
	if (value == 0) {
	    if (!(str = PF_strmcat(str, "thousandth ", strEnd))) {
		return (Char *) 0;
	    }
	    return (str);		/* Even thousand */
	}
	if (!(str = PF_strmcat(str, "thousand ", strEnd))) {
	    return (Char *) 0;
	}
	if (value < 100) {	/* as in THREE thousand AND FIVE */
	    if (!(str = PF_strmcat(str, "and ", strEnd))) {
		return (Char *) 0;
	    }
	}
    }

    if (value >= 100L) {
	if (!(str = PF_strmcat(str, cardinals0[value/100], strEnd))) {
	    return (Char *) 0;
	}
	value = value % 100;
	if (value == 0) {
	    if (!(str = PF_strmcat(str, "hundredth ", strEnd))) {
		return (Char *) 0;
	    }
	    return (str);		/* Even hundred */
	}
	if (!(str = PF_strmcat(str, "hundred ", strEnd))) {
	    return (Char *) 0;
	}
    }

    if (value >= 20) {
	if ((value%10) == 0) {
	    if (!(str = PF_strmcat(str, ordinals20[(value-20)/10], strEnd))) {
		return (Char *) 0;
	    }
	    return (str);		/* Even ten */
	}
	if (!(str = PF_strmcat(str, cardinals20[(value-20)/10], strEnd))) {
	    return (Char *) 0;
	}
	value = value % 10;
    }

    if (!(str = PF_strmcat(str, ordinals0[value], strEnd))) {
	return (Char *) 0;
    }
    return (str);
} 

/*
 *	Digits
 */
Char *
PF_intToDigitEnglish(strBegin, strEnd, value)
Char *strBegin;
Char *strEnd;
LgInt value;
{
    Char *str;
    Char digBuf[256];
    String buf;
    Char c;

    str = strBegin;
    *str = '\0';

    if (value < 0) {
	if (!(str = PF_strmcat(str, "minus ", strEnd))) {
	    return (Char *) 0;
	}
	value = (-value);
	if (value < 0) {	/* Over flow */
	    return ("infinity");
	}
    }

    buf = digBuf;
    sprintf(buf, "%ld", value);
    while ((c = *buf++)) {
	if (! (c >= '0' && c <= '9') ) {
	    EH_problem("");
	    return (Char *) 0;
	}
	if (!(str = PF_strmcat(str, cardinals0[c-'0'], strEnd))) {
	    return (Char *) 0;
	}
    }
    return (str);
}

/*
 *	String To Digits in English
 */
char *
PF_strToDigitEnglish(char *strBegin, char *strEnd, String digitStr)
{
    Char *str;
    Char c;

    str = strBegin;
    *str = '\0';
    while ((c = *digitStr++)) {
	if (! (c >= '0' && c <= '9') ) {
	    EH_problem("");
	    return (Char *) 0;
	}
	if (!(str = PF_strmcat(str, cardinals0[c-'0'], strEnd))) {
	    return (Char *) 0;
	}
    }
    return (str);
}
