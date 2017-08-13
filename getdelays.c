/*
*   This utility is based on linux/tools/accounting/getdelays.c
*   (https://github.com/torvalds/linux/blob/master/tools/accounting/getdelays.c) and
*   uses libnl to get per-pid delay accounting statistics from the kernel.
*   Usage:
*       make
*       sudo ./getdelays <PID>
*/

#include <netlink/attr.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <unistd.h>
#include <linux/taskstats.h>

#define average_ms(t, c) (t / 1000000ULL / (c ? c : 1))

static void print_delayacct(struct taskstats *t)
{
    printf("[PID] %d", t->ac_pid);
	printf("\nCPU   %15s%15s%15s%15s%15s\n"
	       "      %15llu%15llu%15llu%15llu%15.3fms\n"
	       "IO    %15s%15s%15s\n"
	       "      %15llu%15llu%15llums\n"
	       "SWAP  %15s%15s%15s\n"
	       "      %15llu%15llu%15llums\n"
	       "RECLAIM  %12s%15s%15s\n"
	       "      %15llu%15llu%15llums\n",
	       "count", "real total", "virtual total",
	       "delay total", "delay average",
	       (unsigned long long)t->cpu_count,
	       (unsigned long long)t->cpu_run_real_total,
	       (unsigned long long)t->cpu_run_virtual_total,
	       (unsigned long long)t->cpu_delay_total,
	       average_ms((double)t->cpu_delay_total, t->cpu_count),
	       "count", "delay total", "delay average",
	       (unsigned long long)t->blkio_count,
	       (unsigned long long)t->blkio_delay_total,
	       average_ms(t->blkio_delay_total, t->blkio_count),
	       "count", "delay total", "delay average",
	       (unsigned long long)t->swapin_count,
	       (unsigned long long)t->swapin_delay_total,
	       average_ms(t->swapin_delay_total, t->swapin_count),
	       "count", "delay total", "delay average",
	       (unsigned long long)t->freepages_count,
	       (unsigned long long)t->freepages_delay_total,
	       average_ms(t->freepages_delay_total, t->freepages_count));
}

int callback_message(struct nl_msg *nlmsg, void *arg) {

    struct nlmsghdr *nlhdr;
    struct nlattr *nlattrs[TASKSTATS_TYPE_MAX + 1];
    struct nlattr *nlattr;
    struct taskstats *stats;
    int rem, answer;

    nlhdr = nlmsg_hdr(nlmsg);

    if ((answer = genlmsg_parse(nlhdr, 0, nlattrs, TASKSTATS_TYPE_MAX, NULL)) < 0) {
        fprintf(stderr, "error parsing msg\n");
        return -1;
    }

    if ((nlattr = nlattrs[TASKSTATS_TYPE_AGGR_PID]) || (nlattr = nlattrs[TASKSTATS_TYPE_NULL])) {
        stats = nla_data(nla_next(nla_data(nlattr), &rem));
        print_delayacct(stats);
    } else {
        fprintf(stderr, "unknown attribute format received\n");
        return -1;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    struct nlmsghdr *hdr;
    struct nl_msg *msg;
    struct nl_sock *sk;
    pid_t pid;
    int err, family, exit_code = 0;

    pid = getpid();
    if (argc >= 2)
        pid = atoi(argv[1]);

    sk = nl_socket_alloc();
    if (sk == NULL) {
        fprintf(stderr, "Error allocating netlink socket");
        exit_code = 1;
        goto teardown;
    }

    if ((err = nl_connect(sk, NETLINK_GENERIC)) < 0) {
        fprintf(stderr, "Error connecting: %s\n", nl_geterror(err));
        exit_code = 1;
        goto teardown;
    }

    if ((family = genl_ctrl_resolve(sk, "TASKSTATS")) == 0) {
        fprintf(stderr, "Error retrieving family id: %s\n", nl_geterror(err));
        exit_code = 1;
        goto teardown;
    }

    if ((err = nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, callback_message, NULL)) < 0) {
        fprintf(stderr, "Error setting socket cb: %s\n", nl_geterror(err));
        exit_code = 1;
        goto teardown;
    }

    if (!(msg = nlmsg_alloc())) {
        fprintf(stderr, "Failed to alloc message: %s\n", nl_geterror(err));
        exit_code = 1;
        goto teardown;
    }

    if (!(hdr = genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0,
        NLM_F_REQUEST, TASKSTATS_CMD_GET, TASKSTATS_VERSION))) {
        fprintf(stderr, "Error setting message header\n");
        exit_code = 1;
        goto teardown;
    }

    if ((err = nla_put_u32(msg, TASKSTATS_CMD_ATTR_PID, pid)) < 0) {
        fprintf(stderr, "Error setting attribute: %s\n", nl_geterror(err));
        exit_code = 1;
        goto teardown;
    }

    if ((err = nl_send_auto(sk, msg)) < 0) {
        fprintf(stderr, "Error sending message: %s\n", nl_geterror(err));
        exit_code = 1;
        goto teardown;
    }

    if ((err = nl_recvmsgs_default(sk)) < 0) {
        fprintf(stderr, "Error receiving message: %s\n", nl_geterror(err));
        exit_code = 1;
        goto teardown;
    }

teardown:
    nl_close(sk);
    nl_socket_free(sk);
    nlmsg_free(msg);
    return exit_code;
}
