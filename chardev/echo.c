#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/malloc.h>

#define BUFFERSIZE 255
#define DEVADDR (void *)0xFFFFFFFF

/* virtual address pointer*/
void *vadd_ptr;


/* Function prototypes */
static d_open_t echo_open;
static d_close_t echo_close;
static d_read_t echo_read;
static d_write_t echo_write;
static d_mmap_t echo_mmap;

/* Character device entry points */

static struct cdevsw echo_cdevsw = {
	.d_version = D_VERSION,
	.d_open = echo_open,
	.d_close = echo_close,
	.d_read = echo_read,
	.d_write = echo_write,
	.d_mmap = echo_mmap,
	.d_name = "echo",
};

struct s_echo {
	char msg[BUFFERSIZE + 1];
	int len;
};

static struct cdev *echo_dev;
static struct s_echo *echomsg;

MALLOC_DECLARE(M_ECHOBUF);
MALLOC_DEFINE(M_ECHOBUF, "echobuffer", "buffer for echo module");

/* Load/unload function */

static int echo_loader(struct module *m__unused, int what, void *arg__unused) {
	int error = 0;

	switch(what) {
		case MOD_LOAD:
			error = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK, &echo_dev, &echo_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, "echo");
			if (error != 0) {
				break;
			}
			echomsg = malloc(sizeof(*echomsg), M_ECHOBUF, M_WAITOK | M_ZERO);
			uprintf("Echo device loaded.\n");
			break;
		case MOD_UNLOAD:
			destroy_dev(echo_dev);
			free(echomsg, M_ECHOBUF);
			uprintf("Echo device unloaded.\n");
			break;
		default:
			error = EOPNOTSUPP;
			break;
	}
	return (error);
}

static int echo_open(struct cdev *dev __unused, int oflags __unused, int devtype __unused, struct thread *td __unused) {
	int error = 0;
	uprintf("Opened device \"echo\" successfully.\n");
	return error;
}

static int echo_close(struct cdev *dev __unused, int fflags __unused, int devtype __unused, struct thread *td __unused) {
	uprintf("Closing device \"echo\".\n");
	return 0;
}

static int echo_read(struct cdev *dev __unused, struct uio *uio, int ioflag __unused) {
	uprintf("Now reading from device '/dev/echo'\n");

	size_t amt;
	int error;

	amt = MIN(uio->uio_resid, uio->uio_offset >= echomsg->len + 1 ? 0 : echomsg->len + 1 - uio->uio_offset);
	while (uio->uio_resid != 0) {
		if ((error = uiomove(echomsg->msg, amt, uio)) != 0) {
			uprintf("uiomove failed!\n");
		}

	}
	return error;

static int echo_write(struct cdev *cdev __unused, struct uio *uio, int ioflag __unused) {
	
	uprintf("Now writing to device '/dev/echo'\n");

	size_t amt;
	int error;

	//should only be able to write from start or end of buffer 
	if (uio->uio_offset != 0  && (uio->uio_offset != echomsg->len)) {
		return (EINVAL);	
	}

	//if writing from start of buffer, zero the echomsg buffer length
	if (uio->uio_offset == 0) {
		echomsg->len = 0;
	}
	
	//whatever's smaller, bytes left to write or the remaining buffer size you're writing to
	amt = MIN(uio->uio_resid, (BUFFERSIZE - echomsg->len));
	error =  uiomove(echomsg->msg + uio->uio_offset, amt, uio);

	//Null terminate string and record length of string now stored in buffer
	echomsg->len = uio->uio_offset;
	echomsg->msg[echomsg->len] = 0;

	if (error != 0) {
		uprintf("Write failed: bad address!\n");
	}
	uprintf("Write completed successfully!\n");
	return error;
} 

static int echo_mmap(struct cdev *cdev, vm_ooffset_t offset, vm_paddr_t *paddr, int nprot, vm_memattr_t *memattr) {
	//need to convert physical device address to virtual address
	printf("test");
	return 0;
}


DEV_MODULE(echo, echo_loader, NULL);