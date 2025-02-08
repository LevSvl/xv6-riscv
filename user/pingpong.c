#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	int p[2], pid;
	uint8 byte;

	pipe(p);

	if((pid = fork()) < 0) {
		goto bad;
	} else if(pid == 0) {
		// read 8 from the pipe
		if(((read(p[0], &byte, 1)) == 1) && (byte == 0x8))
			printf("%d: received ping\n", getpid());
		else {
			goto bad;
		}

		// write 9 to the pipe
		byte = 0x9;
		write(p[1], &byte, 1);

	} else {
		// write 8 to the pipe
		byte = 0x8;
		write(p[1], &byte, 1);

		// read 9 from the pipe
		if(((read(p[0], &byte, 1)) == 1) && (byte == 0x9))
			printf("%d: received pong\n", getpid());
		else {
			goto bad;
		}
	}

	close(p[0]);
	close(p[1]);

  exit(0);
bad:
	close(p[0]);
	close(p[1]);
	exit(1);
}
