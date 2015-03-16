#include "rip.h"
#include<stdio.h>

int main() {
//defining localhost

	inet_pton(AF_INET, "127.0.0.1", &(localhost));
//	printf("%s\n",inet_ntoa(localhost));
	inet_pton(AF_INET, "192.10.0.1", &(dummy1));

	inet_pton(AF_INET, "210.10.0.1", &(dummy2));
	//printf("%s\n",inet_ntoa(dummy));
	rte_head = alloc_route_entry();




	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf("\nSocket creation error \n");
		exit(-1);
	}
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(RIP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));

	/* bind socket */
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof my_addr)) {
		printf("\nSocket bind error in input()\n");
		perror("");
		exit(0);
	}
	fill_route_table();

	print_route_table();

	//Once router initializes for the first time,a REQUEST Packet is constructed ,i.e sent to all neighbor
	request_routing_table();


//	input_processor();

	return 0;
}
