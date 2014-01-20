
CPPFLAGS= -g
OBJ= rbtree.o inttree.o test.o
TARGET= test

$(TARGET): $(OBJ)

all: $(TARGET)

clean:
	$(RM) test *.o