#include<stdio.h>
#include<stdlib.h>
#include <netinet/in.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <curses.h>

#include "rip.h"
#include "logger.h"

/*
 * Few Important declerations
 */

/*
 * Methods related to routing table
 */

//Initialises router
void init() {
	fill_route_table();

	//TODO make a better feature for this
	print_route_table();

	//Once router initializes for the first time,a REQUEST Packet is constructed ,i.e sent to all neighbor
	//request_routing_table();

}

// routine to add a route in the routing table
int add_route_entry(struct in_addr destination, uint32_t metric,
		struct in_addr nexthop) {
	LOG_DEBUG("\nIN alloc_route_entry\n");
	struct route_entry *rte = NULL;

	LOG_DEBUG("\nOUT alloc_route_entry\n");

	return -1;
}

// routine to set a route entery  with routing values
struct route_entry* set_route_entry(struct route_entry *rte,
		struct in_addr dest, uint32_t metric, struct in_addr nexthop) {
	LOG_DEBUG("\nIN set_route_entry\n");

	//	unsigned long old_timeout = rte->timeout;

	rte->destination.s_addr = dest.s_addr;
	rte->nexthop.s_addr = nexthop.s_addr;
	rte->metric = metric;
	rte->routechange = 1;
	/*
	 if ((now - old_timeout) > TIMEOUT_INTERVAL && old_timeout != 0)
	 rte->garbage = now;
	 */
	rte->timeout = time(0);

	LOG_DEBUG("\nOUT set_route_entry\n");
	return rte;
}

//allocate a rip_entry memory
struct route_entry* alloc_route_entry() {

	LOG_DEBUG("\nIN alloc_route_entry\n");
	struct route_entry *node;

	node = (struct route_entry *) malloc(sizeof(struct route_entry));
	if (node == NULL) {
		WARN("\nError allocating route entry\n");
		exit(-1);
	}
	LOG_DEBUG("\nOUT alloc_route_entry\n");

	return node;
}
//Show Entire route Table
void print_route_table() {

	//clear();
	//refresh();
	struct route_entry *temp_node = 0; //used for RT traversal
	int rt_count = 1;
	if (rte_head != NULL)
		temp_node = rte_head; // making temp_node the head of the Routing Table Linked List
	else
		return;
	printf("\nSr\tDestination\tMetric\tNexthop\tTimeout\tGarbage");

	do {
		//Printing each route table entry

		printf("\n%d\t%s\t%d\t", rt_count++, inet_ntoa(temp_node->destination),
				temp_node->metric);
		printf("%s\t%d\t", inet_ntoa(temp_node->nexthop), temp_node->timeout);

		printf("%d", temp_node->garbage);

		temp_node = temp_node->next;
	} while (temp_node != NULL);

}

//Fill Routing Table , Manually adds routes to the routing table

int fill_route_table() {

	LOG_DEBUG("\nIN fill_route_entry\n");

	int count = 0, metric = 2;
	char ch = 'y';
	struct route_entry *temp;

	temp = rte_head;

	if (rte_head != NULL) {

		rte_head = set_route_entry(rte_head, localhost, 13, dummy1);
	}
	//get y/n before entering the loop
	scanf("%c%*c", &ch);

	while (ch != 'n') {

		if (validate_route_entry(metric) != -1) {
			temp->next = alloc_route_entry();
			temp = temp->next;
			temp = set_route_entry(temp, dummy2, count, dummy1);
			count = count + 1;
		}
		//	flush(stdout);
		scanf("%c%*c", &ch); //loop terminator varible

	}
	LOG_DEBUG("\nOUT fill_route_entry\n");

	return count;

}

//Validate Route Entry
int validate_route_entry(int metric) {

	LOG_DEBUG("\nIN validate_route_entry\n");

	//-- Till metric is supported

	if (metric >= INF || metric < 0)
		return -1;
	LOG_DEBUG("\nOUT validate_route_entry\n");

	return 0;
}

/*
 * #####################################################################
 *  Function Related to rip protocols like msg sent & received via rip port
 */

