
OUTPUT_DIR = build
FNAME = demo_mp
RELEASE_OUTPUT_FNAME = $(OUTPUT_DIR)/$(FNAME)
DEBUG_OUTPUT_FNAME = $(RELEASE_OUTPUT_FNAME)_debug

SRC = main.c memory.c

FLAGS = -pipe -Wall -fopenmp
FLAGS_DEBUG = -g
FLAGS_RELEASE = -O2

INCLUDE =
LIB = -lrt

debug: clean

	mkdir -p $(OUTPUT_DIR)/
	gcc $(FLAGS) $(FLAGS_DEBUG) $(LIB) $(INCLUDE) $(SRC) -o $(DEBUG_OUTPUT_FNAME)

release:

	mkdir -p $(OUTPUT_DIR)/
	gcc $(FLAGS) $(FLAGS_RELEASE) $(LIB) $(INCLUDE) $(SRC) -o $(RELEASE_OUTPUT_FNAME)

clean:

	rm -f $(OUTPUT_DIR)/*

distclean:

	rm -Rf $(OUTPUT_DIR)

