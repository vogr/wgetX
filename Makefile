MAKEFLAGS += --warn-undefined-variables --no-builtin-rules

NAME := wgetX
SRC_FILES := wgetX.c url.c

ARCHIVE := wgetX_OGIER.zip
ARCHIVE_SRC := \
	wgetX.c wgetX.h \
	url.c url.h \
	Makefile README.md

# Choose between BUILD
BUILD := release

ifeq ($(BUILD),debug)
SUFFIX := .debug
else ifeq ($(BUILD),release)
SUFFIX :=
else
$(error Unknown BUILD type $(BUILD). Accepted values are BUILD=debug and BUILD=release)
endif

ifndef ($(ARGS))
ARGS :=
endif

BIN := $(NAME)$(SUFFIX)
OBJDIR := objects$(SUFFIX)
OBJECTS := $(SRC_FILES:%.c=$(OBJDIR)/%.o)
# Auto dependency generation for gcc (see http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/)
DEPDIR := $(OBJDIR)/deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d


# see https://src.fedoraproject.org/rpms/redhat-rpm-config/blob/master/f/buildflags.md
# and https://nullprogram.com/blog/2019/11/15/ (for -z noexecstack)
CFLAGS.debug := -ggdb3 -O0 # -Og # Og currently optimizes out too many variables (see https://gcc.gnu.org/bugzilla//show_bug.cgi?id=78685)
CFLAGS.release := -O2 -flto
CFLAGS ?= -std=gnu11 \
	-Wall -Wextra -Wpedantic \
	-Wformat=2 \
	-Wswitch-default -Wswitch-enum \
	-Wfloat-equal \
	-Wconversion \
	-pipe \
	-D_FORTIFY_SOURCE=2 \
	-fexceptions \
	-fasynchronous-unwind-tables \
	-fstack-protector-strong \
	-fstack-clash-protection \
	-fcf-protection \
	-Wl,-z,noexecstack \
	$(CFLAGS.$(BUILD))
# Note : `-fstack-protector-strong`, `-fstack-clash-protection`, `-fcf-protection` add runtime
# checks and may therefore have a performance impact.


.PHONY: all
all: $(BIN)

.PHONY: run
run: $(BIN)
	./$(BIN) $(ARGS)

.PHONY: redo
redo: clean
	$(MAKE) all

.PHONY: rerun
rerun: redo
	$(MAKE) run ARGS=$(ARGS)

.PHONY: record
record: $(BIN)
	rr record ./$(BIN) $(ARGS)


$(BIN): $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJECTS): $(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR) $(DEPDIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS)

.PHONY: archive
archive: $(ARCHIVE)
$(ARCHIVE): $(ARCHIVE_SRC)
	zip $@ $^

.PHONY: clean
clean:
	rm -f $(NAME) $(NAME).debug $(ARCHIVE)
	if [ -d objects ]; then rm -r objects; fi
	if [ -d objects.debug ]; then rm -r objects.debug; fi

DEPFILES := $(SRC_FILES:%.c=$(DEPDIR)/%.d)
include $(wildcard $(DEPFILES))
