/* Copyright (C) 1992, 1995-2002, 2005-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Don't use __attribute__ __nonnull__ in this compilation unit.  Otherwise gcc
   optimizes away the name == NULL test below.  */
#define _GL_ARG_NONNULL(params)

#include <config.h>

/* Specification.  */
#include <stdlib.h>

#include <errno.h>
#if !_LIBC
# define __set_errno(ev) ((errno) = (ev))
#endif

#include <string.h>
#include <unistd.h>

#if !_LIBC
# define __environ      environ
#endif

#if _LIBC
/* This lock protects against simultaneous modifications of 'environ'.  */
# include <bits/libc-lock.h>
__libc_lock_define_initialized (static, envlock)
# define LOCK   __libc_lock_lock (envlock)
# define UNLOCK __libc_lock_unlock (envlock)
#else
# define LOCK
# define UNLOCK
#endif

/* In the GNU C library we must keep the namespace clean.  */
#ifdef _LIBC
# define unsetenv __unsetenv
#endif

#if _LIBC || !HAVE_UNSETENV

int
unsetenv (const char *name)
{
  size_t len;

  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  len = strlen (name);

#if HAVE_DECL__PUTENV /* native Windows */
  /* The Microsoft documentation
     <https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/putenv-wputenv>
     says:
       "Don't change an environment entry directly: instead,
        use _putenv or _wputenv to change it."
     Note: Microsoft's _putenv updates not only the contents of _environ but
     also the contents of _wenviron, so that both are in kept in sync.

     The way to remove an environment variable is to pass to _putenv a string
     of the form "NAME=".  (NB: This is a different convention than with glibc
     putenv, which expects a string of the form "NAME"!)  */
  {
    int putenv_result;
    char *name_ = malloc (len + 2);
    if (name_ == NULL)
      return -1;
    memcpy (name_, name, len);
    name_[len] = '=';
    name_[len + 1] = 0;
    putenv_result = _putenv (name_);
    /* In this particular case it is OK to free() the argument passed to
       _putenv.  */
    free (name_);
    return putenv_result;
  }
#else

  LOCK;

  char **ep = __environ;
  while (*ep != NULL)
    if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
      {
        /* Found it.  Remove this pointer by moving later ones back.  */
        char **dp = ep;

        do
          dp[0] = dp[1];
        while (*dp++);
        /* Continue the loop in case NAME appears again.  */
      }
    else
      ++ep;

  UNLOCK;

  return 0;
#endif
}

#ifdef _LIBC
# undef unsetenv
weak_alias (__unsetenv, unsetenv)
#endif

#else /* HAVE_UNSETENV */

# undef unsetenv
# if !HAVE_DECL_UNSETENV
#  if VOID_UNSETENV
extern void unsetenv (const char *);
#  else
extern int unsetenv (const char *);
#  endif
# endif

/* Call the underlying unsetenv, in case there is hidden bookkeeping
   that needs updating beyond just modifying environ.  */
int
rpl_unsetenv (const char *name)
{
  int result = 0;
  if (!name || !*name || strchr (name, '='))
    {
      errno = EINVAL;
      return -1;
    }
  while (getenv (name))
# if !VOID_UNSETENV
    result =
# endif
      unsetenv (name);
  return result;
}

#endif /* HAVE_UNSETENV */
