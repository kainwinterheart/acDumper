#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.66])
AC_INIT(acDumper, 0.1, http://facebook.com/kain.winterheart)
AM_INIT_AUTOMAKE(acDumper, 0.1)
AC_CONFIG_SRCDIR([main.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([pcrecpp], [main], [], [AC_MSG_ERROR([libpcrecpp is missing.])])
AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR([libpthread is missing.])])
AC_CHECK_LIB([rudeconfig], [main], [], [AC_MSG_ERROR([librudeconfig is missing.])])
AC_CHECK_LIB([z], [deflate], [], [AC_MSG_ERROR([libz is missing.])])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h fenv.h float.h limits.h malloc.h memory.h netinet/in.h stddef.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h syslog.h unistd.h mysql/mysql.h pthread.h zlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([alarm bzero localtime_r memset mkdir modf regcomp rint stpcpy strchr strpbrk strrchr strstr strtol strtoul strtoull])

AC_OUTPUT(Makefile)
