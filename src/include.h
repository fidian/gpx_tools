// Any system-defined includes should be included from here only.
#include "../config.h"

#ifdef NEED_CTYPE_H
# include <ctype.h>
#endif

#ifdef NEED_STDIO_H
# include <stdio.h>
#endif

#ifdef NEED_STDLIB_H
# ifdef STDC_HEADERS
#  include <stdlib.h>
#  include <stddef.h>
# else
#  ifdef HAVE_STDLIB_H
#   include <stdlib.h>
#  endif
# endif
#endif

#ifdef NEED_STRING_H
# ifdef HAVE_STRING_H
#  if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#   include <memory.h>
#  endif
#  include <string.h>
# endif
#endif

#ifdef NEED_EXPAT_H
# ifdef EXPAT_XMLPARSE
#  error THIS IS NEVER DEFINED!!
#  include <xmlparse.h>
# else
#  include <expat.h>
# endif
#endif

#define __FILE2__ ((strrchr(__FILE__, '/')?:__FILE__ - 1)+1)
#define DEBUG(x)
//#define DEBUG(x) fprintf(stderr, "[%s %d] %s\n", __FILE2__, __LINE__, x); fflush(stderr)
