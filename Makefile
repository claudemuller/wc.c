build-debug:
	clang -g -o bin/wc src/*.c

clean:
	rm -rf bin/*
