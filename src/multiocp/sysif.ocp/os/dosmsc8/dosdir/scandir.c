/*
 * scandir.c
 * 
 * MS-DOS compatible version of scandir(3B).
 *
 * Uses MS-DOS compatible directory(3C) functions.
 *
 */


/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct dirent (through namelist). Returns -1 if there were any errors.
 */


#include <stdlib.h>
#include "dosos.h"
#include "dir.h"

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct dirent
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof (struct dirent) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

int
scandir( dirname, namelist, select, dcomp)
	const char *dirname;
	struct dirent *(*namelist[]);
	int (*select)(struct dirent *);
	int (*dcomp)(const void *, const void *);
{
	register struct dirent *d, *p, **names;
	register size_t nitems;
	long arraysz;
	DIR *dirp;

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);

	/*
	 * Calculate the array size by counting the number of entries. This
	 * is really the only way to do this under DOS
	 */
	for (arraysz = 0; (d = readdir(dirp)) != NULL; arraysz++) 
            ;

	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		return(-1);

	/*
	 * Now reinit the directory
         */
	closedir(dirp);
	if ((dirp = opendir(dirname)) == NULL)
		return(-1);

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			return(-1);
		p->d_namlen = d->d_namlen;
		bcopy(d->d_name, p->d_name, p->d_namlen + 1);
		++nitems;
		names[nitems-1] = p;
	}
	closedir(dirp);
	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(struct dirent *), dcomp);
	*namelist = names;
	return(nitems);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
int
alphasort(d1, d2)
	const void *d1;
	const void *d2;
{
	return(strcmp((*(struct dirent **)d1)->d_name,
	    (*(struct dirent **)d2)->d_name));
}
