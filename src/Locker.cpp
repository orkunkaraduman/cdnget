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


#include "Locker.h"


namespace cdnget
{


Locker locker;
Locker::FD_LOCKS Locker::fd_locks;
pthread_mutex_t Locker::fd_locks_mutex = PTHREAD_MUTEX_INITIALIZER;


Locker::Locker()
{
}

Locker::~Locker()
{
}

string Locker::getfnfromfd(int fd)
{
	string result = "";
	char s[FILENAME_MAX] = { 0 };
	char fn[FILENAME_MAX] = { 0 };
	sprintf(s, "/proc/self/fd/%d", fd);
	if (readlink(s, fn, sizeof(fn)) >= 0)
	{
		result = fn;
	}
	return result;
}

bool Locker::lock(int fd)
{
	if (fd < 0) return false;
	bool result = false;
	string fn = getfnfromfd(fd);
	pthread_mutex_lock(&fd_locks_mutex);
	if (fd_locks.find(fn) == fd_locks.end() && lockf(fd, F_TLOCK, 0) == 0)
	{
		fd_locks.insert(fn);			
		result = true;
	}
	pthread_mutex_unlock(&fd_locks_mutex);
	return result;
}

bool Locker::lock(FILE *file)
{
	if (NULL == file) return false;
	return lock(fileno(file));
}

bool Locker::unlock(int fd)
{
	bool result = false;
	string fn = getfnfromfd(fd);
	pthread_mutex_lock(&fd_locks_mutex);
	if (lockf(fd, F_ULOCK, 0) == 0)
	{
		fd_locks.erase(fn);
		result = true;
	}
	pthread_mutex_unlock(&fd_locks_mutex);
	return result;
}

bool Locker::unlock(FILE *file)
{
	return unlock(fileno(file));
}

bool Locker::testLock(int fd)
{
	bool result = false;
	string fn = getfnfromfd(fd);
	pthread_mutex_lock(&fd_locks_mutex);
	if (fd_locks.find(fn) != fd_locks.end() || lockf(fd, F_TEST, 0) != 0)
	{
		result = true;
	}
	pthread_mutex_unlock(&fd_locks_mutex);
	return result;
}

bool Locker::testLock(FILE *file)
{
	return testLock(fileno(file));
}


}
