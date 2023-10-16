# Compiler flags
WFLAGS  = -Wall -Wextra -pedantic -Wdouble-promotion -Wformat=2 -Winit-self \
          -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-local-typedefs \
          -Wunused -Wuninitialized -Wsuggest-attribute=pure \
          -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wfloat-equal \
          -Wundef -Wshadow \
          -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings \
          -Wconversion -Wlogical-op \
          -Wmissing-field-initializers \
          -Wmissing-format-attribute -Wpacked -Winline -Wredundant-decls \
          -Wvector-operation-performance -Wdisabled-optimization \
          -Wstack-protector

#CWFLAGS      = -Wdeclaration-after-statement -Wc++-compat -Wbad-function-cast \
#               -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wunsuffixed-float-constants
#-Wzero-as-null-pointer-constant -Wpadded

DEBUGFLAGS   = -g3 -O0
RELEASEFLAGS = -g0 -O3
CFLAGS       = -std=c17 -lm

# Includes
INCLUDE_FLAGS = -Iinclude `sdl2-config --cflags`

# Linker flags
LIBRARIES   = m pthread GL GLU
LIBRARY_FLAGS += $(foreach lib,$(LIBRARIES),-l$(lib)) `sdl2-config --libs`

SRCFILES := $(wildcard src/*.c)
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
DEPFILES := $(patsubst %.c,%.d,$(SRCFILES))

OBJFILES_DEBUG = $(patsubst %.o,debug/%.o,$(OBJFILES))
OBJFILES_RELEASE = $(patsubst %.o,release/%.o,$(OBJFILES))

DEPFILES_DEBUG = $(patsubst %.d,debug/%.d,$(DEPFILES))
DEPFILES_RELEASE = $(patsubst %.d,release/%.d,$(DEPFILES))

.DEFAULT: ogl_debug

.PHONY: all ogl_debug ogl_relase run clean

# Shorthands
all: ogl_debug ogl_release Makefile
	@echo "  [ finished ]"

# Executable linking
ogl_debug: $(OBJFILES_DEBUG) $(SRCFILES) Makefile
	@echo "  [ Linking $@ ]" && \
	gcc $(OBJFILES_DEBUG) -o $@ $(LIBRARY_FLAGS)

ogl_release: $(OBJFILES_RELEASE) $(SRCFILES) Makefile
	@echo "  [ Linking $@ ]" && \
	gcc $(OBJFILES_RELEASE) -o $@ $(LIBRARY_FLAGS)

# Source file compilation
debug/%.o: %.c Makefile
	@echo "  [ Compiling $< ]" && \
	mkdir debug/$(dir $<) -p && \
	gcc $(CFLAGS) $(INCLUDE_FLAGS) $(WFLAGS) $(CWFLAGS) $(DEBUGFLAGS) -MMD -MP -c $< -o $@

release/%.o: %.c Makefile
	@echo "  [ Compiling $< ]" && \
	mkdir release/$(dir $<) -p && \
	gcc $(CFLAGS) $(INCLUDE_FLAGS) $(WFLAGS) $(CWFLAGS) $(RELEASEFLAGS) -MMD -MP -c $< -o $@

clean:
	-@rm -f $(wildcard $(OBJFILES_DEBUG) $(OBJFILES_RELEASE) $(DEPFILES_DEBUG) $(DEPFILES_RELEASE) ogl_debug ogl_release) && \
	rm -rfv ogl_debug ogl_release && \
	rm -rfv `find ./ -name "*~"` && \
	echo "  [ clean main done ]"
