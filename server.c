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
#include <asm-generic/socket.h>

#define BUFF_SIZE 256
singleList users, groups, requests, files;

// Define function
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option);
int readWithCheck(int sock, char buff[BUFF_SIZE], int length);
void sendCode(int sock, int code);
void signUp(int sock, singleList *users, char *data);
int signIn(int sock, singleList users, user_struct **loginUser, char *data);
int updateRequest(singleList *requests, char group_name[50], char owner[50], int request_from_user);
int checkRequestExit(singleList requests, char group_name[50], char owner[50], int request_from_user);
void readUserFile(singleList *users);
void readGroupFile(singleList *groups);
void readRequestFile(singleList* requests);
void readFileFile(singleList *files);
void writeToGroupFile(singleList groups);
int checkExistence(int type, singleList list, char string[50]);
void* findByName(int type, singleList list, char string[50]);
int createGroup(int sock, singleList * groups, user_struct *loginUser, char *data);
int addGroupToJoinedGroups(singleList users, char username[50], char group_name[50]);
int isOwnerOfGroup(singleList groups, char group_name[], char username[]);
int addMember(singleList groups, char group_name[50], char username[50]);
int isUserAMember(singleList users, char group_name[50], char username[50]);
singleList unJoinedGroups(singleList groups, singleList users, char username[50]);
singleList joinedGroups(singleList users, char username[50]);
singleList unJoinedMembers(singleList users, char group_name[50]);
singleList getAllMembersOfGroup(singleList groups, char group_name[50]);
void convertSimpleUsersToString(singleList simple_user, char str[1000]);
void convertSimpleGroupsToString(singleList simple_group, char str[1000]);
void convertUserRequestsToString(singleList requests, char str[1000]);
void deleteRequest(singleList *requests, char group_name[50], char user_name[50], int request_from_user);
void kickMemberOut(singleList groups, singleList users, char group_name[50], char username[50]);
void * handleThread(void *my_sock);

