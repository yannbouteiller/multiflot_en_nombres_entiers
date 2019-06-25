all: fichiers.o graphe.o algorithmes.o main.o multiflot

fichiers.o : fichiers.c fichiers.h
	gcc -Wall -o fichiers.o -c fichiers.c -O3 -lm
	
graphe.o : graphe.c graphe.h
	gcc -Wall -o graphe.o -c graphe.c -O3
	
algorithmes.o : algorithmes.c algorithmes.h
	gcc -Wall -o algorithmes.o -c algorithmes.c -O3
	
main.o : main.c graphe.h fichiers.h
	gcc -Wall -o main.o -c main.c -O3
	
multiflot : fichiers.o graphe.o main.o
	gcc -Wall -o multiflot main.o algorithmes.o graphe.o fichiers.o -O3 -lm
clean:
	rm -rf *.o
