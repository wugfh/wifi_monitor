# mlo 模拟

## 实现方式
仿照mtcp，使用多个socket控制多个网卡以实现多链路。

## 底层协议
### wifi协议
IEEE 802.11ax
### 网络协议
*  传输层：udp
*  网络层：IPv4
*  链路层：IEEE802.11

## 系统结构
### 上行
```
                        .---- path 1 --.
                       /                \
 数据流  --- sta_send -------path 2 ------ ap_recv --- 数据流
                       \                /
                        `---- path 3 --`
```
### 下行
```
                        .---- path 1 --.
                       /                \
 数据流  --- ap_send -------path 2 ------ sta_recv --- 数据流
                       \                /
                        `---- path 3 --`
```

## 程序结构
### send
```
                        .---- send_thread 1-------
                       /               
 数据流  --- read_thread -------send_thread 2 ----
                       \                
                        `---- send_thread n ------
```
### recvive 
```
packets ------ read_thread 1 ---.
                                 \      
packets ------ read_thread 2 ----- write_thread ----- 数据流
                                 /      
packets ------ read_thread n ---'
```

## 执行流程
### 编译流程
1. 首先确保网卡与脚本对的上，脚本网卡为 wlxc43cb06572b6 wlxc43cb06567f2
2. 执行config.sh，config.sh 配置AP的IP地址，并使用udhcpd开启相应的dhcp服务。udhcp可以使用包管理器安装
3. 使用hostapd开启AP，hostapd的版本要在3.10以上。命令示例如下。hostapd的配置文件可以参考已有文件  
    ~~~
    sudo hostapd hostapd/ap_5/wlxc43cb06567f2.conf   
    ~~~
4. 运行cmake。如果没有相应库，cmake会自动安装。主要使用库为
    ~~~
    nl (cli route genl等衍生库都需要)
    pcap
    ~~~
5. make

### 执行流程
可执行文件分为ap_recv和sta_send。都需要相应的配置文件，在ap_config与sta_config下都有相应的配置文件，文件属性为json。执行命令示例如下
~~~
sudo ./sta_send ../sta_config/transmit.json 
~~~
~~~
sudo ./ap_recv ../ap_config/transmit.json 
~~~

## 教程
https://github.com/morrownr/USB-WiFi

## 参考库
[iperf3](https://github.com/esnet/iperf)  
[udp-multipath](https://github.com/greensea/mptunnel)  
[MLVPN](https://github.com/zehome/MLVPN/)  
[multipath UDP tunnel](https://github.com/angt/glorytun)

## 参考文档
https://siflower.github.io/2020/08/12/wifi_architecture_and_configuration_manual/

## 如何更新hostapd
https://github.com/morrownr/USB-WiFi/blob/main/home/AP_Mode/Upgrade_hostapd.md