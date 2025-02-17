#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

// xor
char* endecrypt(char* text, int key, int len){
    char* text_cpy = (char*)malloc(sizeof(char)*len+1);   
    for(int i = 0; i < len; i++){
        text_cpy[i] = text[i];
    }

    int key_cpy = key;
    int key_len = 0;

    while(key_cpy > 1){
        key_cpy = key_cpy / 256;
        key_len++;
    }

    key_cpy = key;
    int curr_len = 0;

    for(int i = 0; i < len; i++){
        text_cpy[i] = text_cpy[i] ^ key_cpy;
        key_cpy = key_cpy >> 8;
        curr_len++;
        if(curr_len == key_len){
            key_cpy = key;
            curr_len = 0;
        }
    }

    text_cpy[len] = '\0';

    return text_cpy;

}

// https://www.geeksforgeeks.org/modular-exponentiation-power-in-modular-arithmetic/
int power(long long x, unsigned int y, int p) 
{ 
    int res = 1;     // Initialize result 
 
    x = x % p; // Update x if it is more than or 
                // equal to p
   
    while (y > 0) 
    { 
        // If y is odd, multiply x with result 
        if (y & 1) 
            res = (res*x) % p; 
 
        // y must be even now 
        y = y>>1; // y = y/2 
        x = (x*x) % p; 
    } 
    return res; 
} 

int main(){

    int server_fd;
    struct sockaddr_in server_addr;
    char buffer[1024];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket connection failed");
        exit(EXIT_FAILURE);
    }

    if((connect(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0){
        perror("socket bind failed");
        exit(EXIT_FAILURE);
    }

    int payload[2];

    printf("waiting for prime/root\n");

    recv(server_fd, &payload, sizeof(int)*2, 0);

    int secret = rand() % 1000000;
    int client_pub_key = power(payload[1], secret, payload[0]);
    int server_pub_key;

    printf("sending client pub key to server\n");
    printf("client pub key: %d\n", client_pub_key);

    send(server_fd, &client_pub_key, sizeof(int), 0);
    recv(server_fd, &server_pub_key, sizeof(int), 0);

    printf("server pub key: %d\n", server_pub_key);

    secret = power(server_pub_key, secret, payload[0]);
    printf("secret: %d\n", secret);

    printf("recieving html from server\n");
    ssize_t recv_size = recv(server_fd, buffer, 1024, 0);
    char* decrypted_message = endecrypt(buffer, secret, recv_size);
    printf("message: %s\n", decrypted_message);

    FILE *f = fopen("recv.html", "w");

    fwrite(decrypted_message, 1, recv_size, f);

    fclose(f);
    system("xdg-open recv.html");

    free(decrypted_message);

    shutdown(server_fd, SHUT_RDWR);

    exit(EXIT_SUCCESS);

}