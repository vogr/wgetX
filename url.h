/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 *
 * Updated by Pierre Pfister
 *
 * Cisco Systems
 * ppfister@cisco.com
 *
 * Updated by Valentin Ogier
 * contact@vogier.fr
 *
 */

#ifndef URL_H
#define URL_H

/* information of an URL*/
struct url_info
{
	char* protocol; // protocol type: http, ftp, etc...
	char* host; // host name
	char* port; 	//port number
	char* path; //path without the first '/'
};

typedef struct url_info url_info;

// Declare the function
int parse_url(char* url, url_info *info);

// parse_url error codes
#define PARSE_URL_OK 0
#define PARSE_URL_NO_SLASH 1
#define PARSE_URL_INVALID_PORT 2
#define PARSE_URL_PROTOCOL_UNKNOWN 3

// parse_url associated error strings
static char const * const parse_url_errstr[] = { "no error" , "no trailing slash", "invalid port", "unknown protocol"};

int parse_port(char *s, unsigned short *port);

void free_url_info_fields(url_info *info);

void print_url_info(url_info *info);

#endif //URL_H
