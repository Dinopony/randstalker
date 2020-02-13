# Arch Target
ARCH := linux

ifeq ($(ARCH),win64)
CC :=x86_64-w64-mingw32-c++
STATIC :=-static
EXT :=.exe
else
CC :=c++
STATIC :=
EXT :=
endif

# Link the object files into a binary
all : main
	$(CC) $(STATIC) -O2 -pipe -s -o randstalker$(EXT) main.o WorldRandomizer.o World.o GameAlterations.o RandomizerOptions.o -lstdc++

# Compile the source files into object files
main :
	$(CC) -O2 -pipe -c RandomizerOptions.cpp -o RandomizerOptions.o -std=c++17
	$(CC) -O2 -pipe -c World.cpp -o World.o -std=c++17
	$(CC) -O2 -pipe -c WorldRandomizer.cpp -o WorldRandomizer.o -std=c++17
	$(CC) -O2 -pipe -c GameAlterations.cpp -o GameAlterations.o -std=c++17
	$(CC) -O2 -pipe -c main.cpp -o main.o -std=c++17

# Clean target
clean :
	rm main.o World.o WorldRandomizer.o GameAlterations.o RandomizerOptions.o randstalker randstalker.exe
