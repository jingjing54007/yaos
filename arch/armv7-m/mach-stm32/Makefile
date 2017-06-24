SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)
DEPS     = $(OBJS:%.o=%.d)

INC     += -I./include
CFLAGS  += $(INC)

.SUFFIXES: .c.o
.SUFFIXES: .s.o
.SUFFIXES: .S.o

all: board $(OBJS) $(DEPS)
ifdef BOARD
	$(LD) -o $(BOARD).o -r boards/$(BOARD)/*.o
endif

.c.o:
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $(INC) $(LIBS) -c $< -o $@

board:
ifdef BOARD
	@$(MAKE) --print-directory -C boards/$(BOARD)
endif

depend dep:
	echo $(CC)
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

clean:
	@rm -f $(OBJS) $(DEPS) .depend
ifdef BOARD
	@rm -f $(BOARD).o
	@$(MAKE) clean -C boards/$(BOARD)
endif

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
