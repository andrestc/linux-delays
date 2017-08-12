#include <netlink/netlink.h>
#include <netlink/socket.h>

int main(int argc, char *argv[])
{
    struct nl_sock *sk;
    sk = nl_socket_alloc();
    
    if (sk == NULL) {
        fprintf(stderr, "Error allocating netlink socket");
        return 1;
    }
    
    nl_socket_free(sk);
}
