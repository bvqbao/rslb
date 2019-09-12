#!/bin/bash

HOST=$1
PORT=$2

python3 -m http.server --cgi --bind $HOST $PORT