# stdnoreturn.m4
# serial 1
dnl Copyright 2012-2025 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# Check for stdnoreturn.h that conforms to C11.

# Prepare for substituting <stdnoreturn.h> if it is not supported.

AC_DEFUN([gl_STDNORETURN_H],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    cygwin*)
      dnl Regardless whether a working <stdnoreturn.h> exists or not,
      dnl we need our own <stdnoreturn.h>, because of the definition
      dnl of _Noreturn done by gnulib-common.m4.
      GL_GENERATE_STDNORETURN_H=true
      ;;
    *)
      AC_CACHE_CHECK([for working stdnoreturn.h],
        [gl_cv_header_working_stdnoreturn_h],
        [AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM(
              [[#include <stdlib.h>
                #include <stdnoreturn.h>
                #if defined _WIN32 && !defined __CYGWIN__
                # include <process.h>
                #endif
                /* Do not check for 'noreturn' after the return type.
                   C11 allows it, but it's rarely done that way
                   and circa-2012 bleeding-edge GCC rejects it when given
                   -Werror=old-style-declaration.  */
                noreturn void foo1 (void) { exit (0); }
                _Noreturn void foo2 (void) { exit (0); }
                int testit (int argc, char **argv)
                {
                  if (argc & 1)
                    return 0;
                  (argv[0][0] ? foo1 : foo2) ();
                }
              ]])],
           [gl_cv_header_working_stdnoreturn_h=yes],
           [gl_cv_header_working_stdnoreturn_h=no])])
      if test $gl_cv_header_working_stdnoreturn_h = yes; then
        GL_GENERATE_STDNORETURN_H=false
      else
        GL_GENERATE_STDNORETURN_H=true
      fi
      ;;
  esac
])
