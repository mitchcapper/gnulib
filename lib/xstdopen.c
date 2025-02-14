/* Ensure that stdin, stdout, stderr are open.
   Copyright (C) 2019-2025 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "xstdopen.h"

#include "stdopen.h"
#include <error.h>
#include "exitfail.h"

#include "gettext.h"
#define _(msgid) dgettext ("gnulib", msgid)

void
xstdopen (void)
{
  int stdopen_errno = stdopen ();
  if (stdopen_errno != 0)
    /* Ignore stdopen_errno in the error message, since it may be misleading
       (see stdopen.c).  */
    error (exit_failure, 0,
           _("failed to open all three standard file descriptors; maybe %s or %s are not working right?"),
           "/dev/null", "/dev/full");
}
