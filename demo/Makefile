.DEFAULT_GOAL = $(TARGET) 

TARGET = demo
CC = gcc
CFLAGS = -I .
SRCS = main.c

vpath %.c ../src
vpath %.h ../src
SRCS += menu_kv.c
CFLAGS += -I ../src


OBJS := $(SRCS:%.c=%.o)

$(TARGET): $(OBJS)
		$(CC) $(OBJS) -o $@

clean:
		rm -rf $(TARGET)
		rm -rf *.o
		rm -rf .deps



$(BUILD_DIR): ; @mkdir -p $@

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE_C = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

%.o : %.c
%.o : %.c $(DEPDIR)/%.d | $(DEPDIR)
		$(COMPILE_C) $(OUTPUT_OPTION) $<

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.c=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
