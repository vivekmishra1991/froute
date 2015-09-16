/*
 * rt_netlink.c
 *Tries to create netlink socket for communicating with kernel's routing table
 */

#include "rip.h"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

struct sockaddr_nl dst_addr;
struct nlmsghdr *nlh = 0;
struct iovec iov;
struct msghdr msg;
struct rtattr *rtap;

#define MAX_PAYLOAD 1024

// buffer to hold the RTNETLINK request
struct {
	struct nlmsghdr nl;
	struct rtmsg rt;
	char buf[8192];
} req;

int add_kernel_rt(struct route_entry *rte) {

	//printf("Lets add a route...!\n");
	int rtl;

	//char dsts[24] = "134.102.155.14";
	int ifcn = 2, pn = 32;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.nl_family = AF_NETLINK;
	sock_addr.nl_pid = getpid();
	sock_addr.nl_groups = 0;
	sock_addr.nl_pad = 0;

	nlh = (struct nlmsghdr*) malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	//Lets create the request
	// attributes of the route entry

	// Initialize RTNETLINK request buffer
	memset(&req, 0, sizeof(req));

	// compute the initial length of the
	// service request
	rtl = sizeof(struct rtmsg);

	// add first attrib:
	// set destination IP addr and increment the
	// RTNETLINK buffer size
	rtap = (struct rtattr *) req.buf;
	rtap->rta_type = RTA_DST;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	//((char *) rtap) + sizeof(struct rtattr)=(char *)(&(rte->destination.s_addr));
	//inet_pton(AF_INET, dsts, ((char *) rtap) + sizeof(struct rtattr));
	memcpy((void*) ((char *) rtap + sizeof(struct rtattr)), &rte->destination,
			sizeof(rte->destination));
	rtl += rtap->rta_len;

	// add second attrib:
	// set ifc index and increment the size
	rtap = (struct rtattr *) (((char *) rtap) + rtap->rta_len);
	rtap->rta_type = RTA_OIF;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	memcpy(((char *) rtap) + sizeof(struct rtattr), &ifcn, 4);
	rtl += rtap->rta_len;

	// setup the NETLINK header
	req.nl.nlmsg_len = NLMSG_LENGTH(rtl);
	req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
	req.nl.nlmsg_type = RTM_NEWROUTE;

	// setup the service header (struct rtmsg)
	req.rt.rtm_family = AF_INET;
	req.rt.rtm_table = RT_TABLE_MAIN;
	req.rt.rtm_protocol = RTPROT_STATIC;
	req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
	req.rt.rtm_type = RTN_UNICAST;
	// set the network prefix size
	req.rt.rtm_dst_len = pn;

	// create the remote address
	// to communicate
	memset(&dst_addr, 0, sizeof(dst_addr));
	dst_addr.nl_family = AF_NETLINK;
	dst_addr.nl_pid = 0;	// Kernel
	dst_addr.nl_groups = 0;

	// initialize & create the struct msghdr supplied
	// to the sendmsg() function
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *) &dst_addr;
	msg.msg_namelen = sizeof(dst_addr);

	// place the pointer & size of the RTNETLINK
	// message in the struct msghdr
	iov.iov_base = (void *) &req.nl;
	iov.iov_len = req.nl.nlmsg_len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	// send the RTNETLINK message to kernel
	sendmsg(nl_fd, &msg, 0);

	return (0);
}
