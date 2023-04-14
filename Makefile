CC = clang++ -O2 TPGM.h TPGM.cpp

clean:
	rm -rf *.out *.gch
niblack:
	$(CC) niblack.cpp
	./a.out integral
	./a.out
bradley:
	$(CC) bradley.cpp
	./a.out
sauvola:
	$(CC) sauvola.cpp
	./a.out integral
	./a.out
