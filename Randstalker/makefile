# Specify compiler
CC=c++

# Specify linker
LINK=c++

all : randstalker

# Link the object files into a binary
randstalker : main
	$(LINK) -O2 -pipe -s -o randstalker main.o World.o GameAlterations.o -lstdc++

# Compile the source files into object files
main :
	$(CC) -O2 -pipe -s -c World.cpp -o World.o -std=c++17
	$(CC) -O2 -pipe -s -c GameAlterations.cpp -o GameAlterations.o -std=c++17
	$(CC) -O2 -pipe -s -c main.cpp -o main.o -std=c++17

# Clean target
clean :
	rm main.o World.o GameAlterations.o randstalker
