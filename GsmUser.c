#include "Sim80x.h"
//##################################################################################################################
void  Gsm_User(uint32_t StartupTime)
{
  
  
}
//##################################################################################################################
void  Gsm_UserNewCall(const char *CallerNumber)
{
  Gsm_CallDisconnect();
	osDelay(20);
	//Gsm_MsgSendText((char *)CallerNumber,"Hello");
	//Gsm_MsgSendTextUnicode((char *)CallerNumber,"سلام");
  //Gsm_CallAnswer();    

}
//##################################################################################################################
void  Gsm_UserNewMsg(char *Number,char *Date,char *Time,char *msg)
{
  char num[16];
  
	strcpy(num,Number);
  
}
//##################################################################################################################
