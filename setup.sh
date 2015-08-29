#!/bin/bash
if [[ $@ =~ --help ]] || (($# != 1)); then
	echo "Usage: $0 <enable-interrupts|disable-interrupts>"
	exit 0
fi

if [ "$1" = "enable-interrupts" ]; then
	backend=interrupt
else
	backend=poll
fi

sed -i.bak "s/^BACKEND=.*$/BACKEND=$backend/" Makefile
