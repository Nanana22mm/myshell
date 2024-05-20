mysh: main.o token.o parser.o process.o signal_handler.o
	gcc -o mysh main.o token.o parser.o process.o signal_handler.o

%.o: %.c
	gcc -g -c $<

main.o: token.h
main.o: parser.h
main.o: process.h
main.o: signal_handler.h

clean:
	rm  -f mysh *.o