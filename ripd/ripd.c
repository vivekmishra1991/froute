#include "rip.h"
#include<stdio.h>
#include<pthread.h>

int main(int argc, char **argv) {
//defining localhost
	//initscr();
	//printf("%s\n", argv[1]);
	inet_pton(AF_INET, argv[1], &(nbr));
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


	//Creating the netlink socket
	nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

		if (!nl_fd) {
			printf("Error socket\n");
			return (0);
		}

		rtn=bind(nl_fd, (struct sockaddr*) &sock_addr, sizeof(sock_addr));
		if (!rtn) {
			printf("Error socket bind\n");
			return (0);
		}



	//Creating "timer thread" that checks routes & deletes if broken route is encountered or timeout expires

	init();

	if (pthread_create(&rt_checker_thread, NULL, rt_checker, NULL)) {
		fprintf(stderr, "Error creating trt_checker_thread\n");
		return 1;
	}


	if (pthread_create(&regular_update_thread, NULL, send_regular_update, NULL)) {
		fprintf(stderr, "Error creating regular_update_thread\n");
		return 1;
	}

	while (1) {
		input_processor();
	}

	pthread_exit(NULL);
	return 0;
}
