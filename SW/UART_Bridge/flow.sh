#!/bin/bash

exit

./waf configure

./waf build run --app=02

# Select tab as field separator.
soffice log.tsv &
