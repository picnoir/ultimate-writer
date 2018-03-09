#!/bin/sh
klFile="keylogger.log"
touch ${klFile} && sudo logkeys --start --output ${klFile} --no-timestamps
