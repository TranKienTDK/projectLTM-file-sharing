// Client side for Project LTM: File Sharing
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h> 
#include <pthread.h>
#include "./communication_code.h"

#define BUFF_SIZE 256

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
int uploadFile(int sock, char groupName[50]);
void createGroup(int sock);
void clearBuff();

// Function check send and receive data
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option) {
    printf("send");
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
	sprintf(codeStr, "%d ", code);
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
    printf("12. Create folder\n");
    printf("13. View folder\n");  
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

    // Gửi mã yêu cầu và cả username, password
    snprintf(buff, BUFF_SIZE, "%d %s|%s", REGISTER_REQUEST, username, password); 
    printf("Sending message: %s\n", buff);
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
        password[strcspn(password, "\n")] = '\0';
        if (strlen(password) == 0) {
            printf("Password is empty! Try again.\n");
        } else {
            break;
        }
    }

    snprintf(buff, BUFF_SIZE, "%d %s|%s", LOGIN_REQUEST, username, password); 
    sendWithCheck(sock, buff, strlen(buff), 0);

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

    while (getchar() != '\n' && getchar() != EOF);

    printf("Enter group name: ");
    fgets(group_name, sizeof(group_name), stdin);
    group_name[strcspn(group_name, "\n")] = '\0';
    
    if (strlen(group_name) == 0) {
        printf("Group name cannot be empty!\n");
        return;
    }

    snprintf(buff, sizeof(buff), "%d %s", CREATE_GROUP_REQUEST, group_name);
    sendWithCheck(sock, buff, strlen(buff), 0);

    readWithCheck(sock, buff, BUFF_SIZE);
    buff[strlen(buff)] = '\0';
    char *response_code = strtok(buff, " ");

    int responseCode = atoi(response_code);
    if (responseCode == EXISTENCE_GROUP_NAME) {
        printf("Group name you typed has been used!\n");
    } else if (responseCode == CREATE_GROUP_SUCCESS) {
        printf("Create group successfully!\n");
    } else {
        printf("An error occurred or the system is under maintenance.\n");
    }
}

