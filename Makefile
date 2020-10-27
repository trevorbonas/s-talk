make:
	gcc -ggdb -o s-talk list.o driver.c boss.c receive.c send.c read.c write.c -lpthread
macos:
	gcc -ggdb -o s-talk tlist.c driver.c boss.c receive.c send.c read.c write.c -lpthread
clean:
	rm s-talk