src=user-program.c
obj=$(src:.c=.o)
exe=$(src:.c=)
# inc=syscall_info.h 

default:
	gcc -g $(src) -o $(exe) $(inc) -I. -lflexsc -lpthread
test: test.c
	gcc -g test.c -o test $(inc) -I. -lflexsc -lpthread

run:
	./$(exe)

clean:
	rm $(obj) $(exe)
