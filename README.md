cdnget
==========

cdnget is a FastCGI based flexible pull-mode Content Delivery Network reverse proxy software compatible with Nginx!

Features
----------

- cdnget is FastCGI application and compatible with all of FastCGI supported web servers: Nginx, Apache and etc. Also cdnget can run multi-instance.
- cdnget handles too many requests at same time and needs low CPU usage and small memory per request.
- cdnget handles unlimited file size to can be applied very huge content (video files, binary files and etc). cdnget tested on very huge news and video sites. It works!

USAGE
==========

Determine web servers working user:group. In generally on Debian, default is www-data:www-data and uid:gid is 33:33.

cdnget store and cache paths must be exist and have correct access rights.

	spawn-fcgi -s /var/run/cdnget.sock -u 33 -g 33 -- /usr/local/bin/cdnget
	spawn-fcgi -a 127.0.0.1 -p 9999 -u 33 -g 33 -- /usr/local/bin/cdnget
	spawn-fcgi -a 0.0.0.0 -p 9999 -u 80 -g 80 -n -- /usr/local/bin/cdnget

Configuration
----------

### FastCGI Parameters

- **CDNGET_HOST**   Content source host. CDN origin. Default "localhost".
- **CDNGET_URI**    Content URI. Variable by request. Default "/".
- **CDNGET_INDEX**  Index file name for directories. Default "index.html"
- **CDNGET_STORE**  Content store path. Should be document root. If path is "", content doesn't be stored and always pull. Default "".
- **CDNGET_CACHE**  Content cache path. Must be in same partition with store path. Default "/var/cache/cdnget".

### Examples

Run cdnget;

	spawn-fcgi -s /var/run/cdnget.sock -u 33 -g 33 -n -- /usr/local/bin/cdnget

**Note:** cdnget store (/var/www) and cache (/var/cache/cdnget) paths must be in same partition and have right access rights.

#### Nginx

```
server {
    listen       80;
    server_name  video.cdn.example.com;
    root /var/www;
    index index.html;

    expires 1d;
    gzip on;

    location / {
        try_files $uri @fallback;
        error_page 403 = @fallback;
        expires 7d;
        add_header X-Cache HIT;
    }

    set $cdn_origin video.example.com;

    location @fallback {
        internal;

        if ($cdn_origin_host = "") {
            set $cdn_origin_host $cdn_origin;
        }

        expires -1;
        gzip off;

        fastcgi_pass unix:/var/run/cdnget.sock;
        fastcgi_pass_request_headers off;
        fastcgi_pass_request_body off;

        fastcgi_index index.html;
        fastcgi_intercept_errors on;

        fastcgi_hide_header Server;
        fastcgi_hide_header Pragma;
        fastcgi_hide_header Expires;
        fastcgi_hide_header Cache-Control;
        fastcgi_hide_header Set-Cookie;
        fastcgi_hide_header X-Powered-By;

        include fastcgi_params;

        fastcgi_param  CDNGET_SCHEME    http;
        fastcgi_param  CDNGET_HOST      $cdn_origin;
        fastcgi_param  CDNGET_URI       $document_uri;
        fastcgi_param  CDNGET_STORE     $document_root;
        fastcgi_param  CDNGET_HTTPHOST  $cdn_origin_host;

        add_header X-Cache MISS;
    }

    location @forward {
        internal;

        if ($cdn_origin_host = "") {
            set $cdn_origin_host $cdn_origin;
        }

        proxy_pass http://$cdn_origin;
        proxy_set_header Host $cdn_origin_host;

        proxy_redirect off;

        add_header X-Cache FORWARD;
    }
}
```

INSTALLATION
==========

	autoreconf --install --force
	./configure
	make
	make install

Prerequisities
----------

- FastCGI development library
- cURL development library

Installing prerequisities on Debian;

	apt-get install libfcgi-dev libcurl3-dev

DEPENDENCIES
==========

- FastCGI library
- cURL library
- spawn-fcgi *(optional; you should need to spawn application with FastCGI)*

Installing dependencies on Debian;

	apt-get install libfcgi0ldbl libcurl3 spawn-fcgi

REPOSITORY
==========

**GitHub** [https://github.com/orkunkaraduman/cdnget](https://github.com/orkunkaraduman/cdnget)

AUTHOR
==========

Orkun Karaduman &lt;orkunkaraduman@gmail.com&gt;

COPYRIGHT AND LICENSE
==========

Copyright (C) 2014  Orkun Karaduman &lt;orkunkaraduman@gmail.com&gt;

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see &lt;http://www.gnu.org/licenses/&gt;.
