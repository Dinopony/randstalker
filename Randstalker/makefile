# Specify compiler
CC=gcc

# Specify linker
LINK=gcc

all : randstalker

# Link the object files into a binary
randstalker : main
	$(LINK) -O2 -s -o randstalker main.o World.o GameAlterations.o -lstdc++

# Compile the source files into object files
main :
	$(CC) -O2 -s -c World.cpp -o World.o -std=c++17
	$(CC) -O2 -s -c GameAlterations.cpp -o GameAlterations.o -std=c++17
	$(CC) -O2 -s -c main.cpp -o main.o -std=c++17

# Clean target
clean :
	rm main.o World.o GameAlterations.o randstalker
