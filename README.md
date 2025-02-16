# libssw

## Description:
ssw is a library containing simple wrappers for basic X11 functions (/stdlib functions) included from 'X11/Xutil.h' (and stdlib.h)

## Installation:

`(g)cc -c -fpic ssw.c -o ssw.o -lX11`
`(g)cc -c -shared ssw.o -o libssw.so -lX11`

then, as root:

`cp libssw.so /usr/lib/`
`cp ssw.h /usr/include/`
