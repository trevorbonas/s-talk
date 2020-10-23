make:
	gcc -o s-talk list.o driver.c boss.c receive.c send.c read.c write.c -lpthread
macosx:
	gcc -o s-talk tlist.c driver.c boss.c receive.c send.c read.c write.c -lpthread
clean:
	rm s-talk