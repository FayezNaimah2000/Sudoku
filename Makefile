all:
	gcc sudoku.c -o sudoku -lpthread -lrt
clean:
	rm sudoku