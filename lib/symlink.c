/* Stub for symlink().
   Copyright (C) 2009-2025 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <sys/stat.h>


#if HAVE_SYMLINK

# undef symlink

/* Create a symlink, but reject trailing slash.  */
int
rpl_symlink (char const *contents, char const *name)
{
  size_t len = strlen (name);
  if (len && name[len - 1] == '/')
    {
      struct stat st;
      if (lstat (name, &st) == 0 || errno == EOVERFLOW)
        errno = EEXIST;
      return -1;
    }
#ifdef _WIN32
  struct stat path_stat;
  stat(contents, &path_stat);
  DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
  if (path_stat.st_mode & _S_IFDIR)
	  flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;

  if (CreateSymbolicLink(name, contents, flags))
	  return 0;

  errno = GetLastError();
  return -1;
#else
  return symlink(contents, name);
#endif
  
}

#else /* !HAVE_SYMLINK */

/* The system does not support symlinks.  */
int
symlink (_GL_UNUSED char const *contents,
         _GL_UNUSED char const *name)
{
  errno = ENOSYS;
  return -1;
}

#endif /* !HAVE_SYMLINK */
