cc = clang
flags = -pedantic-errors -ansi -Wall -Wextra -Werror -Ofast -march=native
ifeq ($(OS), Windows_NT)
	linker_flags = -lmingw32
endif
linker_flags += -lSDL2main -lSDL2

all: tt.c
	$(cc) $(flags) tt.c -o tt $(linker_flags)
