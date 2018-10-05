/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*
 * This example program demonstrates the propagation of a
 * Data Unit through Open C Layers;
 * UPPER_, N_ and LOWER_.
 */

#include "estd.h"
#include "eh.h"
#include "du.h"

DU_Pool  *G_duMainPool;     /* Buffer Pool */
#define MAXBUFSIZE 1548	/* no special significance */
#define NUOFBUFS 22	/* no special significance */
#define NUOFVIEWS (2*NUOFBUFS)	/* no special significance */


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

main()
{
    static char n_sdu[] = "(N)-SDU";
    DU_View view;        

    G_duMainPool = DU_buildPool(MAXBUFSIZE, NUOFBUFS, NUOFVIEWS);
    if ( ! G_duMainPool ) {
	EH_fatal("G_duMainPool");
    }
    UPPER_init(&n_sdu[0], (Int) sizeof(n_sdu));
}


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

UPPER_init(Char *data, Int size)
{
    DU_View view;

    /* Allocate a Data Unit */
    view = DU_alloc(G_duMainPool, size);
    if ( ! view ) {
        EH_fatal("view");
    }
    BS_memCopy(data, DU_data(view), size);

    /* deliver n_sdu to (N) layer */
    N_dataReq(view);
}


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

N_dataReq(DU_View view)
{
    static char n_pci[] = "(N)-PCI  ";
#define NPCI_SIZE (sizeof(n_pci)-1) /* Don't want the '\0' */

    /* Add (N) layer Protocol Control Information
     * and pass it down to layer below.
     */
    DU_prepend(view, NPCI_SIZE);
    BS_memCopy(n_pci, DU_data(view), NPCI_SIZE);

    LOWER_dataReq(view);
}


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

LOWER_dataReq(DU_View view)
{
    printf("(N-1)-SDU = %s\n", (char *) DU_data(view));
    DU_free(view);
}
     