int sendmesg(unsigned short int addrfamily, int port, struct in_addr send_to_ip,
		int sock, char *message, int len) {

	//message = "Hello world , my beautiful world";
	struct sockaddr_in send_to;
	int num_bytes = 0;
	send_to.sin_family = addrfamily;
	send_to.sin_port = htons(port);
	send_to.sin_addr.s_addr = send_to_ip.s_addr;
	memset(send_to.sin_zero, '\0', sizeof(send_to.sin_zero));
	if ((num_bytes = sendto(sockfd, message, len, 0,
			(struct sockaddr *) &send_to, sizeof(struct sockaddr))) == -1) {
		WARN("\nsend socket error \n");
		perror("");
		return -1;
	}
	return num_bytes;
}

//This method runs inside an  independent thread, that continuously reads from rip port for incoming RIP packets ,
//and call appropriate handler(REQUEST/RESPONSE) accordingly
void input_processor() {
	struct sockaddr_in from;
	socklen_t fromsize = sizeof(from);
	bzero(&from, sizeof(from));
	char msgbuf[1024];
	memset(msgbuf, '\0', 1024);

	LOG_DEBUG("\nIN input_processor\n");

	//If valid message , call appropriate handler as per the command

	recvfrom(sockfd, msgbuf, 1024, 0, (struct sockaddr*) &from, &fromsize);

	if (!is_valid_message(from))
		return;

	//printf("%s\n",inet_ntoa(from.sin_addr));

	//Sending incoming packet to its respective handler
	if ((int) msgbuf[0] == REQUEST)          //msgbuf[0] stores the command byte
		request_handler(from.sin_addr, msgbuf);
	else if ((int) msgbuf[0] == RESPONSE)
		response_handler(from.sin_addr, msgbuf);

	LOG_DEBUG("\nOUT input_processor\n");

}

/*
 * This check validity of the source of rip packet
 */
int is_valid_message(struct sockaddr_in from) {

	if (!is_neighbour(from.sin_addr) || ntohs(from.sin_port) != RIP_PORT) {
		WARN("INVALID ADDRESS");
		return 0;
	}
	return 1;
}

/*
 * This checks , whether given route exist in the routing table
 */

int route_exist(struct rip_entry *re, struct route_entry** prev_rte) {
	struct route_entry* rte;

	rte = rte_head;
	while (rte != NULL) {

		if (rte->destination.s_addr == re->ipaddr.s_addr) {
			*prev_rte = rte;
			return 1;
		} else {
			*prev_rte = rte;
			rte = rte->next;
		}
	}

	return 0;

}

/*
 *	Checks the given source addr passed , is neighbour of the host router
 */
int is_neighbour(struct in_addr from_addr) {
	return 1;

}

/*
 * Checks sanity checks for each rip entry
 */

int is_valid_rip_entry(struct rip_entry* re) {
	if (re->metric > INF || re->metric < 0)
		return 0;
	return 1;
}

/*
 * PACKET & HEADER CONSTRUCTION
 */

//This function constructs a Request Packet , Requesting entire routing table
void request_routing_table() {

	struct rip_header rh;
	struct rip_entry re;
	char message[1024] = { '\0' };
	char *msg_ptr;

	//construct requesting message header
	rh.command = REQUEST;
	rh.opn1 = 0;
	rh.version = VERSION;

	msg_ptr = message;
	memset(msg_ptr, '\0', 1024);
	msg_ptr = copy_header_to_packet(rh, msg_ptr);

	//Set special rte entry to ask for entire routing table , metric=INF,AF=0
	/*
	 set_rip_entry(&re, 0, localhost, INF);

	 msg_ptr = copy_rip_entry_to_packet(re, msg_ptr);
	 */

	//Testing
	set_rip_entry(&re, 1, localhost, 0);
	msg_ptr = copy_rip_entry_to_packet(re, msg_ptr);

	set_rip_entry(&re, 1, dummy1, 0);
	msg_ptr = copy_rip_entry_to_packet(re, msg_ptr);
	set_rip_entry(&re, 1, dummy2, 0);
	msg_ptr = copy_rip_entry_to_packet(re, msg_ptr);

	sendmesg(AF_INET, RIP_PORT, localhost, sockfd, message, 64);

//	input_processor();
}

char * copy_header_to_packet(struct rip_header rh, char* msg_ptr) {

	memcpy(msg_ptr, (void*) &rh, sizeof(rh));
	msg_ptr += sizeof(rh);

	return msg_ptr;
}