// Function check send and receive data
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option) {
	int sendByte = 0;
	sendByte = send(sock, buff, length, option);
	if(sendByte > 0){
		printf("%s\n", buff);
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
	sprintf(codeStr, "%d ", code);
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
            {
                int i = 0;
                list.cur = list.root;
                while (list.cur != NULL) {
                    i++;
                    if(strcmp(((group_struct*)list.cur->element)->group_name,string) != 0) {
                        list.cur = list.cur->next;
                    }
                    else {
                        return 1;
                    }
                }
			return 0; 
            }
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
int updateRequest(singleList *requests, char group_name[50], char user_name[50], int request_from_user) {
    int tmp = checkRequestExit(*requests, group_name, user_name, request_from_user);
	if (tmp != 0)
		return 1 - tmp ;
	request_struct *request = (request_struct*)malloc(sizeof(request_struct));
	strcpy(request->group_name, group_name);
	strcpy(request->user_name, user_name);
	request->request_from_user = request_from_user;
	insertEnd(requests, request);
	return 1 ;
}

int checkRequestExit(singleList requests, char group_name[50], char user_name[50], int request_from_user) {
    requests.cur = requests.root;
	while (requests.cur != NULL) {
		if(strcmp( ((request_struct*)requests.cur->element)->group_name, group_name) == 0 && strcmp( ((request_struct*)requests.cur->element)->user_name, user_name) == 0){
			if ( ((request_struct*)requests.cur->element)->request_from_user == request_from_user){
				return 1;
			}
			else {
				return 2;
			}
		}
		requests.cur = requests.cur->next; 
	}
	return 0;
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

void readGroupFile(singleList *groups) {
	
	// clear list
	deleteSingleList(groups);
	FILE *fp;
	fp = fopen("./storage/group.txt","r");
	char str_tmp[100];

	
	while (1)
	{	
		char c = fgetc(fp);
    	if (c != EOF){
			int res = fseek( fp, -1, SEEK_CUR );
			// fgets (str_tmp, 100, fp);
		}else
        	break;
		//====================initialize============================================
		group_struct *group_element = (group_struct*) malloc(sizeof(group_struct));
		singleList members;
		createSingleList(&members);
		singleList files;
		createSingleList(&files);

		

		//======================end initialize======================================
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(group_element->group_name, str_tmp);
		//=====================read members=========================================
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(group_element->owner, str_tmp);
		
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		int number_of_members = atoi(str_tmp);
		group_element->number_of_members = number_of_members; // number_of_members
		for(int i = 0; i < number_of_members; i++){
			simple_user_struct *member_element = (simple_user_struct*) malloc(sizeof(simple_user_struct));
			fgets(str_tmp, 100, fp);
			str_tmp[strlen(str_tmp)-1] = '\0';
			strcpy(member_element->user_name, str_tmp);
			insertEnd(&members, member_element);
		}
		group_element->members = members;
		//===================end read members========================================
		//=====================read files============================================
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		int number_of_files = atoi(str_tmp);
		group_element->number_of_files = number_of_files;
		for(int i = 0; i < number_of_files; i++){
			simple_file_struct *file_element = (simple_file_struct*) malloc(sizeof(simple_file_struct));
			fgets (str_tmp, 100, fp);
			if(str_tmp[strlen(str_tmp)-1] == '\n'){
				str_tmp[strlen(str_tmp)-1] = '\0';
			}
			strcpy(file_element->file_name, str_tmp);
			insertEnd(&files, file_element);
		}
		group_element->files = files;
		//=====================end read files=========================================
		insertEnd(groups, group_element); // add group_element to group_list
	}


	fclose(fp);
}

void readRequestFile(singleList* requests) {
    char username[50], group_name[50], user_name[50];
	int request_from_user ;
	FILE * f = fopen("./storage/request.txt","r");

	if (f == NULL) {
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	while (1) {	
		char c = fgetc(f);
		if (c != EOF){
			int res = fseek( f, -1, SEEK_CUR );
		}
        else {
			break;
		}
		request_struct *request = (request_struct*)malloc(sizeof(request_struct));
		fgets(group_name, 50, f);
		group_name[strlen(group_name) -1 ] = '\0';

		fgets(user_name, 50, f);
		user_name[strlen(user_name) -1 ] = '\0';

		fscanf(f,"%d\n", &request_from_user);

		strcpy(request->group_name, group_name);
		strcpy(request->user_name, user_name);
		request->request_from_user = request_from_user;

		insertEnd(requests, request);
	}
	fclose(f);
}

void readFileFile(singleList *files) {
    FILE *fp;
	char str_tmp[100];
	str_tmp[0] = '\0';
	fp = fopen("./storage/file.txt", "r");
	if(fp == NULL) {
		fprintf(stderr, "File missing: can not find \"file.txt\".\n");
		exit(-1);
	}

	while(1) {
		char c = fgetc(fp);
    	if (c != EOF){
			int res = fseek( fp, -1, SEEK_CUR );
		}else
        	break;
	
		file_struct *file = (file_struct*)malloc(sizeof(file_struct));
		// name
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->name, str_tmp);
		// owner
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->owner, str_tmp);
		// group
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->group, str_tmp);
		// uploaded
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->uploaded_at, str_tmp);
		fgets (str_tmp, 100, fp);
		if(str_tmp[strlen(str_tmp)-1] == '\n'){
			str_tmp[strlen(str_tmp)-1] = '\0';
		}
		file->downloaded_times = atoi(str_tmp);
		insertEnd(files, file);
	}
	fclose(fp);
}

void writeToGroupFile(singleList groups){
	group_struct* group = NULL;
	FILE *fp;
	fp = fopen("./storage/group.txt","w");
	groups.cur = groups.root;
	while (groups.cur != NULL)
	{
		group = (group_struct*)(groups.cur->element);
		fprintf(fp,"%s\n", group->group_name);
		fprintf(fp,"%s\n", group->owner);
		fprintf(fp,"%d\n", group->number_of_members);
		singleList members;
		createSingleList(&members);
		members = group->members;
		members.cur = members.root;
		while(members.cur!=NULL){
			fprintf(fp,"%s\n",((simple_user_struct*)members.cur->element)->user_name);
			members.cur = members.cur->next;
		}
		fprintf(fp,"%d\n", group->number_of_files);
		singleList files;
		createSingleList(&files);
		files = group->files;
		files.cur = files.root;
		while(files.cur!=NULL){
			fprintf(fp,"%s\n",((simple_file_struct*)files.cur->element)->file_name);
			files.cur = files.cur->next;
		}
		groups.cur = groups.cur->next;
	}
	fclose(fp);
}

