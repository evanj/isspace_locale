CFLAGS=-Wall -Wextra -Werror --std=c17

all: locale_isspace

format:
	clang-format --Werror -i *.c

clean:
	$(RM) locale_isspace
