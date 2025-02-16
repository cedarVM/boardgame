# The boardgame of all time

## Description:
Truly a boardgame

## Installation (approximately):

`(g)cc -c -fpic ssw.c -o ssw.o -lX11`
`(g)cc -shared ssw.o -o libssw.so -lX11`
`(g)cc -I./ -L./ help.c -o help -lssw`

## Notes:
Need to check for collision.
Timeline is finishing this by Sunday
