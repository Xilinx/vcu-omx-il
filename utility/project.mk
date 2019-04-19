THIS.utility:=$(call get-my-dir)

UTILITY_SRCS+=\
	$(THIS.utility)/logger.cpp\

UNITTESTS+=$(UTILITY_SRCS)
UNITTESTS+=$(shell find $(THIS.utility)/unittests -name "*.cpp")
