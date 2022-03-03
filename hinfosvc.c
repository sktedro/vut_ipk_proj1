/**
 * @brief Simple HTTP server
 * @author Patrik Skaloš (xskalo01)
 */


// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

// Header files
#include "hinfosvc.h"


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
char *constructResponse(char *status, char *responseBody){

  // Get additional response data
  int responseLen = strlen(responseBody);
  char responseLenStr[16];
  sprintf(responseLenStr, "%d", responseLen);

  // Allocate space for the response
  char *response = malloc(128 + responseLen);
  if(!response){
    fprintf(stderr, "Could not allocate memory for a response. Not serving\n");
    return NULL;
  }
  response[0] = '\0';

  // Concatenate all parts of the response to one string
  strcat(response, "HTTP/1.1 ");
  strcat(response, status);
  strcat(response, "\r\nContent-Length: ");
  strcat(response, responseLenStr);
  strcat(response, "\r\nConnection: close\r\n\r\n");
  strcat(response, responseBody);

  return response;
}


/**
 * @brief Fetch and parse host name
 *
 * @return allocated char array containing the host name
 */
char *getHostname(){
  char *hostname = malloc(1025);
  hostname[1024] = '\0';
  gethostname(hostname, 1024);

  if(hostname[1024] != '\0'){
    fprintf(stderr, "Host name to be returned is too long (longer than 1024 characters). Not serving\n");
    return NULL;
  }

  if(!hostname){
    fprintf(stderr, "Could not allocate memory for a response. Not serving\n");
    return NULL;
  }

  return hostname;
}


/**
 * @brief Fetch and parse cpu model
 *
 * @return allocated char array containing the cpu model name
 */
char *getCpuName(){

  // Open the /proc/cpuinfo file
  FILE *file = fopen("/proc/cpuinfo", "rb");
  if(!file){
    fprintf(stderr, "Could not open file /proc/cpuinfo. Not serving\n");
    return NULL;
  }

  // The line we're searching for starts with:
  char searchExpr[] = "model name";

  // Read the file line by line and stop at the line beginning with searchExpr
  char *line = NULL;
  ssize_t read;
  size_t len = 0;
  while ((read = getline(&line, &len, file)) != -1) {
    int found = 1;
    for(int i = 0; i < strlen(searchExpr) && i < (int)read; i++){
      if(line[i] != searchExpr[i]){
        found = 0;
        break;
      }
    }
    if(found == 1){
      len = (size_t)read;
      break;
    }
  }

  // We can close the file now
  fclose(file);

  // Couldn't find the cpu name
  if(len < 0){
    fprintf(stderr, "Could not retreive CPU information. Not serving\n");
    return NULL;
  }

  // Allocate memory for the response body
  char *responseBody = malloc(len);
  if(!responseBody){
    fprintf(stderr, "Could not allocate memory for a response. Not serving\n");
    return NULL;
  }

  // Parse the line and return only the CPU model
  for(int i = 0; i < len; i++){

    // Copy the rest of the line after the colon
    if(line[i] == ':'){

      // Shift by 2 to ignore the ':' and the next space
      i += 2;

      // Copy the characters to the response body
      for(int j = 0; line[i + j] != '\n' && line[i + j] != '\0'; j++){
        responseBody[j] = line[i + j];
        responseBody[j + 1] = '\0';
      }

      break;
    }
  }

  return responseBody;
}


/**
 * @brief Fetch and parse cpu load
 *
 * @return allocated char array containing cpu load in %
 */
char *getCpuLoad(){

  // Initialize the data to -1
  int data[2][10];
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < 10; j++){
      data[i][j] = -1;
    }
  }

  // Allocate some memory for a temporary string
  char *tmp = malloc(64);
  if(!tmp){
    fprintf(stderr, "Could not manually allocate a temporary string. Not serving\n");
    return NULL;
  }

  // Do the cycle two times since we need a difference of it in time
  for(int time = 0; time < 2; time++){

    // Read the /proc/stat file
    FILE *file = fopen("/proc/stat", "rb");
    if(!file){
      fprintf(stderr, "Could not open file /proc/cpuinfo. Not serving\n");
      return NULL;
    }

    // Read the first line containing the overall cpu info
    char *line = NULL;
    ssize_t read;
    size_t len = 0;
    read = getline(&line, &len, file);

    int column = 0;

    // Skip "cpu  "
    int i = 5;

    // Read the data
    while(column < 10 && i < (int)read){

      // Read a number and save it to the tmp array
      for(int j = 0; line[i] != ' ' && i < (int)read; i++, j++){
        tmp[j] = line[i];
        tmp[j + 1] = '\0';
      }

      // Convert the string to an integer and save it
      char *tolptr;
      int val = strtol(tmp, &tolptr, 10);
      if(tolptr[0] && tolptr[0] != ' ' && tolptr[0] != '\n'){
        fprintf(stderr, "File /proc/stat is corrupt. Not serving\n");
        return NULL;
      }
      data[time][column] = val;

      // Skip the space between values
      i++;

      column++;
    }

    // Close the stream
    fclose(file);

    // Wait for a second before getting the next data batch
    if(time == 0){
      sleep(1);
    }
  }

  // Check if all the numbers have been retrieved
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < 10; j++){
      if(data[i][j] == -1){
        fprintf(stderr, "File /proc/stat is corrupt. Not serving\n");
        return NULL;
      }
    }
  }

  // Calculate the load from retrieved data and print as a whole number
  int idle[2];
  int total[2] = {0, 0};
  for(int i = 0; i < 2; i++){
    idle[i] = data[i][3] + data[i][4];
    for(int j = 0; j < 10; j++){
      total[i] += data[i][j];
    }
  }
  int load = 100 - 100 * (idle[1] - idle[0]) / (total[1] - total[0]);
  snprintf(tmp, 64, "%d%%", load);
  
  return tmp;
}


