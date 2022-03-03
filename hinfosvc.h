/*
 *
 * Constants
 *
 */

#define BACKLOG 256
#define MAX_HEADER_LEN 8096


/*
 * 
 * Functions
 *
 */


/**
 * @brief Construct a HTTP response given status (eg "200 OK") and a response
 * body
 *
 * @param status (char *, eg "400 Bad Request")
 * @param responseBody (char *)
 *
 * @return response as allocated char array
 */
char *constructResponse(char *status, char *responseBody);


/**
 * @brief Fetch and parse host name
 *
 * @return allocated char array containing the host name
 */
char *getHostname();


/**
 * @brief Fetch and parse cpu model
 *
 * @return allocated char array containing the cpu model name
 */
char *getCpuName();


/**
 * @brief Fetch and parse cpu load
 *
 * @return allocated char array containing cpu load in %
 */
char *getCpuLoad();


/**
 * @brief Get response based on the request header
 *
 * @param headerLen (int)
 * @param header (char *)
 *
 * @return response to reply with in an allocated char array
 */
char *getResponse(int headerLen, char *header);
