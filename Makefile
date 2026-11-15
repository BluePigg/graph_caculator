graph: graph.c
	gcc -O3 graph.c -o graph -lraylib -lm

clean:
	rm -f graph

.PHONY: clean
