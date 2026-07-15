// Observe packets on the chosen network.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>

// ETHERNET ; header
struct _ethhdr
{
    unsigned char dest[6];
    unsigned char src[6];
    unsigned short protocol;
};

// IPV4
struct ipv4_header
{
    unsigned char version_ihl; // version 4 bits + IHL (4bits)
    unsigned char tos;
    unsigned short total_len;
    unsigned short id;
    unsigned short frag_off; //3bit flags, fragment_offset 13bit
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
    unsigned int saddr; //src addr
    unsigned int daddr; //dst addr
};

static void decode_ip(struct ipv4_header *ip)
{
    struct in_addr src, dst;
    src.s_addr = ip->saddr;
    dst.s_addr = ip->daddr;

    printf("IP: %s -> %s | protocol: %d | TTL: %d\n",
            inet_ntoa(src), inet_ntoa(dst), ip->protocol, ip->ttl);

    if (ip->protocol == 6) //tcp 
    {
        struct tcphdr *tcp = (struct tcphdr*)((char*)ip + (ip->version_ihl & 0x0F) * 4);
        printf(" TCP: %d -> %d | Flags: %s%s%s%s%s%s\n",
                ntohs(tcp->source), ntohs(tcp->dest),
                (tcp->syn ? "SYN ":""),(tcp->ack ? "ACK ":""),
                (tcp->fin ? "FIN ":""), (tcp->rst ? "RST ":""),
                (tcp->psh ? "PSH ":""), (tcp->urg ? "URG ":""));
    }
}

int main()
{
    //create raw socket
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0)
    {
        perror("socket (need root)");
        return 1;
    }

    // enable promiscuous mode, i.e. receive all packets
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "enp3s0"); // get the 'dev' name (device) from 'ip link show', usually wlan0 (WiFi) or enp3s0/eth0 (Ethernet)
    ioctl(sock, SIOCGIFFLAGS, &ifr);
    ifr.ifr_flags |= IFF_PROMISC;
    ioctl(sock, SIOCGIFFLAGS, &ifr);

    printf("packet inspection. send SIGEXIT to quit (ctrl+c)\n");

    char buffer[65536];
    while (1)
    {
        int len = recv(sock, buffer, sizeof(buffer), 0);
        if (len > 0)
        {
            struct _ethhdr *eth = (struct _ethhdr*)buffer;
            //check if it is IP, which has protocol 0x0800
            if(ntohs(eth->protocol) == 0x0800)
            {
                struct ipv4_header *ip = (struct ipv4_header*)(buffer + 14);
                decode_ip(ip);
            }
        }
    }
}


