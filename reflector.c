#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_packet.h>
#include <bpf/bpf_helpers.h>

#define DEST_PORT           7777

SEC("xdp")  
int xdp_reflector(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct ethhdr *eth = data;
    struct iphdr* iph = data+sizeof(*eth);
    struct udphdr *udph = NULL;

    if (data + sizeof(*eth) > data_end)
        return XDP_PASS;

    __u32 hdr_size = iph->ihl*4;
    if(hdr_size < sizeof(*iph))
		return XDP_PASS;
    
    if(data+sizeof(*eth)+hdr_size+1 > data_end)
        return XDP_PASS;
    
    udph = data+sizeof(*eth)+hdr_size;

    if(bpf_ntohs(udph->dest) != DEST_PORT){
        return XDP_PASS;
    }

    __u8 h_tmp[ETH_ALEN];

    //swap mac
    __builtin_memcpy(h_tmp, eth->h_dest, ETH_ALEN);
    __builtin_memcpy(eth->h_dest, eth->h_source, ETH_ALEN);
    __builtin_memcpy(eth->h_source, h_tmp, ETH_ALEN);

    //swap ip
    __builtin_memcpy(h_tmp, &iph->daddr, sizeof(iph->saddr));
    __builtin_memcpy(&iph->daddr, &iph->saddr, sizeof(iph->saddr));
    __builtin_memcpy(&iph->saddr, h_tmp, sizeof(iph->saddr));

    //swap udp
    __builtin_memcpy(h_tmp, &udph->dest, sizeof(iph->saddr));
    __builtin_memcpy(&udph->dest, &udph->source, sizeof(iph->saddr));
    __builtin_memcpy(&udph->source, h_tmp, sizeof(iph->saddr));

    return XDP_TX;
}
