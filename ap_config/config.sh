sudo ifconfig wlxc43cb06cfbc6 192.168.131.1
sudo udhcpd ./udhcp/wlxc43cb06cfbc6.conf

sudo ifconfig wlxc43cb06567f2 192.168.134.1
sudo udhcpd ./udhcp/wlxc43cb06567f2.conf 

sudo systemctl stop NetworkManager


sudo hostapd hostapd/ap_5/wlxc43cb06567f2.conf -B
sudo hostapd hostapd/ap_5/wlxc43cb06cfbc6.conf -B

sudo iw dev wlxc43cb06572b6 interface add mon_48 type monitor
sudo iw dev wlxc43cb06572b6 del
sudo iw dev wlxc43cb06cd942 interface add mon_165 type monitor
sudo iw dev wlxc43cb06cd942 del
sudo ifconfig mon_48 up
sudo ifconfig mon_165 up
sudo iw dev mon_48 set channel 48
sudo iw dev mon_165 set channel 165