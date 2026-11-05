# ECE-362-Final-Project
Final Project


GUDIE TO MAKEFILE: 

build, upload, monitor -> as stated
all -> all of above
clean/fullclean -> pio clean or manually deleting .vscode
call -> clean then all
test -> runs whatever is in test dir
rebuild -> clean then build

if no suffix after, it will run src/main.c

if there is a suffix that is the name of a directory (right now theres keypad, lcd, sound) it will run main.c inside the directory,
ignore src/main.c and src/*/main.c (except for the one in the directory called)

ie. if you ran 'make all lcd' it would run lcd/main.c and the other files not named main.c (in case you need functions from other file)