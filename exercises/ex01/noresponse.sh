#!/bin/bash

cat ../useful\ files/gmane.linux.kernel | grep -a '^From: ' | sed 's/.*<\([^@]*@[^>]*\)>/\1/' | sed 's/^From: //' | sort -u | sed 's!.*\[mailto:\([^]]*\)].*!\1!' | sed 's/\mailto.*//' | sed 's/^@//' | sed 's/(.*)//' | sed 's/^[^@]*$//' | grep -a -v '^$' | sort -u > senders

cat ../useful\ files/gmane.linux.kernel | grep -a -v '^From: ' | grep -a -o '<[^@]*@[^>]*>' | sed 's/.*<\([^@]*@[^>]*\)>[^<]*/\1/g' | grep -a -v '^[0-9].*' | grep -a -v '^$' | sed 's!.*mailto:.*!!' | sort -u > receivers

join -v 1 -1 1 -2 1 senders receivers > noresponsesenders

cat ../useful\ files/gmane.linux.kernel | grep -a '^References: ' | sed 's/.*<\([^>]*\)>/\1/' | sort -u > referenced
cat ../useful\ files/gmane.linux.kernel | grep -a '^Message-ID: ' | sed 's/.*<\([^>]*\)>/\1/' | sort -u > outgoing

join -v 1 -1 1 -2 1 outgoing referenced > noresponsemails
