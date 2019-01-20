#include<poll.h>
#include<sys/select.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    struct timeval tv;

    tv.tv_sec  = 0;
    tv.tv_usec = timeout * 1000;

    fd_set  read_fds[nfds];
    fd_set write_fds[nfds];

    int  read_fds_tail = 0;
    int write_fds_tail = 0;

    for (int i = 0; i < nfds; i++) {
        FD_ZERO(&read_fds[i]);
        FD_ZERO(&write_fds[i]);

        if (fds[nfds - 1].events == POLLIN) {
            FD_SET(fds[nfds - 1].fd, &read_fds[read_fds_tail]);
            read_fds_tail++;
        }

        if (fds[nfds - 1].events == POLLOUT) {
            FD_SET(fds[nfds - 1].fd, &write_fds[write_fds_tail]);
            write_fds_tail++;
        }
    }

    int retval = select(nfds, read_fds, write_fds, NULL, &tv);

    // TODO: Put some kind of post-processing logic here to rebuild the responses back into a poll format.

    // Both poll and select have compatible return values.
    return retval;
}