/**
 * @brief Get response based on the request header
 *
 * @param headerLen (int)
 * @param header (char *)
 *
 * @return response to reply with in an allocated char array
 */
char *getResponse(int headerLen, char *header){

  // Check the beginning of the header
  char headerBeginning[] = "GET /";
  for(int i = 0; i < 5; i++){
    if(header[i] != headerBeginning[i]){
      fprintf(stderr, "The beginning of the received HTTP request is invalid\n");
      return NULL;
    }
  }

  // Get the target received in the header as a string
  int targetLen = 0;
  while(targetLen < headerLen && header[targetLen + 5] != ' '){
    targetLen++;
  }
  char target[targetLen];
  for(int i = 0; i < targetLen && header[i + 5] != ' '; i++){
    target[i] = header[i + 5];
    target[i + 1] = '\0';
  }

  // Construct a response based on the target
  char *responseBody;
  if(!strcmp(target, "hostname")){
    responseBody = getHostname();

  }else if(!strcmp(target, "cpu-name")){
    responseBody = getCpuName();

  }else if(!strcmp(target, "load")){
    responseBody = getCpuLoad();
    
  }else{
    fprintf(stderr, "Bad request received: %s\n", target);
    return constructResponse("400 Bad Request", "");
  }

  printf("Serving a request for '%s'\n", target);

  return constructResponse("200 OK", responseBody);
}


/*
 *
 * Main
 *
 */


int main(int argc, char **argv){

  // Check the arguments
  if(argc < 2){
    fprintf(stderr, "Please provide a port as the first argument\n");
    return 1;
  }
  char *tolptr = NULL;
  int port = strtol(argv[1], &tolptr, 10);
  if(tolptr[0] || port < 0 || port > 65535){
    fprintf(stderr, "Could not recognize port %s\n", argv[1]);
    return 1;
  }

  // Start the server
  printf("Starting HTTP server on port %d\n", port);

  // Initialize the server socket
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  // Reuse the address and the port
  setsockopt(
      serverSocket, 
      SOL_SOCKET, 
      SO_REUSEADDR | SO_REUSEPORT, 
      &(int){1}, 
      sizeof(int)
      );

  // Set up server address
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the socket to the address
  bind(
      serverSocket, 
      (struct sockaddr *) &serverAddr, 
      sizeof(serverAddr)
      );

  // Start listening
  int listening = listen(serverSocket, BACKLOG);
  if(listening != 0){
    fprintf(stderr, "An error occured when establishing connection to port %d. Exiting", port);
    return 1;
  }

  printf("Listening on port %d\n", port);

  // Requests accepting loop
  while(1){

    // Accept a request
    int client_fd = accept(serverSocket, NULL, NULL);
    if(client_fd == -1){
      fprintf(stderr, "An error occured while accepting a connection. Not serving\n");
      continue;
    }

    // Read the request
    char header[MAX_HEADER_LEN + 1];
    int headerLen = read(client_fd, header, MAX_HEADER_LEN + 1);
    if(headerLen >= MAX_HEADER_LEN){
      fprintf(stderr, "A HTTP request longer than the limit (%d) was received! Not serving\n", MAX_HEADER_LEN);
      close(client_fd);
      continue;
    }else if(headerLen == -1){
      fprintf(stderr, "An error occured when reading the HTTP request. Not serving\n");
      close(client_fd);
      continue;
    }

    // Construct a response for the client
    char *response = getResponse(headerLen, header);
    if(!response){
      close(client_fd);
      continue;
    }

    // Send the response and free the allocated memory
    send(client_fd, response, strlen(response), 0);
    free(response);

    // Close the descriptor
    close(client_fd);
  }

  return 0;
}
