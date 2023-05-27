#ifndef __CMD_PROC_H_
#define __CMD_PROC_H_

#define MAX_CMDSTRING_SIZE 15 /* Maximum size of the command string */ 
#define SOF_SYM '#'	          /* Start of Frame Symbol */
#define EOF_SYM '!'           /* End of Frame Symbol */

/**\file cmdproc.h
 *
 * */


/** \brief Enum with error codes
 * 
 *  0: Executed function with success;		                                                          	    
 * -1: empty string;          
 * -2: an invalid command was found;                         
 * -3: a CS error is detected (command not executed);        
 * -4: string format is wrong;                               
 * -5: string size is full and a new char is inputted; 						
 * -6: wrong CheckSum value; 	
 * -7: String to long to be command;
 * 							*/
 
enum error{ 	OVF_STR = -7,
		ERR_SUM,			/* -6 */	
		STR_FULL,			/* -5 */
		STR_FORMAT_ERR,		/* -4 */
		CS_ERR,				/* -3 */
		INV_COMAND,			/* -2 */
		EMPTY_STRING,		/* -1 */
		OK};				/*  0 */
		

/* Function prototypes */
/**
 * \brief Function responsible for processing the command string
 * 
 * Comands\n
 *   P: Atributs values Kp, Ti and Td;\n
 *   S: Returns values of setpoint, output, error;\n
 *   R: Resets values of Kp, Ti and Td ('0');\n
 *   A: Atributs values to Kp, Ti and Td with ascii convertion;
 * \returns 0: OK;\n
 *  -1:Empty String;\n
 *  -2:Invalid Cmd;\n
 *  -3:Cmd with errors;\n
 *  -4: String format wrong;\n
 *  -6: Error Checksum; 
 */
int cmdProcessor(void);

/**
 * \brief function responsible for insert a new char in command string
 * \param char new character
 * \returns 0: OK -5: string full
 */
int newCmdChar(unsigned char newChar);

/**
 * \brief Function responsible for reset command string
 * \returns 0: OK
 */
int resetCmdString(void);

/**
 * \brief Function responsible for insert a string in command string
 * \param string to be turned into a command
 * \returns 0: OK\n -7: string too big 
 */
int newCmdStr(char* newCmd);


/**
 * \brief Converts value in ASCII char and writes it in command string
 * \param char new character
 * \returns 0: OK -5: string full
 */
int newCmdCharASCII(unsigned char newChar);

/**
 * \brief Prints command in cmdString in ascii for debug
 * \returns 0: OK\n -1: Empty string
 */
int stringDebug(void);

/**
 * \brief Calculates checksum
 * \returns Check Sum
 */
unsigned char checkSumCalc(void);

#endif
