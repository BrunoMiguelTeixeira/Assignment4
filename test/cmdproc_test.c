#include <stdio.h>
#include "cmdproc.h"
#include "../unity/unity.h"

extern char Kp, Ti, Td, CheckSum;
extern int setpoint, output, error; 


void test_cmdProcessor_should_res_KP_Ti_Td(void)
{
	char CheckSum = (unsigned char)('P'+'1'+'2'+'3');
	resetCmdString();
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('3');
	newCmdChar(CheckSum);
	newCmdChar('!');
	stringDebug();
	TEST_ASSERT_EQUAL_INT(0,cmdProcessor());
	TEST_ASSERT_EQUAL_INT('1', Kp);
	TEST_ASSERT_EQUAL_INT('2', Ti);
	TEST_ASSERT_EQUAL_INT('3', Td);	
}


void test_EmptyString_should_0(void)
{
	resetCmdString();
	TEST_ASSERT_EQUAL_INT(-1,cmdProcessor());
}

void test_FullString(void)
{
	resetCmdString();
	/* Fills the string max size(10) with SOF */
	int i;
	for(i=0;i<15;i++){
		newCmdChar('#');
	}
	TEST_ASSERT_EQUAL_INT(-5,newCmdChar('#'));
	
}

void test_InvalidCmd(void)
{
	resetCmdString();
	newCmdChar('#');
	newCmdChar('D');	/*check invalid command D */
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(-2,cmdProcessor());
}

void test_InvalidSOF(void)
{	
	resetCmdString();
	newCmdChar('S');	/* check no SOF */
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(-4,cmdProcessor());
	resetCmdString();
	newCmdChar('+');	/* check invalid SOF */
	newCmdChar('S');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(-4,cmdProcessor());
}

void test_InvalidEOF(void)
{	
	resetCmdString();
	newCmdChar('#');	/* check wrong EOF */
	newCmdChar('S');
	newCmdChar('-');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-4,cmdProcessor(),"wrong char passed!");
	newCmdChar('#'); 	/* check no EOF */
	newCmdChar('S');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-4,cmdProcessor(),"Nothing passed");
}


void test_invalid_P_CmdWrongNumParam(void)
{
	/*check no Kp, Ti Td*/
	resetCmdString();
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-3,cmdProcessor(),"missing values still run");
	/*check no Ti, Td*/
	resetCmdString();
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-3,cmdProcessor(),"missing values still run");
	/*check no Td*/
	resetCmdString();
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-3,cmdProcessor(),"missing values still run");
	/*check no Checksum*/
	resetCmdString();
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-3,cmdProcessor(),"missing values still run");
}

void test_invalid_sumCheck(void)
{
	char CheckSum = (unsigned char)('P'+'1'+'2'+'3'+'+');
	/* check SumCheck error */
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('3');
	newCmdChar(CheckSum);
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-6,cmdProcessor(),"not wrong SumCheck");
}

void test_cmdProcessor_S(void){

	resetCmdString();
	newCmdChar('#');
	newCmdChar('S');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(0,cmdProcessor());
	
	resetCmdString();
	newCmdChar('#');
	newCmdChar('S');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(-3,cmdProcessor());
}

void test_newCmdStr(void){
	char test_string_1[] = "QWERTYQWERTYQUERTY\0";
	TEST_ASSERT_EQUAL_INT(-7,newCmdStr(test_string_1));
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0,cmdProcessor(), "BIG STRING TEST");
	
	char test_string_2[]= "P123\0";
	TEST_ASSERT_EQUAL_INT(0,newCmdStr(test_string_2));
	TEST_ASSERT_EQUAL_INT_MESSAGE(0,cmdProcessor(),"SMALL STRING TEST");
	
	char test_string_3[]= "S\0";
	TEST_ASSERT_EQUAL_INT(0,newCmdStr(test_string_3));
	TEST_ASSERT_EQUAL_INT_MESSAGE(0,cmdProcessor(),"S cmd");
	
	char test_string_4[]= "\0";
	TEST_ASSERT_EQUAL_INT(0,newCmdStr(test_string_4));
	TEST_ASSERT_EQUAL_INT_MESSAGE(-2,cmdProcessor(),"Empty cmd");
}


void test_cmdProcessorASCII(void){
	resetCmdString();
	newCmdChar('#');
	newCmdChar('A');
	newCmdCharASCII(1);
	newCmdCharASCII(10);
	newCmdCharASCII(24);
	newCmdCharASCII(checkSumCalc());
	newCmdChar('!');
	stringDebug();
	TEST_ASSERT_EQUAL_INT(0,cmdProcessor());
	TEST_ASSERT_EQUAL_INT(1, Kp);
	TEST_ASSERT_EQUAL_INT(10, Ti);
	TEST_ASSERT_EQUAL_INT(24, Td);	
}

void test_stringDebug_empty(void){
	resetCmdString();
	TEST_ASSERT_EQUAL_INT(-1,stringDebug());
}


int main(void) 
{
	UNITY_BEGIN();
	RUN_TEST(test_cmdProcessor_should_res_KP_Ti_Td);
	RUN_TEST(test_EmptyString_should_0);
	RUN_TEST(test_FullString);
	RUN_TEST(test_InvalidCmd);
	RUN_TEST(test_InvalidSOF);
	RUN_TEST(test_InvalidEOF);
	RUN_TEST(test_invalid_P_CmdWrongNumParam);
	RUN_TEST(test_invalid_sumCheck);
	RUN_TEST(test_cmdProcessor_S);
	RUN_TEST(test_newCmdStr);
	RUN_TEST(test_cmdProcessorASCII);
	RUN_TEST(test_stringDebug_empty);
	return UNITY_END();
}