void* SendFileToServer(int new_socket, char fname[50])
{
	
    FILE *fp = fopen(fname,"rb");
    if(fp==NULL)
    {
        printf("File open error");
    }   

    /* Read data from file and sendWithCheck it */
    while(1)
    {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[1024]={0};
        int nread = fread(buff,1,1024,fp);

        /* If read was success, sendWithCheck data. */
        if(nread > 0)
        {
            write(new_socket, buff, nread);
        }
		readWithCheck(new_socket, buff, BUFF_SIZE);
		if(strcasecmp(buff, "continue") != 0){
			break;
		}
        if (nread < 1024)
        {
            if (feof(fp))
            {
                printf("End of file\n");
            }
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
}

// void* SendFileToServer(int new_socket, char fname[100]) {
//     FILE *fp = fopen(fname, "rb");
//     if (fp == NULL) {
//         printf("File open error\n");
//         return NULL;
//     }

//     unsigned char buff[1024] = {0};
//     int nread;

//     while ((nread = fread(buff, 1, 1024, fp)) > 0) {
//         write(new_socket, buff, nread);
//         if (nread < 1024) {
//             if (feof(fp)) {
//                 printf("End of file\n");
//                 sendWithCheck(new_socket, "EOF", 4, 0);  // Gửi thông báo EOF đến server
//                 break;
//             }
//             if (ferror(fp)) {
//                 printf("Error reading\n");
//                 break;
//             }
//         }
//     }
//     fclose(fp);
// }

int uploadFile(int sock, char groupName[50]){
	char fileName[50], filePath[100], pathToUpload[100];
	char buffer[BUFF_SIZE];
    char send[BUFF_SIZE];
    char* response_code;
    char* data;
    int RESPONSE;
    clearBuff();

    while (1) {
        printf("Enter file name: ");
        fgets(fileName, sizeof(fileName), stdin);
        fileName[strcspn(fileName, "\n")] = '\0';
        if (strlen(fileName) == 0) {
            printf("File name is empty! Try again.\n");
        } else {
            break;
        }
    }

    do {
        printf("Enter path to file on your system: ");
        fgets(filePath, sizeof(filePath), stdin);
        filePath[strcspn(filePath, "\n")] = '\0';
        if (strlen(filePath) == 0) {
            printf("File path is empty! Try again.\n");
        } else {
            break;
        }
        if (fopen(filePath, "r") != NULL) {
            break;
        } else {
            printf("File is not available!\n");
        }
    } while(1);

    while(1) {
        printf("Enter path to upload: ");
        fgets(pathToUpload, sizeof(pathToUpload), stdin);
        pathToUpload[strcspn(pathToUpload, "\n")] = '\0';
        if (strlen(pathToUpload) == 0) {
            printf("File path is empty! Try again.\n");
        } else {
            break;
        }
    }
    printf("%s %s %s\n", fileName, filePath, pathToUpload);

    snprintf(send, sizeof(send), "%d %s|%s|%s", UPLOAD_REQUEST, fileName, filePath, pathToUpload);
    sendWithCheck(sock, send, strlen(send), 0);
    memset(buffer, 0, sizeof(buffer));

    SendFileToServer(sock, filePath);

    readWithCheck(sock, buffer, BUFF_SIZE);
    response_code = strtok(buffer, " ");
    RESPONSE = atoi(response_code);
    data = strtok(NULL, " ");
    printf("data: %d\n", RESPONSE);
    if (RESPONSE == UPLOAD_SUCCESS) {
        printf("Upload file successfully!\n");
        return 1;
    } else {
        printf("Cannot upload file! Error system!\n");
        return 0;
    }
    return 0;
}

void createFolder(int sock) {
    char folder_name[50], folder_path[50], buff[BUFF_SIZE];
    clearBuff();
    while (1) {
        printf("Enter folder name: ");
        fgets(folder_name, sizeof(folder_name), stdin);
        folder_name[strcspn(folder_name, "\n")] = '\0';
        if (strlen(folder_name) == 0) {
            printf("Folder name is empty! Try again.\n");
        } else {
            break;
        }
    }

    while (1) {
        printf("Enter folder path: ");
        fgets(folder_path, sizeof(folder_path), stdin);
        folder_path[strcspn(folder_path, "\n")] = '\0';
        if (strlen(folder_path) == 0) {
            printf("Folder path is empty! Try again.\n");
        } else {
            break;
        }
    }

    snprintf(buff, BUFF_SIZE, "%d %s|%s", CREATE_FOLDER_REQUEST, folder_name, folder_path); 
    sendWithCheck(sock, buff, strlen(buff), 0);

    readWithCheck(sock, buff, BUFF_SIZE);
    int responseCode = atoi(buff);
    if (responseCode == CREATE_FOLDER_SUCCESS) {
        printf("Folder created successfully!\n");
    } else {
        printf("Failed to create folder. Error code: %d\n", responseCode);
    }
}

void viewFolderData(int sock) {
    char folder_path[50], buff[BUFF_SIZE];
    char send[BUFF_SIZE];
    char* response_code;
    char* data;
    int RESPONSE;
    clearBuff();

    while (1) {
        printf("Enter folder path: ");
        fgets(folder_path, sizeof(folder_path), stdin);
        folder_path[strcspn(folder_path, "\n")] = '\0';  // Remove newline character
        if (strlen(folder_path) == 0) {
            printf("Folder path is empty! Try again.\n");
        } else {
            break;
        }
    }

    // snprintf(buff, BUFF_SIZE, "%d %s", VIEW_FOLDER_REQUEST, folder_path);
    // sendWithCheck(sock, buff, strlen(buff), 0);

    snprintf(send, sizeof(send), "%d %s", VIEW_FOLDER_REQUEST, folder_path);
    sendWithCheck(sock, send, strlen(send), 0);
    memset(buff, 0, sizeof(buff));

    readWithCheck(sock, buff, BUFF_SIZE);
    printf("%s\n", buff);
    response_code = strtok(buff, " ");
    RESPONSE = atoi(response_code);
    data = strtok(NULL, " ");
    // int responseCode = atoi(buff);
    if (RESPONSE == VIEW_FOLDER_SUCCESS) {
        // char *data = strchr(buff, ' ') + 1;  // Assuming the data starts after the first space
        char *token = strtok(data, "+");

        printf("Folder contents:\n");
        while (token != NULL) {
            printf("%s\n", token);
            token = strtok(NULL, "+");
        }
    } else if (RESPONSE == VIEW_FOLDER_FAIL) {
        printf("You cannot view folder of another group!\n");
    } else {
        printf("Failed to view folder contents\n");
    }
}



// MENU APPLICATION
void navigation(int sock) {
    int z1, z2, z3;
    char buffer[100], code[10], username[50], password[50];
    char send[BUFF_SIZE];
    char* response_code;
	char* data;
    int RESPONSE;
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
                            createGroup(sock);                           
                            break;
                        case 2: 
                            sendCode(sock,VIEW_GROUP_NO_JOIN);
                            memset(buffer, 0, sizeof(buffer)); 
                            readWithCheck(sock, buffer, 1000);
                            response_code = strtok(buffer, " ");
                            RESPONSE = atoi(response_code);	
		                    data = strtok(NULL, " ");
                            printf("============= Available Group No Join ===============\n");                       
                            char available_group[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                            int num_of_available_groups = printAvailableElements(data, available_group);
                            int selected_group;
                            if ( RESPONSE == VIEW_GROUP_NO_JOIN) {
                                printf("Which group do you want to join? (1-%d): ", num_of_available_groups);
                                scanf("%d", &selected_group);
                                while (selected_group < 1 || selected_group > num_of_available_groups) {
                                    printf("Group you choose doesn't exist! Please try again!\n");
                                    printf("Which group do you want to join? (1-%d): ", num_of_available_groups);
                                    scanf("%d", &selected_group);
                                }
                                char join_group[BUFF_SIZE];
                                snprintf(join_group, sizeof(join_group), "%d %s", JOIN_GROUP_REQUEST, available_group[selected_group - 1]);     
                                sendWithCheck(sock, join_group, strlen(join_group), 0);
                                memset(buffer, 0, sizeof(buffer));
                                readWithCheck(sock, buffer, 1000);
                                response_code = strtok(buffer, " ");
                                RESPONSE = atoi(response_code);	
		                        data = strtok(NULL, " ");
                                if (RESPONSE== REQUESTED_TO_JOIN) {
                                    printf("Request to join group successfully!\n");
                                } else if (RESPONSE == ALREADY_REQUESTED_TO_JOIN) {
                                    printf("You have already requested to join this group!\n");
                                } else if (RESPONSE == HAS_BEEN_INVITED) {
                                    printf("You have been invited to this group!\n");
                                }
                                else {
                                    printf("Something went wrong!\n");
                                }
                            }
                            else {
                                printf("You have joined all groups!\n");
                            }
                            break;
                        case 3:
                            sendCode(sock,VIEW_GROUP_JOINED);
                            memset(buffer, 0, sizeof(buffer)); 
                            readWithCheck(sock, buffer, 1000);
                            response_code = strtok(buffer, " ");
                            RESPONSE = atoi(response_code);	
		                    data = strtok(NULL, " ");
                            printf("==================== Available Group Joined ====================\n");
                            num_of_available_groups = printAvailableElements(data, available_group);
                            if (RESPONSE == VIEW_GROUP_JOINED) {
                                printf("Which group do you want to access? (1-%d): ", num_of_available_groups);
                                scanf("%d", &selected_group);
                                while (selected_group < 1 || selected_group > num_of_available_groups) {
                                    printf("Group you choose doesn't exist! Please try again!\n");
                                    printf("Which group do you want to access? (1-%d): ", num_of_available_groups);
                                    scanf("%d", &selected_group);
                                }
                                snprintf(send, sizeof(send), "%d %s", ACCESS_GROUP_REQUEST, available_group[selected_group - 1]);     
                                sendWithCheck(sock, send, strlen(send), 0);
                                memset(buffer, 0, sizeof(buffer));
                                readWithCheck(sock, buffer, 1000);
                                response_code = strtok(buffer, " ");
                                RESPONSE = atoi(response_code);	
		                        data = strtok(NULL, " ");
                                if (RESPONSE == ACCESS_GROUP_SUCCESS) {
                                    printf("=> Access %s successfully!\n", available_group[selected_group - 1]);
                                    z3 = 0;
                                }
                                else {
                                    printf("Something went wrong!\n");
                                }

                            }
                            else {
                                printf("You have not joined any groups!\n");
                                z3 = 11;
                            }
                            while (z3 != 11) {
                                z3 = menu3(available_group[selected_group - 1]);
                                switch(z3) {
                                    case 7:
                                        sendCode(sock, VIEW_USER_NOT_IN_GROUP);
                                        memset(buffer, 0, sizeof(buffer)); 
                                        readWithCheck(sock, buffer, 1000);
                                        response_code = strtok(buffer, " ");
                                        RESPONSE = atoi(response_code);	
		                                data = strtok(NULL, " ");
                                        printf("==================== Available Members ====================\n");
                                            char available_members[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                                            int number_of_available_members = printAvailableElements(data, available_members);
                                            if (RESPONSE == VIEW_USER_NOT_IN_GROUP) {
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
                                                snprintf(send, sizeof(send), "%d %s", INVITE_MEMBER_REQUEST, available_members[selected_member - 1]);     
                                                sendWithCheck(sock, send, strlen(send), 0);
                                                memset(buffer, 0, sizeof(buffer));
                                                readWithCheck(sock, buffer, 1000);
                                                response_code = strtok(buffer, " ");
                                                RESPONSE = atoi(response_code);	
		                                        data = strtok(NULL, " ");
                                                if (RESPONSE == INVITE_SUCCESS) {
                                                    printf("Invite successfully!\n");
                                                } else if (RESPONSE == HAS_BEEN_INVITED) {
                                                    printf("This member has been invited!\n");
                                                } else if (RESPONSE == ALREADY_REQUESTED_TO_JOIN) {
                                                    printf("This member has already requested to join this group!\n");
                                                } else {
                                                    printf("Something wrong!\n");
                                                }
                                            }
                                            else {
                                                printf("This group already has all member!\n");
                                            }
                                        break;
                                    case 6:
                                        sendCode(sock, VIEW_REQUEST_IN_GROUP);
                                        memset(buffer, 0, sizeof(buffer)); 
                                        readWithCheck(sock, buffer, 1000);
                                        response_code = strtok(buffer, " ");
                                        RESPONSE = atoi(response_code);	
		                                data = strtok(NULL, " ");
                                        printf("==================== Available Requests ====================\n");
                                        if (RESPONSE != NOT_OWNER_OF_GROUP) {
                                            char available_requests[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                                            int number_of_available_requests = printAvailableElements(data, available_requests);
                                            if(RESPONSE == VIEW_REQUEST_IN_GROUP){
                                                int selected_request;
                                                printf("Which request do you want to approve? (1-%d): \n", number_of_available_requests);
                                                printf("Or enter 0 to back: ");
                                                scanf("%d", &selected_request);
                                                while (selected_request > number_of_available_requests) {
                                                    printf("Member you choose doesn't exist! Please try again!\n");
                                                    printf("Which member do you want to invite? (1-%d): ", number_of_available_requests);
                                                    printf("Or enter 0 to back: ");
                                                    scanf("%d", &selected_request);
                                                }
                                                if (selected_request == 0) {
                                                    break;
                                                }
                                                snprintf(send, sizeof(send), "%d %s", APPROVE_REQUEST, available_requests[selected_request-1]);     
                                                sendWithCheck(sock,send, strlen(send), 0);
                                                memset(buffer, 0, sizeof(buffer));
                                                readWithCheck(sock, buffer, 1000);
                                                response_code = strtok(buffer, " ");
                                                RESPONSE = atoi(response_code);	
		                                        data = strtok(NULL, " ");
                                                if(RESPONSE == APPROVE_SUCCESS) {
                                                    printf("Approve successfully\n");
                                                }
                                                else {
                                                    printf("Something wrong!!!\n");
                                                }
                                            }
                                            else {
                                                printf("This group does not have any requests\n");
                                            }
                                        }
                                        else {
                                            printf("--->Only leader of group can do this<---\n");
                                        }
                                        break;

                                    case 1:
                                        if( uploadFile(sock, available_group[selected_group-1]) == 1){
										    
									    }
                                        //uploadFile(sock, available_group[selected_group-1]);
									    break;

                                    case 5:
									printf("======================= All Files ========================\n");
									sendCode(sock, VIEW_FILES_REQUEST);
									memset(buffer, 0, sizeof(buffer)); 
                                    readWithCheck(sock, buffer, 1000);
                                    response_code = strtok(buffer, " ");
                                    RESPONSE = atoi(response_code);	
                                    data = strtok(NULL, " ");
									if(atoi(buffer) != MEMBER_WAS_KICKED){
										char available_files[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
										int number_of_available_files = printAvailableElements(buffer, available_files);
									}else{
										printf("You have been kicked out of this group.\n");
										z3 = 11;
									}
									break;

                                    case 8:
                                        sendCode(sock, VIEW_USER_IN_GROUP);
                                        memset(buffer, 0, sizeof(buffer)); 
                                        readWithCheck(sock, buffer, 1000);
                                        response_code = strtok(buffer, " ");
                                        RESPONSE = atoi(response_code);	
		                                data = strtok(NULL, " ");
                                        if (RESPONSE != MEMBER_WAS_KICKED) {
                                            printf("====================== All Members =======================\n");
                                            if (RESPONSE != NOT_OWNER_OF_GROUP) {
                                                char available_members[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                                                int number_of_available_members = printAvailableElements(data, available_members);
                                                if (RESPONSE == VIEW_USER_IN_GROUP) {
                                                    int selected_member;
                                                    printf("Which member do you want to kick? (1-%d): ", number_of_available_members);
                                                    printf("Or enter 0 to back: ");
                                                    scanf("%d", &selected_member);
                                                    while (selected_member > number_of_available_members) {
                                                        printf("Member you choose doesn't exist! Please try again!\n");
                                                        printf("Which member do you want to kick? (1-%d): ", number_of_available_members);
                                                        printf("Or enter 0 to back: ");
                                                        scanf("%d", &selected_member);
                                                    }
                                                    if (selected_member == 0) {
                                                        break;
                                                    }
                                                    snprintf(send, sizeof(send), "%d %s", KICK_MEMBER_REQUEST, available_members[selected_member - 1]);
                                                    sendWithCheck(sock, send, sizeof(send), 0);
                                                    memset(buffer, 0, sizeof(buffer));
                                                    readWithCheck(sock, buffer, 1000);
                                                    response_code = strtok(buffer, " ");
                                                    RESPONSE = atoi(response_code);	
                                                    data = strtok(NULL, " ");
                                                    if (RESPONSE == KICK_MEMBER_SUCCESS) {
                                                        printf("Kick member successfully!\n");
                                                    }
                                                } else {
                                                    printf("This group doesn't have any member to kick!\n");
                                                }
                                            } else {
                                                printf("--->Only leader of group can do this<---\n");
                                            }
                                        } else {
                                            printf("You have been kicked out of this group.\n");
										    z3 = 11;
                                        }
                                        break;

                                    case 9:
                                        // VIEW ALL USERS
                                        sendCode(sock, VIEW_USERS_OF_GROUP_REQUEST);
                                        memset(buffer, 0, sizeof(buffer));
                                        readWithCheck(sock, buffer, 1000);
                                        response_code = strtok(buffer, " ");
                                        RESPONSE = atoi(response_code);	
		                                data = strtok(NULL, " ");
                                        printf("====================== All Members In Group =======================\n");
                                        if (RESPONSE != MEMBER_WAS_KICKED) {
                                            char available_members[20][50] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0'};
										    int number_of_available_members = printAvailableElements(data, available_members);
                                            if (number_of_available_members > 0) {

                                            } else {
                                                printf("Your group is just you!\n");
                                            }
                                        } else {
                                            printf("You have been kicked out of this group.\n");
										    z3 = 11;
                                        }
                                        break;

                                    case 10:
                                        // QUIT GROUP
                                        sendCode(sock, QUIT_GROUP_REQUEST);
                                        memset(buffer, 0, sizeof(buffer));
                                        readWithCheck(sock, buffer, 1000);
                                        response_code = strtok(buffer, " ");
                                        RESPONSE = atoi(response_code);	
		                                data = strtok(NULL, " ");
                                        if (RESPONSE == QUIT_GROUP_SUCCESS) {
                                            printf("Quit group successfully!\n");
                                            z3 = 11;
                                        } else if (RESPONSE == IS_OWNER_OF_GROUP) {
                                            printf("---> You are the owner of this group <---\n");
                                        } else {
                                            printf("Something wrong!\n");
                                        }
                                        break;

                                    case 12:
                                        createFolder(sock);
                                        break;

                                    case 13:
                                        viewFolderData(sock);
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
                            memset(buffer, 0, sizeof(buffer));
                            readWithCheck(sock, buffer, 1000);
                            response_code = strtok(buffer, " ");
                            RESPONSE = atoi(response_code);	
		                    data = strtok(NULL, " ");
                            printf("====================== Notifications ======================\n");
                            char available_notifications[20][50] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
                            int number_of_available_notifications = printAvailableElements(data, available_notifications);
                            if (RESPONSE == NOTIFICATION_REQUEST)
                            {
                                int selected_notification;
                                printf("Which group request do you want to accept? (1-%d): Or 0 to back ", number_of_available_notifications);
                                scanf("%d", &selected_notification);
                                if (selected_notification == 0)
                                {
                                    break;
                                }
                                snprintf(send, sizeof(send), "%d %s", ACCEPT_INVITE_REQUEST, available_notifications[selected_notification - 1]);     
                                sendWithCheck(sock, send, strlen(send), 0);
                                memset(buffer, 0, sizeof(buffer));
                                readWithCheck(sock, buffer, 1000);
                                response_code = strtok(buffer, " ");
                                RESPONSE = atoi(response_code);	
		                        data = strtok(NULL, " ");                              
                                if (RESPONSE == ACCEPT_SUCCESS)
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