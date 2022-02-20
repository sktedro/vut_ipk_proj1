check_response(){
  TARGET=$1
  RESPONSE=$(curl "http://localhost:$PORT/$TARGET" 2>/dev/null)
  if [ -z "$RESPONSE" ]; then
    echo "Response of '""$TARGET""' empty!"
  else
    echo "Response of '""$TARGET""': $RESPONSE"
  fi
}

PORT=$1
if [ -z $PORT ]; then
  echo Please specify a port as the first argument
  exit 0
fi

./hinfosvc $PORT & 
PID="$!"
echo Server on port $PORT started, PID: "$PID"

check_response "hostname"
check_response "cpu-name"
check_response "load"

kill "$PID"
echo "Server killed (PID: $PID)"
