#!/bin/sh

socat TCP-LISTEN:$1,fork,reuseaddr EXEC:'python3 server.py'
