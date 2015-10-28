#GOTAGS=-tags test_build

all:
	go build $(GOTAGS) .
.PHONY: all

go.test: *.go
	go test -x -c $(GOTAGS) .

test: go.test
	LD_LIBRARY_PATH=../lib ./go.test -test.v
.PHONY: test

clean:
	rm -f go.test
.PHONY: clean
