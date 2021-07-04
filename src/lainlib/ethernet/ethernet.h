#include <stdint.h>

/************************
 *** Team Kitty, 2021 ***
 ***    Lainlib       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

struct mac_address {
    uint8_t MAC[6];
} __attribute__((packed));

enum packet_types {
    ET_ARP = 0x0806,
    ET_IP4 = 0x0800,
    ET_IP6 = 0x86DD,
};

struct ethernet_packet {
    mac_address Dest;
    mac_address Source;
    uint16_t Type;
    uint8_t Payload[];
};

#ifdef  __cplusplus
}
#endif