void convertGroupsInviteToString(singleList requests, char str[1000]){
	str[0] = '\0';
	requests.cur = requests.root;
	while(requests.cur != NULL)
  	{
		strcat(str, ((request_struct*)requests.cur->element)->group_name);
		if(requests.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
		requests.cur = requests.cur->next;
  	}
}

int addGroupToJoinedGroups(singleList users, char username[50], char group_name[50]) {
    users.cur = users.root;
	while(users.cur != NULL)
	{
		if(strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			simple_group_struct *group_element = (simple_group_struct*) malloc(sizeof(simple_group_struct));
			strcpy(group_element->group_name, group_name);
			insertEnd(&((user_struct*)users.cur->element)->joined_groups, group_element);
			((user_struct*)users.cur->element)->count_group++;
			return 1;
		}
		users.cur = users.cur->next;
	}
	return 0;
}

int isOwnerOfGroup(singleList groups, char group_name[], char username[]) {
	groups.cur = groups.root;
	while(groups.cur != NULL) {
		if( strcmp( ((group_struct*)groups.cur->element)->group_name, group_name) == 0) {
			if( strcmp( ((group_struct*)groups.cur->element)->owner, username) == 0){
				return 1;
			}
		}
		groups.cur = groups.cur->next;
	}
	return 0;
}

int addMember(singleList groups, char group_name[50], char username[50]){
	printf("add %s member to %s\n", username, group_name);
	singleList members;
	createSingleList(&members);
  	groups.cur = groups.root;
	while(groups.cur != NULL) {
		if(strcmp(((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			simple_user_struct *member_element = (simple_user_struct*) malloc(sizeof(simple_user_struct));
			strcpy(member_element->user_name, username);
			insertEnd(&((group_struct*)groups.cur->element)->members, member_element);
			((group_struct*)groups.cur->element)->number_of_members += 1;
			return 1;
		}
		groups.cur = groups.cur->next;
	}
	return 0;
}

int isUserAMember(singleList users, char group_name[50], char username[50]){
	users.cur = users.root;
	while(users.cur != NULL){
		if( strcmp( ((user_struct*)users.cur->element)->user_name, username ) == 0){
			((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.root;
			while (((user_struct*)users.cur->element)->joined_groups.cur != NULL)
			{
				if(strcmp(((simple_group_struct*)((user_struct*)users.cur->element)->joined_groups.cur->element)->group_name, group_name) == 0){
					return 1;
				}
				((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.cur->next;
			}
			
		}
		users.cur = users.cur->next;
	}
	return 0;
}

singleList unJoinedGroups(singleList groups, singleList users, char username[50]) {
    singleList joined_groups;
	createSingleList(&joined_groups);
	singleList un_joined_groups;
	createSingleList(&un_joined_groups);
	users.cur = users.root;
	while(users.cur != NULL) {
		if(strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			groups.cur = groups.root;
			joined_groups = ((user_struct*)users.cur->element)->joined_groups;
			break;
		}
		users.cur = users.cur->next;
	}

	groups.cur = groups.root;
	while(groups.cur != NULL) {
		int check = 0;
		joined_groups.cur = joined_groups.root;
		while(joined_groups.cur != NULL)
		{
			if( strcmp( ((group_struct*)groups.cur->element)->group_name, ((simple_group_struct*)joined_groups.cur->element)->group_name) == 0) {
				check = 1;
				break;
			}
			joined_groups.cur = joined_groups.cur->next;
		}
		if(check == 0){
			simple_group_struct *group_element = (simple_group_struct*) malloc(sizeof(simple_group_struct));
			strcpy(group_element->group_name, ((group_struct*)groups.cur->element)->group_name);
			insertEnd(&un_joined_groups, group_element);
		}
		groups.cur = groups.cur->next;
	}
	return un_joined_groups;
}

singleList joinedGroups(singleList users, char username[50]) {
	singleList joined_groups;
	createSingleList(&joined_groups);
	users.cur = users.root;
	while(users.cur != NULL) {
		if(strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			joined_groups = ((user_struct*)users.cur->element)->joined_groups;
			break;
		}
		users.cur = users.cur->next;
	}
	return joined_groups;
}

singleList unJoinedMembers(singleList users, char group_name[50]){
	singleList un_joined_members;
	createSingleList(&un_joined_members);
	users.cur = users.root;
	while(users.cur != NULL)
	{
		int check = 0;
		((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.root;
		while(((user_struct*)users.cur->element)->joined_groups.cur != NULL)
		{
			if( strcmp( ((simple_group_struct*)((user_struct*)users.cur->element)->joined_groups.cur->element)->group_name, group_name) == 0)
			{
				check = 1;
				break;
			}
			((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.cur->next;
		}
		if(check == 0){
			simple_user_struct *member_element = (simple_user_struct*) malloc(sizeof(simple_user_struct));
			strcpy(member_element->user_name, ((user_struct*)users.cur->element)->user_name);
			insertEnd(&un_joined_members, member_element);
		}
		users.cur = users.cur->next;
	}
	return un_joined_members;
}

singleList getRequests(singleList requests,char current_group[50]) {
	singleList requests_of_group;
	createSingleList(&requests_of_group);
	requests.cur = requests.root;
	while(requests.cur != NULL) {
		if(strcmp(((request_struct*)requests.cur->element)->group_name, current_group) == 0 && ((request_struct*)requests.cur->element)->request_from_user == 1){
			request_struct *request_element = (request_struct*) malloc(sizeof(request_struct));
			strcpy(request_element->group_name, ((request_struct*)requests.cur->element)->group_name);
			strcpy(request_element->user_name, ((request_struct*)requests.cur->element)->user_name);
			request_element->request_from_user = ((request_struct*)requests.cur->element)->request_from_user;
			insertEnd(&requests_of_group, request_element);
		}
		requests.cur = requests.cur->next;
	}
	return requests_of_group;
}

singleList getInvites(singleList requests,char user_name[50]){
	singleList invites_of_user;
	createSingleList(&invites_of_user);
	requests.cur = requests.root;
	while(requests.cur != NULL){
		if(strcmp(((request_struct*)requests.cur->element)->user_name, user_name) == 0 && ((request_struct*)requests.cur->element)->request_from_user == 0){
			request_struct *request_element = (request_struct*) malloc(sizeof(request_struct));
			strcpy(request_element->group_name, ((request_struct*)requests.cur->element)->group_name);
			strcpy(request_element->user_name, ((request_struct*)requests.cur->element)->user_name);
			request_element->request_from_user = ((request_struct*)requests.cur->element)->request_from_user;
			insertEnd(&invites_of_user, request_element);
		}
		requests.cur = requests.cur->next;
	}
	return invites_of_user;
}

singleList getAllMembersOfGroup(singleList groups, char group_name[50]){
	singleList members;
	createSingleList(&members);
	groups.cur = groups.root;
	while (groups.cur != NULL)
	{
		if(strcmp( ((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			members = ((group_struct*)groups.cur->element)->members;
			break;
		}
		groups.cur = groups.cur->next;
	}
	return members;
}

void convertSimpleUsersToString(singleList simple_user, char str[1000]){
	str[0] = '\0';
	simple_user.cur = simple_user.root;
	while(simple_user.cur != NULL)
  	{
		strcat(str, ((simple_user_struct*)simple_user.cur->element)->user_name);
		if(simple_user.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
    	simple_user.cur = simple_user.cur->next;
  	}
}

void convertSimpleGroupsToString(singleList simple_group, char str[1000]) {
    str[0] = '\0';
    simple_group.cur = simple_group.root;
    while (simple_group.cur != NULL) {
        strcat(str, ((simple_group_struct*)simple_group.cur->element)->group_name);
        if (simple_group.cur->next == NULL) {
            str[strlen(str)] = '\0';
        }
        else {
            strcat(str, "+");
        }
        simple_group.cur = simple_group.cur->next;
    }
}

void convertUserRequestsToString(singleList requests, char str[1000]){
	str[0] = '\0';
	requests.cur = requests.root;
	while(requests.cur != NULL)
  	{
		strcat(str, ((request_struct*)requests.cur->element)->user_name);
		if(requests.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
		requests.cur = requests.cur->next;
  	}
}

void deleteRequest(singleList *requests, char group_name[50], char user_name[50], int request_from_user) {
	if( strcmp( ((request_struct*)(*requests).root->element)->group_name, group_name) == 0 && strcmp( ((request_struct*)(*requests).root->element)->user_name, user_name) == 0){
		if ( ((request_struct*)(*requests).root->element)->request_from_user == request_from_user){
			(*requests).root = deleteBegin(requests);
		}
	}
	else {
		(*requests).cur = (*requests).prev = (*requests).root;
		while((*requests).cur != NULL)
		{
			if( strcmp( ((request_struct*)(*requests).cur->element)->group_name, group_name) == 0 && strcmp( ((request_struct*)(*requests).cur->element)->user_name, user_name) == 0){
				if ( ((request_struct*)(*requests).cur->element)->request_from_user == request_from_user){
					break;
				}
			}
			(*requests).prev = (*requests).cur;
			(*requests).cur = (*requests).cur->next;
		}
		if((*requests).cur != NULL){
			(*requests).prev->next = (*requests).cur->next;
			(*requests).cur = (*requests).prev;
		}
	}

}

void kickMemberOut(singleList groups, singleList users, char group_name[50], char username[50]){
	
	//delete user in singleList groups
	groups.cur = groups.root;
	while( groups.cur != NULL){
		if( strcmp( ((group_struct*)groups.cur->element)->group_name, group_name ) == 0){
			singleList members;
			createSingleList(&members);
			members = ((group_struct*)groups.cur->element)->members;
			if( strcmp(username, ((simple_user_struct*)members.root->element)->user_name) == 0){
				members.root = members.root->next;
				((group_struct*)groups.cur->element)->members = members;
			}else{
				members.cur = members.prev = members.root;
				while (members.cur != NULL && strcmp( ((simple_user_struct*)members.cur->element)->user_name, username) != 0)
				{
					members.prev = members.cur;
					members.cur = members.cur->next;
				}
				node *newNode = members.cur;
				members.prev->next = members.cur->next;
				members.cur = members.prev;
				free(newNode);
				((group_struct*)groups.cur->element)->members = members;
				
			}
			break;
		}
		groups.cur = groups.cur->next;
	}
	//delete group in joined_group
	users.cur = users.root;
	while (users.cur != NULL)
	{
		if( strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			((user_struct*)users.cur->element)->count_group -= 1;
			singleList joined_groups;
			createSingleList(&joined_groups);
			joined_groups = ((user_struct*)users.cur->element)->joined_groups;
			if( strcmp(group_name, ((simple_group_struct*)joined_groups.root->element)->group_name) == 0){
				joined_groups.root = joined_groups.root->next;
				((user_struct*)users.cur->element)->joined_groups = joined_groups;
			}else{
				joined_groups.cur = joined_groups.prev = joined_groups.root;
				while (joined_groups.cur != NULL && strcmp(group_name, ((simple_group_struct*)joined_groups.cur->element)->group_name) != 0)
				{
					joined_groups.prev = joined_groups.cur;
					joined_groups.cur = joined_groups.cur->next;
				}
				node *newNode = joined_groups.cur;
				joined_groups.prev->next = joined_groups.cur->next;
				joined_groups.cur = joined_groups.prev;
				free(newNode);
				((user_struct*)users.cur->element)->joined_groups = joined_groups;
			}
			break;
		}
		users.cur = users.cur->next;
	}
	
	writeToGroupFile(groups);
	// saveFiles(*files);
	saveUsers(users);
}

void writeToRequestFile(singleList requests) {
    request_struct* request = NULL;
	FILE * f = fopen("./storage/request.txt","w");
	requests.cur = requests.root;
	while (requests.cur != NULL)
	{
		request = (request_struct*)(requests.cur->element);
		fprintf(f,"%s\n", request->group_name);
		fprintf(f,"%s\n", request->user_name);
		fprintf(f,"%d\n", request->request_from_user);
		requests.cur = requests.cur->next;
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
    createSingleList(&requests);
    createSingleList(&files);

    readUserFile(&users);
    readGroupFile(&groups);
    readRequestFile(&requests);
    // readFileFile(&files);
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

// SIGN UP SERVER
void signUp(int sock, singleList *users, char *data) {
    char username[50], password[50];
    singleList groups;
    char *separator = strchr(data, '|');
    if (separator == NULL) {
        printf("Invalid input format. Expected format: username|password\n");
        sendCode(sock, INVALID_FORMAT);
        return;
    }

    // Tách username
    size_t username_len = separator - data;
    if (username_len >= sizeof(username)) {
        printf("Username too long!\n");
        sendCode(sock, INVALID_FORMAT);
        return;
    }
    strncpy(username, data, username_len);
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


// SIGNIN SERVER
int signIn(int sock, singleList users, user_struct **loginUser, char *data) {
    char buff[BUFF_SIZE];
    char username[50], password[50];

    // Tìm dấu phân tách '|'
    char *separator = strchr(data, '|');
    if (separator == NULL) {
        printf("Invalid input format. Expected format: username|password\n");
        sendCode(sock, INVALID_FORMAT);
        return 0;
    }

    // Tách username
    size_t username_len = separator - data;
    if (username_len >= sizeof(username)) {
        printf("Username too long!\n");
        sendCode(sock, INVALID_FORMAT);
        return 0;
    }
    strncpy(username, data, username_len);
    username[username_len] = '\0';

    // Tách password
    char *password_start = separator + 1;
    size_t password_len = strlen(password_start);
    if (password_len >= sizeof(password)) {
        printf("Password too long!\n");
        sendCode(sock, INVALID_FORMAT);
        return 0;
    }
    strncpy(password, password_start, password_len);
    password[password_len] = '\0';

    printf("Extracted username: '%s', password: '%s'\n", username, password);
    if (checkExistence(1, users, username) == 0) {
        sendCode(sock, NON_EXISTENCE_USERNAME);
        return 0;
    }
    *loginUser = (user_struct *)(findByName(1, users, username));
    if (strcmp((*loginUser)->password, password) == 0) {
        sendCode(sock, LOGIN_SUCCESS);
        return 1;
    }
    sendCode(sock, INCORRECT_PASSWORD);
    return 0;
}

// CREATE GROUP SERVER
int createGroup(int sock, singleList * groups, user_struct *loginUser, char *data) {
    char noti[100], cmd[100];
    int result;
    if (checkExistence(2, *groups, data) == 1) {
        printf("EXISTENCE_GROUP_NAME\n");
        sendCode(sock, EXISTENCE_GROUP_NAME);
        result = 0;
        return result;
    }
    else {
        group_struct *group_element = (group_struct*) malloc(sizeof(group_struct));
        singleList members, files;

        createSingleList(&members);
        createSingleList(&files);

        strcpy(group_element->group_name, data);
		strcpy(group_element->owner, loginUser->user_name);
		printf("Ownner of group: %s\n", group_element->owner);
		group_element->files = files;
		group_element->members = members;
		group_element->number_of_files = 0;
		group_element->number_of_members = 0;

        insertEnd(groups, group_element);

        addGroupToJoinedGroups(users, loginUser->user_name, group_element->group_name);

        strcpy(cmd, "mkdir ");
		strcat(cmd, "./files/\'");
		strcat(cmd, data);
		strcat(cmd, "\'");
		system(cmd);

        printf("CREATE_GROUP_SUCCESS %s\n",data);
        sendCode(sock, CREATE_GROUP_SUCCESS);
        result = 1;
        return result;
    }
}

void * handleThread(void *my_sock) {
    int new_socket = *((int *)my_sock);
	int REQUEST;
	char* request_code;
	char* data;
	char buff[BUFF_SIZE];
	user_struct *loginUser = NULL;
	char response[BUFF_SIZE];

    while(1) {
        memset(buff, 0, sizeof(buff)); 
		readWithCheck(new_socket, buff, BUFF_SIZE);
		printf("%s\n",buff);
		request_code = strtok(buff, " ");
        REQUEST = atoi(request_code);		
		data = strtok(NULL, " ");
        switch(REQUEST) {
            case REGISTER_REQUEST:
            printf("REGISTER_REQUEST\n");
                signUp(new_socket, &users, data);
                saveUsers(users);
                break;
            case LOGIN_REQUEST:
                printf("LOGIN_REQUEST\n");
                if(signIn(new_socket, users, &loginUser, data) == 1) {
                    while(REQUEST != LOGOUT_REQUEST) {
                        memset(buff, 0, sizeof(buff));
						readWithCheck(new_socket, buff, BUFF_SIZE);
						printf("%s\n",buff);
                        request_code = strtok(buff, " ");
        				REQUEST = atoi(request_code);
						data = strtok(NULL, " ");
                        switch(REQUEST) {
                            case CREATE_GROUP_REQUEST: 
                                printf("CREATE_GROUP_REQUEST\n");
                                int result = createGroup(new_socket, &groups, loginUser, data);
                                if (result == 1) {
                                    writeToGroupFile(groups);
                                    saveUsers(users);
                                }
                                break;
                            case VIEW_GROUP_NO_JOIN:
								printf("VIEW_GROUP_NO_JOIN\n");
								singleList un_joined_group;
                                createSingleList(&un_joined_group);
                                un_joined_group = unJoinedGroups(groups, users, loginUser->user_name);
                                char str[200];
                                convertSimpleGroupsToString(un_joined_group, str);
								if(strlen(str) == 0){
									sendCode(new_socket, NO_GROUP_TO_JOIN);
									break;
								}
								snprintf(response, sizeof(response), "%d %s", VIEW_GROUP_NO_JOIN, str);
								sendWithCheck(new_socket, response, strlen(response) + 1, 0);
								break;
                            case JOIN_GROUP_REQUEST:
                                printf("JOIN_GROUP_REQUEST\n");                              
                                    printf("You choose group: %s\n", data);
                                    int tmp = updateRequest(&requests, data, loginUser->user_name, 1);
                                    if (tmp == 1) {
										printf("Update request successfully\n");
										sendCode(new_socket , REQUESTED_TO_JOIN);
										writeToRequestFile(requests);
									} else if (tmp == 0) {
										printf("Request already exist\n");
										sendCode(new_socket , ALREADY_REQUESTED_TO_JOIN);
									} else if (tmp == -1) {
										printf("User has been invited to the group\n");
										sendCode(new_socket , HAS_BEEN_INVITED);
									}
                                    break;                               
							case VIEW_GROUP_JOINED:
								printf("VIEW_GROUP_JOINED\n");	
								singleList joined_group;
								createSingleList(&joined_group);
								joined_group = joinedGroups(users, loginUser->user_name);
								char joined_str[200];
								convertSimpleGroupsToString(joined_group, joined_str);
								if(strlen(joined_str) == 0){
									sendCode(new_socket, NO_GROUP_TO_ACCESS);
									break;
								}
								snprintf(response, sizeof(response), "%d %s", VIEW_GROUP_JOINED, joined_str);
								sendWithCheck(new_socket, response, strlen(response) + 1, 0);				
								break;
							case ACCESS_GROUP_REQUEST:
								printf("ACCESS_GROUP_REQUEST\n");							
								if (REQUEST != NO_GROUP_TO_ACCESS) {
									printf("Group has choosen: %s\n", data);
									char current_group[50];
									strcpy(current_group, data);
									sendCode(new_socket, ACCESS_GROUP_SUCCESS);						
									while (REQUEST != BACK_REQUEST) {
										memset(buff, 0, sizeof(buff));
										readWithCheck(new_socket, buff, BUFF_SIZE);
										printf("%s\n",buff);
                        				request_code = strtok(buff, " ");
        								REQUEST = atoi(request_code);
										data = strtok(NULL, " ");

										switch(REQUEST) {
											case VIEW_USER_NOT_IN_GROUP:
												printf("VIEW_USER_NOT_IN_GROUP\n");
												singleList unjoined_members;
												createSingleList(&unjoined_members);
												unjoined_members = unJoinedMembers(users, current_group);
												convertSimpleGroupsToString(unjoined_members, str);
												if(strlen(str)==0){
													sendCode(new_socket, NO_MEMBER_TO_INVITE);
													break;
												}
												snprintf(response, sizeof(response), "%d %s", VIEW_USER_NOT_IN_GROUP, str);
												sendWithCheck(new_socket, response, strlen(response) + 1, 0);
												break;
											
											case VIEW_USER_IN_GROUP:
												if (isUserAMember(users, current_group, loginUser->user_name) == 1) {
													printf("VIEW_USER_IN_GROUP\n");
													if(isOwnerOfGroup(groups, current_group,loginUser->user_name) == 0){
															sendCode(new_socket, NOT_OWNER_OF_GROUP);
													} else {
														singleList members;
														createSingleList(&members);
														members = getAllMembersOfGroup(groups, current_group);
														convertSimpleUsersToString(members, str);
														if (strlen(str) == 0) {
															sendCode(new_socket, NO_MEMBER_TO_KICK);
															break;
														}
														snprintf(response, sizeof(response), "%d %s", VIEW_USER_IN_GROUP, str);
														sendWithCheck(new_socket, response, strlen(response) + 1, 0);
													}
												} else {
													printf("You was kicked!\n");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;
																					
											case INVITE_MEMBER_REQUEST:
												printf("INVITE_MEMBER_REQUEST\n");
												printf("group = %s, member = %s\n", current_group, data);
												int tmp = updateRequest(&requests, current_group, data, 1);
												if (tmp == 1) {
													printf("Update request successfully\n");
													sendCode(new_socket , INVITE_SUCCESS);
													writeToRequestFile(requests);
												}
												else if (tmp == 0) {
													printf("Request already exist\n");
													sendCode(new_socket, HAS_BEEN_INVITED);
												}
												else if (tmp == -1) {
													printf("User has requested to join this group\n");
													sendCode(new_socket, ALREADY_REQUESTED_TO_JOIN);
												}
												break;

											case VIEW_REQUEST_IN_GROUP:
												printf("VIEW_REQUEST_IN_GROUP\n");
												if(isOwnerOfGroup(groups, current_group,loginUser->user_name) == 0){
														sendCode(new_socket, NOT_OWNER_OF_GROUP);
												}
												else {
													singleList request_list;
													createSingleList(&request_list);
													request_list = getRequests(requests, current_group);
													convertUserRequestsToString(request_list, str);
													if(strlen(str)==0){
														sendCode(new_socket, NO_REQUEST_TO_APPROVE);
														break;
													}
													snprintf(response, sizeof(response), "%d %s", VIEW_REQUEST_IN_GROUP, str);
													sendWithCheck(new_socket, response, strlen(response) + 1, 0);
												}
												break;
											case APPROVE_REQUEST:												
												printf("group = %s, member = %s\n", current_group, data);												
												//delete request
												deleteRequest(&requests, current_group, data, 1);
												writeToRequestFile(requests);
												if (addMember(groups, current_group, data) + addGroupToJoinedGroups(users, data, current_group) == 2) {
													sendCode(new_socket, APPROVE_SUCCESS);
													saveUsers(users);
													writeToGroupFile(groups);
												}																								
												break;

											case KICK_MEMBER_REQUEST:
												if (isUserAMember(users, current_group, loginUser->user_name) == 1) {
													printf("KICK_MEMBER_REQUEST\n");
													if(isOwnerOfGroup(groups, current_group,loginUser->user_name) == 0){
														sendCode(new_socket, NOT_OWNER_OF_GROUP);
													} else {
														printf("group = %s kick member = %s\n", current_group, data);
														kickMemberOut(groups, users, current_group, data);
														sendCode(new_socket, KICK_MEMBER_SUCCESS);
														singleList members1;
														createSingleList(&members1);
														members1 = getAllMembersOfGroup(groups, current_group);
														printUser(members1);
													}
												} else {
													printf("You was kicked!\n");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;

											case VIEW_USERS_OF_GROUP_REQUEST:
												printf("VIEW_USERS_OF_GROUP_REQUEST\n");
												if (isUserAMember(users, current_group, loginUser->user_name) == 1) {
													singleList members;
													createSingleList(&members);
													members = getAllMembersOfGroup(groups, current_group);
													convertSimpleUsersToString(members, str);
													if(strlen(str)==0){
														sendCode(new_socket, NO_REQUEST_TO_APPROVE);
														break;
													}
													snprintf(response, sizeof(response), "%d %s", VIEW_USERS_OF_GROUP_REQUEST, str);
													sendWithCheck(new_socket, response, strlen(response) + 1, 0);
												} else {
													printf("You was kicked!\n");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;

											case QUIT_GROUP_REQUEST:
												printf("QUIT_GROUP_REQUEST\n");
												if (isOwnerOfGroup(groups, current_group, loginUser->user_name) == 1) {
													sendCode(new_socket, IS_OWNER_OF_GROUP);
												} else {
													kickMemberOut(groups, users, current_group, loginUser->user_name);
													sendCode(new_socket, QUIT_GROUP_SUCCESS);
												}
												// REQUEST = BACK_REQUEST;
												break;
											
											case BACK_REQUEST:
												printf("BACK_REQUEST\n");
												writeToGroupFile(groups);
												break;
											default:
												break;
										}
									}
								}
								break;
							case NOTIFICATION_REQUEST: 
								printf("NOTIFICATION_REQUEST\n");
								singleList request_list;
								createSingleList(&request_list);
								request_list = getInvites(requests, loginUser->user_name);
								convertGroupsInviteToString(request_list, str);
								if(strlen(str) == 0){
									sendCode(new_socket,NO_INVITE);
									break;
								}
								snprintf(response, sizeof(response), "%d %s", NOTIFICATION_REQUEST, str);
								sendWithCheck(new_socket, response, strlen(response)+1, 0);
							case ACCEPT_INVITE_REQUEST: 
									printf("group = %s\n", data);
									//delete request
									deleteRequest(&requests, data, loginUser->user_name, 0);
									writeToRequestFile(requests);
									if (addMember(groups, data, loginUser->user_name) + addGroupToJoinedGroups(users, loginUser->user_name, buff) == 2) {
										sendCode(new_socket, ACCEPT_SUCCESS);
										saveUsers(users);
										writeToGroupFile(groups);
									}
									else {
										sendCode(new_socket , NO_ACCEPT_INVITE);
									}
								
								break;
                            case LOGOUT_REQUEST:
                                printf("LOGOUT_REQUEST\n");
                                loginUser = NULL;
                                sendCode(new_socket, LOGIN_SUCCESS);
                                break;

                            default:
                                break;
                        }
                    }
                }  
                break;
            default:
                break;  
        }
    }
	close(new_socket);
}
    
