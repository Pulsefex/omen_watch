#include "sim800l.h"
#include "system_init.h"
#include <stdbool.h>

extern uint16_t RxCounter;

char sim_cmd_sw_flw_ctrl[LEN_CMD_SW_FLW_CTRL]="AT+IFC=1,1";
char sim_cmd_rej_in_call[LEN_CMD_REJ_IN_CALL]="AT+GSMBUSY=x";
char sim_cmd_signal_strenth[LEN_CMD_SIG_STR]="AT+CSQ";
char sim_cmd_set_text_mode[LEN_CMD_TEXT_MODE]="AT+CMGF=x"; //set text mode to module SIM
char sim_cmd_read_sms[LEN_CMD_READ_SMS]="AT+CMGR=xx,x";   //read sms command, x = sms index
char sim_cmd_dele_sms[LEN_CMD_DELE_SMS]="AT+CMGD=xx";   //delete sms command, x = sms index
char sim_cmd_send_sms[LEN_CMD_SEND_SMS + LEN_PHONE_NUM]="AT+CMGS=\"+xxxxxxxxxxx\"";
char sim_cmd_set_cnmi_mode[LEN_CMD_CNMI_MODE]="AT+CNMI=0,0,0,0,0"; //do nothing with new sms, let progran handle it
char sim_cmd_res_ok[LEN_CMD_RES_OK]="\r\nOK\r\n";//respond from module SIM, expected to be "OK"
char sim_cmd_res_err[LEN_CMD_RES_ERR]="\r\nERROR\r\n";//respond from module SIM, expected to be "error"


/*-----------------------------------------------------------------
 * return sms state (read/unread)
 */
uint8_t sim_get_sms_state(char* buf)
{
    char* sms_unread ="REC UNREAD";
    char* sms_read ="REC READ";
    uint8_t i;
    //if this is "REC UNREAD"
    for(i=0; i<MIN_BUFFER; i++)
        if( *(buf+i) == '"' ){
            if( !strncmp(buf+i+1,sms_unread,10 ) )
                return SMS_UNREAD;
            else if( !strncmp(buf+i+1,sms_read,8 ) )
                return SMS_READ;
        }
    if( i == MIN_BUFFER )
        return NO_SMS;
}


/*-----------------------------------------------------------------
 * find character in buffer, return 255/0 if not found
 */
int find_c(char* buffer, uint8_t end1, uint8_t end2, char c)
{
    uint8_t i;
    //find forward
    if( end1 <= end2 ){
        end2 = end2-end1;
        for( i=0; i <= end2; i++ )
            if( buffer[end1+i] == c ){
                i=end1+i;
                break;
            }
    }
    //find backward
    else{
        end2 = end1-end2;
        for( i=0; i <= end2; i++ )
            if( buffer[end1-i] == c ){
                i=end1-i;
                break;
            }
    }
    return i;
}



/*-----------------------------------------------------------------
 * read sms
 * return input sms_idx if it is out of range
 * return respond of module SIM
 */
uint8_t sim_read_sms(uint8_t sms_idx, uint8_t mode, char* buf)
{
    if( sms_idx > MAX_SMS ) return IDX_OOR;
    if( mode > 1 ) return IDX_OOR;

    memset(buf,0,MIN_BUFFER);        //cleanup buffer first
    RxCounter=0;                     //reset received buffer counter
    sim_cmd_read_sms[LEN_CMD_READ_SMS-4]=(sms_idx / 10)+0x30;    //set sms index
    sim_cmd_read_sms[LEN_CMD_READ_SMS-3]=(sms_idx % 10)+0x30;    //set sms index
    sim_cmd_read_sms[LEN_CMD_READ_SMS-1]=mode+0x30;    //set sms index
    push_cmd(sim_cmd_read_sms,LEN_CMD_READ_SMS);    //send cmd to module sim
    putchar('\n');
    while( (sim_check_res(buf) != SIM_RES_OK) && (sim_check_res(buf) != SIM_RES_ERROR) );
    return sim_check_res(buf);
}


