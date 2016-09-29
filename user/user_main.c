#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include "driver/uart.h"
#include "gpio.h"
#include "http_get.h"

LOCAL struct espconn conn1;
LOCAL void initDone();
LOCAL void setupUDP();
LOCAL void wifiEventHandler(System_Event_t *event);
LOCAL void recvCB(void *arg, char *pData, unsigned short len);

static volatile os_timer_t blink_led_timer;
static esp_tcp tcp;


LOCAL void initDone() {
  os_printf("Initialized! \n");

  struct station_config config;
  strcpy(config.ssid, SSID);
  strcpy(config.password, PASSWORD);

  wifi_set_opmode_current(STATION_MODE);
  wifi_station_set_config_current(&config);
  wifi_station_connect();
}

LOCAL void recvCB(void *arg, char *pData, unsigned short len) {
  os_printf("Received data: %s\n", pData );
}

LOCAL void setupUDP() {
  esp_udp udp;
  udp.local_port = 25867;

  conn1.type = ESPCONN_UDP;
  conn1.state = ESPCONN_NONE;
  conn1.proto.udp = &udp;

  espconn_create(&conn1);
  espconn_regist_recvcb(&conn1, recvCB);
  os_printf("Listening for data\n");
}

static void print_ip(const char* str, esp_tcp* tcp)
{
  /*os_printf("%s %d.%d.%d.%d:%d \n", str,
     tcp->remote_ip[0],
     tcp->remote_ip[1],
     tcp->remote_ip[2],
     tcp->remote_ip[3],
     tcp->remote_port);*/
  os_printf("-%s\n",str);
}

static void webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
  struct espconn *pesp_conn = arg;
  char fmt[] = "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connexion: Close\r\n"
    "\r\n%s" ;
  char data[256];
  if ( strncmp( pusrdata, "GET / ", 6 ) == 0 ) {
    os_sprintf( data, "<h1>hello</h1>" );
  } else if ( strncmp( pusrdata, "GET /on ", 8 ) == 0 ) {
    gpio_output_set(0, BIT2, BIT2, 0);
    os_sprintf( data, "<h1>on</h1>" );
  } else if ( strncmp( pusrdata, "GET /off ", 9 ) == 0 ) {
    gpio_output_set(BIT2, 0, BIT2, 0);
    os_sprintf( data, "<h1>off</h1>" );
  } else if ( strncmp( pusrdata, "GET /get ", 9 ) == 0 ) {
    uint16 value = system_adc_read();
    char tmp[32];
    os_sprintf( tmp, "<h1>%d</h1>", value );
    os_sprintf( data, tmp );
  } else if ( strncmp( pusrdata, "GET /http ", 10 ) == 0 ) {
    http_get(system_adc_read());
    os_sprintf( data, "http" );
  }
  espconn_sent( pesp_conn, data, strlen(data) );

  os_printf("receive    | %s\n", pusrdata);
  espconn_disconnect(arg);
}

static void  webserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;
    print_ip( "reconnect", pesp_conn->proto.tcp );
}

static void  webserver_discon(void *arg)
{
    struct espconn *pesp_conn = arg;
    print_ip( "disconnect", pesp_conn->proto.tcp );
}

LOCAL void tcpConnect(void *arg) {

  struct espconn *pesp_conn = arg;
  print_ip( "connect", pesp_conn->proto.tcp );

  espconn_regist_recvcb(pesp_conn, webserver_recv);
  espconn_regist_reconcb(pesp_conn, webserver_recon);
  espconn_regist_disconcb(pesp_conn, webserver_discon);

}

LOCAL void setupTCP() {

  tcp.local_port = 80 ;
  conn1.type = ESPCONN_TCP ;
  conn1.state = ESPCONN_NONE ;
  conn1.proto.tcp = &tcp ;

  espconn_regist_connectcb(&conn1, tcpConnect);

  espconn_accept(&conn1);

  os_printf("Listening for data\n");
  os_timer_disarm(&blink_led_timer);
  gpio_output_set(0, BIT2, BIT2, 0);

  http_polling();
}

LOCAL void wifiEventHandler(System_Event_t *event) {
  switch (event->event) {
    case EVENT_STAMODE_GOT_IP:
      os_printf("IP: %d.%d.%d.%d\n", IP2STR(&event->event_info.got_ip.ip));
      //setupUDP();
      setupTCP();
    break;
    default:
      os_printf("WiFi Event: %d\n", event->event);
  }
}

void user_rf_pre_init(void) {
}

void some_timerfunc(void *arg)
{
    //Do blinky stuff
    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    {
        //Set GPIO2 to LOW
        gpio_output_set(0, BIT2, BIT2, 0);
    }
    else
    {
        //Set GPIO2 to HIGH
        gpio_output_set(BIT2, 0, BIT2, 0);
    }
}

void user_init(void) {
 gpio_init();
 PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
 gpio_output_set(0, BIT2, BIT2, 0);
 os_timer_disarm(&blink_led_timer);
 os_timer_setfn(&blink_led_timer, (os_timer_func_t *)some_timerfunc, NULL);
 os_timer_arm(&blink_led_timer, 1000, 1);

 uart_div_modify(0, UART_CLK_FREQ / BIT_RATE_115200);

 system_init_done_cb(initDone);
 wifi_set_event_handler_cb(wifiEventHandler);
}
