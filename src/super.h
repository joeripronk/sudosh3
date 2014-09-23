/*
 * $Id$
 *
 * Copyright 2004 and $Date$
 *
 * Licensed under the Open Software License version 2.0
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the Open Software
 * License, version 2.0 by Lauwrence E. Rosen.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.	See the Open Software License for details.
 */

#include "config.h"

#ifdef __STRICT_ANSI__
#	ifdef HAVE_FEATURES_H
#		include <features.h>
#	endif
#	define __USE_POSIX
#	define __USE_BSD
#	define __USE_MISC
#endif

#ifdef HAVE_STDIO_H
#	include <stdio.h>
#endif

#ifdef HAVE_STDARG_H
#	include <stdarg.h>
#endif

#ifdef HAVE_STRING_H
#	include <string.h>
#else
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif
#endif

#ifdef HAVE_STDLIB_H
#	include <stdlib.h>
#endif

#ifdef HAVE_ERRNO_H
#	include <errno.h>
#endif

#ifdef HAVE_STDLIB_H
#	include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#	include <time.h>
#	include <sys/time.h>
#else
#	ifdef HAVE_TIME_H
#		include <time.h>
#	endif
#	ifdef HAVE_SYS_TIME_H
#		include <sys/time.h>
#	endif
#endif

#ifdef HAVE_SYS_STAT_H
#	include <sys/stat.h>
#endif

#ifdef HAVE_TERMIOS_H
#	include <termios.h>
#endif

#ifdef HAVE_UTIL_H
#	include <util.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif

#ifdef HAVE_LIBUTIL_H
#	include <libutil.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#	include <sys/ioctl.h>
#endif

#ifdef HAVE_STROPTS_H
#	include <stropts.h>
#endif

#ifdef HAVE_SYS_TERMIO_H
#	include <sys/termio.h>
#endif

#ifdef HAVE_PTY_H
#	include <pty.h>
#endif

#ifdef HAVE_FCNTL_H
#	include <fcntl.h>
#endif

#ifdef HAVE_SIGNAL_H
#	include <signal.h>
#endif

#ifdef HAVE_SYSLOG_H
#	include <syslog.h>
#endif

#ifdef HAVE_PWD_H
#	include <pwd.h>
#endif

#ifdef HAVE_USERSEC_H
#	include <usersec.h>
#endif

#ifdef HAVE_DIRENT_H
#	include <dirent.h>
#endif

#ifdef HAVE_CTYPE_H
#	include <ctype.h>
#endif

#ifndef SIGCHLD
#	define SIGCHLD SIGCLD
#endif

#ifndef TRUE
#	define TRUE (1==1)
#endif

#ifndef FALSE
#	define FALSE (!TRUE)
#endif
