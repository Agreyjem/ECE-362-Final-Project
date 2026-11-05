ENV ?= proton

KNOWN_TARGETS = build upload monitor clean fullclean rebuild all call debug
EXTRA := $(strip $(filter-out $(KNOWN_TARGETS), $(MAKECMDGOALS)))

# All subdir entry points, relative to src/: e.g. "drivers/main.c sensor/main.c"
ALL_SUBDIR_ENTRIES := $(patsubst src/%,%,$(wildcard src/*/main.c))

ifeq ($(EXTRA),)
  # make build -> keep src/main.c, drop every subdir entry
  EXCLUDE_ENTRIES := $(ALL_SUBDIR_ENTRIES)
else
  # make build drivers -> keep drivers/main.c, drop src/main.c and other subdir entries
  ifeq ($(wildcard src/$(EXTRA)/main.c),)
    $(error No entry point at src/$(EXTRA)/main.c)
  endif
  EXCLUDE_ENTRIES := $(filter-out $(EXTRA)/main.c,$(ALL_SUBDIR_ENTRIES)) main.c
endif

SRC_FILTER_FLAGS := +<*> $(foreach e,$(EXCLUDE_ENTRIES),-<$(e)>)
BUILD_FILTER = PLATFORMIO_BUILD_SRC_FILTER="$(SRC_FILTER_FLAGS)"

build:
	$(BUILD_FILTER) pio run -e $(ENV)

upload:
	$(BUILD_FILTER) pio run -e $(ENV) -t upload

monitor:
	pio device monitor -b 115200

debug:
	pio debug

clean:
	pio run -t clean

fullclean:
	rm -rf .pio
	rm -f src/CMakeLists.txt CMakeLists.txt
	pio run --target fullclean --environment $(ENV)

test: clean
	pio test -vvv

rebuild: clean build

all: build upload monitor

call: fullclean build upload monitor

%:
	@: