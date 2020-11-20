#!/bin/bash

time awk -F '|' 'BEGIN {x=0} {x=x+$5} END {print x}' lineitem.tbl
