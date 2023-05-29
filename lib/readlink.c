/* Read the contents of a symbolic link.
   Copyright (C) 2003-2007, 2009-2025 Free Software Foundation, Inc.

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

/* Specification.  */
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#if !HAVE_READLINK

/* readlink() substitute for systems that don't have a readlink() function,
   such as DJGPP 2.03 and mingw32.  */
#ifndef _WIN32
ssize_t
readlink (char const *file, _GL_UNUSED char *buf,
          _GL_UNUSED size_t bufsize)
{
  struct stat statbuf;

  /* In general we should use lstat() here, not stat().  But on platforms
     without symbolic links, lstat() - if it exists - would be equivalent to
     stat(), therefore we can use stat().  This saves us a configure check.  */
  if (lstat (file, &statbuf) >= 0)
    errno = EINVAL;
  return -1;
}
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ioapiset.h>
#include <winioctl.h>
#define __extension__
#define _ANONYMOUS_UNION	__extension__
typedef struct _REPARSE_DATA_BUFFER {
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	_ANONYMOUS_UNION union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	} DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, * PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
// https://github.com/joyent/libuv/blob/1dc2709b999a84520ab1b3c56c0e082bf8617c1f/src/win/fs.c#L971
ssize_t readlink(char const* file, char* target,
	size_t target_len) {
	HANDLE handle = CreateFileA(file, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);//need FILE_FLAG_BACKUP_SEMANTICS  to be able to open directory ssymlinks
	if (handle == INVALID_HANDLE_VALUE)
		return -1;

	char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	REPARSE_DATA_BUFFER* reparse_data = (REPARSE_DATA_BUFFER*)buffer;
	WCHAR* w_target;
	DWORD w_target_len;
	DWORD bytes;

	if (!DeviceIoControl(handle,
		FSCTL_GET_REPARSE_POINT,
		NULL,
		0,
		buffer,
		sizeof buffer,
		&bytes,
		NULL)) {
		CloseHandle(handle);
		return -1;
	}

	if (reparse_data->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
		/* Real symlink */
		w_target = reparse_data->SymbolicLinkReparseBuffer.PathBuffer +
			(reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset /
				sizeof(WCHAR));
		w_target_len =
			reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength /
			sizeof(WCHAR);

		/* Real symlinks can contain pretty much everything, but the only thing */
		/* we really care about is undoing the implicit conversion to an NT */
		/* namespaced path that CreateSymbolicLink will perform on absolute */
		/* paths. If the path is win32-namespaced then the user must have */
		/* explicitly made it so, and we better just return the unmodified */
		/* reparse data. */
		if (w_target_len >= 4 &&
			w_target[0] == L'\\' &&
			w_target[1] == L'?' &&
			w_target[2] == L'?' &&
			w_target[3] == L'\\') {
			/* Starts with \??\ */
			if (w_target_len >= 6 &&
				((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
					(w_target[4] >= L'a' && w_target[4] <= L'z')) &&
				w_target[5] == L':' &&
				(w_target_len == 6 || w_target[6] == L'\\')) {
				/* \??\«drive»:\ */
				w_target += 4;
				w_target_len -= 4;

			}
			else if (w_target_len >= 8 &&
				(w_target[4] == L'U' || w_target[4] == L'u') &&
				(w_target[5] == L'N' || w_target[5] == L'n') &&
				(w_target[6] == L'C' || w_target[6] == L'c') &&
				w_target[7] == L'\\') {
				/* \??\UNC\«server»\«share»\ - make sure the final path looks like */
				/* \\«server»\«share»\ */
				w_target += 6;
				w_target[0] = L'\\';
				w_target_len -= 6;
			}
		}

	}
	else if (reparse_data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
		/* Junction. */
		w_target = reparse_data->MountPointReparseBuffer.PathBuffer +
			(reparse_data->MountPointReparseBuffer.SubstituteNameOffset /
				sizeof(WCHAR));
		w_target_len = reparse_data->MountPointReparseBuffer.SubstituteNameLength /
			sizeof(WCHAR);

		/* Only treat junctions that look like \??\«drive»:\ as symlink. */
		/* Junctions can also be used as mount points, like \??\Volume{«guid»}, */
		/* but that's confusing for programs since they wouldn't be able to */
		/* actually understand such a path when returned by uv_readlink(). */
		/* UNC paths are never valid for junctions so we don't care about them. */
		if (!(w_target_len >= 6 &&
			w_target[0] == L'\\' &&
			w_target[1] == L'?' &&
			w_target[2] == L'?' &&
			w_target[3] == L'\\' &&
			((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
				(w_target[4] >= L'a' && w_target[4] <= L'z')) &&
			w_target[5] == L':' &&
			(w_target_len == 6 || w_target[6] == L'\\'))) {
			SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
			CloseHandle(handle);
			return -1;
		}

		/* Remove leading \??\ */
		w_target += 4;
		w_target_len -= 4;

	}
	else {
		/* Reparse tag does not indicate a symlink. */
		SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
		CloseHandle(handle);
		return -1;
	}


	int rd= WideCharToMultiByte(CP_UTF8,
		0,
		w_target,
		w_target_len,
		target,
		target_len,
		NULL,
		NULL);
	target[rd] = '\0';
	CloseHandle(handle);
	return rd;


}

#endif



#else /* HAVE_READLINK */

# undef readlink

/* readlink() wrapper that uses correct types, for systems like cygwin
   1.5.x where readlink returns int, and which rejects trailing slash,
   for Solaris 9.  */

ssize_t
rpl_readlink (char const *file, char *buf, size_t bufsize)
{
# if READLINK_TRAILING_SLASH_BUG
  size_t file_len = strlen (file);
  if (file_len && file[file_len - 1] == '/')
    {
      /* Even if FILE without the slash is a symlink to a directory,
         both lstat() and stat() must resolve the trailing slash to
         the directory rather than the symlink.  We can therefore
         safely use stat() to distinguish between EINVAL and
         ENOTDIR/ENOENT, avoiding extra overhead of rpl_lstat().  */
      struct stat st;
      if (stat (file, &st) == 0 || errno == EOVERFLOW)
        errno = EINVAL;
      return -1;
    }
# endif /* READLINK_TRAILING_SLASH_BUG */

  ssize_t r = readlink (file, buf, bufsize);

# if READLINK_TRUNCATE_BUG
  if (r < 0 && errno == ERANGE)
    {
      /* Try again with a bigger buffer.  This is just for test cases;
         real code invariably discards short reads.  */
      char stackbuf[4032];
      r = readlink (file, stackbuf, sizeof stackbuf);
      if (r < 0)
        {
          if (errno == ERANGE)
            {
              /* Clear the buffer, which is good enough for real code.
                 Thankfully, no test cases try short reads of enormous
                 symlinks and what would be the point anyway?  */
              r = bufsize;
              memset (buf, 0, r);
            }
        }
      else
        {
          if (bufsize < r)
            r = bufsize;
          memcpy (buf, stackbuf, r);
        }
    }
# endif

# if defined __CYGWIN__
  /* On Cygwin 3.3.6, readlink("/dev/null") returns "\\Device\\Null", which
     is unusable.  Better fail with EINVAL.  */
  if (r > 0 && strncmp (file, "/dev/", 5) == 0 && buf[0] == '\\')
    {
      errno = EINVAL;
      return -1;
    }
# endif

  return r;
}

#endif /* HAVE_READLINK */
