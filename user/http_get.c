#include "http_get.h"
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>


LOCAL struct _esp_tcp user_tcp;
LOCAL struct espconn user_tcp_conn;
LOCAL os_timer_t test_timer;
LOCAL os_timer_t polling_timer;
ip_addr_t tcp_server_ip;
LOCAL unsigned short value ;

LOCAL void ICACHE_FLASH_ATTR
user_tcp_recon_cb(void *arg, sint8 err)
{
   //error occured , tcp connection broke. user can try to reconnect here.

    os_printf("reconnect callback, error code %d !!! \r\n",err);
}

LOCAL void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
   //received some data from tcp connection

    os_printf("tcp recv !!! %s \r\n", pusrdata);

}

LOCAL void ICACHE_FLASH_ATTR
user_tcp_sent_cb(void *arg)
{
   //data sent successfully

    os_printf("tcp sent succeed !!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR
user_tcp_discon_cb(void *arg)
{
   //tcp disconnect successfully

    os_printf("tcp disconnect succeed !!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR
user_tcp_connect_cb(void *arg)
{
  struct espconn *pespconn = arg;

  os_printf("connect succeed !!! \r\n");

  espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
  espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
  espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

  char tmp[256];
  os_sprintf( tmp, NET_URL, value );
  espconn_sent(pespconn, tmp, os_strlen(tmp));
}

LOCAL void ICACHE_FLASH_ATTR
user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    if (ipaddr == NULL)
    {
        os_printf("user_dns_found NULL \r\n");
        return;
    }

   //dns got ip
    os_printf("user_dns_found %d.%d.%d.%d \r\n",
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if (tcp_server_ip.addr == 0 && ipaddr->addr != 0)
    {
      // dns succeed, create tcp connection
        os_timer_disarm(&test_timer);
        tcp_server_ip.addr = ipaddr->addr;
        os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4); // remote ip of tcp server which get by dns

        pespconn->proto.tcp->remote_port = 80; // remote port of tcp server
        pespconn->proto.tcp->local_port = espconn_port(); //local port of ESP8266

        espconn_regist_connectcb(pespconn, user_tcp_connect_cb); // register connect callback
        espconn_regist_reconcb(pespconn, user_tcp_recon_cb); // register reconnect callback as error handler

        espconn_connect(pespconn); // tcp connect
    }
}

LOCAL void ICACHE_FLASH_ATTR
user_dns_check_cb(void *arg)
{
    struct espconn *pespconn = arg;
    espconn_gethostbyname(pespconn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // recall DNS function
    os_timer_arm(&test_timer, 1000, 0);
}


void http_get(unsigned short v) {
  user_tcp_conn.proto.tcp = &user_tcp;
  user_tcp_conn.type = ESPCONN_TCP;
  user_tcp_conn.state = ESPCONN_NONE;

  value = v ;

  tcp_server_ip.addr = 0;
  espconn_gethostbyname(&user_tcp_conn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // DNS function

  os_timer_setfn(&test_timer, (os_timer_func_t *)user_dns_check_cb, &user_tcp_conn);
  os_timer_arm(&test_timer, 1000, 0);
}

LOCAL void ICACHE_FLASH_ATTR
polling_cb(void *arg)
{
  uint16 v = system_adc_read();
  os_printf("adc read %d", v);
  http_get(v);
  os_timer_arm(&polling_timer, 30000, 0);
}

void http_polling() {
  os_timer_setfn(&polling_timer, (os_timer_func_t *)polling_cb, NULL);
  os_timer_arm(&polling_timer, 30000, 0);
}
