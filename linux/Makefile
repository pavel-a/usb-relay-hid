
ARCH=x86

WORKING_FOLDER=./bin/$(ARCH)
INCLUDE_DIRS=usb
LIBRARIES_DIRS=usb/$(ARCH)
LIBRARIES_FLAGS=usb

CC=gcc

solution_c_src := $(wildcard $(addsuffix /*.c, sources ) )
solution_c_obj := $(addprefix $(WORKING_FOLDER)/, $(patsubst sources/%,%,$(solution_c_src:.c=.o) ) )

all: create_dir hidusb-relay-cmd
clean:
	rm -rf ./bin

create_dir:
	mkdir -p $(WORKING_FOLDER)

$(WORKING_FOLDER)/%.o : sources/%.c*
	$(CC) -O -Wall $(addprefix -Iinclude/, $(INCLUDE_DIRS)) -c $^ -o $@

hidusb-relay-cmd: $(solution_c_obj)
	$(CC) -o $(WORKING_FOLDER)/$@ $^ $(addprefix -Llib/, $(LIBRARIES_DIRS)) $(addprefix -l, $(LIBRARIES_FLAGS))
