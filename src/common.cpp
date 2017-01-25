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


#include "common.h"
#include "Path.h"


namespace cdnget
{


string appname = "cdnget";
string appver = "0.9.3";
string appdesc = "CDN Reverse Proxy";

string useragent = appname + "_" + appver;

size_t vbuf_size = 256*1024;
size_t thread_stack_size = 10*1024*1024;


pthread_t createThread(void *(*start_routine)(void *), void *arg, bool detached)
{
	pthread_t th;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if (detached) pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, thread_stack_size);
	if (0 != pthread_create(&th, &attr, start_routine, arg)) th = 0;
	pthread_attr_destroy(&attr);
	return th;
}

bool createMutex(pthread_mutex_t *mutex, bool nonrecursive)
{
	bool result = true;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	if (!nonrecursive) pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	if (0 != pthread_mutex_init(mutex, &attr)) result = false;
	pthread_mutexattr_destroy(&attr);
	return result;
}

bool mkdir_p_proc(string part, string strPath, bool dir)
{
	if (!dir) return true;
	return (0 == mkdir(strPath.c_str(), 0755));
}

bool mkdir_p(const string &strPath)
{
	Path path(strPath);
	path.normalize();
	return path.exec(mkdir_p_proc);
}


}
