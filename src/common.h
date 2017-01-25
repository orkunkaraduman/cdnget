/*
 * cdnget - CDN Reverse Proxy
 * Copyright (C) 2014  Orkun Karaduman <orkunkaraduman@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once


#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <cstring>
#include <clocale>
#include <climits>
#include <cerrno>
#include <istream>

#include <ostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <set>
#include <deque>
#include <map>

#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <pthread.h>
#include <fcgio.h>
#include <curl/curl.h>
#include <curl/easy.h>


using namespace std;


namespace cdnget
{


extern string appname;
extern string appver;
extern string appdesc;

extern string useragent;

extern size_t vbuf_size;
extern size_t thread_stack_size;


pthread_t createThread(void *(*start_routine)(void *), void *arg = NULL, bool detached = false);
bool createMutex(pthread_mutex_t *mutex, bool nonrecursive = false);
bool mkdir_p(const string &strPath);


}
