#include <stdio.h>

#include "cmdproc.h"

/* PID parameters */
/* Note that in a real application these vars would be extern */
char Kp, Ti, Td, CheckSum;

/* Process variables */ 
/* Note that in a real application these vars would be extern */
int setpoint, output, error; 

/* Internal variables */
static char cmdString[MAX_CMDSTRING_SIZE];
static unsigned char cmdStringLen = 0; 


int cmdProcessor(void)
{
	int i, j;
	/* Detect empty cmd string */
	if(cmdStringLen == 0)
		return EMPTY_STRING; 
	
	/* Find index of SOF */
	for(i = 0; i < cmdStringLen; i++) {
		if(cmdString[i] == SOF_SYM) {
			break;
		}
	}
	
	/* Send error in case SOF isn't found */
	if (i == cmdStringLen){
	 return STR_FORMAT_ERR;
	}
	
	/* Find index of EOF */
	for(j = 0; j < cmdStringLen; j++) {
		if(cmdString[j] == EOF_SYM) {
			break;
		}
	}
	
	/* Send error in case EOF isn't found */
	if (cmdString[j] != '!'){
	 return STR_FORMAT_ERR;
	}
	
	/* If a SOF was found look for commands */
	if(i < cmdStringLen) {
		switch(cmdString[i+1]){
			/* In case of the P command is detected */
			case 'P': 
				/* If there is not enough values */
				if(j - i != 6){
					resetCmdString();
					return CS_ERR;
				}
					
				Kp = cmdString[i+2];
				Ti = cmdString[i+3];
				Td = cmdString[i+4];
				
				/* Wrong SumCheck on string return error */
				if((char)(cmdString[i+1]+Kp+Ti+Td)!=cmdString[i+5]){
					printf("%u",cmdString[i+5]);
					resetCmdString();
					return ERR_SUM;
					}
				break;
			
			/* In case of the S command is detected */
			case 'S':
				if(j - i != 2){
					resetCmdString();
					return CS_ERR;
				}
				printf("Setpoint = %d, Output = %d, Error = %d\n", setpoint, output, error);
				resetCmdString();
				break;
			case 'R':
				if(j - i != 2){
					resetCmdString();
					return CS_ERR;
				}
				Kp = '0';
				Ti = '0';
				Td = '0';
				resetCmdString();
				break;

			case 'A':
				/* If there is not enough values */
				if(j - i != 14){
					resetCmdString();
					return CS_ERR;
				}
				/* passing Char value to decimal so its well interpreted */
				Kp = (cmdString[i+2]-'0')*100 +(cmdString[i+3]-'0')*10+ (cmdString[i+4]-'0');
				Ti = (cmdString[i+5]-'0')*100 +(cmdString[i+6]-'0')*10+ (cmdString[i+7]-'0');
				Td = (cmdString[i+8]-'0')*100 +(cmdString[i+9]-'0')*10+ (cmdString[i+10]-'0');
				
				/* temporary variables */
				char checkssum=0;
				int a;

				for (a=0;a<11;a++){
					checkssum+=(unsigned char)cmdString[a];
				}

				/* Wrong SumCheck on string return error */
				if((char)(checkssum!=((cmdString[i+11]-'0')*100+(cmdString[i+12]-'0')*10+cmdString[i+13]-'0')))
				{
					resetCmdString();
					return ERR_SUM;
				}
				break;

			default:
				return INV_COMAND;
		}
	}
			return OK;

}

int newCmdChar(unsigned char newChar)
{
	/* If cmd string not full add char to it */
	if (cmdStringLen < MAX_CMDSTRING_SIZE) {
		cmdString[cmdStringLen] = newChar;
		cmdStringLen +=1;

		return OK;		
	}
	/* If cmd string full return error */
	return STR_FULL;
}

int newCmdStr(char* newCmd)
{
	int cnt = 0;
	int i,checksum = 0;
	
	/*String size*/
	for(i = 0; newCmd[i] != '\0'; i++)
	{
		cnt++;
	}
	
	/*String size error ( -1 to add checksum)*/
	if(cnt > (MAX_CMDSTRING_SIZE - 3))
	{
		return OVF_STR;
	}
	
	
	cmdString[0] = '#';
	
	/*checksum*/
	if (cnt > 1)
	{
		for(i = 0; i < cnt + 1; i++)
		{
			checksum = checksum + newCmd[i];
			cmdString[i+1] = newCmd[i];
		}
		cmdString[cnt+1] = (unsigned char)(checksum);
		cmdString[cnt+2] = '!';
		cmdStringLen = cnt + 3;	
	}
	else
	{
		for(i = 0; i < cnt + 1; i++)
		{
			cmdString[i+1] = newCmd[i];
		}
		cmdString[cnt+1] = '!';
		cmdStringLen = cnt + 2;	
	}
	

	
	return OK;
}

int resetCmdString(void)
{
	cmdStringLen = 0;		
	return OK;
}

int newCmdCharASCII(unsigned char newChar)
{
	/* If cmd string not full add char to it */
	if (cmdStringLen < MAX_CMDSTRING_SIZE) {
		int a= newChar%10;
		int b= newChar/10;

		/* if value in newChar is higher than 100*/
		if(b>=10){
			cmdString[cmdStringLen]='0'+ (b/10);
			cmdStringLen++;
			cmdString[cmdStringLen]='0'+ (b%10);
			cmdStringLen++;
			cmdString[cmdStringLen]='0'+a;
			cmdStringLen++; 
			return OK;
		}

		/* if value in newChar is between 10 and 99 */
		if(b!=0){
			cmdString[cmdStringLen]='0';
			cmdStringLen++;
			cmdString[cmdStringLen]='0'+b;
			cmdStringLen++;
			cmdString[cmdStringLen]='0'+a;
			cmdStringLen++;

			return OK;
		}

		/* if value in newChar is less than 10 */
		cmdString[cmdStringLen]='0';
		cmdStringLen++;
		
		cmdString[cmdStringLen]='0';
		cmdStringLen++;

		cmdString[cmdStringLen]='0'+a;
		cmdStringLen++;
		
		return OK;		
	}
	/* If cmd string full return error */
	return STR_FULL;
}


int stringDebug(void){
	
	if(cmdStringLen == 0)
		return EMPTY_STRING;
		
	printf("%c",cmdString[0]);
	printf("%c",cmdString[1]);
	int i;
	for(i=0;i<cmdStringLen-1;i++){
		printf("%c",cmdString[i+2]);
		if((i+1)%3==0)
			printf(" ");
	}
	printf("\n");
	return OK;
}



unsigned char checkSumCalc(void){
	int i;
	char sum;
	for(i=0;i<cmdStringLen;i++){
		sum+=cmdString[i];
	}
	return sum;
}