struct rip_entry* set_rip_entry(struct rip_entry *re, uint16_t addr_fm,
		struct in_addr ipaddr, uint32_t metric) {

	re->addr_fm = addr_fm;
	re->ipaddr.s_addr = ipaddr.s_addr;
	if (metric >= 0)
		re->metric = metric;
	re->zero1 = 0;
	re->zero2 = 0;
	re->zero3 = 0;
	return re;

}

char * copy_rip_entry_to_packet(struct rip_entry re, char* msg_ptr) {

	memcpy(msg_ptr, (void*) &re, sizeof(re));
	msg_ptr += sizeof(re);

	return msg_ptr;
}

/*#####################################################################################################
 *
 * INCOMING PACKETS HANDLERS
 */

/*
 * This function handles the request coming from neighbor routers ,
 * most of the time for entire routing table (AF=0,metric=INF)
 */
void request_handler(struct in_addr from, char* msgbuf) {

	struct in_addr ips[25] = { 0 };
	char message[1024] = { '\0' };
	int num_entries;
	struct rip_entry* re;

	num_entries = get_number_of_entries(msgbuf);

	//Extract ip addresses being requested
//	extract_ip_addr(ips, msgbuf);
	msgbuf += sizeof(struct rip_header);
	re = (void*) msgbuf;

	//TODO check the first rip entry in request msgbuf , whether AF=0 && metric=INF

	construct_response_message(re, message, num_entries, from);

	sendmesg(AF_INET, RIP_PORT, from, sockfd, message, 1024);

}

void response_handler(struct in_addr from, char* msgbuf) {
//	printf("\nInside resp handler");
	printf("\n__________________________________________________");

	struct rip_entry *re;
	struct rip_header rh;
	struct route_entry *rte;
	int num_entries;
	num_entries = get_number_of_entries(msgbuf);
	msgbuf += sizeof(struct rip_header);
	re = (struct rip_entry*) msgbuf;
	while (num_entries--) {
		if (is_valid_rip_entry(re)) {    //if valid rip_entries
			re->metric = MIN(re->metric + 1, INF);

			/* if route exist then add to rt table else try to update earlier route(if exist) */
			if (!route_exist(re, &rte)) {
				//if doesn't exist ,add route
				if (re->metric != INF) { //This prevents broken links(INF metric) to be added at first place
					rte->next = alloc_route_entry();
					set_route_entry(rte->next, re->ipaddr, re->metric, from);
				}
			} else {
				set_route_entry(rte, re->ipaddr, re->metric, from);
			}

		}
		re++;

	}
	print_route_table();
	return;

}

/*#####################################################################################################
 *
 *Utility functions
 */

int get_number_of_entries(char* msgbuf) {

	struct rip_entry* re;
	int eindex = 0;      //eindex counts the number of rip entries in the packet

	msgbuf += sizeof(struct rip_header); //move buffer pointer to point to the first rip_entry

	re = (struct rip_entry*) msgbuf;
	if (re->addr_fm == 0 && re->metric == INF) //Special Packet asking for entire routing table
	{
		return 1;
	}

	while (*msgbuf != '\0') {
		eindex++;
		msgbuf += sizeof(struct rip_entry);
	}
	return eindex;
}

void extract_ip_addr(struct in_addr* ips, char* msgbuf) {
	struct rip_entry* re;

	msgbuf += sizeof(struct rip_header); //move buffer pointer to point to the first rip_entry
	while (*msgbuf != '\0') {
		re = (struct rip_entry*) msgbuf;
		*ips = re->ipaddr;
		ips++;

		msgbuf += sizeof(struct rip_entry);
	}
}

