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


#include "common.h"
#include "Downloader.h"


namespace cdnget
{


class Worker
{
typedef map<pthread_t, Worker *> Workers;

private:
	static pthread_mutex_t workers_mutex;
	static Workers workers;
	pthread_mutex_t mutex;
	bool _terminate;
	pthread_t th;
	FCGX_Request request;
	Downloader downloader;
	static void *work(Worker *worker);
	
public:
	Worker();
	virtual ~Worker();
	static int getWorkersCount();
	static void terminateAll();
	void terminate();
	bool create();
	
};


}
