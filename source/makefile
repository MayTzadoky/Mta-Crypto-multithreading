CC = gcc
LIBS = -lmta_rand -lmta_crypt -lcrypto -lpthread 
dataStructure = Queue.c
RM = rm -f

SRCS := $(subst ./,,$(shell find . -name "*.c"))
SRCS := $(filter-out %Queue.c, $(SRCS))

OBJS := $(patsubst %.c,%.out,$(SRCS))

all: $(OBJS)


%.out: %.c
	@$(CC) $^ $(dataStructure) $(LIBS) -L$(PWD) -o $@

clean: 
	@-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)
