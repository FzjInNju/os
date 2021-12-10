main: main.cpp my_print.o
	g++ -std=c++11 -m32 main.cpp my_print.o -o main
	./main
my_print.o: my_print.asm
	nasm -f elf32 my_print.asm -o my_print.o
.PHONY : clean
clean:
	rm -rf main my_print.o

