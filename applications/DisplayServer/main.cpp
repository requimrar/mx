// main.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <pthread.h>

static uint64_t framebuffer = 0;
static uint64_t width = 0;
static uint64_t height = 0;
static uint64_t bpp = 0;





void do_child()
{
	printf("in child, sleep(1000ms)\n");
	usleep(1000000);
	printf("awake, reading\n");


	int s = socket(AF_UNIX, SOCK_DGRAM, 0);
	struct sockaddr_un addr;

	addr.sun_family = AF_UNIX;

	strcpy(addr.sun_path, "/some/socket");
	connect(s, (struct sockaddr*) &addr, sizeof(addr));

	char* out = new char[128];
	ssize_t res = read(s, out, 128);

	printf("read (%ld): %s\n", res, out);
}

int main(int argc, char** argv)
{
	assert(argc == 5);
	/*
		1. framebuffer address
		2. framebuffer width
		3. framebuffer height
		4. framebuffer bpp
	*/

	framebuffer	= (uint64_t) argv[1];
	width		= (uint64_t) argv[2];
	height		= (uint64_t) argv[3];
	bpp			= (uint64_t) argv[4] / 8;		// kernel gives us BITS per pixel, but we really only care about BYTES per pixel.

	// printf("Display server online\n");
	// printf("Forking process...\n");



	// while(true)
	// {
	// 	usleep(10 * 1000);
	// 	printf(".");
	// 	fflush(stdout);
	// }




	int res = fork();
	if(res == 0)
	{
		printf("in child, forking again.\n");

		int r2 = fork();
		if(r2 == 0)
		{
			printf("in child of child\n");
		}
		else
		{
			printf("in parent of child's child: pid = %d\n", r2);
		}

		while(1);
	}
	else
	{
		// char d[10] = { 0 };
		printf("parent: child proc has pid %d\n", res);
		while(1);
	}






	// printf("opening shit\n");
	// {
	// 	int s = socket(AF_UNIX, SOCK_DGRAM, 0);
	// 	struct sockaddr_un addr;

	// 	addr.sun_family = AF_UNIX;

	// 	strcpy(addr.sun_path, "/some/socket");
	// 	bind(s, (struct sockaddr*) &addr, sizeof(addr));


	// 	const char* msg = "HELLO, WORLD!\n";
	// 	write(s, msg, strlen(msg) + 1);

	// 	usleep(1500000);

	// 	printf("closing\n");
	// 	close(s);
	// }





	// {
	// 	FILE* f = fopen("/texts/1984.txt", "r");
	// 	struct stat s;

	// 	fstat((int) f->__fd, &s);
	// 	printf("file is %ld bytes long\n", s.st_size);

	// 	uint8_t* x = (uint8_t*) malloc(s.st_size + 1);
	// 	memset(x, 0, s.st_size + 1);

	// 	fread(x, 1, s.st_size, f);
	// 	fclose(f);

	// 	printf("%p, %p\n", x, x + s.st_size);

	// 	x[s.st_size] = 0;
	// 	printf("%s\n", x);

	// 	fflush(stdout);
	// }


















	// that's really all we need to do, except watch for messages and flush the screen on occasion.
	return 0;
}




















