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

#ifndef INT2ENGLISH_H_
#define INT2ENGLISH_H_

Char *
PF_intToCardinalEnglish(Char *strBegin, Char *strEnd, LgInt value);

Char *
PF_intToOrdinalEnglish(Char *strBegin, Char *strEnd, LgInt value);

Char *
PF_intToDigitEnglish(Char *strBegin, Char *strEnd, LgInt value);

Char *
PF_strToDigitEnglish(Char *strBegin, Char *strEnd, String digitStr);

#endif /* INT2ENGLISH_H_ */
