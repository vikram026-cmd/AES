#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>

#define BUFFER_SIZE 100
char device_buffer[BUFFER_SIZE] = "Initial Virtual Device Data\n";
int data_length = 28;

int io_read(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb)
{
    int status;

    if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK)
    {
        return status;
    }

    if (msg->i.xtype & _IO_XTYPE_MASK)
    {
        return ENOSYS;
    }

    if (ocb->offset >= data_length)
    {
        _IO_SET_READ_NBYTES(ctp, 0);
        return (_RESMGR_NPARTS(0));
    }

    int bytes_to_read = data_length - ocb->offset;
    if (bytes_to_read > msg->i.nbytes)
    {
        bytes_to_read = msg->i.nbytes;
    }

    printf("[Device Manager] Intercepted read(). Transferring %d bytes to client...\n", bytes_to_read);

    SETIOV(ctp->iov, device_buffer + ocb->offset, bytes_to_read);
    _IO_SET_READ_NBYTES(ctp, bytes_to_read);

    ocb->offset += bytes_to_read;

    return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb)
{
    int status;

    if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK)
    {
        return status;
    }

    if (msg->i.xtype & _IO_XTYPE_MASK)
    {
        return ENOSYS;
    }

    int bytes_to_write = msg->i.nbytes;
    if (bytes_to_write > (BUFFER_SIZE - 1))
    {
        bytes_to_write = BUFFER_SIZE - 1;
    }

    printf("[Device Manager] Intercepted write(). Capturing input stream...\n");

    resmgr_msgread(ctp, device_buffer, bytes_to_write, sizeof(msg->i));
    device_buffer[bytes_to_write] = '\0'; // Append string null terminator
    data_length = bytes_to_write;         // Reset internal index size tracking metric
    ocb->offset = 0;                       // Reset file position offset tracker

    _IO_SET_WRITE_NBYTES(ctp, bytes_to_write);
    return (_RESMGR_NPARTS(0));
}

int main()
{
    dispatch_t *dpp;
    resmgr_attr_t resmgr_attr;
    resmgr_context_t *ctp;
    resmgr_connect_funcs_t connect_funcs;
    resmgr_io_funcs_t io_funcs;
    iofunc_attr_t attr;
    struct sched_param param;

    param.sched_priority = 10;
    if (sched_setscheduler(0, SCHED_RR, &param) == -1)
    {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }

    if (ThreadCtl(_NTO_TCTL_IO_PRIV, 0) == -1)
    {
        perror("ThreadCtl privileges failed. Run as root.");
        exit(EXIT_FAILURE);
    }

    if ((dpp = dispatch_create()) == NULL)
    {
        perror("dispatch_create failed");
        exit(EXIT_FAILURE);
    }

    memset(&resmgr_attr, 0, sizeof(resmgr_attr));
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);

    io_funcs.read = io_read;
    io_funcs.write = io_write;

    iofunc_attr_init(&attr, S_IFCHR | 0666, NULL, NULL);
    attr.nbytes = data_length;

    if (resmgr_attach(dpp, &resmgr_attr, "/dev/virt_device", _FTYPE_ANY, 0,
                      &connect_funcs, &io_funcs, &attr) == -1)
    {
        perror("resmgr_attach failed");
        exit(EXIT_FAILURE);
    }

    if ((ctp = resmgr_context_alloc(dpp)) == NULL)
    {
        perror("resmgr_context_alloc failed");
        exit(EXIT_FAILURE);
    }

    printf("Resource Manager Active. Virtual device path mounted at: /dev/virt_device\n");
    printf("Listening for POSIX open(), read(), and write() operations...\n");

    while (1)
    {
        if ((ctp = resmgr_block(ctp)) == NULL)
        {
            perror("resmgr_block failed");
            exit(EXIT_FAILURE);
        }
        resmgr_handler(ctp); // Decode message frames and call io_read/io_write
    }

    return 0;
}
