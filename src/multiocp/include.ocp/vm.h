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

/*
 * SCCS Revision: @(#)vm.h	1.1    Released: 6/13/90
 */

#ifndef VM_H
#define VM_H

#ifndef O_RDONLY
#include <fcntl.h>
#endif

typedef struct VM_MsgDesc {
    Int	fileDesc;
    Int mode;
    Char fileName[128];
} VM_MsgDesc;

#define VM_RDONLY O_RDONLY
#define VM_WRONLY O_WRONLY
#define VM_RDWR O_RDWR
#define VM_CREAT O_CREAT
#define VM_APPEND O_APPEND

EXTERN VM_MsgDesc * VM_open(Char *msgName1, Int mode);

void VM_init();
Int VM_close(VM_MsgDesc *msgDesc);
#endif
	
