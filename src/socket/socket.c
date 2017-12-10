#include <assert.h>
#include <poll.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <c0runtime.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>

#define INT16_T_MAX (0x7FFF)
#define INT16_T_MIN (-(0x8000))

#define INT32_T_MAX 0x7FFFFFFF
#define INT32_T_MIN 0x80000000

#define ROUND_TO_4(x) ((((x) + 3) / 4) * 4)

typedef c0_int socket_t;

socket_t dgram_socket() {
    // 0 is default protocol, which is UDP in this context.
    socket_t sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        c0_abort("Socket creation failed.");
    }

    return sock;
}

c0_int recv_msglen (socket_t sock) {
    struct sockaddr src_addr;
    socklen_t addrlen;
    size_t len = 0;
    int flags = MSG_TRUNC | MSG_PEEK;

    ssize_t pkt_size = recvfrom(sock, NULL, len, flags, &src_addr, &addrlen);
    if (-1 == pkt_size) {
        c0_error("recv_msglen failed.");
    }
    assert(pkt_size <= INT32_T_MAX);

    c0_int rv = (c0_int) pkt_size;
    return rv;
}

c0_int c0_recvfrom (socket_t sock, c0_array buf, c0_int len,
                    c0_pointer src_sin_family,
                    c0_pointer src_sin_port,
                    c0_pointer src_sin_addr) {
    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    size_t read_len = (size_t) len;
    int flags = MSG_TRUNC;

    // UDP packets are guaranteed to be no more than 64k, but we also guarantee
    // that we're only reading len bytes. 
    // Perhaps in future, this should be 64k for more safety?
    /* TODO: Find a way to not allocate here. */
    void* tmp_buf = calloc(ROUND_TO_4(len), 1);
    if (NULL == tmp_buf) {
        c0_abort("Failed to allocate memory in recvfrom.");
    }

    ssize_t rv = recvfrom(sock, tmp_buf, len, flags,
                          (struct sockaddr*) &src_addr, &addrlen);
    if (-1 == rv) {
        free(tmp_buf);
        c0_error("recvfrom failed.");
    } else if (rv < len) {
        c0_abort("Packet read was incorrect length.");
    }

    assert(rv == len);

    for (int i = 0; i < (ROUND_TO_4(len) / 4); i++) {
        *((c0_int*) c0_array_sub(buf, i, sizeof(c0_int))) = ((int*) tmp_buf)[i];
    }
    /* TODO */
    free(tmp_buf);

    *((c0_int*) c0_deref(src_sin_family)) = ntohs(src_addr.sin_family);
    *((c0_int*) c0_deref(src_sin_port)) = ntohs(src_addr.sin_port);
    *((c0_int*) c0_deref(src_sin_addr)) = ntohl(src_addr.sin_addr.s_addr);

    return rv;
}

c0_int c0_sendto (socket_t sock, c0_array buf, c0_int len,
                  c0_int dst_sin_family,
                  c0_int dst_sin_port,
                  c0_int dst_sin_addr) {
    struct sockaddr_in dst_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    size_t write_len = (size_t) len;
    int flags = 0;

    assert(SHRT_MIN <= dst_sin_family);
    assert(dst_sin_family <= SHRT_MAX);

    assert(0 <= dst_sin_port);
    assert(dst_sin_port <= USHRT_MAX);

    // dst_sin_addr, being a 32-bit c0_int, is necessarily the same size as a
    // uint32_t, which dst_addr.sin_addr.s_addr is.

    dst_addr.sin_family = htons(dst_sin_family);
    dst_addr.sin_port = htons(dst_sin_port);
    dst_addr.sin_addr.s_addr = htonl(dst_sin_addr);
    memset(dst_addr.sin_zero, sizeof(dst_addr.sin_zero), 0);

    /* TODO: Find a way to not allocate here. */
    void* tmp_buf = calloc(ROUND_TO_4(len), 1);
    if (NULL == tmp_buf) {
        c0_abort("Failed to allocate memory in recvfrom.");
    }

    for (int i = 0; i < (ROUND_TO_4(len) / 4); i++) {
        ((int*) tmp_buf)[i] = *((c0_int*) c0_array_sub(buf, i, sizeof(c0_int)));
    }
    /* TODO */

    ssize_t rv = sendto(sock, tmp_buf, len, flags,
                        (struct sockaddr*) &dst_addr, addrlen);
    if (-1 == rv) {
        free(tmp_buf);
        c0_error("sendto failed.");
    } else if (rv < len) {
        c0_abort("Sent less than expected.");
    }
    free(tmp_buf);

    assert(rv == len);
    return rv;
} 

c0_int poll_read (socket_t sock, c0_int timeout) {
    struct pollfd sockfd;
    nfds_t nfds = 1;
    int rv;
    
    sockfd.fd = sock;
    sockfd.events = POLLIN;
    sockfd.revents = 0;
    rv = poll(&sockfd, nfds, timeout);

    if (-1 == rv) {
        c0_error("poll_read failed.");
    }
    else if (1 == rv && sockfd.revents & POLLERR) {
        // TODO
        c0_abort("poll_read got POLLERR.");
    }
    else if (1 == rv && sockfd.revents & POLLHUP) {
        // TODO
        c0_error("poll_read got POLLHUP.");
    }
    else if (1 == rv && sockfd.revents & POLLNVAL) {
        // TODO
        c0_abort("poll_read got POLLNVAL.");
    }
    
    assert(0 == rv || (1 == rv && sockfd.revents & POLLIN));
    return rv;
}

c0_int poll_write (socket_t sock, c0_int timeout) {
    struct pollfd sockfd;
    nfds_t nfds = 1;
    int rv;
    
    sockfd.fd = sock;
    sockfd.events = POLLOUT;
    sockfd.revents = 0;
    rv = poll(&sockfd, nfds, timeout);

    if (-1 == rv) {
        c0_error("poll_write failed.");
    }
    else if (1 == rv && sockfd.revents & POLLERR) {
        // TODO
        c0_abort("poll_write got POLLERR.");
    }
    else if (1 == rv && sockfd.revents & POLLHUP) {
        // TODO
        c0_error("poll_write got POLLHUP.");
    }
    else if (1 == rv && sockfd.revents & POLLNVAL) {
        // TODO
        c0_abort("poll_write got POLLNVAL.");
    }
    
    assert(0 == rv || (1 == rv && sockfd.revents & POLLOUT));
    return rv;
}

c0_int c0_ntohl(c0_int netlong) {
    assert (INT32_T_MIN <= netlong && netlong <= INT32_T_MAX);

    c0_int rv = ntohl(netlong);
    
    assert (INT32_T_MIN <= rv && rv <= INT32_T_MAX);
    return rv;
}

c0_int c0_ntohs(c0_int netshort) {
    assert (INT16_T_MIN <= netshort && netshort <= INT16_T_MAX);

    c0_int rv = ntohs(netshort);
    
    assert (INT16_T_MIN <= rv && rv <= INT16_T_MAX);
    return rv;
}

c0_int c0_htonl(c0_int hostlong) {
    assert (INT32_T_MIN <= hostlong && hostlong <= INT32_T_MAX);

    c0_int rv = htonl(hostlong);
    
    assert (INT32_T_MIN <= rv && rv <= INT32_T_MAX);
    return rv;
}

c0_int c0_htons(c0_int hostshort) {
    assert (INT16_T_MIN <= hostshort && hostshort <= INT16_T_MAX);

    c0_int rv = htons(hostshort);
    
    assert (INT16_T_MIN <= rv && rv <= INT16_T_MAX);
    return rv;
}
