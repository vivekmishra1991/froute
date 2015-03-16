typedef char byte;

/*
 * rip.h
 *
 *
 */

#ifndef RIP_H_
#define RIP_H_

#include<stdint.h>
#include <netinet/in.h>

#define INF 16
#define RIP_PORT 52100//real port is 520
#define REQUEST 1
#define RESPONSE 2
#define VERSION 1
#define INVALID_METRIC 20

/**
 *struct rip_header - rip entry header

 @command-Command REQUEST or RESPONSE
 @version-rip version number 1 or 2
 @opn1-this field remains zero

 */

struct rip_header {

	unsigned char command;
	uint8_t version;
	uint16_t opn1;

};

/**
 *struct rip_entry -RTE
 *@addr_fm - address family,1 for inet4
 *@route_tag -Tags route as IGP/EGP route respectively
 *@ipaddr-Ipv4 Address of received packet in Request , to be sent back unchanged in Response.
 *@netmask -netid mask
 *@next_hop-The immediate next hop IP address to which packets to the destination specified by this route entry should be forwarded
 *@metric-cost or metric to be associated with response Sender
 */
struct rip_entry {

	uint16_t addr_fm;
	uint16_t zero1;
	struct in_addr ipaddr;
	uint32_t zero2;
	uint32_t zero3;
	uint32_t metric;

};

struct route_entry {
    struct  route_entry *next;		/*linked list pointer */
    struct in_addr destination;
	uint32_t metric;
	struct in_addr nexthop; /*next router along the path to destination */
	byte routeflag; /*route change flag */
	int timeout;
	int garbage;
};


int sockfd;
struct in_addr localhost,dummy1,dummy2;
struct route_entry* rte_head; //first entry of the routing table , keeps track of the table linked list


/*
 * Prototypes
 */
char* copy_header_to_packet(struct rip_header,char*);
struct rip_entry* set_rip_entry(struct rip_entry*,uint16_t,struct in_addr ,uint32_t);
char* copy_rip_entry_to_packet(struct rip_entry,char*);
void request_handler(struct in_addr,char*);
void response_handler(struct in_addr,char*);
int get_number_of_entries(char*);
void extract_ip_addr(struct in_addr*,char*);
void construct_response_message(struct rip_entry* ,char* ,int,struct in_addr);
void print_rip_entry(struct rip_entry*);
int  route_exist(struct rip_entry* ,struct route_entry**);
//struct rip_entry* get_first_rip_entry(char*);




#endif /* RIP_H_ */
