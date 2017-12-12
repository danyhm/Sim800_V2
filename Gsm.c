#include "Sim80x.h"


//######################################################################################################
bool  Gsm_Ussd(char *send,char *receive)
{
  uint8_t answer;
  char str[32];
  snprintf(str,sizeof(str),"AT+CUSD=1,\"%s\"\r\n",send);
  memset(Sim80x.Gsm.Msg,0,sizeof(Sim80x.Gsm.Msg));
  answer = Sim80x_SendAtCommand(str,60000,2,"\r\n+CUSD:","\r\n+CME ERROR:");
  if((answer == 1) && (Sim80x.Gsm.Msg[0]!=0))
  {
    if(receive!=NULL)
      receive = Sim80x.Gsm.Msg;
    return true;
  }
  else
    return false;
}
//######################################################################################################
bool  Gsm_CallAnswer(void)
{
  uint8_t answer = Sim80x_SendAtCommand("ATA\r\n",20000,2,"\r\nOK\r\n","\r\nERROR\r\n");	//@todo NO CARRIER answer
  if(answer == 1)
  {
    Sim80x.Gsm.GsmVoiceStatus = GsmVoiceStatus_IAnswerTheCall;
    return true;
  }
  else
    return false;
}
//######################################################################################################
bool  Gsm_CallDisconnect(void)
{
  uint8_t answer = Sim80x_SendAtCommand("ATH\r\n",20000,2,"ATH\r\r\nOK\r\n","ATH\r\r\nERROR\r\n"); //@TODO no error in datasheet? datamode ?
  if(answer == 1)
  {
    Sim80x.Gsm.GsmVoiceStatus = GsmVoiceStatus_Idle;
    return true;
  }
  else
    return false;
}
//######################################################################################################
GsmVoiceStatus_t     Gsm_Dial(char *Number,uint8_t WaitForAnswer_second)
{
  char str[24];
  uint32_t  wait = WaitForAnswer_second*1000;
  Sim80x.Gsm.GsmVoiceStatus = GsmVoiceStatus_Calling;
  snprintf(Sim80x.Gsm.DiallingNumber,sizeof(Sim80x.Gsm.DiallingNumber),"%s",Number);
  snprintf(str,sizeof(str),"ATD%s;\r\n",Number);
  Sim80x_SendString(str);
  while(wait>0)
  {    
    if(Sim80x.Gsm.GsmVoiceStatus != GsmVoiceStatus_Calling)
      return Sim80x.Gsm.GsmVoiceStatus;
    osDelay(100);
    wait-=100;
  } 
  if(wait==0)
  {
    Gsm_CallDisconnect();
    Sim80x.Gsm.GsmVoiceStatus=GsmVoiceStatus_ReturnNoAnswer;
    return  Sim80x.Gsm.GsmVoiceStatus; 
  }
  Sim80x.Gsm.GsmVoiceStatus = GsmVoiceStatus_ReturnError;
  return Sim80x.Gsm.GsmVoiceStatus;
}
//######################################################################################################
GsmVoiceStatus_t     Gsm_GetLastVoiceActivity(void)
{ 
  return Sim80x.Gsm.GsmVoiceStatus;  
}
//######################################################################################################
//######################################################################################################
//######################################################################################################
GsmMsgFormat_t  Gsm_MsgGetFormat(void)
{
  uint8_t   answer;
  answer = Sim80x_SendAtCommand("AT+CMGF?\r\n",1000,2,"+CMGF: 0\r\n\r\nOK\r\n","+CMGF: 1\r\n\r\nOK\r\n");
  if(answer == 1)
  {
    Sim80x.Gsm.MsgFormat = GsmMsgFormat_PDU;
    return GsmMsgFormat_PDU;
  }
  else if(answer == 2)
  {
    Sim80x.Gsm.MsgFormat = GsmMsgFormat_Text;
    return GsmMsgFormat_Text;
  }
  else
    return GsmMsgFormat_Error;  
}
//######################################################################################################
bool  Gsm_MsgSetFormat(GsmMsgFormat_t GsmMsgFormat)
{
  uint8_t   answer;
  if(GsmMsgFormat == GsmMsgFormat_PDU)
    answer = Sim80x_SendAtCommand("AT+CMGF=0\r\n",1000,2,"AT+CMGF=0\r\r\nOK\r\n","AT+CMGF=0\r\r\nERROR\r\n");
  else  if(GsmMsgFormat == GsmMsgFormat_Text)
    answer = Sim80x_SendAtCommand("AT+CMGF=1\r\n",1000,2,"AT+CMGF=1\r\r\nOK\r\n","AT+CMGF=1\r\r\nERROR\r\n");
  else
    return false;
  if(answer == 1)
  {
    if(GsmMsgFormat == GsmMsgFormat_PDU)
      Sim80x.Gsm.MsgFormat = GsmMsgFormat_PDU;
    if(GsmMsgFormat == GsmMsgFormat_Text)
      Sim80x.Gsm.MsgFormat = GsmMsgFormat_Text;
    return true;    
  }
  else
    return false;  
}
//######################################################################################################
GsmMsgMemory_t  Gsm_MsgGetMemoryStatus(void)
{
  uint8_t   answer;
  answer = Sim80x_SendAtCommand("AT+CPMS?\r\n",1000,2,"AT+CPMS?\r\r\n+CPMS: \"SM\"","AT+CPMS?\r\r\n+CPMS: \"ME\"");
  if(answer == 1)
    Sim80x.Gsm.MsgMemory = GsmMsgMemory_OnSim;
  else if(answer == 2)
    Sim80x.Gsm.MsgMemory = GsmMsgMemory_OnModule;
  else
    Sim80x.Gsm.MsgMemory = GsmMsgMemory_Error;
  return  Sim80x.Gsm.MsgMemory; 
}
//######################################################################################################
bool  Gsm_MsgSetMemoryLocation(GsmMsgMemory_t GsmMsgMemory)
{
  uint8_t   answer;
  if(GsmMsgMemory == GsmMsgMemory_OnSim)
  {
    answer = Sim80x_SendAtCommand("AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n",1000,1,"\r\n+CPMS:");
    if(answer == 1)
    {
      Sim80x.Gsm.MsgMemory = GsmMsgMemory_OnSim;
      return true;
    }
    else
      return false;
  }
  if(GsmMsgMemory == GsmMsgMemory_OnModule)
  {
    answer = Sim80x_SendAtCommand("AT+CPMS=\"ME\",\"ME\",\"ME\"\r\n",1000,1,"\r\n+CPMS:");
    if(answer == 1)
    {
      Sim80x.Gsm.MsgMemory = GsmMsgMemory_OnModule;
      return true;
    }
    else
      return false;
  }
  return false;
}
//######################################################################################################
GsmTECharacterSet_t     Gsm_MsgGetCharacterFormat(void)
{
  Sim80x.Gsm.TeCharacterFormat = (GsmTECharacterSet_t)Sim80x_SendAtCommand("AT+CSCS?\r\n",1000,7,"\r\n+CSCS: \"GSM\"\r\n","\r\n+CSCS: \"UCS2\"\r\n","\r\n+CSCS: \"IRA\"\r\n","\r\n+CSCS: \"HEX\"\r\n","\r\n+CSCS: \"PCCP\"\r\n","\r\n+CSCS: \"PCDN\"\r\n","\r\n+CSCS: \"8859-1\"\r\n"); //@TODO fix for qtel
  return Sim80x.Gsm.TeCharacterFormat;
}
//######################################################################################################
bool  Gsm_MsgSetCharacterFormat(GsmTECharacterSet_t GsmTECharacterSet)
{
  uint8_t answer;
  switch(GsmTECharacterSet)
  {
    case GsmTECharacterSet_Error:
    return false;
    case GsmTECharacterSet_GSM:
      answer = Sim80x_SendAtCommand("AT+CSCS=\"GSM\"\r\n",1000,2,"AT+CSCS=\"GSM\"\r\r\nOK\r\n","AT+CSCS=\"GSM\"\r\r\n+CME ERROR");  
    break;
    case GsmTECharacterSet_UCS2:
      answer = Sim80x_SendAtCommand("AT+CSCS=\"UCS2\"\r\n",1000,2,"AT+CSCS=\"UCS2\"\r\r\nOK\r\n","AT+CSCS=\"UCS2\"\r\r\n+CME ERROR");  
    break;
    case GsmTECharacterSet_IRA:
      answer = Sim80x_SendAtCommand("AT+CSCS=\"IRA\"\r\n",1000,2,"AT+CSCS=\"IRA\"\r\r\nOK\r\n","AT+CSCS=\"IRA\"\r\r\n+CME ERROR");  
    break;
    case GsmTECharacterSet_HEX:
      answer = Sim80x_SendAtCommand("AT+CSCS=\"HEX\"\r\n",1000,2,"AT+CSCS=\"HEX\"\r\r\nOK\r\n","AT+CSCS=\"HEX\"\r\r\n+CME ERROR");  
    break;
    case GsmTECharacterSet_PCCP:
      answer = Sim80x_SendAtCommand("AT+CSCS=\"PCCP\"\r\n",1000,2,"AT+CSCS=\"PCCP\"\r\r\nOK\r\n","AT+CSCS=\"PCCP\"\r\r\n+CME ERROR");  
    break;
    case GsmTECharacterSet_PCDN:
      answer = Sim80x_SendAtCommand("AT+CSCS=\"PCDN\"\r\n",1000,2,"AT+CSCS=\"PCDN\"\r\r\nOK\r\n","AT+CSCS=\"PCDN\"\r\r\n+CME ERROR");  
    break;
    case GsmTECharacterSet_8859_1:
      answer = Sim80x_SendAtCommand("AT+CSCS=\"8859-1\"\r\n",1000,2,"AT+CSCS=\"8859-1\"\r\r\nOK\r\n","AT+CSCS=\"8859-1\"\r\r\n+CME ERROR");  
    break;   
  }  
  Gsm_MsgGetCharacterFormat();
  if(answer == 1)
    return true;
  else
    return false;
}
//######################################################################################################
bool  Gsm_MsgRead(uint8_t index)
{
  uint8_t answer;
  char str[16];
  memset(Sim80x.Gsm.Msg,0,sizeof(Sim80x.Gsm.Msg));
  memset(Sim80x.Gsm.MsgDate,0,sizeof(Sim80x.Gsm.MsgDate));
  memset(Sim80x.Gsm.MsgNumber,0,sizeof(Sim80x.Gsm.MsgNumber));
  memset(Sim80x.Gsm.MsgTime,0,sizeof(Sim80x.Gsm.MsgTime));  
  sprintf(str,"AT+CMGR=%d\r\n",index);
  answer = Sim80x_SendAtCommand(str,5000,2,"\r\nOK\r\n","\r\nERROR\r\n");
  if((answer == 1 ) && (Sim80x.Gsm.MsgReadIsOK==1))
    return true;
  else
    return false;    
}
//######################################################################################################
bool  Gsm_MsgDelete(uint8_t index)
{
  uint8_t answer;
  char str[16];
  char strP1[22];
  char strP2[22];
  sprintf(str,"AT+CMGD=%d\r\n",index);
  sprintf(strP1,"AT+CMGD=%d\r\r\nOK\r\n",index);
  sprintf(strP2,"AT+CMGD=%d\r\r\nERROR\r\n",index);
  answer = Sim80x_SendAtCommand(str,25000,2,strP1,strP2);
  if(answer == 1)
    return true;
  else
    return false;  
}
//######################################################################################################
bool  Gsm_MsgGetServiceNumber(void)
{
  uint8_t answer;
  answer = Sim80x_SendAtCommand("AT+CSCA?\r\n",5000,1,"\r\n+CSCA:");
  if((answer == 1) && (Sim80x.Gsm.MsgServiceNumber[0]!=0))
    return true;
  else
    return false;
}
//######################################################################################################
bool  Gsm_MsgSetServiceNumber(char *ServiceNumber)
{
  uint8_t answer;
  char str[32];
  char strParam[32];
  snprintf(str,sizeof(str),"AT+CSCA=\"%s\",145\r\n",ServiceNumber);
  snprintf(strParam,sizeof(str),"AT+CSCA=\"%s\"\r\r\nOK\r\n",ServiceNumber);
  answer = Sim80x_SendAtCommand(str,5000,1,strParam);
  if(answer == 1)
    return true;
  else
    return false;
}
//######################################################################################################
bool  Gsm_MsgGetTextModeParameter(void)
{
  uint8_t answer;
  answer = Sim80x_SendAtCommand("AT+CSMP?\r\n",500,1,"\r\nOK\r\n");
  if(answer == 1)
    return true;
  else
    return false;  
}
//######################################################################################################
bool  Gsm_MsgSetTextModeParameter(uint8_t fo,uint8_t vp,uint8_t pid,uint8_t dcs)
{
  uint8_t answer;
  char str[32];
  char strParam[32];
  snprintf(str,sizeof(str),"AT+CSMP=%d,%d,%d,%d\r\n",fo,vp,pid,dcs);
  snprintf(strParam,sizeof(strParam),"AT+CSMP=%d,%d,%d,%d\r\nOK\r\n",fo,vp,pid,dcs);
  answer = Sim80x_SendAtCommand(str,500,1,strParam);
  if(answer == 1)
    return true;
  else
    return false;
}
//######################################################################################################
bool  Gsm_MsgSendText(char *Number,char *msg)
{
  uint8_t answer;
  uint8_t Timeout=60;
  char str[128];
  Sim80x.Gsm.MsgSent=0;
  if(Sim80x.Gsm.MsgFormat != GsmMsgFormat_Text)
    Gsm_MsgSetFormat(GsmMsgFormat_Text);
	if(Sim80x.Gsm.TeCharacterFormat != GsmTECharacterSet_GSM)
		Gsm_MsgSetCharacterFormat(GsmTECharacterSet_GSM);
	if(Sim80x.Gsm.MsgTextModeParameterFo!=17 || Sim80x.Gsm.MsgTextModeParameterVp!=167 || Sim80x.Gsm.MsgTextModeParameterPid!=0 || Sim80x.Gsm.MsgTextModeParameterDcs!=0)
		Gsm_MsgSetTextModeParameter(17,167,0,0);	 		//make sure we're in right scheme
  snprintf(str,sizeof(str),"AT+CMGS=\"%s\"\r\n",Number);
  answer = Sim80x_SendAtCommand(str,10000,1,"\r\r\n> ");
  osDelay(100);
  if(answer != 1)
  {
    snprintf(str,sizeof(str),"%c",ESC_char_Dec);
    Sim80x_SendString(str);
    return false;
  }
  strcpy(Sim80x.Gsm.MsgSentNumber,Number);
  snprintf(str,sizeof(str),"%s%c",msg,SUB_char_Dec);
  Sim80x_SendString(str);
  while(Timeout>0)
  {
    osDelay(1000);
    Timeout--;
    if(Sim80x.Gsm.MsgSent == 1)
      return true;
  }
  return false;
}
//######################################################################################################
bool  Gsm_MsgSendTextUnicode(char *Number,char *msg)
{
	uint8_t answer;
  uint8_t Timeout=60;
  char str[128];
	char *ucs2num , *ucs2msg;	//alternative is to dedicated a known amount of ram for each instead of using calloc
  Sim80x.Gsm.MsgSent=0;
  if(Sim80x.Gsm.MsgFormat != GsmMsgFormat_Text)
    Gsm_MsgSetFormat(GsmMsgFormat_Text);
	if(Sim80x.Gsm.TeCharacterFormat != GsmTECharacterSet_UCS2)
		Gsm_MsgSetCharacterFormat(GsmTECharacterSet_UCS2);
	if(Sim80x.Gsm.MsgTextModeParameterFo!=17 || Sim80x.Gsm.MsgTextModeParameterVp!=167 || Sim80x.Gsm.MsgTextModeParameterPid!=0 || Sim80x.Gsm.MsgTextModeParameterDcs!=8)
		Gsm_MsgSetTextModeParameter(17,167,0,8);	 		//make sure we're in right scheme
	//Make Number USC2 format
	answer = Utf8toUcs2(&ucs2num,Number);
	if(answer!=true)
	{
		Gsm_MsgSetCharacterFormat(GsmTECharacterSet_GSM); //restore Character set
		return false;
	}
	//
	snprintf(str,sizeof(str),"AT+CMGS=\"%s\"\r\n",ucs2num);
  answer = Sim80x_SendAtCommand(str,10000,1,"\r\r\n> ");
  osDelay(100);
	if(answer != 1)
  {
    snprintf(str,sizeof(str),"%c",ESC_char_Dec);
    Sim80x_SendString(str);
		free(ucs2num);
		Gsm_MsgSetCharacterFormat(GsmTECharacterSet_GSM); //restore Character set
    return false;
  }
	free(ucs2num);
	//Make msg USC2 format
	answer = Utf8toUcs2(&ucs2msg,msg);
	if(answer != true)
  {
    snprintf(str,sizeof(str),"%c",ESC_char_Dec);
    Sim80x_SendString(str);
		Gsm_MsgSetCharacterFormat(GsmTECharacterSet_GSM); //restore Character set
    return false;
  }
	//
	strcpy(Sim80x.Gsm.MsgSentNumber,Number);
	snprintf(str,sizeof(str),"%s%c",ucs2msg,SUB_char_Dec); //msg must be USC2
	free(ucs2msg);
  Sim80x_SendString(str);
	while(Timeout>0)
  {
    osDelay(1000);
    Timeout--;
    if(Sim80x.Gsm.MsgSent == 1)
		{
			Gsm_MsgSetCharacterFormat(GsmTECharacterSet_GSM); //restore Character set
      return true;
		}
  }
	Gsm_MsgSetCharacterFormat(GsmTECharacterSet_GSM); //restore Character set
  return false;
}
//######################################################################################################
bool Utf8toUcs2(char **des,const char *str)
{
	uint16_t i = 0,j = 0,c = 0,v = 0;
	uint16_t len = strlen(str);
	//--- Count the length needed for calloc
	while(i < len)
	{
		v = (uint8_t)str[i];
		if(	v <= 0x7F	){
			j += 4;
			i += 1;}
		else if( v <= 0xDF ){
			j += 4;
			i += 2;}
		else if( v <= 0xEF )
			i += 3;
		else if( v <= 0xF4 )
			i += 4;
	}
	//---
	*des = calloc(j+1, sizeof(char));
	if (*des==NULL) 
		return false;
	//
  i = 0;
	j = 0;
	c = 0;
	v = 0;
	//
	while(i < len)
	{
		v = (uint8_t)str[i];
		if(	v <= 0x7F	){
			c = (uint16_t)v;
			sprintf(*des + j, "%04X", c); 
			j += 4;
			i += 1;}
		else if( v <= 0xDF ){
			c  = ((uint16_t)(v  & 0x1F)) << 6;
			v  =  (uint8_t)str[i+1];
			c |=  (uint16_t)(v & 0x3F);
			sprintf(*des + j, "%04X", c); 
			j += 4;
			i += 2;}
		else if( v <= 0xEF )
			i += 3;
		else if( v <= 0xF4 )
			i += 4;
	}
	return true;
}
//######################################################################################################
