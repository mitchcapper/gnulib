/* Copyright (C) 2000, 2009-2025 Free Software Foundation, Inc.
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

#include <config.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include<handleapi.h>
#include <stdio.h>
#endif
/* Specification.  */
#include <spawn.h>

#include <errno.h>
#include <unistd.h>

#if !_LIBC
# define __sysconf(open_max) getdtablesize ()
#endif

#if REPLACE_POSIX_SPAWN
# include "spawn_int.h"
#endif

/* Add an action to FILE-ACTIONS which tells the implementation to call
   'close' for the given file descriptor during the 'spawn' call.  */
int
posix_spawn_file_actions_addclose (posix_spawn_file_actions_t *file_actions,
                                   int fd)
#undef posix_spawn_file_actions_addclose
{
  int maxfd = __sysconf (_SC_OPEN_MAX);

  /* Test for the validity of the file descriptor.  */
  if (fd < 0 || fd >= maxfd)
    return EBADF;
#ifdef _WIN32
  DWORD flags;
  HANDLE hdl = (HANDLE)_get_osfhandle(fd);
  if (GetHandleInformation(hdl, &flags) && (flags & HANDLE_FLAG_INHERIT)) {
	  fprintf(stderr, "posix_spawn_file_actions_addclose called with file descriptor %d (wh: %p) however this FD already has the inherit flag on it.  GNULIB will leak any fd that pre-has inherit set, to stop call SetHandleInformation(_get_osfhandle(%d),HANDLE_FLAG_INHERIT,0);",fd,hdl,fd );
	  exit(1);
  }
	  
#endif
#if !REPLACE_POSIX_SPAWN
  return posix_spawn_file_actions_addclose (file_actions, fd);
#else
  /* Allocate more memory if needed.  */
  if (file_actions->_used == file_actions->_allocated
      && __posix_spawn_file_actions_realloc (file_actions) != 0)
    /* This can only mean we ran out of memory.  */
    return ENOMEM;

  {
    struct __spawn_action *rec;

    /* Add the new value.  */
    rec = &file_actions->_actions[file_actions->_used];
    rec->tag = spawn_do_close;
    rec->action.open_action.fd = fd;

    /* Account for the new entry.  */
    ++file_actions->_used;

    return 0;
  }
#endif
}
