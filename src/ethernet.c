#include "ethernet.h"
#include "utils.h"
#include "driver.h"
#include "arp.h"
#include "ip.h"
/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf)
{
    if (buf->len < sizeof(ether_hdr_t)){
        printf("ethernet_in: buf not complete error\n");
        return;
    }
    //save the head_ether
    ether_hdr_t* head_ether = (ether_hdr_t*)buf->data;
    if (buf_remove_header(buf, sizeof(ether_hdr_t))){
        printf("ethernet_in: head remove error\n");
        return;
    }
    if (net_in(buf, swap16(head_ether->protocol16), head_ether->src)){
        return;
    }
}
/**
 * @brief 处理一个要发送的数据包
 * 
 * @param buf 要处理的数据包
 * @param mac 目标MAC地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol)
{
    if (buf->len < ETHERNET_MIN_TRANSPORT_UNIT)
    {
        if (buf_add_padding(buf, ETHERNET_MIN_TRANSPORT_UNIT - buf->len))
        {
            printf("ethernet_out: padding error\n");
            return;
        }
    }
    if (buf_add_header(buf, sizeof(ether_hdr_t)))
    {
        printf("buf_add_header: add_head error\n");
        return;
    }
    ether_hdr_t* head_ether = (ether_hdr_t*)buf->data;
    for (int i=0;i<NET_MAC_LEN;i++){
        (head_ether->dst)[i] = mac[i];
    }
    for (int i=0;i<NET_MAC_LEN;i++){
        (head_ether->src)[i] = net_if_mac[i];
    }
    head_ether->protocol16 = swap16(protocol);
    driver_send(buf);
}
/**
 * @brief 初始化以太网协议
 * 
 */
void ethernet_init()
{
    buf_init(&rxbuf, ETHERNET_MAX_TRANSPORT_UNIT + sizeof(ether_hdr_t));
}

/**
 * @brief 一次以太网轮询
 * 
 */
void ethernet_poll()
{
    if (driver_recv(&rxbuf) > 0)
        ethernet_in(&rxbuf);
}
