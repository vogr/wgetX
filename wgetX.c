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
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "url.h"
#include "wgetX.h"


int main(int argc, char *argv[]) {
  const char *file_name = "received_page";
  if (argc < 2) {
    fprintf(stderr,
        "wgetX: missing URL\n"
        "Usage: wgetX <URL> [output]\n\n"
        "wgetX will download the file available at <URI> over HTTP and save it "
        "at [output] (defaults to \"received_page\")\n"
      );
    return 1;
  }

  char *input_url = argv[1];

  // Get optional file name
  if (argc > 2) {
    file_name = argv[2];
  }

  // First parse the URL
  url_info info = {0};
  struct http_reply reply = {0};
  int ret = 0;

  char *url = input_url;
  do {

    // free previous allocations (except at first iteration when they have not been allocated yet).
    if (url != input_url) {
      free(reply.reply_buffer);
      free_url_info_fields(&info);
    }

    ret = parse_url(url, &info);
    if (ret) {
      fprintf(stderr, "Could not parse URL '%s': %s\n", url, parse_url_errstr[ret]);
      return 2;
    }

    // If needed for debug
    // print_url_info(&info);

    // Download the page

    ret = download_page(&info, &reply);
    if (ret) {
      return 3;
    }

    if (url != input_url) {
      free(url);
    }
    url = redirects_to(&reply);
    if (url != NULL) {
      fprintf(stderr, "Following redirect to %s\n", url);
    }

  } while (url != NULL);

  // If needed for debug
  // printf("%s\n", reply.reply_buffer);

  // Now parse the responses
  char *response = read_http_reply(&reply);
  if (response == NULL) {
    fprintf(stderr, "Could not parse http reply\n");
    return 4;
  }

  // Write response to a file
  write_data(file_name, response, (size_t)(reply.reply_buffer + reply.reply_length - response));

  // Free allocated memory
  free(reply.reply_buffer);

  // Just tell the user where the file is
  fprintf(stderr, "The file has been saved as %s.\n", file_name);
  return 0;
}

int download_page(url_info *info, http_reply *reply) {

/*
  * To be completed:
  *   You will first need to resolve the hostname into an IP address.
  *
   *   Option 1: Simplistic
  *     Use gethostbyname function.
  *
  *   Option 2: Challenge
  *     Use getaddrinfo and implement a function that works for both IPv4 and IPv6.
  *
  */

  int ret = 0;

  /* les passages suivants sont fortement inspirés de l'exemple "UDP Echo server/client"
   * dans la manpage `getaddrinfo(3)`:
   *      création de la struct addrinfo hints
   *      appel à getaddrinfo
   *      boucle for pour trouver
   */
  // On va créer une `struct addrinfo hints pour signaler à `getaddrinfo` de
  // nous donner des adresses sur lesquelles on pourra transmettre
  // avec le protocole TCP.
  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
    .ai_socktype = SOCK_STREAM, /* TCP socket */
  };

  struct addrinfo *addresses_linked_list = NULL;
  ret = getaddrinfo(
      info->host,
      info->port,
      &hints,
      &addresses_linked_list
  );
  if (ret != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
    exit(1);
  }


  struct addrinfo *address = NULL;
  int tcp_socket = 0;
  for (address = addresses_linked_list; address != NULL; address = address->ai_next) {
    tcp_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (tcp_socket == -1) {
      // could not create socket
      continue;
    }

    if (connect(tcp_socket, address->ai_addr, address->ai_addrlen) != -1) {
      // Connection successful!
      break;
    }
  }

  if (address == NULL) {
    fprintf(stderr, "Connection failed: could not connect using any of the addresses found.\n");
    exit(1);
  }

  // The list of addresses is no longer required
  freeaddrinfo(addresses_linked_list);

/*
  * To be completed:
  *   Next, you will need to send the HTTP request.
  *   Use the http_get_request function given to you below.
  *   It uses malloc to allocate memory, and snprintf to format the request as a string.
  *
  *   Use 'write' function to send the request into the socket.
  *
  *   Note: You do not need to send the end-of-string \0 character.
  *   Note2: It is good practice to test if the function returned an error or not.
  *   Note3: Call the shutdown function with SHUT_WR flag after sending the request
  *          to inform the server you have nothing left to send.
  *   Note4: Free the request buffer returned by http_get_request by calling the 'free' function.
  *
  */

  char* request = http_get_request(info);
  size_t request_length = strlen(request);
  if (write(tcp_socket, request, request_length) == -1) {
    fprintf(stderr, "Write failure: %s\n", strerror(errno));
    exit(1);
  }

  shutdown(tcp_socket, SHUT_WR);
  free(request);

/*
  * To be completed:
  *   Now you will need to read the response from the server.
  *   The response must be stored in a buffer allocated with malloc, and its address must be save in reply->reply_buffer.
  *   The length of the reply (not the length of the buffer), must be saved in reply->reply_buffer_length.
  *
  *   Important: calling recv only once might only give you a fragment of the response.
  *              in order to support large file transfers, you have to keep calling 'recv' until it returns 0.
  *
  *   Option 1: Simplistic
  *     Only call recv once and give up on receiving large files.
  *     BUT: Your program must still be able to store the beginning of the file and
  *          display an error message stating the response was truncated, if it was.
  *
  *   Option 2: Challenge
  *     Do it the proper way by calling recv multiple times.
  *     Whenever the allocated reply->reply_buffer is not large enough, use realloc to increase its size:
  *        reply->reply_buffer = realloc(reply->reply_buffer, new_size);
  *
  *
  */


  size_t buffer_length = 2048;
  size_t chunk_size = 2048;
  reply->reply_length = 0;
  reply->reply_buffer = malloc(sizeof(char[buffer_length]));

  ssize_t offset = 0;
  do {
    offset = recv(tcp_socket, reply->reply_buffer + reply->reply_length, chunk_size, 0);
    if (offset == -1) {
      fprintf(stderr, "Error in recv: %s\n", strerror(errno));
      exit(1);
    }
    reply->reply_length += (size_t)offset;
    if (reply->reply_length + chunk_size > buffer_length) {
      buffer_length += chunk_size;
      reply->reply_buffer = realloc(reply->reply_buffer, buffer_length);
    }
  } while (offset != 0);

  return 0;
}

