# stack-trace.m4
# serial 2
dnl Copyright (C) 2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_STACK_TRACE_EARLY],
[
  AC_MSG_CHECKING([whether to enable debugging facilities])
  AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([[--disable-debug]],
       [turn off debugging facilities])],
    [case "$enableval" in
       yes | no) ;;
       *) AC_MSG_WARN([invalid argument supplied to --enable-debug])
          enable_debug=yes
          ;;
     esac
    ],
    [enable_debug=yes])
  AC_MSG_RESULT([$enable_debug])

  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  if test $enable_debug = yes; then
    dnl The first choice is libbacktrace by Ian Lance Taylor.
    dnl Maintained at https://github.com/ianlancetaylor/libbacktrace,
    dnl mirrored into GCC, installed as part of GCC by a few distros.
    dnl It produces source file names and line numbers, if the binary
    dnl is compiled with debug information.
    AC_CACHE_CHECK([for libbacktrace], [gl_cv_lib_backtrace], [
      gl_saved_LIBS="$LIBS"
      LIBS="$gl_saved_LIBS -lbacktrace"
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
           [[#include <backtrace.h>
           ]],
           [[struct backtrace_state *state =
               backtrace_create_state (NULL, 0, NULL, NULL);
           ]])],
        [gl_cv_lib_backtrace=yes],
        [gl_cv_lib_backtrace=no])
      LIBS="$gl_saved_LIBS"
    ])
    if test $gl_cv_lib_backtrace = yes; then
      AC_DEFINE([HAVE_LIBBACKTRACE], [1],
        [Define if you have the libbacktrace library.])
      CAN_PRINT_STACK_TRACE=1
      LIBS="$LIBS -lbacktrace"
    else
      dnl The second choice is GCC's libasan, installed as part of GCC.
      dnl It produces source file names and line numbers, if the binary
      dnl is compiled with debug information.
      AC_CACHE_CHECK([for libasan], [gl_cv_lib_asan], [
        gl_saved_LIBS="$LIBS"
        LIBS="$gl_saved_LIBS -lasan"
        dnl We need AC_RUN_IFELSE here, not merely AC_LINK_IFELSE, because on
        dnl NetBSD libasan exists but every program that links to it immediately
        dnl prints "This sanitizer is not compatible with enabled ASLR" to
        dnl standard error and exits with code 0, without even invoking main().
        AC_RUN_IFELSE(
          [AC_LANG_PROGRAM(
             [[#include <stdio.h>
               extern
               #ifdef __cplusplus
               "C"
               #endif
               void __sanitizer_print_stack_trace (void);
             ]],
             [[remove ("conftest.c");
               __sanitizer_print_stack_trace ();
             ]])],
          [if test -f conftest.c; then
             dnl main() was not reached. NetBSD!
             gl_cv_lib_asan=no
           else
             gl_cv_lib_asan=yes
           fi
          ],
          [gl_cv_lib_asan=no],
          [case "$host_os" in
             netbsd*) gl_cv_lib_asan="guessing no" ;;
             *)       gl_cv_lib_asan="guessing yes" ;;
           esac
          ])
        LIBS="$gl_saved_LIBS"
      ])
      case "$gl_cv_lib_asan" in
        *yes)
          AC_DEFINE([HAVE_LIBASAN], [1],
            [Define if you have the libasan library.])
          CAN_PRINT_STACK_TRACE=1
          LIBS="$LIBS -lasan"
          ;;
        *)
          dnl The third choice is libexecinfo.
          dnl It does not produce source file names and line numbers, only addresses
          dnl (which are mostly useless due to ASLR) and _sometimes_ function names.
          case "$host_os" in
            *-gnu* | gnu* | darwin* | freebsd* | dragonfly* | netbsd* | openbsd* | solaris*)
              dnl execinfo might be implemented on this platform.
              CAN_PRINT_STACK_TRACE=1
              dnl On *BSD system, link all programs with -lexecinfo. Cf. m4/execinfo.m4.
              case "$host_os" in
                freebsd* | dragonfly* | netbsd* | openbsd*)
                  LIBS="$LIBS -lexecinfo"
                  ;;
              esac
              dnl Link all programs in such a way that the stack trace includes the
              dnl function names. '-rdynamic' is equivalent to '-Wl,-export-dynamic'.
              case "$host_os" in
                *-gnu* | gnu* | openbsd*)
                  LDFLAGS="$LDFLAGS -rdynamic"
                  ;;
              esac
              ;;
          esac
          ;;
      esac
    fi
  fi
])

AC_DEFUN([gl_STACK_TRACE],
[
  AC_REQUIRE([AC_C_INLINE])
])