/* c-vararg.h: Top layer for stdarg and varargs.

Copyright (C) 1993 Karl Berry.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef C_VARARG_H
#define C_VARARG_H

/* See `kpathsea/init-path.c' for an example of use.  The idea is to say
   PVAR1(type1, parameter1, ap) in the function header, and then end
   the function with two }}'s.  We do this to avoid having to specify
   the argument list (with types) twice -- once in the function header,
   and once in a (hypothetical) VA_START1.  */

#if __STDC__
#include <stdarg.h>

#define PVAR1H(p1) (p1, ...)
#define PVAR2H(p1, p2) (p1, p2, ...)
#define PVAR3H(p1, p2, p3) (p1, p2, p3, ...)

#define PVAR1C(t1, n1,  ap) \
  (t1 n1, ...) { va_list ap; va_start (ap, n1);
#define PVAR2C(t1, n1,  t2, n2,  ap) \
  (t1 n1, t2 n2, ...) { va_list ap; va_start (ap, n2);
#define PVAR3C(t1, n1,  t2, n2,  t3, n3,  ap) \
  (t1 n1, t2 n2, t3 n3, ...) { va_list ap; va_start (ap, n3);

#else /* not __STDC__ */
#include <varargs.h>

#define PVAR1H(p1) ()
#define PVAR2H(p1, p2) ()
#define PVAR3H(p1, p2, p3) ()

#define PVAR1C(t1, n1,  ap) \
  (va_alist) va_dcl { t1 n1; va_list ap; va_start (ap); \
                      n1 = va_arg (ap, t1);
#define PVAR2C(t1, n1,  t2, n2,  ap) \
  (va_alist) va_dcl { t1 n1; t2 n2; va_list ap; va_start (ap); \
                      n1 = va_arg (ap, t1); n2 = va_arg (ap, t2);
#define PVAR3C(t1, n1,  t2, n2,  t3, n3,  ap) \
  (va_alist) va_dcl { t1 n1; t2 n2; t3 n3; va_list ap; va_start (ap); \
                      n1 = va_arg (ap, t1); n2 = va_arg (ap, t2); \
                      n3 = va_arg (ap, t3);
#endif /* not __STDC__ */

#endif /* not C_VARARG_H */
