NAME=msicpfiforeader
INSTALLPATH=/home/wsadmin/wsmsicp/msicpfiforeader/
INST=/usr/bin/install
CC=gcc
LDFLAGS=-lmysqlclient -L/usr/lib64/mysql -lpthread -lm
#LDFLAGS=-lmysqlclient
CFLAG=-c -g
CFLAGS=-Wall  -o

all : $(NAME)
$(NAME): msicpfiforeader.o wsfifo.o  property.o dprint.o
	$(CC) $(CFLAGS) $@ $(LDFLAGS) msicpfiforeader.o wsfifo.o  property.o dprint.o

%.o: %.c
	$(CC) $(CFLAG)  $<

clean:
	rm -f *.o 2>/dev/null
	rm -f $(NAME) core*
install:all
install:
	$(INST) $(NAME) $(INSTALLPATH)

uninstall:
	rm -f $(INSTALLPATH)$(NAME) 2>/dev/null
