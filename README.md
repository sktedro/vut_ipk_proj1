# Brief

This project is a simple HTTP server capable of serving these GET requests:
- `/hostname`: returns the host name (eg. `dell`)
- `/cpu-name`: returns the model of the CPU (eg. `Intel(R) Core(TM) i5-8365U
  CPU @ 1.60GHz`)
- `/load`: returns CPU load calculated over 1 second period as a whole number
  (eg. `42%`)

The response body is not ending with a new line and the response to any other 
request is `400 Bad Request`.

#### Author

Patrik Skaloš

# Usage

## Compiling

Simply execute `make`

## Starting the server

#### Using MAKE

Simply execute `make run`

To change the port, please edit the Makefile. The default port is `12321`.

#### The standard way

Execute `./hinfosvc PORT` while PORT is a whole number in range `<0, 65535>`

## Example usage

#### In a browser

Access `http://localhost:PORT/TARGET` while the supported targets are 
`hostname`, `cpu-name` and `load`.

#### Using curl

Execute `curl http://localhost:PORT/TARGET` while the supported targets are 
`hostname`, `cpu-name` and `load`.

Since the responses lack a trailing new line, to have the response printed 
nicely, I recommend executing `curl http://localhost:PORT/TARGET; echo`
instead.

## Testing

Execute `make test`, which starts the server, sends all supported requests,
prints the responses and finally, kills the server.
