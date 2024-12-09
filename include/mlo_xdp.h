#pragma once

#include <linux/bpf.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <xdp/libxdp.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <unordered_map>
#include <string>

class mlo_xdp{
    int ifidx;
    char ifname[20];
    std::unordered_map<std::string, int> prog_set;
public:
    int load_xdp_prog(char* file, char* prog);
    int unload_xdp_prog(int prog_id, bool unload_all);
};