void write_data(const char *path, const char *data, size_t len) {
  FILE *write_stream = fopen(path, "w");
  if (write_stream == NULL) {
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    exit(1);
  }
  if (fwrite(data, sizeof(*data), len, write_stream) < len) {
    fprintf(stderr, "Error writing data to %s\n", path);
    exit(1);
  }
  if (fclose(write_stream) != 0) {
    fprintf(stderr, "Error closing file: %s\n", strerror(errno));
    exit(1);
  }
}

char* http_get_request(url_info *info) {
  size_t buffer_length = 100 + strlen(info->path) + strlen(info->host);
  char *request_buffer = malloc(sizeof(char[buffer_length]));
  snprintf(request_buffer, buffer_length, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
    info->path, info->host);
  return request_buffer;
}

char *next_line(char *buff, size_t len) {
  // Warning: next_line might return a pointer to the adress
  // of the null terminator of the buffer (ie buff + len).
  if (len == 0) {
    return NULL;
  }

  char *last = buff + len - 1;
  while (buff != last) {
    if (*buff == '\r' && *(buff+1) == '\n') {
      return (buff + 2);
    }
    buff++;
  }
  return NULL;
}

int parse_status(char* status_line) {
  /*
   * Verify that the status line corresponds to a HTTP response
   * and return the status code. A canonical response looks like
   * HTTP/V.V SSS ......\r\n
   * where V.V is the http protocol version number and SSS is
   * the status code.
   * We will not try to parse the whole line, but only the first
   * fields.
   */

  int ret = 0;
  char buffer[14] = {0};
  snprintf(buffer, 14, "%s", status_line);
  int V1 = 0;
  int V2 = 0;
  int status = 0;
  ret = sscanf(buffer, "HTTP/%1d.%1d %3d ", &V1, &V2, &status);
  if (ret != 3) {
    fprintf(stderr, "Could not parse first line of http response (beginning with  '%s') (%d)\n", buffer, ret);
    exit(1);
  }
  return status;
}

char *read_http_reply(struct http_reply *reply) {

  int status = parse_status(reply->reply_buffer);
  if (status != 200) {
    fprintf(stderr, "Server returned status %d (should be 200)\n", status);
    return NULL;
  }


  /*
  * To be completed:
  *   The previous code only detects and parses the first line of the reply.
  *   But servers typically send additional header lines:
  *     Date: Mon, 05 Aug 2019 12:54:36 GMT<CR><LF>
  *     Content-type: text/css<CR><LF>
  *     Content-Length: 684<CR><LF>
  *     Last-Modified: Mon, 03 Jun 2019 22:46:31 GMT<CR><LF>
  *     <CR><LF>
  *
  *   Keep calling next_line until you read an empty line, and return only what remains (without the empty line).
  *
  *   Difficul challenge:
  *     If you feel like having a real challenge, go on and implement HTTP redirect support for your client.
  *
  */

  char *line = reply->reply_buffer;
  char *n_line = NULL;
  size_t remaining_length = reply->reply_length;
  do {
    // Note: we skip the status line.
    n_line = next_line(line, remaining_length);
    if (n_line == NULL) {
      // end of buffer or end of headers.
      fprintf(stderr, "Malformed reply: can't find blank line marking end of headers.\n");
      exit(1);
    }
    remaining_length -= (size_t)(n_line - line);
    line = n_line;
  } while (strncmp(line, "\r\n", 2) != 0);

  return (line + 2);


}

char *redirects_to(struct http_reply *reply) {
  /*
   * Returns if request is not a redirect.
   * Returns char* to the adress the reply redirects to
   * if the status code is in the range 3xx.
   * The char* points to an address malloc-ed on the heap.
   */
  int status = parse_status(reply->reply_buffer);
  if (status < 300 || status > 399) {
    return NULL;
  }

  // Let's find the address of the line containing
  // the Location header.
  char *line = reply->reply_buffer; // pointer to current line
  char *n_line = NULL; // pointer to next line (ie after next \r\n)
  size_t remaining_length = reply->reply_length;
  do {
    // Note: we skip the status line.
    n_line = next_line(line, remaining_length);
    if (n_line == NULL || strncmp(n_line, "\r\n", 2) == 0) {
      // end of buffer or end of headers.
      fprintf(stderr, "Error: status code asks for a redirect (status %d), but Location header can't be found.\n", status);
      exit(1);
    }
    remaining_length -= (size_t)(n_line - line);
    line = n_line;
  } while (strncmp(line, "Location: ", 10) != 0);
  // don't compare null terminator

  n_line = next_line(line, remaining_length);
  // Template:
  //    Location: http://aaaaaaaaaa.com/a.html<CR><LF>

  char *url_begins = line + 10;
  char *url_ends = n_line - 3;
  // url_length = actual length + 1 (for null terminator)
  ptrdiff_t url_length_ptrdiff = url_ends - url_begins + 2;
  if (url_length_ptrdiff <= 1) {
    fprintf(stderr, "Error: malformed Location header.\n");
    exit(1);
  }
  size_t url_length = (size_t)url_length_ptrdiff;
  char *redirect_location = malloc(sizeof(char[url_length]));
  snprintf(redirect_location, url_length, "%s", url_begins);

  return redirect_location;
}
