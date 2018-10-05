/* c-auto.h.  Generated automatically by configure.  */
/* c-auto.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if the closedir function returns void instead of int.  */
/* #undef CLOSEDIR_VOID */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your putenv doesn't waste space when the same environment
   variable is assigned more than once, with different (malloced)
   values.  This is true only on NetBSD/FreeBSD, as far as I know. See
   xputenv.c.  */
/* #undef SMART_PUTENV */

/* Define if you are using GNU libc or otherwise have global variables
   `program_invocation_name' and `program_invocation_short_name'.  */
/* #undef HAVE_PROGRAM_INVOCATION_NAME */

/* Define if you have the basename function.  */
/* #undef HAVE_BASENAME */

/* Define if you have the bcopy function.  */
/* #undef HAVE_BCOPY */

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the <assert.h> header file.  */
#define HAVE_ASSERT_H 1

/* Define if you have the <dirent.h> header file.  */
// NOT WINNT #define HAVE_DIRENT_H 1

/* Define if you have the <float.h> header file.  */
#define HAVE_FLOAT_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <pwd.h> header file.  */
/* #define HAVE_PWD_H 1 */

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file.  */
#define HAVE_SYS_DIR_H  1 // COMES FROM OCP

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <unistd.h> header file.  */
// NOTYET WINNT #define HAVE_UNISTD_H 1
