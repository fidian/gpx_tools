AC_INIT([gpx-tools], [1.0], [fidian@rumkin.com])

dnl Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  src/Makefile
  ])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_EXEEXT

AC_CHECK_FUNCS([memset])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_C_CONST
AC_HEADER_STDC

if test $CC = yes; then
  CFLAGS="$CFLAGS -Wall -fomit-frame-pointer -O2"
fi

AC_ARG_WITH(expathdr,
  [  --with-expathdr[=DIR]   Use this to specify the location of expat.h],
  [ xpathdr="$withval" ], [
        case "$target" in
        *-*-darwin*)
                if test -f /sw/include/expat.h ; then
                        xpathdr=/sw/include/
                fi
                ;;
        *-*-freebsd*)
                if test -f /usr/local/include/expat.h ; then
                        xpathdr=/usr/local/include
                fi
                ;;
        *)      ;;
        esac
]
)

if test "x-$xpathdr" != "x-" ; then
        CFLAGS="$CFLAGS -I$xpathdr"
fi

AC_MSG_CHECKING(for libexpat)
AC_ARG_WITH(libexpat,
  [  --with-libexpat[=DIR]   Use this to specify expat library .],
  [ CFLAGS="$CFLAGS -L$withval"
    EXPAT_LIB="-L$withval -lexpat"
  ], [
        case "$target" in
        *-*-darwin*)
                if test -f /sw/lib/libexpat.a ; then
                        EXPAT_LIB=/sw/lib/libexpat.a
                          AC_DEFINE(HAVE_LIBEXPAT, 1, [Defined if you have libexpat])
                          AC_SUBST(EXPAT_LIB)
                fi
                ;;
        *-*-freebsd*)
                if test -f /usr/local/lib/libexpat.a ; then
                        EXPAT_LIB=/usr/local/lib/libexpat.a
                          AC_DEFINE(HAVE_LIBEXPAT, 1, [Defined if you have libexpat])
                          AC_SUBST(EXPAT_LIB)
                fi
                ;;
        *)
                        EXPAT_LIB=-lexpat
                ;;
        esac
  ]
)
AC_MSG_RESULT($EXPAT_LIB)

AC_CHECK_LIB([expat], [XML_ParserCreate])

AC_OUTPUT
