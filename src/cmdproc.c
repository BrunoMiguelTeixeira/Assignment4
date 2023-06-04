#include <stdio.h>

#include "cmdproc.h"

/* database*/
int led[4] = {0,0,0,0};
int but[4] = {0,1,0,1};
int temp = 25;

/* Internal variables */
static char cmdString[MAX_CMDSTRING_SIZE];
static unsigned char cmdStringLen = 0;

int cmdProcessor(void)
{
	int i, j;
	
	/* Detect empty cmd string */
	if (cmdStringLen == 0)
		return EMPTY_STRING;

	/* Find index of SOF */
	for (i = 0; i < cmdStringLen; i++)
	{
		if (cmdString[i] == SOF_SYM)
		{
			break;
		}
	}

	/* Send error in case SOF isn't found */
	if (cmdString[i] != SOF_SYM)
	{
		return STR_FORMAT_ERR;
	}

	/* Find index of EOF */
	for (j = 0; j < cmdStringLen; j++)
	{
		if (cmdString[j] == EOF_SYM)
		{
			break;
		}
	}

	/* Send error in case EOF isn't found */
	if (cmdString[j] != EOF_SYM)
	{
		return STR_FORMAT_ERR;
	}
	
	/* If a SOF was found look for commands */
	if (i < cmdStringLen)
	{
		switch (cmdString[i + 1])
		{
		/* In case of the D command is detected */
		case 'D':
			/* If there is not enough values */
			if ( (j - i + 1) != 5)
			{
				resetCmdString();
				return CS_ERR;
			}
			
			int index = cmdString[i + 2] - '0';
			int state = cmdString[i + 3] - '0';
			
			if ((index > 3 || index < 0) || (state > 1 || state < 0))
			{
				resetCmdString();
				return CS_ERR;
			}
			
			led[index] = state;
			break;
			
		/* In case of the T command is detected */
		case 'T':
			if ( (j - i + 1) != 3)
			{
				resetCmdString();
				return CS_ERR;
			}
			
			printf("%d",temp);
			break;
			
		/* In case of the B command is detected */
		case 'B':
			if ( (j - i + 1) != 4)
			{
				resetCmdString();
				return CS_ERR;
			}
			
			int index = cmdString[i + 2] - '0';
			
			if (index > 3 || index < 0)
			{
				resetCmdString();
				return CS_ERR;
			}
			
			printf("%d",but[index]);
			break;
		/* In case of the no command is detected */
		default:
			return INV_COMAND;
		}
	}
	resetCmdString();
	return OK;
}

int newCmdChar(unsigned char newChar)
{
	/* If cmd string not full add char to it */
	if (cmdStringLen < MAX_CMDSTRING_SIZE)
	{
		cmdString[cmdStringLen] = newChar;
		cmdStringLen += 1;

		return OK;
	}
	/* If cmd string full return error */
	return STR_FULL;
}

int newCmdStr(char *newCmd)
{
	int cnt = 0;
	int i, checksum = 0;

	/*String size*/
	for (i = 0; newCmd[i] != '\0'; i++)
	{
		cnt++;
	}

	/*String size error ( -1 to add checksum)*/
	if (cnt > (MAX_CMDSTRING_SIZE - 3))
	{
		return OVF_STR;
	}

	cmdString[0] = '#';

	/*checksum*/
	if (cnt > 1)
	{
		for (i = 0; i < cnt + 1; i++)
		{
			checksum = checksum + newCmd[i];
			cmdString[i + 1] = newCmd[i];
		}
		cmdString[cnt + 1] = (unsigned char)(checksum);
		cmdString[cnt + 2] = '!';
		cmdStringLen = cnt + 3;
	}
	else
	{
		for (i = 0; i < cnt + 1; i++)
		{
			cmdString[i + 1] = newCmd[i];
		}
		cmdString[cnt + 1] = '!';
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
	if (cmdStringLen < MAX_CMDSTRING_SIZE)
	{
		int a = newChar % 10;
		int b = newChar / 10;

		/* if value in newChar is higher than 100*/
		if (b >= 10)
		{
			cmdString[cmdStringLen] = '0' + (b / 10);
			cmdStringLen++;
			cmdString[cmdStringLen] = '0' + (b % 10);
			cmdStringLen++;
			cmdString[cmdStringLen] = '0' + a;
			cmdStringLen++;
			return OK;
		}

		/* if value in newChar is between 10 and 99 */
		if (b != 0)
		{
			cmdString[cmdStringLen] = '0';
			cmdStringLen++;
			cmdString[cmdStringLen] = '0' + b;
			cmdStringLen++;
			cmdString[cmdStringLen] = '0' + a;
			cmdStringLen++;

			return OK;
		}

		/* if value in newChar is less than 10 */
		cmdString[cmdStringLen] = '0';
		cmdStringLen++;

		cmdString[cmdStringLen] = '0';
		cmdStringLen++;

		cmdString[cmdStringLen] = '0' + a;
		cmdStringLen++;

		return OK;
	}
	/* If cmd string full return error */
	return STR_FULL;
}

int stringDebug(void)
{

	if (cmdStringLen == 0)
		return EMPTY_STRING;

	printf("%c", cmdString[0]);
	printf("%c", cmdString[1]);
	int i;
	for (i = 0; i < cmdStringLen - 1; i++)
	{
		printf("%c", cmdString[i + 2]);
		if ((i + 1) % 3 == 0)
			printf(" ");
	}
	printf("\n");
	return OK;
}

unsigned char checkSumCalc(void)
{
	int i;
	char sum;
	for (i = 0; i < cmdStringLen; i++)
	{
		sum += cmdString[i];
	}
	return sum;
}
