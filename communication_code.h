//================= REQUEST CODE ===================//
#define REGISTER_REQUEST 0
#define LOGIN_REQUEST 1

#define CREATE_GROUP_REQUEST 11
#define JOIN_GROUP_REQUEST 12
#define ACCESS_GROUP_REQUEST 13
#define LOGOUT_REQUEST 14
#define SEARCH_FILE_REQUEST 15

#define UPLOAD_REQUEST 131
#define DOWNLOAD_REQUEST 132
#define DELETE_REQUEST 133
#define VIEW_FILES_REQUEST 134
#define BACK_REQUEST 135
#define KICK_MEMBER_REQUEST 136
#define APPROVE_REQUEST 137
#define NO_REQUEST_WERE_ACCEPTED 138
#define INVITE_MEMBER_REQUEST 139
#define NOTIFICATION_REQUEST 140
#define QUIT_GROUP_REQUEST 141
#define VIEW_USERS_OF_GROUP_REQUEST 142
#define RENAME_REQUEST 143

//==================================================//

//====================== ALERT =====================//
#define REGISTER_SUCCESS 800
#define LOGIN_SUCCESS 801
#define ACCOUNT_BLOCKED 802
#define CREATE_GROUP_SUCCESS 803
#define JOIN_GROUP_SUCCESS 804
#define ACCESS_GROUP_SUCCESS 805
#define LOGOUT_SUCCESS 806
#define UPLOAD_SUCCESS 807
#define MEMBER_WAS_KICKED 808
#define REQUESTED_TO_JOIN 809
#define APPROVE_SUCCESS 810
#define INVITE_SUCCESS 811
#define ACCEPT_SUCCESS 812
#define QUIT_GROUP_SUCCESS 813

//==================================================//

//===================== INVALID ====================//
#define INCORRECT_PASSWORD 900
#define EXISTENCE_USERNAME 901
#define EXISTENCE_GROUP_NAME 902
#define EXISTENCE_FILE_NAME 903
#define NON_EXISTENCE_USERNAME 904
#define NOT_OWNER_OF_GROUP 905
#define NO_FILE_TO_DELETE 906
#define NO_FILE_TO_DOWNLOAD 907
#define NO_GROUP_TO_JOIN 908
#define NO_MEMBER_TO_KICK 909
#define NO_GROUP_TO_ACCESS 910
#define ALREADY_REQUESTED_TO_JOIN 911
#define HAS_BEEN_INVITED 912
#define NO_REQUEST_TO_APPROVE 913
#define NO_INVITE 914
#define NO_MEMBER_TO_INVITE 915
#define NO_ACCEPT_INVITE 916
#define IS_OWNER_OF_GROUP 917
#define NO_FILE_TO_RENAME 918
#define INVALID_FORMAT 919
//==================================================//