/*-----------------------------------------------------------------
 * delete sms
 * return respond of module SIM
 */
uint8_t sim_dele_sms(uint8_t sms_idx, char* buf)
{
    if( sms_idx >= MAX_SMS ) return IDX_OOR;
    memset(buf,0,MIN_BUFFER);        //cleanup buffer first
    RxCounter=0;                     //reset received buffer counter
    sim_cmd_dele_sms[LEN_CMD_DELE_SMS-2]=(sms_idx / 10)+0x30;    //set sms index
    sim_cmd_dele_sms[LEN_CMD_DELE_SMS-1]=(sms_idx % 10)+0x30;    //set sms index
    push_cmd(sim_cmd_dele_sms,LEN_CMD_DELE_SMS);    //send cmd to module sim
    putchar('\n');
    while( (sim_check_res(buf) != SIM_RES_OK) && (sim_check_res(buf) != SIM_RES_ERROR) );
    return sim_check_res(buf);
}


/*-----------------------------------------------------------------
 * send sms
 * return respond of module SIM
 */
uint8_t sim_send_sms(char* phone_number, char* text, char* buf)
{
    strncpy(&sim_cmd_send_sms[LEN_CMD_SEND_SMS-1],phone_number,LEN_PHONE_NUM);
    memset(buf,0,MIN_BUFFER);                //cleanup buffer first
    RxCounter=0;                     //reset received buffer counter
    push_cmd(sim_cmd_send_sms,LEN_CMD_SEND_SMS+LEN_PHONE_NUM);       //send cmd to module sim
    putchar('\n');
    HAL_Delay(10);
    push_cmd(text,LEN_PUBLISH_MES);
    putchar(26);
    while( (sim_check_res(buf) != SIM_RES_OK) && (sim_check_res(buf) != SIM_RES_ERROR) );
    return sim_check_res(buf);
}


/*-----------------------------------------------------------------
 * set text/pdu mode for sms
 * return respond of module SIM
 */
uint8_t sim_set_text_mode(uint8_t mode, char* buf)
{
    if( mode > 1 ) return IDX_OOR;
    memset(buf,'0',MIN_BUFFER);        //cleanup buffer first
    RxCounter=0;                     //reset received buffer counter
    sim_cmd_set_text_mode[LEN_CMD_TEXT_MODE-1]=mode+0x30;    //set mode index
    push_cmd(sim_cmd_set_text_mode,LEN_CMD_TEXT_MODE);    //send cmd to module sim;
    // Delay(100);
    // putchar('\r');
    putchar('\n');
    while( (sim_check_res(buf) != SIM_RES_OK) && (sim_check_res(buf) != SIM_RES_ERROR) );
    return sim_check_res(buf);
}


/*-----------------------------------------------------------------
 * set behaviour for new message
 * return respond of module SIM
 */
uint8_t sim_set_cnmi_mode(uint8_t mode, uint8_t mt, uint8_t bm, uint8_t ds, uint8_t bfr, char* buf)
{
    if( (mode > 3) || (mt > 3) || (bm > 2) || (ds > 1) || (bfr > 1) ) return IDX_OOR;
    memset(buf,0,MIN_BUFFER);        //cleanup buffer first
    RxCounter=0;                     //reset received buffer counter
    sim_cmd_set_cnmi_mode[LEN_CMD_CNMI_MODE-9]=mode+0x30;    //set mode index
    push_cmd(sim_cmd_set_cnmi_mode,LEN_CMD_CNMI_MODE);    //send cmd to module sim
    putchar('\n');
    while( (sim_check_res(buf) != SIM_RES_OK) && (sim_check_res(buf) != SIM_RES_ERROR) );
    return sim_check_res(buf);
}


/*-----------------------------------------------------------------
 * push command to module sim
 */
void push_cmd(char* cmd, uint8_t cmd_len)
{
    uint8_t i;
    for( i=0; i < cmd_len; i++)
    {
        putchar(*(cmd+i));
    }
}

/*-----------------------------------------------------------------
 * check signal strength
 */