void construct_response_message(struct rip_entry* e, char* message,
		int num_entries, struct in_addr send_to) {

	struct rip_header rh;
	struct rip_entry re;
	struct route_entry *traverser;    //used to traverse entire routing table

	memset(message, '\0', 1024);
	traverser = rte_head;

	//Creating response header
	rh.command = RESPONSE;
	rh.version = VERSION;
	message = copy_header_to_packet(rh, message);

	while (num_entries--) {

		traverser = rte_head; //reset to start of Routing Table after each ip lookup

		while (traverser != NULL) {
			if (traverser->destination.s_addr == e->ipaddr.s_addr //If route exist
			&& send_to.s_addr != traverser->nexthop.s_addr) { //Searching for ips & avoiding split horizon

				set_rip_entry(&re, AF_INET, traverser->destination,
						traverser->metric);
				message = copy_rip_entry_to_packet(re, message);

				e->metric = INVALID_METRIC; 	//change metric

			}
			traverser = traverser->next;
		}

		if (e->metric != INVALID_METRIC) { //iF METRIC IS NOT CHANGED, OR ISN'T IN THE ROUTE TABLE PUT inf IN THE packet
			set_rip_entry(&re, AF_INET, e->ipaddr, INF);
			message = copy_rip_entry_to_packet(re, message);
		}
		e++;
	}

}

//Function to calc min ,

int MIN(int x, int y) {
	return x < y ? x : y;
}

/*
 ######################################################################################
 * Timer Functons like deletion process , route checker , regular updates , triggered updates
 */

/*
 * checks every route entry & finds broken links
 */
void rt_checker() {

	while (1) {   //Loops forever defending the table from invalid elements
		sleep(1);
		struct route_entry *traverser, *prev_traverser = NULL;

		if (rte_head != NULL) {
			traverser = rte_head;

			do {
				time_t now = time(0);

				if ((now - traverser->timeout) > TIMEOUT_INTERVAL) // if route's time has expired , mark it invalid
				{
					//starting garbage collection timer
					if (traverser->metric != INF) {	//if timer is not initialized yet
						traverser->garbage = now;
					}

					//Marking route as invalid , to be read by all routers around that given route has expired
					traverser->metric = INF;
					traverser->routechange = 1;
					//Now checking whether garbage collection timer has expired
					if ((now - traverser->garbage) > GARBAGE_INTERVAL) {
						printf("\nStarting Deletion Process for %s\n",
								inet_ntoa(traverser->destination));
						delete_rte(prev_traverser, traverser);
						traverser = rte_head;
						print_route_table();
					}
				}
				prev_traverser = traverser;

				if (traverser != NULL)
					traverser = traverser->next;
			} while (traverser != NULL);
		} //uppermost if ends here
	}	//Outer while loop ends here

	return;
}

//delete a route
void delete_rte(struct route_entry* prev_node, struct route_entry* node) {
	if (node == rte_head) {
		if (node->next != 0) {

			rte_head = node->next;  //Making route_head pointing to nothing
			free(node); //if only single entry in routing table , prev_node is null so just free the table
			return;
		} else {
			rte_head = 0;
			free(node);
			return;
		}
	} else {
		prev_node->next = node->next;
		free(node);
	}
	return;
}

//Sending regular updates every 30 sec to every router in periphery
void send_regular_update() {

	while (1) {

		char message[1024];
		//TODO Take all neighbour route into consideration, not just localhost
		construct_regular_message(message, localhost, rte_head);
		sleep(10);
	}
}

void construct_regular_message(char* message, struct in_addr send_to,
		struct route_entry* rt_read_pos) {

	struct rip_header rh;
	struct rip_entry re;
	struct route_entry *traverser;    //used to traverse entire routing table
	int msg_entry_count = 0;

	char * init_msg_ptr = message;
	memset(message, '\0', 1024);

	//Creating response header
	rh.command = RESPONSE;
	rh.version = VERSION;
	message = copy_header_to_packet(rh, message);

	traverser = rt_read_pos;

	while (traverser != NULL) {

		if (msg_entry_count != MAX_RIP_ENTRY_LIMIT) {

			if (send_to.s_addr != traverser->nexthop.s_addr) { // avoiding split horizon

				set_rip_entry(&re, AF_INET, traverser->destination,
						traverser->metric);
				message = copy_rip_entry_to_packet(re, message);
				msg_entry_count++;
			}
		} else {
			sendmesg(AF_INET, RIP_PORT, localhost, sockfd, init_msg_ptr, 1024);

			construct_regular_message(init_msg_ptr, send_to, traverser);
			return;

		}
		traverser = traverser->next;
	}
	sendmesg(AF_INET, RIP_PORT, localhost, sockfd, init_msg_ptr, 1024);
	return;

}

