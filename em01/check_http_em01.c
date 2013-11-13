/* $Id: check_http_em01.c,v 1.3 2005/11/02 21:31:42 gary Exp $
 *
 * check_snmp_cpu.c -- check CPU utilisation via SNMP
 *
 * Copyright (C) Gary Wall & Fredrik Löhr 2005
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <getopt.h>
#include <curl/curl.h>

#define PROGNAME "check_http_em01"
#define SUMMARY "Check temperature, humidity & ilumination via eSensors EM01."
#define OPTIONS "-U <url> [-p]\n\t--tempwarn <warning temp> --tempcrit <critical temp>\n\t--humwarn <warning humdity> --humcrit <critical humidity>\n\t--ilumwarn <warning ilumination> --ilumcrit <critical ilumination>\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option." 

#if (LIBCURL_VERSION_NUM < 0x070907)
#error "This program requires libcurl 7.9.7 or later."
#endif

/* Nagios standard return values */
#define OK        0
#define WARNING   1
#define CRITICAL  2
#define UNKNOWN   3
/* #define DEPENDENT 4 - unused */

float temp_warn = 28.0, temp_crit = 30.0;
float	ilum_warn = 35.0, ilum_crit = 40.0;
float hum_warn = 65.0, hum_crit = 70.0; 
int perfdata = 0, verbose = 0;
char *fqurl = NULL;

typedef struct data {
  int type;
  union {
    CURL *curl;
    FILE *file;
  } handle;

  /* This is the documented biggest possible buffer chunk we can get from
     libcurl in one single callback! */
  char buffer[CURL_MAX_WRITE_SIZE];

  char *readptr; /* read from here */
  int bytes;     /* bytes available from read pointer */

  CURLMcode m;   /* stored from a previous url_fread() */
} URL_FILE;

CURLM *multi_handle;

static size_t callback(char *buffer, size_t size, size_t nitems, void *userp)
{
	URL_FILE *url = (URL_FILE *) userp;
	size *= nitems;

	if (verbose)
		printf("callback %d size bytes\n", size);

	memcpy(url->readptr, buffer, size);
	url->bytes += size;
 
	return size;
}

void url_fclose(URL_FILE *file)
{
	curl_multi_remove_handle(multi_handle, file->handle.curl);
	curl_easy_cleanup(file->handle.curl);
}

size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file)
{
  fd_set fdread;
  fd_set fdwrite;
  fd_set fdexcep;
  int maxfd;
  struct timeval timeout;
  int rc;
  int still_running = 0;

  if (!file->bytes) { /* no data available at this point */

    file->readptr = file->buffer; /* reset read pointer */

    if (CURLM_CALL_MULTI_PERFORM == file->m) {
      while (CURLM_CALL_MULTI_PERFORM ==
            curl_multi_perform(multi_handle, &still_running)) {
        if (file->bytes) {
          if (verbose)
						printf("(fread) WOAH! THis happened!\n");

          break;        
        }
      }
      if (!still_running) {
				if (verbose)
  	      printf("DONE RUNNING AROUND!\n");

        return 0;
      }
    }

    do {

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);
  
      /* set a suitable timeout to fail on */
      timeout.tv_sec = 100; /* 1 minute */
      timeout.tv_usec = 0;

      /* get file descriptors from the transfers */
      curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

      rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

      switch (rc) {
      case -1:
        /* select error */
        break;
      case 0:
        break;
      default:
        /* timeout or readable/writable sockets */
				if (verbose)
        	printf("select() returned %d!\n", rc);

        do {
          file->m = curl_multi_perform(multi_handle, &still_running);
          
          if (file->bytes)
            /* we have received data, return that now */
            break;
          
        } while(CURLM_CALL_MULTI_PERFORM == file->m);

        
        if(!still_running && verbose)
          printf("DONE RUNNING AROUND!\n");
        
        break;
      }
    } while (still_running && (file->bytes <= 0));
  }
  else
		if (verbose)
    	printf("(fread) Skip network read\n");

  if (file->bytes) {
    /* data already available, return that */
    int want = size * nmemb;

    if (file->bytes < want)
      want = file->bytes;

    memcpy(ptr, file->readptr, want);
    file->readptr += want;
    file->bytes -= want;

		if (verbose)
	    printf("(fread) return %d bytes\n", want);

    return want;
  }

  return 0; /* no data available to return */
}

URL_FILE *url_fopen(char *url, char *operation)
{
  URL_FILE *file;
  int still_running;
  (void) operation;

  file = (URL_FILE *) malloc(sizeof(URL_FILE));

  if(!file)
    return NULL;

  memset(file, 0, sizeof(URL_FILE));

  file->type = 1; /* marked as URL, use 0 for plain file */
  file->handle.curl = curl_easy_init();

  curl_easy_setopt(file->handle.curl, CURLOPT_URL, url);
  curl_easy_setopt(file->handle.curl, CURLOPT_FILE, file);
  curl_easy_setopt(file->handle.curl, CURLOPT_VERBOSE, verbose);
  curl_easy_setopt(file->handle.curl, CURLOPT_WRITEFUNCTION, callback);

  if(!multi_handle)
    multi_handle = curl_multi_init();

  curl_multi_add_handle(multi_handle, file->handle.curl);

  while (CURLM_CALL_MULTI_PERFORM ==
        curl_multi_perform(multi_handle, &still_running));

  /* if still_running would be 0 now, we should return NULL */

  return file;
}

void print_help(void)
{
  printf("Program: " PROGNAME "\nSummary: " SUMMARY "\n\nOptions: " 
					OPTIONS "\n");
}

