#!/bin/bash
if [[ $@ =~ --help ]] || (($# != 1)); then
	echo "Usage: $0 <interrupt|poll>"
	exit 0
fi

case $@ in
	interrupt|interrupts)
		backend=interrupt;;
	poll)
		backend=poll;;
	*)
		backend=poll;
		echo "Illegal option $@, defaulting to poll.";;
esac

sed -i.bak "s/^BACKEND=.*$/BACKEND=$backend/" Makefile
