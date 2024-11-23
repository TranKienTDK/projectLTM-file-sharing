// Server side for Project LTM: File Sharing
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include "communication_code.h"
#include "linked_list.h"
#include <time.h>
#include <pthread.h>

#define BUFF_SIZE 100
singleList users, groups;

// Define function
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option);
int readWithCheck(int sock, char buff[BUFF_SIZE], int length);
void sendCode(int sock, int code);
void signUp(int sock, singleList *users);
int signIn(int sock, singleList users, user_struct **loginUser);
void readUserFile(singleList *users);
int checkExistence(int type, singleList list, char string[50]);
void* findByName(int type, singleList list, char string[50]);
void * handleThread(void *my_sock);

// Function check send and receive data
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option) {
	int sendByte = 0;
	sendByte = send(sock, buff, length, option);
	if(sendByte > 0){
		
	}
    else {
		close(sock);
		pthread_exit(0);
	}
}

int readWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int recvByte = 0;
	recvByte = recv(sock, buff, length, 0);

	if(recvByte > 0) {
		return recvByte;
	}
    else { 
		printf("server recv error\n");
		close(sock);
		pthread_exit(0);
	}
}

void sendCode(int sock, int code){
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	printf("-->Response: %s\n", codeStr);
	sendWithCheck(sock , codeStr , strlen(codeStr) + 1 , 0); 
}

int checkExistence(int type, singleList list, char string[50]) {
    switch(type) {
        case 1:
            // Check user
            {
                int i = 0;
                list.cur = list.root;
                while (list.cur != NULL) {
                    i++;
                    if (strcmp(((user_struct*)list.cur->element)->user_name, string) != 0) {
                        list.cur = list.cur->next;
                    }
                    else {
                        return 1;
                    }
                }
                return 0;
            }
            break;
        case 2:
            // Check group
            break;
        case 3:
            // Check file
            break;
        default:
            printf("Type is invalid! (1,2 or 3)\n");
            break;    
    }
}

void* findByName(int type, singleList list, char string[50]) {
    switch(type) {
        case 1:
            // Check user
            {
                int i = 0;
			    list.cur = list.root;
			    while (list.cur != NULL) {
                    i++;
                    if(strcmp(((user_struct*)list.cur->element)->user_name,string) != 0) {
                        list.cur = list.cur->next;
                    }
                    else {
                        return list.cur->element;
                    }   
                }
			    return NULL; 
            }
            break;
        case 2:
            // Check group
            break;
        case 3:
            // Check file
            break;
        default:
            printf("Type is invalid! (1,2 or 3)\n");
            break; 
    }
}

void readUserFile(singleList* users) {
    char username[50], password[50], group_name[50];
    int status, count_group;
    FILE * f = fopen("./storage/user.txt","r");

    if (f == NULL) {
        perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
    }

    while(1) {
        singleList groups;
		createSingleList(&groups);

		char c = fgetc(f);
    	if (c != EOF) { 
			int res = fseek( f, -1, SEEK_CUR );
		}
        else {
        	break;
		}

        fgets(username, 50, f);
		username[strlen(username) -1 ] = '\0';

		fgets(password, 50, f);
		password[strlen(password) -1 ] = '\0';

		fscanf(f,"%d\n", &status);

		if(fscanf(f, "%d\n", &count_group) > 0){
			for(int i = 0; i < count_group; i++){
				fgets(group_name, 50, f);
				group_name[strlen(group_name) -1]  = '\0';
				insertEnd(&groups, strdup(group_name));
			}
		}

		user_struct *user = (user_struct*)malloc(sizeof(user_struct));
		strcpy(user->user_name, username);
		strcpy(user->password, password);
		user->status = status;
		user->joined_groups = groups;
		user->count_group = count_group;

		insertEnd(users, user);

	}
    fclose(f);
}

//=============== MAIN ==================
int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Please input port number\n");
        return 0;
    }
    char *port_number = argv[1];
	int port = atoi(port_number);
	int opt = 1;
	int server_fd, new_socket; 
	struct sockaddr_in address;
	int addrlen = sizeof(address);

    // Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( port ); 
	
	// Forcefully attaching socket to the port 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) { 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

    //============================Start to communicate with client=====================================
    char buff[100];

    createSingleList(&users);
    createSingleList(&groups);

    readUserFile(&users);
    while(1) {
		pthread_t tid; 

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		printf("New request from sockfd = %d.\n",new_socket);	
        pthread_create(&tid, NULL, &handleThread, &new_socket);
    }
	close(server_fd);
	return 0;  
}

