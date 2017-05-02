objects=burning.o string_p.o
burning:$(objects)
	gcc -o buring $(objects)
burning.o:burning.c
	gcc -c burning.c
string_p.o:string_p.c
	gcc -c string_p.c
clear:
	rm $(objects)
