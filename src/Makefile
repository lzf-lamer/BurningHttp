objects=burning.o string_p.o w_algorithm.o
burning:$(objects)
	gcc -o buring $(objects)
burning.o:burning.c
	gcc -c burning.c
string_p.o:string_p.c
	gcc -c string_p.c
w_algorithm.o:w_algorithm.c
	gcc -c w_algorithm.c
clear:
	rm $(objects)
