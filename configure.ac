AC_PREREQ(2.59)
AC_INIT([cdnget], [0.93], [orkunkaraduman@gmail.com])
AM_INIT_AUTOMAKE([1.11 -Wall -Werror foreign])

AC_PROG_CC([gcc])
AC_PROG_CXX([g++])

CPPFLAGS="-Wall -Werror -pedantic -g0 -O2"
CFLAGS="-std=c11"
CXXFLAGS="-std=c++11"

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	bin/Makefile
])

AC_HEADER_STDC
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_PID_T

AC_CHECK_HEADERS([ \
	unistd.h \
	fcntl.h \
	sys/syscall.h \
	sys/types.h \
	sys/file.h \
	sys/stat.h \
	sysexits.h \
	pthread.h \
	fcgio.h \
	curl/curl.h \
	curl/easy.h \
])

AC_CHECK_LIB([pthread], [pthread_create],, [AC_MSG_ERROR([pthread library is missing])])
AC_CHECK_LIB([fcgi], [FCGX_Init],, [AC_MSG_ERROR([FastCGI library is missing])])
AC_CHECK_LIB([fcgi++], [FCGX_Init],, [AC_MSG_ERROR([FastCGI++ library is missing])])
AC_CHECK_LIB([curl], [curl_easy_init],, [AC_MSG_ERROR([CURL library is missing])])

AC_OUTPUT
