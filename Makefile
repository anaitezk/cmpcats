# List of object files
OBJ = cmpcats.o compareDirectories.o list.o mergeDirectories.o

# Default target
all: cmpcats

# Rule to compile cmpcats.o
cmpcats.o: cmpcats.c compareDirectories.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to compile compareDirectories.o
compareDirectories.o: compareDirectories.c compareDirectories.h list.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to compile list.o
list.o: list.c list.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to link the object files and create the executable
cmpcats: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Rule to compile compareDirectories.o
mergeDirectories.o: mergeDirectories.c mergeDirectories.h list.h
	$(CC) $(CFLAGS) -c -o $@ $<


# Clean target
clean:
	rm -f cmpcats $(OBJ)