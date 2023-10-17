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
          -Wstack-protector \
          -Wno-suggest-attribute=format

#CWFLAGS      = -Wdeclaration-after-statement -Wc++-compat -Wbad-function-cast \
#               -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wunsuffixed-float-constants
#-Wzero-as-null-pointer-constant -Wpadded

DEBUGFLAGS   = -g3 -O0
RELEASEFLAGS = -g0 -O3
CFLAGS       = -std=c2x -lm

# Includes
INCLUDE_FLAGS = -Iinclude `sdl2-config --cflags`

# Linker flags
LIBRARIES   = m pthread GL GLU
LIBRARY_FLAGS += $(foreach lib,$(LIBRARIES),-l$(lib)) `sdl2-config --libs`

SRCFILES := $(wildcard src/*.c)
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
DEPFILES := $(patsubst %.c,%.d,$(SRCFILES))

OBJFILES_DEBUG = $(patsubst %.o,build/debug/%.o,$(OBJFILES))
OBJFILES_RELEASE = $(patsubst %.o,build/release/%.o,$(OBJFILES))

DEPFILES_DEBUG = $(patsubst %.d,build/debug/%.d,$(DEPFILES))
DEPFILES_RELEASE = $(patsubst %.d,build/release/%.d,$(DEPFILES))

.DEFAULT: debug

.PHONY: run_debug run_release debug release run clean

run_debug: debug
	build/debug/debug

run_release: release
	build/debug/debug

debug: $(OBJFILES_DEBUG) $(SRCFILES) Makefile
	@echo "  [ Linking $@ ]" && \
	gcc $(OBJFILES_DEBUG) -o build/$@/$@ $(LIBRARY_FLAGS)

release: $(OBJFILES_RELEASE) $(SRCFILES) Makefile
	@echo "  [ Linking $@ ]" && \
	gcc $(OBJFILES_RELEASE) -o build/$@/$@ $(LIBRARY_FLAGS)

build/debug/%.o: %.c Makefile
	@echo "  [ Compiling $< ]" && \
	mkdir build/debug/$(dir $<) -p && \
	gcc $(CFLAGS) $(INCLUDE_FLAGS) $(WFLAGS) $(CWFLAGS) $(DEBUGFLAGS) -MMD -MP -c $< -o $@

build/release/%.o: %.c Makefile
	@echo "  [ Compiling $< ]" && \
	mkdir build/release/$(dir $<) -p && \
	gcc $(CFLAGS) $(INCLUDE_FLAGS) $(WFLAGS) $(CWFLAGS) $(RELEASEFLAGS) -MMD -MP -c $< -o $@

clean:
	-@rm -rfv build

compile_commands.json: clean
	bear -- make debug
