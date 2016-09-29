### `system_init_done_cb`
Call this API in user_init to register a system-init-done callback.

### `wifi_set_event_handler_cb`
Register Wi-Fi event handler

```c
typedef struct {
       struct ip_addr ip;
       struct ip_addr mask;
       struct ip_addr gw;
} Event_StaMode_Got_IP_t;

typedef union {
   Event_StaMode_Connected_t //connected;
   Event_StaMode_Disconnected_t //disconnected;
   Event_StaMode_AuthMode_Change_t //auth_change;
   Event_StaMode_Got_IP_t //       got_ip;
   Event_SoftAPMode_StaConnected_t //sta_connected;
   Event_SoftAPMode_StaDisconnected_t //sta_disconnected;
   Event_SoftAPMode_ProbeReqRecved_t //ap_probereqrecved;
} Event_Info_u;
typedef struct _esp_event {
    uint32 event;
    Event_Info_u event_info;
} System_Event_t;
```

### `wifi_set_opmode_current`
Sets WiFi working mode as station, soft-AP or station+soft-AP, and won’t save to flash

- `NULL_MODE` – Null mode. (0)
- `STATION_MODE` – Station mode. (1) - Wi-Fi Clients
- `SOFTAP_MODE` – Soft Access Point (AP) mode. (2) - "Hubs", e.g Router
- `STATIONAP_MODE` – Station + Soft Access Point (AP) mode. (3)

### `wifi_station_set_config_current`
Set WiFi station configuration, won’t save to flash
Note:
- This API can be called only if ESP8266 station is enabled.
- If `wifi_station_set_config_current` is called in `user_init` , there is no need to call `wifi_station_connect` after that, ESP8266 will connect to router automatically; otherwise, need `wifi_station_connect` to connect.
- In general, `station_config.bssid_set` need to be 0, otherwise it will check bssid which is the MAC address of AP.

```c
struct station_config {
	uint8 ssid[32];
	uint8 password[64];
	uint8 bssid_set;
	uint8 bssid[6];
};
```
Note:
BSSID as MAC address of AP, will be used when several APs have the same SSID.
If station_config.bssid_set==1 , station_config.bssid has to be set, otherwise, the connection will fail.
In general, station_config.bssid_set need to be 0.