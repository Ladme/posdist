posdist: src/main.c
	gcc src/main.c -I$(groan) -L$(groan) -D_POSIX_C_SOURCE=200809L -o posdist -lgroan -lm -std=c99 -pedantic -Wall -Wextra -O3 -march=native

install: posdist
	cp posdist ${HOME}/.local/bin