int parse_args(int argc, char **argv)
{
  int c, opt_index = 0;
  opterr = 0;

  static struct option long_options[] = {
    {"tempwarn", 1, 0, 0},
		{"tempcrit", 1, 0, 0},
		{"humwarn", 1, 0, 0},
		{"humcrit", 1, 0, 0},
		{"ilumwarn", 1, 0, 0},
		{"ilumcrit", 1, 0, 0},
  };

  while ((c = getopt_long(argc, argv, "hh:H:U:p0v", long_options,
          &opt_index)) != -1) {

    switch (c) {
      case 0:
        if (strcmp(long_options[opt_index].name, "tempwarn"))
          temp_warn = strtod(optarg, NULL);
        else if (strcmp(long_options[opt_index].name, "tempcrit"))
          temp_crit = strtod(optarg, NULL);
        else if (strcmp(long_options[opt_index].name, "humwarn"))
          hum_warn = strtod(optarg, NULL);
        else if (strcmp(long_options[opt_index].name, "humcrit"))
          hum_crit = strtod(optarg, NULL);
        else if (strcmp(long_options[opt_index].name, "ilumwarn"))
          ilum_warn = strtod(optarg, NULL);
        else if (strcmp(long_options[opt_index].name, "ilumcrit"))
          ilum_crit = strtod(optarg, NULL);

        break;
      case 'h':
        print_help();
        exit(UNKNOWN);
      case 'p':
        perfdata = 1;
        break;
      case 'U':
        fqurl = optarg;
        break;
      case 'v':
        verbose = 1;
        break;
      case '?':
        print_help();
        exit(UNKNOWN);
    }
  }

  if (fqurl != NULL)
    return 1;

  return 0;
}

int main(int argc, char *argv[])
{
	URL_FILE *handle;
	float temp, ilum, hum;
	int nread, ret = OK;
	char buffer[256], *tmpstr = NULL, *tmp1 = NULL, *tmp2 = NULL;

	if (!parse_args(argc, argv)) {
		print_help();
		exit(UNKNOWN);
	}
	
	if (!(handle = url_fopen(fqurl, "r"))) {
		printf("Couldn't url_fopen() %s\n", fqurl);
		exit(UNKNOWN);
	}

	if (!(nread = url_fread(buffer, sizeof(buffer), 1, handle))) {
		printf("No data returned from HTTP request\n");
		exit(UNKNOWN);
	}
	
	url_fclose(handle);
	free(handle);

	if (verbose)
		printf("Data: %s\n", buffer);

	/* Parse the output into temperature, humidity & ilumination */

	if (!(tmpstr = strdup(buffer))) {
		printf("Could not allocate memory to duplicate buffer!\n");
		exit(UNKNOWN);
	}	

	if (!(tmp1 = strstr(tmpstr, "E01"))) {
		printf("String 'E01' not found as expected in output!\n");
		exit(UNKNOWN);
	}

	if (!(tmp2 = strstr(tmpstr, "HU"))) {
		printf("String 'HU' not found as expected in output!\n");
		exit(UNKNOWN);
	} else
		tmp2 = '\0';

	temp = strtod(tmp1 + 12, NULL);

	if (verbose)
		printf("temperature: %2.1f\n", temp);

	if (!(tmp2 = strstr(tmpstr, "IL"))) {
		printf("String 'IL' not found as expected in output!\n");
		exit(UNKNOWN);
	} else
		tmp2 = '\0';

	hum = strtod(tmp1 + 19, NULL);

	if (verbose)
		printf("humidity: %2.1f\n", hum);

	if (!(tmp2 = strstr(tmpstr, "</body>"))) {
		printf("String '</body>' not found as expected in output!\n");
    exit(UNKNOWN);
  } else
		tmp2 = '\0';

	ilum = strtod(tmp1 + 28, NULL);
	free(tmpstr);

	if (verbose)
		printf("ilumination: %2.1f\n", ilum);

	if (temp >= temp_crit) {
		printf("Temperature: %2.1f (threshold %2.1f), ", temp, temp_crit);
		ret = CRITICAL;
	} else if (temp >= temp_warn) {
		printf("Temperature: %2.1f (threshold %2.1f), ", temp, temp_warn);
		if (ret < CRITICAL)
			ret = WARNING;
	} else {
		printf("Temperature: %2.1f, ", temp);
		if (ret < WARNING)
			ret = OK;
	}

	if (hum >= hum_crit) {
		printf("Humidity: %2.1f%% (threshold %2.1f%%), ", hum, hum_crit);
		ret = CRITICAL;
	} else if (hum >= hum_warn) {
		printf("Humidity: %2.1f%% (threshold %2.1f%%), ", hum, hum_warn);
		if (ret < CRITICAL)
      ret = WARNING;
	} else {
		printf("Humidity: %2.1f%%, ", hum);
		if (ret < WARNING)
      ret = OK;
	}

	if (ilum >= ilum_crit) {
		printf("Ilumination: %2.1f (threshold %2.1f)", ilum, ilum_crit);
		ret = CRITICAL;
	} else if (ilum >= ilum_warn) {
		printf("Ilumination: %2.1f (threshold %2.1f)", ilum, ilum_warn);
		if (ret < CRITICAL)
      ret = WARNING;
	} else {
		printf("Ilumination: %2.1f", ilum);
		if (ret < WARNING)
      ret = OK;
  }

	if (perfdata)
		printf(" | temperature=%2.1f humidity=%2.1f ilumination=%2.1f",
						temp, hum, ilum);

	printf("\n");

	return ret;	
}
