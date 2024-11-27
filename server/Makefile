CROSS_COMPILE ?=
CC=$(CROSS_COMPILE)gcc
CFLAGS= -Wall -Wextra -g
TARGET= aesdsocket
SRC= aesdsocket.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)
