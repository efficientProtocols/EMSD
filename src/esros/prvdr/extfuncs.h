/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of ESRO. An implementation of RFC-2188.
 *
 *ESRO is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *ESRO is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with ESRO; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 *
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 *
*/
/* psq.c:65:OF */ extern void PSQ_init (void); /* () */
/* usq.c:46:OF */ extern void USQ_init (void); /* () */
/* config.c:49:OF */ extern SuccFail EROP_init (Int, Int, Int, Int); /* () */
/* eropfsm.c:53:OF */ extern void FSM_init (void); /* () */
/* trnsdgrm.c:84:OF */ extern void FSM_TransDiag_init (void); /* () */
/* udp_if.c:131:OF */ extern int UDP_init (Int); /* () */
extern PUBLIC Void TMR_init();
extern PUBLIC SuccFail LOPS_init (void);
extern SuccFail G_init(void);
extern int getopt();
/* psq.c:134:OF */ extern void PSQ_poll (void); /* () */
extern PUBLIC Void TMR_poll();
extern Void G_exit(Int unUsed);
extern SuccFail UDP_poll(void);
extern PUBLIC Void TMR_stopClockInterrupt();
extern int getConf();

