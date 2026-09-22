#!/bin/sh

aclocal || exit 1
autoheader || exit 1
automake --foreign -a -c  || exit 1
autoconf || exit 1

