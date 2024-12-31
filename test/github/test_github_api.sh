#!/usr/bin/env bash
CWD="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)" && cd "$CWD"
set -ex

export SERVER_URL="http://localhost:8000"

function finish {
  exit_code=$?
  SERVER_PID=$(ps aux | grep -v grep | grep "uvicorn" | xargs | cut -d' ' -f2)
  echo "killing server pid $SERVER_PID"
  [[ -n "${SERVER_PID}" ]] && ((SERVER_PID > 0)) && kill -15 $SERVER_PID
  exit $exit_code
}
trap finish EXIT INT

(
  export PYTHONPATH=$PWD

  # start server
  uvicorn mock_api:app --workers 4 &
  sleep 2

  # wait for server
  curl --retry-connrefused \
       --connect-timeout 10 \
       --retry 10 \
       --retry-delay 2 \
       "$SERVER_URL/ping"
)

(
  cd ../..
  mkn run -p github
)
