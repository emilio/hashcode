CFLAGS := -Wall -std=c99 -g
TARGET := practice
INPUTS := $(wildcard *.in)

all: $(TARGET)
	@echo > /dev/null

clean:
	rm -f $(TARGET)

# Since the code contains enough assertions,
# we rely on passing the examples running fine.
test: $(TARGET)
	./$< logo.in > /dev/null 2>&1
	./$< learn_and_teach.in > /dev/null 2>&1
	./$< right_angle.in > /dev/null 2>&1
