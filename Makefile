ENV ?= proton

KNOWN_TARGETS = build upload monitor clean fullclean rebuild all call
EXTRA := $(strip $(filter-out $(KNOWN_TARGETS), $(MAKECMDGOALS)))
FILE := $(if $(EXTRA),$(EXTRA).c,main.c)

build:
	PLATFORMIO_BUILD_SRC_FILTER="-<*.c> +<$(FILE)>" pio run -e $(ENV)	
# 	pio run -e $(ENV)

upload:
	PLATFORMIO_BUILD_SRC_FILTER="-<*.c> +<$(FILE)>" pio run -e $(ENV) -t upload
# 	pio run --target upload -e $(ENV)

monitor:
# 	pio device monitor -e $(ENV)
	pio device monitor -b 115200

debug:
	pio debug

clean:
	pio run -t clean

fullclean:
	rm -rf .pio
	rm -f src/CMakeLists.txt CMakeLists.txt
	pio run --target fullclean --environment proton

test: clean
	pio test -vvv

rebuild: clean build

all: build upload monitor

call: fullclean build upload monitor

%:
	@:
