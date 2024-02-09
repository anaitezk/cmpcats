# List of object files
OBJ = cmpcats.o compareDirectories.o

# Default target
all: cmpcats

# Rule to compile cmpcats.o
cmpcats.o: cmpcats.c compareDirectories.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to compile compareDirectories.o
compareDirectories.o: compareDirectories.c compareDirectories.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to link the object files and create the executable
cmpcats: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Clean target
clean:
	rm -f cmpcats $(OBJ)