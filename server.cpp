#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <set>

// xor
char* endecrypt(char* text, int key){
    int len = strlen(text);
    char* text_cpy = (char*)malloc(sizeof(char)*len+1);   
    strcpy(text_cpy, text);

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

int prime(int num){
    int is_prime = 1;

    for(int i = 2; i < num; i++){
        if(num % i == 0){
            is_prime = 0;
        }
    }

    return is_prime;
}

int generate_prime(){
    int num = rand() % 1000000 + 10000000;
    int is_prime = 0;

    while(is_prime == 0){
        num++;
        is_prime = prime(num);
    }

    return num;

}

// https://www-users.cse.umn.edu/~garrett/coding/Overheads/11_sunze_proots.pdf
int calculate_prim_root(int prime){

    std::set<int> prime_divisors;

    int prime_cpy = prime-1;

    while(prime_cpy > 1){
        for(int i = 2; i <= prime-1; i++){
            if (prime_cpy % i == 0){
                prime_divisors.insert(i);
                prime_cpy = prime_cpy / i;
                break;
            }
        }
    }

    for(int i = 2; i < prime; i++){
        int is_root = 1;
        for(auto div : prime_divisors){
            if(power(i, ((prime-1)/div), prime) == 1){
                is_root = 0;
            }
        }
        if(is_root == 1){
            return i;
        }
    }

    return -1;

}


int main(){

    srand(time(NULL));

    int server_fd, client_fd;
    struct sockaddr_in server_addr;

    FILE *f;

    f = fopen("index.html", "r");

    if(f == NULL){
        printf("html file not found");
    }

    fseek(f, 0L, SEEK_END);
    int message_len = ftell(f);
    fseek(f, 0L, SEEK_SET);

    char* message = (char*)malloc(message_len+1);

    fread(message, 1, message_len, f);

    message[message_len] = '\0';

    fclose(f);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if((bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0){
        perror("socket bind failed");
        exit(EXIT_FAILURE);
    }

    if((listen(server_fd, 5)) < 0){
        perror("socket listen failed");
        exit(EXIT_FAILURE);
    }

    printf("server socket created.\n");

    while((client_fd = accept(server_fd, NULL, 0)) > 0){
        printf("accepted connection\n");
        printf("generating prime\n");
        int prime = generate_prime();
        printf("prime: %d\n", prime);
        printf("calculating primitive root\n");
        int prim_root = calculate_prim_root(prime);
        printf("prim root: %d\n", prim_root);

        int payload[2] = {prime, prim_root};

        printf("sending prime/root to client\n");

        send(client_fd, payload, sizeof(int)*2, 0);

        int secret = rand() % 1000000;
        int server_pub_key = power(prim_root, secret, prime);
        int client_pub_key;

        printf("waiting for clients pub key\n");

        recv(client_fd, &client_pub_key, sizeof(int), 0);
        printf("client pub key: %d\n", client_pub_key);

        printf("sending server pub key\n");

        send(client_fd, &server_pub_key, sizeof(int), 0);
        printf("server pub key: %d\n", server_pub_key);

        secret = power(client_pub_key, secret, prime);

        printf("secret: %d\n", secret);

        printf("sending html to client\n");
        char* encrypted_message = endecrypt(message, secret);
        send(client_fd, encrypted_message, message_len, 0);
        printf("message:%s\n", encrypted_message);

        free(encrypted_message);
        
    }

    shutdown(server_fd, SHUT_RDWR);

    free(message);

    exit(EXIT_SUCCESS);

}