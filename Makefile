OBJS   = board.o element.o boardb.o board_helpfuncs.o list.o
SOBJS  = element.o boardb.o board_helpfuncs.o list.o board_server.o
POBJS  = element.o boardb.o board_helpfuncs.o list.o boardpost.o
SOURCE = board.c element.c boardb.c board_helpfuncs.c list.c board_server.c boardpost.c
HEADER = board_errors.h board_helpfuncs.h boardb.h element.h list.h
OUT    = board
SOUT   = board_server
POUT   = boardpost
CC     = gcc
FLAGS  = -g -c -Wall

ALL: $(OUT) $(SOUT) $(POUT)

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $@

$(SOUT): $(SOBJS)
	$(CC) -g $(SOBJS) -o $@

$(POUT): $(POBJS)
	$(CC) -g $(POBJS) -o $@

boardpost.o: boardpost.c
	$(CC) $(FLAGS) $?

board.o: board.c
	$(CC) $(FLAGS) $?

element.o: element.c
	$(CC) $(FLAGS) $?

boardb.o: boardb.c
	$(CC) $(FLAGS) $?

board_helpfuncs.o: board_helpfuncs.c
	$(CC) $(FLAGS) $?

list.o: list.c
	$(CC) $(FLAGS) $?

board_server.o: board_server.c
	$(CC) $(FLAGS) $?

clean:
	rm -f $(OBJS) $(OUT)
	rm -f $(SOBJS) $(SOUT)
	rm -f $(POBJS) $(POUT)

count:
	wc $(SOURCE) $(header)
