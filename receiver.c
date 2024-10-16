#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      
#include <arpa/inet.h>    
#include <sys/time.h>     

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *output_file = argv[1];
    FILE *fp = fopen(output_file, "w");
    if (fp == NULL) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

     // Allocates memory for the buffer and sets up socket address structs
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];
    socklen_t addr_len = sizeof(client_addr);

    // Initialize the timeval structs and memory variables
    struct timeval t1, t2;
    int prev_packet_num = -1;  // Initialize with an invalid packet number
    double prev_P = 0;         // To store the size of the previous packet
    struct timeval prev_time;  // To store the time of the previous packet

    // A) CreateS a Datagram Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //setting up server information like IP address and port number
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available address
    server_addr.sin_port = htons(8080);  // Listening port number

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // C) Read data from the socket
        double P = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (P == -1) {
            perror("recvfrom failed");
            continue;
        }
        //We used P to store the size of the incoming packet so that we can estimate rate using this value later on

        int packet_num = atoi(buffer);  // Convert buffer to an integer to read the packet number 
        gettimeofday(&t2, NULL);        // Time of packet arrival

        // Check if a previous packet exists
        if (prev_packet_num != -1) {
            // If the previous packet number was odd, and the current packet number is the next consecutive one (even)
            if (prev_packet_num % 2 == 1 && packet_num == prev_packet_num + 1) {
                // Calculate time difference in microseconds using current t2 value and prev_time value which we stored
                double delta_t = (t2.tv_sec - prev_time.tv_sec) * 1000000.0 + (t2.tv_usec - prev_time.tv_usec);
                //Multiply P value by 8 since we want it to be in bits while recvfrom gives output in bytes
                double C = prev_P * 8 / delta_t;  // Estimate C = P / (t2 - t1) whose units are bits per microsecond which is same as Megabits per second

                // Write the result to the output file and print it to terminal as well
                fprintf(fp, "First packet: %d Second packet: %d Rate: %f\n", prev_packet_num, packet_num, C);
                printf("First packet: %d Second packet: %d Rate: %f\n", prev_packet_num, packet_num, C);
            } 
        }

        // Update the previous packet information only when the current packet number is odd
        if (packet_num % 2 == 1) {
            prev_packet_num = packet_num;
            prev_P = P;
            prev_time = t2;
        }
    }

    fclose(fp);
    close(sockfd);  // Close the socket
    return 0;
}