void signUp(int sock, singleList *users) {
    char buff[BUFF_SIZE];
    int size;
    char username[50], password[50];
    singleList groups;

    sendCode(sock, REGISTER_REQUEST);

    // Nhận cả username và password trong một lần
    size = readWithCheck(sock, buff, BUFF_SIZE - 1);
    buff[size] = '\0'; // Đảm bảo chuỗi kết thúc
    printf("%s %ld\n", buff, strlen(buff));

    // Kiểm tra định dạng trước khi tách
    char *separator = strchr(buff, '|');
    if (separator == NULL) {
        printf("Invalid input format. Expected format: username|password\n");
        sendCode(sock, INVALID_FORMAT);
        return;
    }

    // Tách username
    size_t username_len = separator - buff;
    if (username_len >= sizeof(username)) {
        printf("Username too long!\n");
        sendCode(sock, INVALID_FORMAT);
        return;
    }
    strncpy(username, buff, username_len);
    username[username_len] = '\0'; // Đảm bảo chuỗi kết thúc

    // Tách password
    char *password_start = separator + 1;
    size_t password_len = strlen(password_start);
    if (password_len >= sizeof(password)) {
        printf("Password too long!\n");
        sendCode(sock, INVALID_FORMAT);
        return;
    }
    strncpy(password, password_start, password_len);
    password[password_len] = '\0'; // Đảm bảo chuỗi kết thúc

    printf("username: '%s', password: '%s'\n", username, password);

    // Kiểm tra sự tồn tại của username
    if (checkExistence(1, *users, username) == 1) {
        sendCode(sock, EXISTENCE_USERNAME);
    } else {
        sendCode(sock, REGISTER_SUCCESS);

        // Tạo thông tin tài khoản mới
        createSingleList(&groups);
        user_struct *user = (user_struct *)malloc(sizeof(user_struct));
        strcpy(user->user_name, username);
        strcpy(user->password, password);
        user->count_group = 0;
        user->status = 1;
        user->joined_groups = groups;
        insertEnd(users, user);

        printf("Account created successfully: username = %s\n", username);
    }
}

// SIGN IN SERVER
int signIn(int sock, singleList users, user_struct **loginUser) {
    char username[50], password[50], buff[BUFF_SIZE];

    sendCode(sock, LOGIN_REQUEST);

    // Nhận cả username và password trong một lần
    readWithCheck(sock, buff, BUFF_SIZE);
    buff[strlen(buff)] = '\0'; // Đảm bảo chuỗi được kết thúc đúng

    // Phân tách username và password từ buff
    sscanf(buff, "%[^|]|%s", username, password); // Giả sử định dạng "username|password"
    username[strlen(username)] = '\0';
    password[strlen(password)] = '\0';

    printf("Received username: '%s', password: '%s'\n", username, password);

    // Kiểm tra sự tồn tại của username
    if (checkExistence(1, users, username) == 0) {
        sendCode(sock, NON_EXISTENCE_USERNAME);
        return 0;
    }

    // Tìm user trong danh sách
    *loginUser = (user_struct *)(findByName(1, users, username));
    if (strcmp((*loginUser)->password, password) == 0) {
        sendCode(sock, LOGIN_SUCCESS);
        return 1;
    }

    // Nếu password không khớp
    sendCode(sock, INCORRECT_PASSWORD);
    return 0;
}

void * handleThread(void *my_sock) {
    int new_socket = *((int *)my_sock);
	int REQUEST;
	char buff[BUFF_SIZE];
	user_struct *loginUser = NULL;

    while(1) {
        readWithCheck(new_socket, buff, 100);
        REQUEST = atoi(buff);
        switch(REQUEST) {
            case REGISTER_REQUEST:
            printf("REGISTER_REQUEST\n");
                signUp(new_socket, &users);
                saveUsers(users);
                break;
            case LOGIN_REQUEST:
                printf("LOGIN_REQUEST\n");
                if(signIn(new_socket, users, &loginUser) == 1) {
                    printf("Login successfully!\n");
                }
                else {
                    printf("Login failed!\n");
                }
                break;
            default:
                break;  
        }
    }
    close(new_socket);
}