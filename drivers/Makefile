include $(BASEDIR)/CONFIGURE

SRCS_ASM = $(wildcard *.S)
SRCS     = led.c # for testing
ifdef MODULE_UART
	SRCS += uart.c
endif
ifdef MODULE_TIMER
	SRCS += timer.c
endif
ifdef MODULE_GPIO
	SRCS += gpio.c
endif
ifdef MODULE_IR
	SRCS += ir.c
endif
ifdef MODULE_CLCD
	SRCS += clcd.c
endif
OBJS = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)
DEPS = $(OBJS:%.o=%.d)

INC     += -I./include
CFLAGS  += $(INC)

.SUFFIXES: .c.o
.SUFFIXES: .s.o
.SUFFIXES: .S.o

all: $(OBJS) $(DEPS)

.c.o:
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $(INC) $(LIBS) -c $< -o $@

depend dep:
	echo $(CC)
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

clean:
	@rm -f $(OBJS) $(DEPS) .depend

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
