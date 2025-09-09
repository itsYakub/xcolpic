CC		:= gcc
CFLAGS	:= -Wall -Wextra -Werror -ggdb3
LDFLAGS	:=
LFLAGS	:= -lX11
SRCS	:= ./xselect.c ./xcolpic.c ./main.c
OBJS	:= $(SRCS:.c=.o)
TARGET	:= xcolpic

# ================================

.PHONY : all

all : $(TARGET)

.PHONY : install

install : all
	cp $(TARGET) /usr/local/bin

.PHONY : clean

clean :
	rm -f $(TARGET)
	rm -f $(OBJS)

.PHONY : uninstall

uninstall : clean
	rm -f /usr/local/bin/$(TARGET)

# ================================

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
