objects = test.o Poller.o Channel.o EventLoop.o

edit:$(objects)
	g++ -g -std=c++11 -o edit $(objects)

test.o:test.c Channel.h EventLoop.h
	g++ -c -g test.c
Channel.o:Channel.c EventLoop.h
	g++ -c -g Channel.c
EventLoop.o:EventLoop.c Channel.h Poller.h
	g++ -c -g EventLoop.c
Poller.o:Poller.c Channel.h
	g++ -c -g Poller.c

.PHONY:clean
clean:
	rm edit $(objects)
