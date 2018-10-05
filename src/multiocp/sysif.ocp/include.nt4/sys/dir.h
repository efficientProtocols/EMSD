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
 * SCCS Revision: @(#)dir.h	1.1    Released: 1/31/88
 */


#ifndef _DIR_H_	/*{*/
#define _DIR_H_


#define MAXNAMLEN	13

struct direct {
  int 	d_namlen;
  char	d_name[MAXNAMLEN];
};

#define dirent direct

typedef enum DirState {
    DirActive,
    DirVirgin,
    DirInactive
} DirState;

typedef struct {
    char 	dirName[128];	/* Name of the directory, including drive */
    char 	*dirNameEnd;	/* End of String Location for dirName */
    DirState 	state;		/* See DirState */
    // HANDLE	hDir;		/* directory handle */
    void *	hDir;		/* directory handle */
    long 	index;		/* Position of a file name inside the directory */
    char 	indexName[MAXNAMLEN];  /* File Name corresponding to indexName */
} DIR;

extern	DIR *opendir();
extern	struct direct *readdir();
extern	long telldir();
extern	void seekdir();
#define rewinddir(dirp)	seekdir((dirp), (long)0)
#define VOID_CLOSEDIR // For KPATHSEA
extern	void closedir();

extern int
scanDirectory(const char * dirname,
	      struct dirent *** namelist,
	      int (*select)(const struct dirent *),
	      int (*dcomp)(const struct dirent * const *,
			   const struct dirent * const *));

extern int
alphaSort(const struct dirent * const * d1,
	  const struct dirent * const * d2);

#endif	/*}*/