uint8_t sim_signal_strength(char* buf)
{
    uint8_t i;
    uint8_t signal_strength;
    memset(buf,0,MIN_BUFFER);        //cleanup buffer first
    RxCounter=0;                     //reset received buffer counter
    push_cmd(sim_cmd_signal_strenth,LEN_CMD_SIG_STR);    //send cmd to module sim
    putchar('\n');
    while( (sim_check_res(buf) != SIM_RES_OK) && (sim_check_res(buf) != SIM_RES_ERROR) );
    if(sim_check_res(buf))
    {
        for(i=0;i<MIN_BUFFER;i++)
        {
            if(buf[i]==':')
            {
                i+=2;
                if(buf[i+1]==',')
                    signal_strength = buf[i]-0x30;
                else
                    signal_strength = 10*(buf[i]-0x30) + (buf[i+1]-0x30);

                return signal_strength;
            }
        }
    }
    else
    {
        return SIM_RES_ERROR;
    }

}

/*-----------------------------------------------------------------
 * check respond status of module sim
 */
uint8_t sim_check_res(char* buf)
{
    uint8_t i;
    for(i=MIN_BUFFER-1;i>0;i--)
    {
        if( *(buf + i) == '\r' )
        {
            if( !strncmp(buf+i,sim_cmd_res_ok,LEN_CMD_RES_OK) )
            {
                return SIM_RES_OK;
            }
            else if( !strncmp(buf+i,sim_cmd_res_err,LEN_CMD_RES_ERR) ){
                return SIM_RES_ERROR;
            }
        }
    }
    return SIM_NO_RES;
}


/*-----------------------------------------------------------------*/
/* extract phone number from a sms message
 * replace ZZ with country code
 */
uint8_t sim_get_sms_contact(char* num,char* buf)
{
    uint8_t i;

    // struct PHONEBOOK temp={"AT+CMGS=\"+ZZxxxxxxxxx\"",FALSE,FALSE};
    if( sim_check_res(buf) ){
        for(i=LEN_CMD_READ_SMS;i<MIN_BUFFER;i++)
            if(buf[i]==',')
                break;
        if( i == 0 ) return NO_SMS;
        strncpy(num,buf+i+2,LEN_PHONE_NUM);
        return SIM_RES_OK;
    }
    else
    {
        return SIM_RES_ERROR;
    }
}

/*-----------------------------------------------------------------*/
/* extract data partion from a sms message
 * src: respond message from module sim
 * det: string to contain sms data
 * return len of data. return 0 if message is empty or exceed
 * message max size
 */
uint8_t sim_get_sms_data(char* data, char* buf)
{
    uint8_t i,j;
    if( sim_check_res(buf) ){
        for(i=MIN_BUFFER-1; i>0; i--)
            if( *(buf+i) == '"' ){
                i+=3;
                break;
            }
        if( i == 0 ) return NO_SMS;
        for(j=MIN_BUFFER-1; j>0; j--)
            if( *(buf+j) == '\r' )
                break;
        strncpy(data,buf+i,(j-i));
        return SIM_RES_OK;
    }
    else
    {
        return SIM_RES_ERROR;
    }
}

/*-----------------------------------------------------------------*/
/* reject incomming call
 * mode_idx:
 *  0:  Enable incoming call
 *  1:  Forbid all incoming calls
 *  2:  Forbid incoming voice calls but enable CSD calls
 * return sim respond flag
 */
uint8_t sim_rej_in_call(uint8_t mode_idx, char* buf)
{
    if( mode_idx > 2 ) return IDX_OOR;

    memset(buf,0,MIN_BUFFER);        //cleanup buffer first
    RxCounter=0;                     //reset received buffer counter
    sim_cmd_rej_in_call[LEN_CMD_REJ_IN_CALL-1]=mode_idx+0x30;    //set mode
    push_cmd(sim_cmd_rej_in_call,LEN_CMD_REJ_IN_CALL);    //send cmd to module sim
    putchar('\n');
    while( (sim_check_res(buf) != SIM_RES_OK) && (sim_check_res(buf) != SIM_RES_ERROR) );
    return sim_check_res(buf);
}
