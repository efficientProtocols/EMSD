/*
 *  Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 *  Copyright (C) 1995  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File name: tmr_intr.c
 *
 * Description: Timer Interupt. MS-DOS Flavor
 *
 * Functions:
 *   tmr_clock_isr(void)
 *   TMR_startClockInterrupt(period)
 *   TMR_stopClockInterrupt(void)
 *
-*/

/*
 * Author: Mohsen Banan, Hugh Shane
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: tmr_intr.c,v 1.2 1999/09/30 00:05:03 mohsen Exp $";
#endif /*}*/

#include <dos.h>

#include "eh.h"
#include "tm.h"
#include "pf.h"
#include "sch.h"
#include "tmr.h"
 
#define PC_TIMER_INT 0x1c
#define PC_CLOCK_FREQ_HZ 18.2
#define PC_CLOCK_PERIOD_MSEC ((int)((1.0/PC_CLOCK_FREQ_HZ)*1000))

EXTERN PUBLIC Void TMR_clock();
EXTERN PUBLIC Int SCH_submit();
EXTERN Int G_isrActive;   /* used by ocp to determine action, declare in g.c */

/* --- Globals --- */

PUBLIC int isrErr;      /* diagnostic */

/* --- ISR Stack --- */

#define ISR_STACK_SIZE 1024
STATIC unsigned isrSaveSS;
STATIC unsigned isrSaveSP;
STATIC Byte isrStack[ISR_STACK_SIZE];
STATIC unsigned isrStackTop;

/* --- Misc local variables --- */

STATIC int clockPeriod;
STATIC void(__cdecl __interrupt __far *tmr_old_isr)();
STATIC int tmr_countdown;
STATIC virgin = TRUE;



/*<
 * Function:    tmr_clock_isr
 *
 * Description: Periodic timer interrupt service routine.
 *
 * Arguments:   None. 
 *
 * Returns:     None. 
 *
 * Caution:     Be sure not to call any functions from here that result in a C compiler
 *              generated stack check (Microsoft C). This includes certain library 
 *              functions like fprintf(), etc.
 *
 * Notes:       All registers, except SS, are stacked by the Microsoft C 
 *              library code prior to entry to this routine. Also, DS is
 *              initialized to the local _DATA segment address.
 * 
>*/
STATIC __interrupt __far
tmr_clock_isr()
{
    static char taskNameTimer[100] = "tmr_clock_isr()";
    unsigned temp;

    if ( ( !G_isrActive ) && ( --tmr_countdown <= 0 ) )
    {
        G_isrActive = 1;                  /* Avoid re-entrancy problems.   */
        tmr_countdown = clockPeriod;    /* re-init the timer             */

	temp = FP_SEG(isrStack);

        __asm                           /* Set up the isr stack.         */
/* tabs! */
        {
	mov ax, ss
	mov isrSaveSS, ax
	mov ax, sp
	mov isrSaveSP, ax

	mov ax, temp
	mov ss, ax
	mov ax, isrStackTop
	mov sp, ax
        }

        TMR_clock();                     /* signal a timer tick          */

				         /* notify the scheduler         */
        isrErr = SCH_submit((Void *)TMR_poll, (Ptr)0, (SCH_Event)0, taskNameTimer);


        __asm                           /* restore the isr stack         */
        {
	mov ax, isrSaveSS
	mov ss, ax

	mov ax, isrSaveSP
	mov sp, ax
        }

        G_isrActive = 0;
    }

    _chain_intr( tmr_old_isr );
}


/*<
 * Function:    TMR_startClockInterrupt
 *
 * Description: Start Clock Interrupt.
 *
 * Arguments:   Desired clock period in milliseconds
 *
 * Returns:     None. 
 *
 * 
>*/

PUBLIC Int
TMR_startClockInterrupt(Int period)
{
Void TMR_stopClockInterrupt(void);

    if ( virgin == TRUE )
    {
        virgin = FALSE;
	G_isrActive = 0;
        isrStackTop = (unsigned)(&isrStack) + ISR_STACK_SIZE - 1;
	period = ( period < PC_CLOCK_PERIOD_MSEC ) ? PC_CLOCK_PERIOD_MSEC : period;
        tmr_countdown = clockPeriod = period/PC_CLOCK_PERIOD_MSEC;
        tmr_old_isr = _dos_getvect(PC_TIMER_INT);
        _dos_setvect(PC_TIMER_INT,tmr_clock_isr);
        atexit(TMR_stopClockInterrupt);
        return (period);
    }
    else
        return (-1);
}


/*<
 * Function:    TMR_clockValidate
 *
 * Description: Check the requested clock value for valid lower bound.
 *
 * Arguments:   None. 
 *
 * Returns:     None. 
 * 
>*/

Int
TMR_clockValidate(Int period)
{
    return ( ( period < PC_CLOCK_PERIOD_MSEC ) ? PC_CLOCK_PERIOD_MSEC : period );
}



/*<
 * Function:    TMR_stopClockInterrupt
 *
 * Description: Stop Clock Interrupt.
 *
 * Arguments:   None. 
 *
 * Returns:     None. 
 * 
>*/

PUBLIC Void
TMR_stopClockInterrupt(void)
{
    _dos_setvect(PC_TIMER_INT,tmr_old_isr);
}
