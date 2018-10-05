/*
 *  Copyright (C) 1995,1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File name: udp_pc.c
 *
 * Module: UDP_IF point of control.
 *
 * Description: Functions for selectively interrupting the flow of UDP 
 *              service data units between the UDP and UDP user layers.
 *
 * Functions:
 * 
 * PUBLIC SuccFail UDP_PC_lossy(Int,Int)
 * PUBLIC SuccFail UDP_PC_inhibit(Int,Int)
 * PUBLIC Int UDP_PC_dataReqInhibit(Void);
 * PUBLIC Int UDP_PC_receiveInhibit(Void);
 *
-*/

/*
 * Author: Mohsen Banan, Hugh Shane
 * History:
 *
 */


#include "estd.h"
#include "tm.h"
#include "udp_pc.h"

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: udp_pc.c,v 1.5 1999/11/14 04:56:23 mohsen Exp $";
#endif /*}*/

#ifdef UDP_PC_	/*{*/

#define INHIBIT TRUE
#define NO_INHIBIT FALSE

/*
 * local objects
 */
static Int nullFunc(Void);
static Int (*receiveFunc)(Void) = &nullFunc;
static Int (*dataReqFunc)(Void) = &nullFunc;
static Int receiveInhibitCounter = 0;
static Int dataReqInhibitCounter = 0;
static Int receiveLossPercent = 0;
static Int dataReqLossPercent = 0;

extern TM_ModuleCB *UDP_modCB;


/*<
 * Function:    nullFunc
 *
 * Description: No inhibit
 *
 * Arguments:   None.
 *
 * Returns:     NO_INHIBIT
 * 
>*/   

STATIC Int
nullFunc( Void )
{
  return NO_INHIBIT;
}



/*<
 * Function:    receiveInhibit
 *
 * Description: 
 *
 * Arguments:	None.
 *
 * Returns: 
 *
>*/

STATIC Int 
receiveInhibit( Void )
{
  if (receiveInhibitCounter-- == 0) {
    receiveFunc = &nullFunc;
    return NO_INHIBIT;
  } else {
    TM_TRACE((UDP_modCB, TM_ENTER, "receiveInhibit = TRUE"));
    return INHIBIT;
  }
}


/*<
 * Function:    dataReqInhibit( Void )
 *
 * Description: 
 *
 * Arguments:	None.
 *
 * Returns: 
 *
>*/

STATIC Int 
dataReqInhibit( Void )
{
  if (dataReqInhibitCounter-- == 0) {
    dataReqFunc = &nullFunc;
    return NO_INHIBIT;
  } else {
    TM_TRACE((UDP_modCB, TM_ENTER, "receiveInhibit = TRUE"));
    return INHIBIT;
  }

}


/*<
 * Function:    receiveLossy
 *
 * Description: 
 *
 * Arguments:	None.
 *
 * Returns: 
 *
>*/

STATIC Int 
receiveLossy( Void )
{
    return NO_INHIBIT;
}


/*<
 * Function:    dataReqLossy
 *
 * Description: 
 *
 * Arguments:	None.
 *
 * Returns: 
 *
>*/

STATIC Int 
dataReqLossy( Void )
{
    return NO_INHIBIT;
}


/*<
 * Function:  PUBLIC SuccFail UDP_PC_inhibit(Int direction, Int next)  
 *
 * Description: inhibit the next N PDUs 
 *
 * Arguments:   direction - send or receive
 *              next - how many PDUs to inhibit 
 *
 * Returns:     
 * 
>*/       
PUBLIC SuccFail
UDP_PC_inhibit(Int direction, Int next)
{
  switch ( direction )
  {
  case UDP_PC_INHIBIT_SEND:
    dataReqInhibitCounter = next;
    dataReqFunc = &dataReqInhibit;
    return SUCCESS;
  case UDP_PC_INHIBIT_RECEIVE:
    receiveInhibitCounter = next;
    receiveFunc = &receiveInhibit;
    return SUCCESS;
  default:
    return FAIL;
  }
}


/*<
 * Function:  PUBLIC SuccFail UDP_PC_lossy(Int direction, Int percent)  
 *
 * Description: inhibit a percentage of all PDUs 
 *
 * Arguments:   direction - send or receive
 *              percent - percentage to lose 
 *
 * Returns:     
 * 
>*/       
PUBLIC SuccFail
UDP_PC_lossy(Int direction, Int percent)
{

printf("\n ******** UDP_PC_lossy: direction = %d, percent = %d *********", 
direction,  percent);
return SUCCESS;

  switch ( direction )
  {
  case UDP_PC_INHIBIT_SEND:
    dataReqLossPercent = percent;
    dataReqFunc = &dataReqLossy;
    return SUCCESS;
  case UDP_PC_INHIBIT_RECEIVE:
    receiveLossPercent = percent;
    receiveFunc = &receiveLossy;
    return SUCCESS;
  default:
    return FAIL;
  }

}



/*<
 * Function:    Int UDP_PC_dataReqInhibit(Void)
 *              Int UDP_PC_ReceiveInhibit(Void)
 *
 * Description: Test if SDU should be inhibited. 
 *
 * Arguments:   None
 *
 * Returns:     FALSE if no inhibit, TRUE if inhibit.
 * 
>*/
PUBLIC Int 
UDP_PC_dataReqInhibit(Void)
{
  return (*dataReqFunc)();
}


/*<
 * Function:    UDP_PC_receiveInhibit
 *
 * Description: 
 *
 * Arguments:	None.
 *
 * Returns: 
 *
>*/

PUBLIC Int 
UDP_PC_receiveInhibit(Void)
{
  return (*receiveFunc)();
}

#endif /*}*/
