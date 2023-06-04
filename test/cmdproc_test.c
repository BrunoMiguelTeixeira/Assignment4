#include <stdio.h>
#include "cmdproc.h"
#include "../unity/unity.h"

extern int led[4];
extern int but[4];
extern int temp;


void test_cmdProcessor_led_response(void)
{
	resetCmdString();
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('0');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('1');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('2');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('3');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	TEST_ASSERT_EQUAL_INT(1, led[0]);
	TEST_ASSERT_EQUAL_INT(1, led[1]);
	TEST_ASSERT_EQUAL_INT(1, led[2]);
	TEST_ASSERT_EQUAL_INT(1, led[3]);
	resetCmdString();
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('1');
	newCmdChar('0');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	TEST_ASSERT_EQUAL_INT(0, led[1]);
	
}


void test_cmdProcessor_wrong(void)
{
	int old_led[4] = {led[0],led[1],led[2],led[3]};
	resetCmdString();
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('0');
	newCmdChar('2');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(CS_ERR,cmdProcessor());
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('0');
	newCmdChar('.');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(CS_ERR,cmdProcessor());
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('5');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(CS_ERR,cmdProcessor());
	newCmdChar('#');
	newCmdChar('D');
	newCmdChar('.');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(CS_ERR,cmdProcessor());
	TEST_ASSERT_EQUAL_INT(old_led[0], led[0]);
	TEST_ASSERT_EQUAL_INT(old_led[1], led[1]);
	TEST_ASSERT_EQUAL_INT(old_led[3], led[2]);
	TEST_ASSERT_EQUAL_INT(old_led[3], led[3]);
}

void test_cmdProcessor_but_response(void)
{
	resetCmdString();
	newCmdChar('#');
	newCmdChar('B');
	newCmdChar('0');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	newCmdChar('#');
	newCmdChar('B');
	newCmdChar('1');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	newCmdChar('#');
	newCmdChar('B');
	newCmdChar('2');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());
	newCmdChar('#');
	newCmdChar('B');
	newCmdChar('3');
	newCmdChar('!');
	TEST_ASSERT_EQUAL_INT(OK,cmdProcessor());	
	
}

int main(void) 
{
	UNITY_BEGIN();
	RUN_TEST(test_cmdProcessor_led_response);
	RUN_TEST(test_cmdProcessor_wrong);
	RUN_TEST(test_cmdProcessor_but_response);
	
	return UNITY_END();
}

