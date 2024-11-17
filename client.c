// Client side for Project LTM: File Sharing
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h> 
#include <pthread.h>
#include "./communication_code.h"

#define BUFF_SIZE 100

// Define function
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option);
int readWithCheck(int sock, char buff[BUFF_SIZE], int length);
void sendCode(int sock, int code);
int menu1();
int menu2();
void navigation(int sock);
void signUp(int sock);
int signIn(int sock);
void clearBuff();

// Function check send and receive data
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option) {
    int sendByte = 0;
    sendByte = send(sock, buff, length, option);
    if (sendByte > 0) {
        
    }
    else {
        close(sock);
        printf("Connection is interrupted.\n");
        exit(0);
    }
}

int readWithCheck(int sock, char buff[BUFF_SIZE], int length) {
    int recvByte = 0;
    recvByte = recv(sock, buff, length, 0);
    if (recvByte > 0) {
        return recvByte;
    }
    else {
        printf("Client recv error\n");
        close(sock);
        exit(0);
    }
}

void sendCode(int sock, int code){
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	sendWithCheck(sock , codeStr , strlen(codeStr) + 1 , 0 );
}

void clearBuff() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

//=============== MAIN ==================
int main(int argc, char *argv[]) {
    pthread_t thread;

    if (argc != 3) {
        printf("Please input IP address and port number\n");
        return 0;
    }

    // IP_address of server
    // Port_number
    char *ip_address = argv[1];
    char *port_number = argv[2];
    int port = atoi(port_number);
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Connecting
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 address from text to binary form
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Connection failed \n");
        return -1;
    }

    do {
        navigation(sock);
    } while(1);

    // Close
    close(sock);

    return 0;
}

int menu1() {
    int choice, catch;
    char err[10];
    printf("\n\n");
    printf("==================== FILE SHARING ===================\n");
    printf("1. Sign up\n");
    printf("2. Sign in\n");
	printf("==========================================================\n");
    printf("=> Enter your choice: ");
    catch = scanf("%d",&choice);

    printf("\n\n");

    if (catch > 0) {
        return choice;
    }
    else {
        fgets(err, 10, stdin);
		err[strlen(err)-1] = '\0';
		printf("\"%s\" is not allowed!\n",err);
		return -1;
    }
}

int menu2() {
    int choice, catch;
    char err[10];
    printf("\n\n");
	printf("========================= GROUPS ========================\n");
	printf("1. Create group\n");
    printf("2. Join group\n");
    printf("3. Access joined group\n");
	printf("4. Notifications\n");
    printf("5. Logout\n");
	printf("=========================================================\n");
    printf("=> Enter your choice: ");
    catch = scanf("%d",&choice);

    printf("\n\n");

    if(catch > 0) return choice;
	else {
		fgets(err, 10, stdin);
		err[strlen(err)-1] = '\0';
		printf("\"%s\" is not allowed!\n",err);
		return -1;
	}
}

// SIGN UP CLIENT
void signUp(int sock) {
    char username[50], password[50], buff[BUFF_SIZE];

    sendCode(sock, REGISTER_REQUEST);
    readWithCheck(sock, buff, BUFF_SIZE);
    printf("========================= SIGNUP ========================\n");

    clearBuff();
    while(1) {
        printf("Enter username: ");
        fgets(username, 50, stdin);
        while(strlen(username) <= 0 || username[0] == '\n') {
            printf("Username is empty!\n");
            printf("Enter username: ");
            fgets(username, 50, stdin);
        }
        sendWithCheck(sock, username, sizeof(username), 0);

        readWithCheck(sock, buff, BUFF_SIZE);
        if(atoi(buff) == EXISTENCE_USERNAME) {
            printf("Username is exist! Try another one\n");
        }
        else {
            break;
        }
    }

    printf("Enter password: ");
    fgets(password, 50, stdin);
    while(strlen(password) <= 0 || password[0] == '\n') {
        printf("Password is empty!\n");
        printf("Enter password: ");
        fgets(password, 50, stdin);
    }
    sendWithCheck(sock, password, sizeof(password), 0);

    readWithCheck(sock, buff, BUFF_SIZE);
    if (atoi(buff) != REGISTER_SUCCESS) {
        printf("Sytem is under maintenance!\n");
    }
    else {
        printf("Register account successfully!\n");
    }
}

// SIGN IN CLIENT
int signIn(int sock) {
    char username[50], password[50], buff[BUFF_SIZE];

    sendCode(sock, LOGIN_REQUEST);
    readWithCheck(sock, buff, BUFF_SIZE);
    printf("========================= SIGNIN ========================\n");

    clearBuff();
    while(1) {
        printf("Enter username: ");
        fgets(username, 50, stdin);
        while(strlen(username) <= 0 || username[0] == '\n') {
            username[0] = '\0';
            printf("Username is empty!\n");
            printf("Enter username: ");
            fgets(username, 50, stdin);
        }
        sendWithCheck(sock, username, sizeof(username), 0);

        readWithCheck(sock, buff, BUFF_SIZE);
        if (atoi(buff) == NON_EXISTENCE_USERNAME) {
            printf("Username is not exist!\n");
        }
        else {
            break;
        }
    }

    printf("Enter password: ");
    fgets(password, 50, stdin);
    while(strlen(password) <= 0 || password[0] == '\n') {
        printf("Password is empty!\n");
        printf("Enter password: ");
        fgets(password, 50, stdin);
    }
    sendWithCheck(sock, password, sizeof(password) + 1, 0);
    readWithCheck(sock, buff, BUFF_SIZE);
    if (atoi(buff) != LOGIN_SUCCESS) {
        printf("Login failed!\n");
        return 0;
    }
    else {
        return 1;
    }
}

void navigation(int sock) {
    int z1, z2;
    char buffer[100], code[10], username[50], password[50];
    z1 = menu1();

    switch(z1) {
        case 1:
            signUp(sock);
            break;
        case 2:
            if (signIn(sock) == 1) {
                z2 = menu2();
                break;
            }
        default:
            break;
    }
}