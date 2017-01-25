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


#include "Worker.h"
#include "App.h"
#include "Path.h"
#include "Locker.h"


namespace cdnget
{


pthread_mutex_t Worker::workers_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
Worker::Workers Worker::workers;


Worker::Worker()
{
	createMutex(&this->mutex);
	this->_terminate = false;
	this->th = 0;
}

Worker::~Worker()
{
	pthread_mutex_destroy(&this->mutex);
}

int Worker::getWorkersCount()
{
	int result = 0;
	pthread_mutex_lock(&workers_mutex);
	result = (int)workers.size();
	pthread_mutex_unlock(&workers_mutex);
	return result;
}

void Worker::terminateAll()
{
	pthread_mutex_lock(&workers_mutex);
	Workers::iterator it = workers.begin();
	while (it != workers.end())
	{
		Workers::iterator cur = it++;
		Worker *o = (*cur).second;
		o->terminate();
	}
	pthread_mutex_unlock(&workers_mutex);
}

void Worker::terminate()
{
	pthread_mutex_lock(&mutex);
	downloader.stop();
	_terminate = true;
	pthread_mutex_unlock(&mutex);
}

bool Worker::create()
{
	bool result = false;
	pthread_mutex_lock(&mutex);
	
	if (th)
	{
		pthread_mutex_unlock(&mutex);
		return result;
	}
	
	if (0 != FCGX_InitRequest(&request, 0, 0))
	{
		app.log("ERR", "FCGI_INITREQUEST");
		pthread_mutex_unlock(&mutex);
		return result;
	}
	app.log("INFO", "FCGI_INITREQUEST");
	
	if (0 != FCGX_Accept_r(&request))
	{
		FCGX_Free(&request, 1);
		app.log("ERR", "FCGI_ACCEPT");
		pthread_mutex_unlock(&mutex);
		return result;
	}
	app.log("INFO", "FCGI_ACCEPT");

	pthread_mutex_lock(&workers_mutex);
	if (!_terminate && (th = createThread((void *(*)(void *))&Worker::work, this, true)))
	{
		workers[th] = this;
		result = true;
		app.log("INFO", "WORKER_CREATE");
	} else
	{
		app.log("ERR", "WORKER_CREATE");
		FCGX_Finish_r(&request);
	}
	pthread_mutex_unlock(&workers_mutex);
	
	pthread_mutex_unlock(&mutex);
	return result;
}

void *Worker::work(Worker *worker)
{
	pthread_mutex_lock(&workers_mutex);
	time_t starttime = time(NULL);
	app.log("INFO", "WORKER_START");
	pthread_mutex_unlock(&workers_mutex);
	
	char *s;
	string str;
	stringstream ss;
	
	FCGX_Request *request = &worker->request;

	fcgi_streambuf cin_fcgi_streambuf(request->in);
	fcgi_streambuf cout_fcgi_streambuf(request->out);
	fcgi_streambuf cerr_fcgi_streambuf(request->err);
	istream tin(&cin_fcgi_streambuf);
	ostream tout(&cout_fcgi_streambuf);
	ostream terr(&cerr_fcgi_streambuf);

	string cdnget_scheme = "http";
	string cdnget_host = "";
	string cdnget_uri = "/";
	string cdnget_index = "index.html";
	string cdnget_store = "";
	string cdnget_cache = "/var/cache/cdnget";
	string cdnget_httphost = "";
	string cdnget_key = "";
	/*long cdnget_limit = 0;
	long cdnget_bw = 0;*/
	s = FCGX_GetParam("CDNGET_SCHEME", request->envp);
	if (NULL != s) cdnget_scheme = s;
	s = FCGX_GetParam("CDNGET_HOST", request->envp);
	if (NULL != s) cdnget_host = s;
	s = FCGX_GetParam("CDNGET_URI", request->envp);
	if (NULL != s) cdnget_uri = s;
	s = FCGX_GetParam("CDNGET_INDEX", request->envp);
	if (NULL != s) cdnget_index = s;
	s = FCGX_GetParam("CDNGET_STORE", request->envp);
	if (NULL != s) cdnget_store = s;
	s = FCGX_GetParam("CDNGET_CACHE", request->envp);
	if (NULL != s) cdnget_cache = s;
	s = FCGX_GetParam("CDNGET_HTTPHOST", request->envp);
	if (NULL != s) cdnget_httphost = s; else cdnget_httphost = cdnget_host;
	//if ("" == cdnget_httphost) cdnget_httphost = cdnget_host;
	s = FCGX_GetParam("CDNGET_KEY", request->envp);
	if (NULL != s) cdnget_key = s;
	
	str = "/cdnget";
	if (cdnget_uri.compare(0, str.length(), str) != 0)
	{
		Path uriPath(cdnget_uri), storePath(cdnget_store + "/"), cachePath(cdnget_cache + "/");
		if ("" == cdnget_cache || 
			!uriPath.normalize() || uriPath.isRelative() || 
			!storePath.normalize() || storePath.isRelative() || 
			!cachePath.normalize() || cachePath.isRelative())
		{
			app.log("ERR", "PATH_ERROR");
		} else
		{		
			string uri = uriPath.getStrPath() + (uriPath.isDir()? cdnget_index: "");
			string url = cdnget_scheme + "://" + cdnget_host + uri;
			
			Path lockPath(cachePath);
			lockPath.append(cdnget_scheme + "/" + cdnget_host + uri + ".lock");
			Path headPath(cachePath);
			headPath.append(cdnget_scheme + "/" + cdnget_host + uri + ".head");
			Path tempPath(cachePath);
			tempPath.append(cdnget_scheme + "/" + cdnget_host + uri + ".temp");		
			Path movePath(storePath);
			movePath.append(uri);

			string lock_path = lockPath.getStrPath();
			string head_path = headPath.getStrPath();
			string temp_path = tempPath.getStrPath();
			string move_path = movePath.getStrPath();
			
			mkdir_p(lockPath.getDirname());
			mkdir_p(headPath.getDirname());
			mkdir_p(tempPath.getDirname());

			int lock_fd = open(lock_path.c_str(), O_RDWR | O_CREAT, 0644);
			if (lock_fd >= 0)
			{
				FILE *lock_file = fdopen(lock_fd, "r+b");
				setvbuf(lock_file, NULL, _IOFBF, vbuf_size+8);
				ftruncate(lock_fd, 0);
				if (locker.lock(lock_fd))
				{
					unlink(head_path.c_str());
					unlink(temp_path.c_str());
					int head_fd = open(head_path.c_str(), O_RDWR | O_CREAT, 0644);
					int temp_fd = open(temp_path.c_str(), O_RDWR | O_CREAT, 0644);
					if (head_fd >= 0 && temp_fd >= 0)
					{
						FILE *head_file = fdopen(head_fd, "r+b");
						setvbuf(head_file, NULL, _IOFBF, vbuf_size+8);
						FILE *temp_file = fdopen(temp_fd, "r+b");
						setvbuf(temp_file, NULL, _IOFBF, vbuf_size+8);
						app.log("INFO", "DOWNLOAD_START");
						if (worker->downloader.download(head_file, temp_file, &tout, url, useragent, cdnget_httphost))
						{
							app.log("INFO", "DOWNLOAD_OK", url);
							if (cdnget_store != "")
							{
								mkdir_p(movePath.getDirname());
								unlink(move_path.c_str());
								if (-1 == link(temp_path.c_str(), move_path.c_str()))
								{
									app.log("ERR", "FILE_LINK");
								}
							}
						} else
						{
							app.log("ERR", "DOWNLOAD_ERROR", url);
						}
						fclose(head_file);
						fclose(temp_file);
					} else
					{
						app.log("ERR", "CACHE_OPEN", url);
						close(head_fd);
						close(temp_fd);
					}
					locker.unlock(lock_fd);
				} else
				{
					app.log("INFO", "REDIRECT_START");
					int head_fd = open(head_path.c_str(), O_RDONLY);
					int temp_fd = open(temp_path.c_str(), O_RDONLY);
					if (head_fd >= 0 && temp_fd >= 0)
					{
						FILE *head_file = fdopen(head_fd, "rb");
						setvbuf(head_file, NULL, _IOFBF, vbuf_size+8);
						FILE *temp_file = fdopen(temp_fd, "rb");
						setvbuf(temp_file, NULL, _IOFBF, vbuf_size+8);
						if (Downloader::redirect(&tout, head_file) && Downloader::redirect(&tout, temp_file))
						{
							app.log("INFO", "REDIRECT_OK", url);
						} else
						{
							app.log("ERR", "REDIRECT_ERROR", url);
						}
						fclose(head_file);
						fclose(temp_file);
					} else
					{
						app.log("ERR", "CACHE_OPEN", url);
						close(head_fd);
						close(temp_fd);
					}
				}
				fclose(lock_file);
			} else
			{
				app.log("ERR", "LOCK_OPEN", url);
				close(lock_fd);
			}
		}
	} else
	{
		tout << "Status: 404" << "\r\n";
		tout << "X-CDN-Engine: " << appname << "\r\n";
		tout << "\r\n";
	}
	
	FCGX_Finish_r(request);

	pthread_mutex_lock(&workers_mutex);
	workers.erase(pthread_self());
	delete worker;
	ss.str("");
	ss << (time(NULL)-starttime) << "s";
	app.log("INFO", "WORKER_FINISH", ss.str());
	pthread_mutex_unlock(&workers_mutex);
	
	return NULL;
}


}
