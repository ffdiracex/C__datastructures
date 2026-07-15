//packet manipulation in userspace
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nfnetlink_queue.h>
#include <linux/netfilter/nfnetlink.h>
#include <arpa/inet.h>
#include <errno.h>

// NETLINK message construction
struct netlink_msg
{
    struct nlmsghdr nlh;
    struct nfgenmsg nfmsg;
    union {
        struct nfqnl_msg_config_cmd cmd;
        struct nfqnl_msg_config_params params;
        struct nfqnl_msg_config_mode mode;
    }payload;
};

int nfqueue_init()
{
    //init socket
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_NETFILTER);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(sock);
        return -1;
    }

    // send bind command to netfilter
    struct {
        struct nlmsghdr nlh;
        struct nfgenmsg nfmsg;
        struct nfqnl_msg_config_cmd cmd;
    } bind_msg;

    memset(&bind_msg, 0, sizeof(bind_msg));
    bind_msg.nlh.nlmsg_len = sizeof(bind_msg);
    bind_msg.nlh.nlmsg_type = (NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_CONFIG;
    bind_msg.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    bind_msg.nlh.nlmsg_seq = 1;
    bind_msg.nlh.nlmsg_pid = getpid();
    bind_msg.nfmsg.nfgen_family = AF_INET;
    bind_msg.nfmsg.version = NFNETLINK_V0;
    bind_msg.nfmsg.res_id = htons(0); //queue num 0
    bind_msg.cmd.command = NFQNL_CFG_CMD_BIND;
    bind_msg.cmd.pf = htons(AF_INET);

    send(sock, &bind_msg, sizeof(bind_msg), 0);

    //set packet copy mode
    struct {
        struct nlmsghdr nlh;
        struct nfgenmsg nfmsg;
        struct nfqnl_msg_config_mode mode;
    } mode_msg;

    memset(&mode_msg, 0, sizeof(mode_msg));
    mode_msg.nlh.nlmsg_len = sizeof(mode_msg);
    mode_msg.nlh.nlmsg_type = (NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_CONFIG;
    mode_msg.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    mode_msg.nlh.nlmsg_seq = 2;
    mode_msg.nlh.nlmsg_pid = getpid();
    mode_msg.nfmsg.nfgen_family = AF_INET;
    mode_msg.nfmsg.version = NFNETLINK_V0;
    mode_msg.nfmsg.res_id = htons(0);
    mode_msg.mode.command = NFQNL_CFG_CMD_BIND;
    mode_msg.mode.mode = NFQNL_COPY_PACKET;
    mode_msg.mode.copy_range = htonl(0xFFFF);

    send(sock, &mode_msg, sizeof(mode_msg),0);

    return sock;
}

// receive and process the packets
int process_packet(int sock)
{
    char buffer[65536];
    struct sockaddr_nl src_addr;
    socklen_t addrlen = sizeof(src_addr);

    int len = recvfrom(sock, buffer, sizeof(buffer), 0,
            (struct sockaddr*)&src_addr, &addrlen);
    if (len<0) return -1;

    struct nlmsghdr *nlh = (struct nlmsghdr*)buffer;

    //check if it is a packet from NETFILTER
    if (nlh->nlmsg_type == (NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_PACKET)
    {
        struct nfgenmsg *nfmsg = (struct nfgenmsg*)(nlh+1);

        //the packet data follows the netfilter header
        // TODO: extract IP/TCP headers

        fprintf(stdout, "Received packet from netfilter queue\n");

        //send verdict (ID=ACCEPT) back
        struct {
            struct nlmsghdr *nlh;
            struct nfgenmsg nfmsg;
            struct nfqnl_msg_verdict_verdict verdict;
        } verdict_msg;

        memset(&verdict_msg, 0, sizeof(verdict_msg));
        verdict_msg.nlh.nlmsg_len = sizeof(verdict_msg);
        verdict_msg.nlh.nlmsg_type = (NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_VERDICT;
        verdict_msg.nlh.nlmsg_flags = NLM_F_REQUEST;
        verdict_msg.nlh.nlmsg_seq = 3;
        verdict_msg.nlh.nlmsg_pid = getpid();
        verdict_msg.nfmsg.nfgen_family = AF_INET;
        verdict_msg.nfmsg.version = NFNETLINK_V0;
        verdict_msg.nfmsg.res_id = htons(0);
        verdict_msg.verdict.id = 0;  // Need to extract packet ID
        verdict_msg.verdict.verdict = NF_ACCEPT;

        send(sock, &verdict_msg, sizeof(verdict_msg), 0);
    }

    return 0;
}

int main()
{
    int sock = nfqueue_init();
    if (sock < 0) { fprintf(stderr, "failed to init netfilter"); return 1;}
    fprintf(stdout,"Netfilter queue 0 active. Set iptables -j NFQUEUE to test.\n");

    while (1) {
        process_packet(sock);
    }

    close(sock);
    return 0;
}



