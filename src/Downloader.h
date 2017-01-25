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


namespace cdnget
{


class Downloader
{
private:
	pthread_mutex_t mutex;
	bool headStarted, headCompleted;
	size_t headLen, bodyLen;
	string status;
	unsigned int statuscode;
	pthread_rwlock_t _stop_rwlock;
	bool _stop;
	CURL *curl;
	void reset();
	FILE *headFile, *bodyFile;
	ostream *outputStream;
	static size_t curl_write_head(char *ptr, size_t size, size_t count, Downloader *downloader);
	static size_t curl_write_body(char *ptr, size_t size, size_t count, Downloader *downloader);
	static int curl_xferinfo(Downloader *downloader, double dltotal, double dlnow, double ultotal, double ulnow);
	//static int curl_xferinfo(Downloader *downloader, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
	
public:
	Downloader();
	virtual ~Downloader();
	static bool redirect(ostream *dest, FILE *file);
	bool download(FILE *headFile, FILE *bodyFile, ostream *outputStream, const string &url, const string &useragent, const string &httphost);
	void stop();

};


}
