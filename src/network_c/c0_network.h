#include <poll.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

int32_t c0_dgram_socket ();


/**
 *  Socket I/O
 */

int32_t c0_recv_msglen (int32_t sockfd);

// src_sin_family, src_sin_port, src_sin_addr are all returned. Likely only
// the last two are necessary, but we keep family just in case.
int32_t c0_recvfrom (int32_t sockfd, int32_t* buf, int32_t len,
                     int32_t* src_sin_family, int32_t* src_sin_port,
                     int32_t* src_sin_addr);

// ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
//                const struct sockaddr *dest_addr, socklen_t addrlen);

int32_t c0_sendto (int32_t sockfd, int32_t* buf, int32_t len,
                   int32_t dst_sin_family, int32_t dst_sin_port,
                   int32_t dst_sin_addr);


/**
 *  Polling sockets for readiness.
 */

// TODO: Timeouts needed?
int32_t c0_poll_read (int32_t sockfd, int32_t timeout);

int32_t c0_poll_write (int32_t sockfd, int32_t timeout);
