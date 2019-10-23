CC := gcc
SRC_FORMAT := c
SRCS := $(wildcard *.$(SRC_FORMAT))
OBJS := $(patsubst %.$(SRC_FORMAT), %.o, $(notdir $(SRCS)) )
INC := -I.
CFLAGS := -g3 -O3

all : test.app

obj : $(OBJS)

test.app : $(OBJS)
	$(CC) $(INC) $(OBJS) -o test.app $(CFLAGS)

%.o : %.$(SRC_FORMAT)
	$(CC) $(INC) -c $< -o $@ $(CFLAGS)

.PHONY:clean

check_app:
	@if test ! -e "ls test.app"; \
    then \
        rm -rf test.app; \
    fi;

check_objs:
	@if test ! -e "ls *.o";\
    then \
        rm -rf *.o; \
    fi;

clean:check_app check_objs
	@echo "[clean app and objs, if they exsist]"
