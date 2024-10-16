#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <packet_size(bits)> <destination_ip> <spacing_ms> <num_packet_pairs>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //using the given arguments to get packet size in bytes, the destination ip address, spacing in time (in milli seconds) between successive packet-pairs
    //& total number of packet pairs to be sent
    int packet_size = atoi(argv[1]) / 8;  // Convert bits to bytes for memset since it takes bytes
    char *dest_ip = argv[2];
    int spacing = atoi(argv[3]);  // Time spacing between packet pairs in ms
    int num_packet_pairs = atoi(argv[4]);

    // Allocates memory for the packet and sets up socket descriptors
    int sockfd;
    struct sockaddr_in dest_addr;
    char *packet = (char*)malloc(packet_size);
    memset(packet, 0, packet_size);  // Initialize packet content to 0 with size same as the given packet size

    // A) CreateS a Datagram Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {    
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //setting up destination information like IP address and port number
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(8080);  // Destination port number
    if (inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr) <= 0) {
        perror("Invalid destination IP address");
        exit(EXIT_FAILURE);
    }

    // Loop to send packet pairs
    for (int i = 0; i < num_packet_pairs; i++) {
        // First packet (Packet 1)
        sprintf(packet, "%d", 2 * i + 1);  // Packet number of first packet in pair
        //B) Send data to the socket
        if (sendto(sockfd, packet, packet_size, 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1) {
            perror("sendto failed");
            exit(EXIT_FAILURE);
        }

        // Second packet (Packet 2)
        sprintf(packet, "%d", 2 * i + 2);  // Packet number of second packet in pair
        //B) Send data to the socket
        if (sendto(sockfd, packet, packet_size, 0, (const struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1) {
            perror("sendto failed");
            exit(EXIT_FAILURE);
        }

        // Wait for the specified time before sending the next pair
        usleep(spacing * 1000);  // Convert milliseconds to microseconds
    }

    free(packet);
    close(sockfd);
    return 0;
}