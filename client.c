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
int menu3(char group_name[50]);
void navigation(int sock);
void signUp(int sock);
int signIn(int sock);
void createGroup(int sock);
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

int printAvailableElements(char str[1000], char available_elements[20][50]) {
    char *token;
    int number_of_available_elements = 0;

    token = strtok(str, "+");

    while (token != NULL) {
        printf( "%d. %s\n", number_of_available_elements + 1, token );
		strcpy(available_elements[number_of_available_elements], token);
    	token = strtok(NULL, "+");
		number_of_available_elements++;
    }
    return number_of_available_elements;
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

int menu3(char group_name[50]) {
    int choice, catch;
    char err[10];
    printf("\n\n");
    printf("========================== %s ========================\n", group_name);
    printf("1. Upload\n");
    printf("2. Download\n");
    printf("3. Delete file\n");
	printf("4. Rename file\n");
    printf("5. View all files\n");
	printf("6. Approve user\n");
	printf("7. Invite user\n");
	printf("8. Kick\n");
	printf("9. View all users\n");
	printf("10. Quit group\n");
	printf("11. Back\n");
	printf("==========================================================\n");
    printf("=> Enter your choice: ");
    catch = scanf("%d", &choice);

    printf("\n\n");

    if (catch > 0) return choice;
    else {
        fgets(err, 10, stdin);
        err[strlen(err) - 1] = '\0';
        printf("\"%s\" is not allowed!\n", err);
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

    // Nhập username
    while (1) {
        printf("Enter username: ");
        fgets(username, 50, stdin);
        username[strcspn(username, "\n")] = '\0'; // Loại bỏ ký tự '\n'
        if (strlen(username) == 0) {
            printf("Username is empty! Try again.\n");
        } else {
            break;
        }
    }

    // Nhập password
    while (1) {
        printf("Enter password: ");
        fgets(password, 50, stdin);
        password[strcspn(password, "\n")] = '\0'; // Loại bỏ ký tự '\n'
        if (strlen(password) == 0) {
            printf("Password is empty! Try again.\n");
        } else {
            break;
        }
    }

    // Gửi cả username và password trong một lần
    snprintf(buff, BUFF_SIZE, "%s|%s", username, password); // Định dạng "username|password"
    printf("%send\n", buff);
    sendWithCheck(sock, buff, strlen(buff), 0);

    // Nhận phản hồi từ server
    readWithCheck(sock, buff, BUFF_SIZE);
    if (atoi(buff) == EXISTENCE_USERNAME) {
        printf("Username already exists! Try another one.\n");
    } else if (atoi(buff) == REGISTER_SUCCESS) {
        printf("Register account successfully!\n");
    } else {
        printf("System is under maintenance!\n");
    }
}


// SIGN IN CLIENT
int signIn(int sock) {
    char username[50], password[50], buff[BUFF_SIZE];

    sendCode(sock, LOGIN_REQUEST);
    readWithCheck(sock, buff, BUFF_SIZE);
    printf("========================= SIGNIN ========================\n");

    clearBuff();

    // Nhập username
    while (1) {
        printf("Enter username: ");
        fgets(username, 50, stdin);
        username[strcspn(username, "\n")] = '\0'; // Loại bỏ ký tự '\n'
        if (strlen(username) == 0) {
            printf("Username is empty! Try again.\n");
        } else {
            break;
        }
    }

    // Nhập password
    while (1) {
        printf("Enter password: ");
        fgets(password, 50, stdin);
        password[strcspn(password, "\n")] = '\0'; // Loại bỏ ký tự '\n'
        if (strlen(password) == 0) {
            printf("Password is empty! Try again.\n");
        } else {
            break;
        }
    }

    // Đảm bảo buffer đủ để chứa chuỗi "username|password"
    snprintf(buff, BUFF_SIZE, "%s|%s", username, password); // Định dạng "username|password"

    // Gửi dữ liệu qua socket
    sendWithCheck(sock, buff, strlen(buff), 0);

    // Nhận phản hồi từ server
    readWithCheck(sock, buff, BUFF_SIZE);
    int responseCode = atoi(buff);
    if (responseCode == LOGIN_SUCCESS) {
        printf("Login successful!\n");
        return 1;
    } else if (responseCode == NON_EXISTENCE_USERNAME) {
        printf("Username does not exist!\n");
    } else if (responseCode == INCORRECT_PASSWORD) {
        printf("Incorrect password!\n");
    } else {
        printf("Login failed! Unknown error.\n");
    }
    return 0;
}


// CREATE GROUP
void createGroup(int sock) {
    char group_name[50], buff[BUFF_SIZE];

    readWithCheck(sock, buff, BUFF_SIZE);
    buff[strlen(buff)] = '\0';
    int responseCode = atoi(buff);
    if (responseCode == 11) {
        printf("Enter group name: ");

        clearBuff();

        fgets(group_name, 50, stdin);
        group_name[strlen(group_name) - 1] = '\0';
        sendWithCheck(sock, group_name, strlen(group_name) + 1, 0);
    }
}

// MENU APPLICATION
void navigation(int sock) {
    int z1, z2, z3;
    char buffer[100], code[10], username[50], password[50];
    z1 = menu1();

    switch(z1) {
        case 1:
            signUp(sock);
            break;
        case 2:
            if (signIn(sock) == 1) {
                do {
                    z2 = menu2();
                    switch(z2) {
                        case 1:
                            sendCode(sock, CREATE_GROUP_REQUEST);
                            createGroup(sock);
                            readWithCheck(sock, buffer, 1000);
                            // printf("%s\n", buffer);
                            if (atoi(buffer) == EXISTENCE_GROUP_NAME) {
                                printf("Group name you typed has been used!\n");
                            }
                            else if (atoi(buffer) == CREATE_GROUP_SUCCESS) {
                                printf("Create group successfully!\n");
                            }
                            break;
                        case 2:
                            printf("========================== Available Group ==========================\n");
                            sendCode(sock, JOIN_GROUP_REQUEST);
                            readWithCheck(sock, buffer, 1000);
                            char available_group[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                            int num_of_available_groups = printAvailableElements(buffer, available_group);
                            int selected_group;
                            if (num_of_available_groups > 0) {
                                printf("Which group do you want to join? (1-%d): ", num_of_available_groups);
                                scanf("%d", &selected_group);
                                while (selected_group < 1 || selected_group > num_of_available_groups) {
                                    printf("Group you choose doesn't exist! Please try again!\n");
                                    printf("Which group do you want to join? (1-%d): ", num_of_available_groups);
                                    scanf("%d", &selected_group);
                                }
                                sendWithCheck(sock, available_group[selected_group - 1], strlen(available_group[selected_group - 1]) + 1 , 0);
                                readWithCheck(sock, buffer, 1000);
                                if (atoi(buffer) == REQUESTED_TO_JOIN) {
                                    printf("Request to join group successfully!\n");
                                } else if (atoi(buffer) == ALREADY_REQUESTED_TO_JOIN) {
                                    printf("You have already requested to join this group!\n");
                                } else if (atoi(buffer) == HAS_BEEN_INVITED) {
                                    printf("You have been invited to this group!\n");
                                }
                                else {
                                    printf("Something went wrong!\n");
                                }
                            }
                            else {
                                printf("You have joined all groups!\n");
                                sendCode(sock, NO_GROUP_TO_JOIN);
                            }
                            break;
                        case 3:
                            printf("==================== Available Group ====================\n");
                            sendCode(sock, ACCESS_GROUP_REQUEST);
                            readWithCheck(sock, buffer, 1000);
                            num_of_available_groups = printAvailableElements(buffer, available_group);
                            if (num_of_available_groups > 0) {
                                printf("Which group do you want to access? (1-%d): ", num_of_available_groups);
                                scanf("%d", &selected_group);
                                while (selected_group < 1 || selected_group > num_of_available_groups) {
                                    printf("Group you choose doesn't exist! Please try again!\n");
                                    printf("Which group do you want to access? (1-%d): ", num_of_available_groups);
                                    scanf("%d", &selected_group);
                                }
                                sendWithCheck(sock, available_group[selected_group - 1], strlen(available_group[selected_group - 1]) + 1, 0);
                                readWithCheck(sock, buffer, 1000);
                                if (atoi(buffer) == ACCESS_GROUP_SUCCESS) {
                                    printf("=> Access %s successfully!\n", available_group[selected_group - 1]);
                                    z3 = 0;
                                }
                                else {
                                    printf("Something went wrong!\n");
                                }

                            }
                            else {
                                printf("You have not joined any groups!\n");
                                sendCode(sock, NO_GROUP_TO_ACCESS);
                                z3 = 11;
                            }
                            while (z3 != 11) {
                                z3 = menu3(available_group[selected_group - 1]);
                                switch(z3) {
                                    case 7:
                                        sendCode(sock, INVITE_MEMBER_REQUEST);
                                        printf("==================== Available Members ====================\n");
                                        readWithCheck(sock, buffer, 1000);
                                        //if (atoi(buffer) != NOT_OWNER_OF_GROUP) {
                                            char available_members[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                                            int number_of_available_members = printAvailableElements(buffer, available_members);
                                            if (number_of_available_members > 0) {
                                                int selected_member;
                                                printf("Which member do you want to invite? (1-%d): ", number_of_available_members);
                                                printf("Or enter 0 to back: ");
                                                scanf("%d", &selected_member);
                                                while (selected_member > number_of_available_members) {
                                                    printf("Member you choose doesn't exist! Pleasy try again!\n");
                                                    printf("Which member do you want to invite? (1-%d): ", number_of_available_members);
                                                    printf("Or enter 0 to back: ");
                                                    scanf("%d", &selected_member);
                                                }
                                                if (selected_member == 0) {
                                                    sendCode(sock, NO_INVITE);
                                                    break;
                                                }
                                                sendWithCheck(sock, available_members[selected_member - 1], strlen(available_members[selected_member-1]) + 1, 0);
                                                readWithCheck(sock, buffer, 1000);
                                                if (atoi(buffer) == INVITE_SUCCESS) {
                                                    printf("Invite successfully!\n");
                                                } else if (atoi(buffer) == HAS_BEEN_INVITED) {
                                                    printf("This member has been invited!\n");
                                                } else if (atoi(buffer) == ALREADY_REQUESTED_TO_JOIN) {
                                                    printf("This member has already requested to join this group!\n");
                                                } else {
                                                    printf("Something wrong!\n");
                                                }
                                            }
                                            else {
                                                printf("This group already has all member!\n");
                                                sendCode(sock, NO_MEMBER_TO_INVITE);
                                            }
                                        //}
                                        // else {
                                        //     printf("--->Only leader of group can do this<---\n");
                                        // }
                                        break;
                                    case 6:
                                        sendCode(sock, APPROVE_REQUEST);
                                        printf("==================== Available Requests ====================\n");
                                        readWithCheck(sock, buffer, 1000);
                                        if (atoi(buffer) != NOT_OWNER_OF_GROUP) {
                                            char available_requests[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                                            int number_of_available_requests = printAvailableElements(buffer, available_requests);
                                            if(number_of_available_requests > 0){
                                                int selected_request;
                                                printf("Which request do you want to approve? (1-%d): \n", number_of_available_requests);
                                                printf("Or enter 0 to back: ");
                                                scanf("%d", &selected_request);
                                                while (selected_request > number_of_available_requests) {
                                                    printf("Member you choose doesn't exist! Pleasy try again!\n");
                                                    printf("Which member do you want to invite? (1-%d): ", number_of_available_requests);
                                                    printf("Or enter 0 to back: ");
                                                    scanf("%d", &selected_request);
                                                }
                                                if (selected_request == 0) {
                                                    sendCode(sock, NO_REQUEST_WERE_ACCEPTED);
                                                    break;
                                                }
                                                sendWithCheck(sock, available_requests[selected_request-1] , strlen(available_requests[selected_request-1]) + 1 , 0 );
                                                readWithCheck(sock, buffer, 1000);
                                                if(atoi(buffer) == APPROVE_SUCCESS) {
                                                    printf("Approve successfully\n");
                                                }
                                                else {
                                                    printf("Something wrong!!!\n");
                                                }
                                            }
                                            else {
                                                printf("This group does not have any requests\n");
                                                sendCode(sock, NO_REQUEST_TO_APPROVE);
                                            }
                                        }
                                        else {
                                            printf("--->Only leader of group can do this<---\n");
                                        }
                                        break;
                                    case 11:
                                        sendCode(sock, BACK_REQUEST);
                                        z3 = 11;
                                        break;
                                }
                            }
                            break;
                        case 4:
                            sendCode(sock, NOTIFICATION_REQUEST);
                            readWithCheck(sock, buffer, 1000);
                            printf("====================== Notifications ======================\n");
                            char available_notifications[20][50] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
                            int number_of_available_notifications = printAvailableElements(buffer, available_notifications);
                            if (number_of_available_notifications > 0)
                            {
                                int selected_notification;
                                printf("Which group request do you want to accept? (1-%d): Or 0 to back ", number_of_available_notifications);
                                scanf("%d", &selected_notification);
                                if (selected_notification == 0)
                                {
                                    sendCode(sock, NO_ACCEPT_INVITE);
                                    break;
                                }
                                sendWithCheck(sock, available_notifications[selected_notification - 1], strlen(available_notifications[selected_notification - 1]) + 1, 0);
                                readWithCheck(sock, buffer, 1000);
                                if (atoi(buffer) == ACCEPT_SUCCESS)
                                {
                                    printf("Accept successfully\n");
                                }
                                else
                                {
                                    printf("Something wrong!!!\n");
                                }
                            }
                            else
                            {
                                printf("You have no notifications\n");
                                sendCode(sock, NO_INVITE);
                            }
                            break;
                        case 5:
                            sendCode(sock, LOGOUT_REQUEST);
                            readWithCheck(sock, buffer, BUFF_SIZE);
                            printf("-->Logout: %s\n", buffer);
                            if (atoi(buffer) == LOGOUT_SUCCESS) {
                                printf("Logout successfully.\n");
                            }
                            break;
                        default:
                            z2 = 1;
                            break;
                    }
                } while(z2 >= 1 && z2 < 5);
            }
            break;
        default:
            break;
    }
}