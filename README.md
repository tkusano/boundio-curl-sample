boundio-curl-sample
===================

Sample program to use http://boundio.jp/ with cURL

How To Compile
--------------

1. Install [cURL](http://curl.haxx.se/) development library
2. Install [JSON-C](http://oss.metaparadigm.com/json-c/) development library
3. Rewrite bound-curl-sample.c with your Request URL and API Access Key
4. type "make"

How To Compile on Debian / Ubuntu
---------------------------------

1. % sudo apt-get install libcurl4-gnutls-dev
2. % sudo apt-get install libjson0-dev
3. editor bound-curl-sample.c
4. make

How To Run
----------

First argument is telephone no.
Second argument is [cast](http://bit.ly/JiDJIJ)

% ./boundio-curl-sample 03-XXXX-XXXX 'file(000001)%%file(000002)'
