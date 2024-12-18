# bison-i18n.m4
# serial 6
dnl Copyright (C) 2005-2006, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

dnl From Bruno Haible.

dnl Support for internationalization of bison-generated parsers.

dnl BISON_I18N
dnl should be used in configure.ac, after AM_GNU_GETTEXT. If USE_NLS is yes, it
dnl sets BISON_LOCALEDIR to indicate where to find the bison-runtime.mo files
dnl and defines YYENABLE_NLS if there are bison-runtime.mo files at all.
AC_DEFUN([BISON_I18N],
[
  if test -z "$USE_NLS"; then
    echo "The BISON-I18N macro is used without being preceded by AM-GNU-GETTEXT." | sed -e 's/-/_/g' 1>&2
    exit 1
  fi
  BISON_LOCALEDIR=
  BISON_USE_NLS=no
  if test "$USE_NLS" = yes; then
    dnl Determine bison's localedir.
    dnl Generally, accept an option --with-bison-prefix=PREFIX to indicate where
    dnl find bison's runtime data. Additionally, for users who have installed
    dnl bison in user directories, query the 'bison' program found in $PATH
    dnl (but not when cross-compiling).
    dnl Usually ${prefix}/share/locale, but can be influenced by the configure
    dnl options --datarootdir and --localedir.
    BISON_LOCALEDIR="${localedir}"
    AC_ARG_WITH([bison-prefix],
      [[  --with-bison-prefix=DIR  search for bison's runtime data in DIR/share]],
      [if test "X$withval" != "X" && test "X$withval" != "Xno"; then
         BISON_LOCALEDIR="$withval/share/locale"
       fi
      ])
    if test $cross_compiling != yes; then
      dnl AC_PROG_YACC sets the YACC variable; other macros set the BISON
      dnl variable. But even is YACC is called "yacc", it may be a script that
      dnl invokes bison and accepts the --print-localedir option.
      dnl YACC's default value is empty; BISON's default value is :.
      if (${YACC-${BISON-:}} --print-localedir) >/dev/null 2>&1; then
        BISON_LOCALEDIR=`${YACC-${BISON-:}} --print-localedir`
      fi
    fi
    if test -n "$BISON_LOCALEDIR"; then
      dnl There is no need to enable internationalization if the user doesn't
      dnl want message catalogs.  So look at the language/locale names for
      dnl which the user wants message catalogs.  This is $LINGUAS.  If unset,
      dnl he wants all of them; if non-empty, he wants some of them.
      USER_LINGUAS="${LINGUAS-%UNSET%}"
      if test -n "$USER_LINGUAS"; then
        BISON_USE_NLS=yes
      fi
    fi
    AC_SUBST([BISON_LOCALEDIR])
  fi

  dnl Define BISON_LOCALEDIR_c and BISON_LOCALEDIR_c_make.
  dnl Find the final value of BISON_LOCALEDIR.
  gl_saved_prefix="${prefix}"
  gl_saved_datarootdir="${datarootdir}"
  gl_saved_localedir="${localedir}"
  gl_saved_bisonlocaledir="${BISON_LOCALEDIR}"
  dnl Unfortunately, prefix gets only finally determined at the end of
  dnl configure.
  if test "X$prefix" = "XNONE"; then
    prefix="$ac_default_prefix"
  fi
  eval datarootdir="$datarootdir"
  eval localedir="$localedir"
  eval BISON_LOCALEDIR="$BISON_LOCALEDIR"
  gl_BUILD_TO_HOST([BISON_LOCALEDIR])
  BISON_LOCALEDIR="${gl_saved_bisonlocaledir}"
  localedir="${gl_saved_localedir}"
  datarootdir="${gl_saved_datarootdir}"
  prefix="${gl_saved_prefix}"

  if test $BISON_USE_NLS = yes; then
    AC_DEFINE([YYENABLE_NLS], [1],
      [Define to 1 to internationalize bison runtime messages.])
  fi
])
