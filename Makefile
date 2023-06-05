CFLAGS=-Wall -Wextra -Werror --std=c17

all: isspace_locale

format:
	clang-format --Werror -i *.c

clean:
	$(RM) isspace_locale
