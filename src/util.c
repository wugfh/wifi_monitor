#include <string.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

uint8_t char2hex(char tmp1){
	if(tmp1 < '0' || tmp1 > '9'){
		if(tmp1 >='A'&& tmp1 <= 'F'){
			tmp1 = tmp1-'A'+10;
		}
		else if(tmp1 >= 'a'&& tmp1 <= 'f'){
			tmp1 = tmp1-'a'+10;
		}
	}
	else{
		tmp1 = tmp1-'0';
	}
	return tmp1;
}

int macaddr_aton(uint8_t* mac_addr, char* src){
    int i;
	for (i = 0; i < ETH_ALEN-1 ; i++) {
		int temp;
		char *cp = strchr(src, ':');
		if (cp) {
			cp++;
		}
		uint8_t tmp1 = char2hex(*(cp-3));
		uint8_t tmp2 = char2hex(*(cp-2));
        temp = ((tmp1<<4)|(tmp2));
		if (temp < 0 || temp > 255)
			return -1;

		mac_addr[i] = temp;
		if (!cp)
			break;
		src = cp;
	}
	uint8_t tmp1 = char2hex(*(src));
	uint8_t tmp2 = char2hex(*(src+1));
    mac_addr[i] = ((tmp1<<4)|(tmp2));
	if (mac_addr[i] < 0 || mac_addr[i] > 255)
			return -1;
	if (i < ETH_ALEN - 1)
		return -1;

	return 0;
}
int macaddr_ntoa(char* dest, uint8_t* mac_addr){
	int i, l;

	l = 0;
	for (i = 0; i < ETH_ALEN ; i++) {
		if (i == 0) {
			sprintf(dest+l, "%02x", mac_addr[i]);
			l += 2;
		} else {
			sprintf(dest+l, ":%02x", mac_addr[i]);
			l += 3;
		}
	}
}

static int set_flag(char *ifname, short flag){ 
    struct ifreq ifr;
	static int skfd = 0;
	if(skfd == 0){
		skfd = socket(AF_INET, SOCK_DGRAM, 0);
	}

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
	fprintf(stderr, ("%s: ERROR while getting interface flags: %s\n"),
		ifname,	strerror(errno));
	return (-1);
    }
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags |= flag;
    if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
	perror("SIOCSIFFLAGS");
	return -1;
    }
    return (0);
}

int enable_interface(char* if_name){
	return set_flag(if_name, (IFF_UP|IFF_RUNNING));
}