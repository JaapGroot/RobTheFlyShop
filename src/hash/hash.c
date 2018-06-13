#include "includes.h"

//Description: Function that opens /dev/urandom/ and get a random number from it
//@input: 	void
//@return: 	unsigned int of a random ten digit number 
unsigned int randomNumber(void)
{
	unsigned int 	randval;
	FILE 		*f;
	
	//open from file /dev/urandom/ and get a 10 digit random number from it.
	f = fopen("/dev/urandom", "r");
	fread(&randval, sizeof(randval), 1, f);
	fclose(f);
	return randval;
}


//Description: 	Function generating a random Salt. For now it doesn't do very much...
//		other than making a hash. The only problem I have is converting the 
//		"randomhash" and making it a readable hexstring from it.
//@input:	nothing, just make the salt for me please...
//@output:	Char* of the salt.
char* generateSalt(void)
{
	unsigned char	numberString[10];
	unsigned int 	randNumber;
	char	*salt;
	
	randNumber = randomNumber();
	
	snprintf(numberString, sizeof(numberString),  "%u", randNumber);
	
	salt = hashString(numberString);
	return salt;
}

//Description: hash a string unsing the hashingmethod of SHA256
//@input: 	unsigned char* of the original string 
//@output:	unsigned char* of the hashed string
char* hashString(char* org)
{
	//hash the original string
	unsigned char	*d = SHA256((const unsigned char*)org, strlen(org), 0);
	//change the hash into a hex string
	static char hexstring[41];
	char hexvalue[3];
	snprintf(hexvalue, 3, "%02x", *d);
	strcpy(hexstring, hexvalue);
	for(int i = 1; i < 20; i++){
		snprintf(hexvalue, 3, "%02x", *(d+i));
		strcat(hexstring, hexvalue);
	}
	hexstring[40] = '\0';
	return hexstring;
}

//Description: hash password using the plaintext password and the salt
//@input:	char* of the password, unsigned char* of the salt
//@output:	char* of the hashed password
char*	hashWsalt(char* pass, char* salt){
	static char	*hashed;
	char		combinedstrings[80];

	//concatinate the salt ant the password	
	strcpy(combinedstrings, salt);
	strcat(combinedstrings, pass);
	//hash the salt and password
	hashed = hashString(combinedstrings);
	//return the hash
	return hashed;
}

//function that generates new password
//@input: char* of password
//@output: hashsalt struct, with hashed password + salt used
struct hashsalt	generateNewPass(char* pass){
	//alloc a hashsalt struct
	struct hashsalt hs;
	//add a new salt to the struct
	strcpy(hs.salt, generateSalt());
	//generate the hash with teh salt
	memcpy(hs.hash, hashWsalt(pass, hs.salt), 40);
	
	//return the struct
	return hs;
}

//function that takes a plaintext password, and compares it to the hash from the database
//@input: hashsalt struct to compare to
//@input: char * to password
int checkPass(struct hashsalt hs, char* pass){
	//hash the password with the old salt
	if(!memcmp(hs.hash, hashWsalt(pass, hs.salt), 40)){
		return(KORE_RESULT_OK);
	}else{
		return(KORE_RESULT_ERROR);
	}
}
