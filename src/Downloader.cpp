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


#include "Downloader.h"
#include "Locker.h"


namespace cdnget
{


Downloader::Downloader()
{
	createMutex(&this->mutex);
	pthread_rwlock_init(&_stop_rwlock, NULL);
	this->reset();
	this->curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, this);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Downloader::curl_write_head);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Downloader::curl_write_body);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, Downloader::curl_xferinfo);
	/*curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, Downloader::curl_xferinfo);*/
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
}

Downloader::~Downloader()
{
	curl_easy_cleanup(curl);
	pthread_rwlock_destroy(&_stop_rwlock);
	pthread_mutex_destroy(&this->mutex);
}

void Downloader::reset()
{
	this->headStarted = false;
	this->headCompleted = false;
	this->headLen = 0;
	this->bodyLen = 0;
	this->status = "";
	this->statuscode = 0;
	pthread_rwlock_wrlock(&_stop_rwlock);
	_stop = false;
	pthread_rwlock_unlock(&_stop_rwlock);
}

size_t Downloader::curl_write_head(char *ptr, size_t size, size_t count, Downloader *downloader)
{
	stringstream ss;
	size_t len = size*count;
	if (!downloader->headStarted)
	{
		for (size_t i = 0; i < len; i++)
		{
			if (ptr[i] == '\n')
			{
				char s1[256] = {0}, s3[256] = {0};
				if (sscanf(downloader->status.c_str(), "%255s %u %255s", s1, &downloader->statuscode, s3) >= 2)
				{
					ss.str("");
					ss << "Status: " << downloader->statuscode << "\r\n";
					ss << "X-CDN-Engine: " << appname << "\r\n";
					downloader->headLen += fprintf(downloader->headFile, "%s", ss.str().c_str());
					*downloader->outputStream << ss.str();
				}
				downloader->headStarted = true;
				return i+1;
			}
			downloader->status += ptr[i];
		}
		return len;
	} else
	{		
		size_t written = fwrite(ptr, size, count, downloader->headFile);
		downloader->headLen += written;
		downloader->outputStream->write(ptr, written);
		return written;
	}
}

size_t Downloader::curl_write_body(char *ptr, size_t size, size_t count, Downloader *downloader)
{
	if (!downloader->headCompleted)
	{
		downloader->headCompleted = true;
		fflush(downloader->headFile);
		locker.unlock(downloader->headFile);
	}
	size_t written = fwrite(ptr, size, count, downloader->bodyFile);
	downloader->bodyLen += written;
	downloader->outputStream->write(ptr, written);
	return written;
}

int Downloader::curl_xferinfo(Downloader *downloader, double dltotal, double dlnow, double ultotal, double ulnow)
//int Downloader::curl_xferinfo(Downloader *downloader, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	bool state;
	pthread_rwlock_rdlock(&downloader->_stop_rwlock);
	state = downloader->_stop;
	pthread_rwlock_unlock(&downloader->_stop_rwlock);
	if (state) return 1;
	usleep(1*1000);
	return 0;
}

bool Downloader::redirect(ostream *dest, FILE *file)
{
	bool result = false;
	size_t len, bufLen = vbuf_size;
	char *buf = (char *)malloc(bufLen);
	if (NULL == buf) return false;
	while (!ferror(file))
	{
		clearerr(file);
		if (0 != (len = fread(buf, 1, bufLen, file)))
		{
			dest->write(buf, len);
			if (dest->bad())
			{
				result = (EPIPE == errno);
				break;
			}
		} else
		{
			if (!locker.testLock(file) && feof(file))
			{
				result = true;
				break;
			}
			usleep(1*1000);
		}
	}
	free(buf);
	return result;
}

void Downloader::stop()
{
	pthread_rwlock_wrlock(&_stop_rwlock);
	_stop = true;
	pthread_rwlock_unlock(&_stop_rwlock);
}

bool Downloader::download(FILE *headFile, FILE *bodyFile, ostream *outputStream, const string &url, const string &useragent, const string &httphost)
{
	bool result = false;
	pthread_mutex_lock(&mutex);
	this->headFile = headFile;
	this->bodyFile = bodyFile;
	this->outputStream = outputStream;
	string str;
	locker.lock(headFile);
	locker.lock(bodyFile);
	ftruncate(fileno(headFile), 0);
	ftruncate(fileno(bodyFile), 0);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
	curl_slist *headers = NULL;
	str = "Host: ";
	str += httphost;
	headers = curl_slist_append(headers, str.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	CURLcode res = curl_easy_perform(curl);
	fflush(headFile);
	fflush(bodyFile);
	locker.unlock(headFile);
	locker.unlock(bodyFile);
	long response_code = 0;
	double content_length = 0.0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
	if (res == CURLE_OK && response_code == 200) result = true;
	curl_slist_free_all(headers);
	reset();
	pthread_mutex_unlock(&mutex);
	return result;
}


}
