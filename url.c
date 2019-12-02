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

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "url.h"

/**
 * parse a URL and store the information in info.
 * return 0 on success, or an integer on failure.
 *
 * Note: I implemented a version which does not modify the `url` variable,
 * and which does not make url_info member variables point to substrings in
 * url (to prevent use-after-free).
 *
 * Url_info fields need to be manually freed, this can be
 * done with free_url_info_fields(url_info*).
 */

int parse_url(char *url, url_info *info)
{
  // url format: [protocol://]<hostname>[:<port>]</path>
  // e.g. https://www.polytechnique.edu:80/index.php
  
  char *colon_slash_slash = NULL;
  char *hostname_ptr = NULL; // pointer in url
  char *protocol = NULL; // will be returned in output struct

  colon_slash_slash = strstr(url, "://");

  if(colon_slash_slash != NULL){ //protocol type is specified
    hostname_ptr = colon_slash_slash + 3; //jump to the host name
    ptrdiff_t protocol_name_length_ptrdiff = colon_slash_slash - url + 1;
    if (protocol_name_length_ptrdiff < 0) {
      fprintf(stderr, "Error: '://' at unexpected position\n");
      exit(1);
    }
    size_t protocol_name_length = (size_t)protocol_name_length_ptrdiff;
    protocol = malloc(sizeof(char[protocol_name_length]));
    snprintf(protocol, protocol_name_length, "%s", url);
  } else {  //no protocol type: using http by default.
    // l'url commence directement par le hostname
    hostname_ptr = url;
    protocol = malloc(sizeof("http"));
    strcpy(protocol, "http");
    // Note: we could also make protocol point to
    // the literal "http" in data segment, but it makes
    // the `free_url_info_fields` function more complex
  }

  /*
   * To be completed:
   *    Return an error (PARSE_URL_PROTOCOL_UNKNOWN) if the protocol is not 'http' using strcmp.
   */
  if (strcmp("http", protocol) != 0) {
    return PARSE_URL_PROTOCOL_UNKNOWN;
  }


  /*
   * To be completed:
   *    Look for the first '/' in host_name_path using strchr
   *    Return an error (PARSE_URL_NO_SLASH) if there is no such '/'
   *    Store the hostname and the queried path in the structure.
   *
   *    Note: The path is stored WITH the first '/'.
   */

  char *path_first_slash = strchr(hostname_ptr, '/');
  char *path = NULL;
  if (path_first_slash != NULL) {
    size_t path_length = strlen(path_first_slash) + 1lu;
    path = malloc(sizeof(char[path_length]));
    snprintf(path, path_length, "%s", path_first_slash);
  }
  else {
    return PARSE_URL_NO_SLASH;
  }

  /*
   * To be completed:
   *   Find first ':' after hostname
   *    If ':' is not found, the port is 80 (store it)
   *    If ':' is found, split the string and use strtol to parse the port.
   *    Return an error if the port is not a number, and store it otherwise.
   *
   * /!\: URL like "https://en.wikipedia.org/wiki/Template:Welcome" are used online
   * Note the colon in the path.
   */
  char *hostname_end_ptr = NULL; // pointer in url
  char *port = NULL; // pointer for output
  char *port_colon = strchr(hostname_ptr, ':');
  if ((port_colon != NULL) && (port_colon < path_first_slash)) {
    ptrdiff_t port_length_ptrdiff = path_first_slash - port_colon; // including terminating null-byte added by snprintf
    if (port_length_ptrdiff < 0) {
      fprintf(stderr, "Error: first slash at unexpected position\n");
      exit(1);
    }
    size_t port_length = (size_t)port_length_ptrdiff;
    port = malloc(sizeof(char[port_length]));
    snprintf(port, port_length, "%s", port_colon+1);

    hostname_end_ptr = port_colon - 1;
  }
  else {
    port = malloc(sizeof("80"));
    strcpy(port, "80");

    hostname_end_ptr = path_first_slash - 1;
  }


  if (parse_port(port, NULL)) {
    return PARSE_URL_INVALID_PORT;
  }

  ptrdiff_t hostname_length_ptrdiff = hostname_end_ptr - hostname_ptr + 2;
  if (hostname_length_ptrdiff < 0) {
    fprintf(stderr, "Error: unexpected hostname length.\n");
    exit(1);
  }
  size_t hostname_length = (size_t)hostname_length_ptrdiff;

  char *hostname = malloc(sizeof(char[hostname_length]));
  snprintf(hostname, hostname_length, "%s", hostname_ptr);


  info->protocol = protocol;
  info->host = hostname;
  info->port = port;
  info->path = path;

  // If everything went well, return 0.
  return 0;
}

int parse_port(char *s, unsigned short *port) {
  if (s[0] == '\0') {
    return 1;
  }
  char *end = NULL;
  errno = 0;
  long int port_int = strtol(s, &end, 10);
  if (errno) {
    return 1;
  }
  else if (*end != '\0') {
    return 1;
  }
  else if (port_int < 0 || port_int > USHRT_MAX) {
    return 1;
  }

  if (port != NULL) {
    *port = (unsigned short) port_int;
  }
  return 0;
}


void free_url_info_fields(url_info *info) {
  // Not an error: we really want to compare the adresses
  free(info->protocol);
  free(info->host);
  free(info->path);
}

/**
 * print the url info to std output
 */
void print_url_info(url_info *info){
  printf("The URL contains following information: \n");
  printf("Protocol:\t%s\n", info->protocol);
  printf("Host name:\t%s\n", info->host);
  printf("Port No.:\t%s\n", info->port);
  printf("Path:\t\t%s\n", info->path);
}
