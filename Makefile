CC=g++
CFLAGS=-Wall -std=c++11 -pthread
TARGET=main
FILES=main.o utils.o WFEHashMap.o LBEHashMap.o Node.o DataNode.o ArrayNode.o MonkeyHashMap.o

%: all

all: $(TARGET)

debug: CFLAGS += -DDEBUG_H
debug: all

$(TARGET): $(FILES)
	$(CC) $(CFLAGS) $^ -o $@
	mkdir -p output

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o $(TARGET)
