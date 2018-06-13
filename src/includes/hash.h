#ifndef hash_h
#define hash_h

//hash and salt struct
//use hashsalt.hash to get the hash
//use hashsalt.salt to get the salt
struct hashsalt {
	union{
		char HS[81];
	struct{
		char hash[40];
		char salt[41];
	};
	};
};

//function prototypes
unsigned int 	randomNumber(void);
char*		generateSalt(void);
char* 		hashString(char* org);
char*		hashWsalt(char* pass, char* salt);
struct hashsalt generateNewPass(char* pass);
int		checkPass(struct hashsalt hs, char* pass);

#endif //hash_h
