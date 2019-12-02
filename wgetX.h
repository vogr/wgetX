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

#ifndef WGETX_H_
#define WGETX_H_

/* Structure used to store the buffer and buffer length when receiving the reply from an http server. */
typedef struct http_reply {

  /* The address of the buffer containing everything received from the server.
    * The memory is allocated using malloc, and its size can be increased with realloc. */
  char *reply_buffer;

  /* The total number of bytes in the reply */
  size_t reply_length;
} http_reply;

/**
 * \brief write the data to a file
 * \param path the path and name of the file
 * \param data the pointer of the buffer that to be written.
 * \param len the number of bytes to be written from the data buffer onto the file.
 */
void write_data(const char *path, const char *data, size_t len);

/**
 * \brief download a page using the http protocol
 * \param info the url information
 * \param reply the output of the request
 * \return 0 on success, an error code on failure
 */
int download_page(url_info *info, http_reply *reply);

/**
 * \brief return a string with a get http request
 * \param info the url information
 * \return the pointer to the get http request string. The pointer must be freed using 'free'.
 */
char* http_get_request(url_info *info);

/**
 * \brief process the http reply from server
 * \param reply the reply structure
 * \return a pointer to the first data byte
 */
char *read_http_reply(struct http_reply *reply);

/*
 * \brief if reply corresponds to a redirect, return a pointer
 * to the redirection url
 */
char *redirects_to(struct http_reply *reply);

#endif /* WGETX_H_ */
