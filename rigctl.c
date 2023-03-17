#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "ermak.h"
#include "dump_state.h"
#include "rigctl.h"


//#define LOG_REQUEST(x)      fprintf(stdout, "RQ:%s\n", x)
//#define LOG_RESPONSE(x)     fprintf(stdout, "RSP:\n%s", x)
#define LOG_REQUEST(x)
#define LOG_RESPONSE(x)
#define LOG_ERRCMD(x)       fprintf(stderr, "cmd '%s' not found\n", x)
//#define LOG_REQUEST(x)      slogf( _SLOG_SETCODE(_SLOGC_TEST, 2), _SLOG_INFO, x)
//#define LOG_RESPONSE(x)     slogf( _SLOG_SETCODE(_SLOGC_TEST, 4), _SLOG_INFO, x)
//#define LOG_ERRCMD(x)       slogf( _SLOG_SETCODE(_SLOGC_TEST, 6), _SLOG_INFO, x)

#define BUF_TX_LEN      (1426)       // TCP Header 54 Bytes, Paket Size = 1480, Ok for All network

#define SEND_REQUEST_TO_ERMAK(x)    ermak_SendRequest(&x)

#define RIGTCL(c, x)        {.command = c, .cb = x}

typedef struct RIGCTL_PARM_t{
    char * parm[3];
}RIGCTL_PARM_t;

typedef void(*cbRigCtl)(RIGCTL_PARM_t*);

typedef struct RIGCTL_COMMAND_t{
	char* 	command;
	cbRigCtl cb;
}RIGCTL_COMMAND_t;


static char _str[BUF_TX_LEN] = {0};
static bool longReply = false;
static char ansSep = '\n';
static int  sendRprt = -1;
static char chkvfo = 0;     // if chkvfo == 1 need a VFOA/VFOB


static void cbRigCTLChkVFO(RIGCTL_PARM_t* pParm);
static void cbRigCTLDumpState(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetVFOinfo(RIGCTL_PARM_t *pParm);
static void cbRigCTLGetPowerState(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetLockMode(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetLockMode(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetFreq(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetFreq(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetVFO(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetVFO(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetMode(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetMode(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetPTT(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetPTT(RIGCTL_PARM_t* pParm);

static void cbRigCTLGetSplitVFO(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetSplitVFO(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetSplitMode(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetSplitMode(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetSplitFreq(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetSplitFreq(RIGCTL_PARM_t* pParm);


static RIGCTL_COMMAND_t _rigctlCommands[] = {
        RIGTCL("\\chk_vfo",         cbRigCTLChkVFO),
        //RIGTCL("\set_vfo_opt",     cbRigCTLSetVFOopt),
        RIGTCL("\\dump_state",      cbRigCTLDumpState),
        RIGTCL("\\get_vfo_info",    cbRigCTLGetVFOinfo),
        RIGTCL("\\get_powerstat",   cbRigCTLGetPowerState),
        RIGTCL("\\get_lock_mode",   cbRigCTLGetLockMode),
        RIGTCL("\\set_lock_mode",   cbRigCTLSetLockMode),
        RIGTCL("\\get_freq",        cbRigCTLGetFreq),
        RIGTCL("\\set_freq",        cbRigCTLSetFreq),
        RIGTCL("\\get_vfo",         cbRigCTLGetVFO),
        RIGTCL("\\set_vfo",         cbRigCTLSetVFO),
        RIGTCL("\\get_mode",        cbRigCTLGetMode),
        RIGTCL("\\set_mode",        cbRigCTLSetMode),
        RIGTCL("\\get_ptt",         cbRigCTLGetPTT),
        RIGTCL("\\set_ptt",         cbRigCTLSetPTT),
        RIGTCL("\\get_split_vfo",   cbRigCTLGetSplitVFO),
        RIGTCL("\\set_split_vfo",   cbRigCTLSetSplitVFO),
        RIGTCL("\\get_split_mode",  cbRigCTLGetSplitMode),
        RIGTCL("\\set_split_mode",  cbRigCTLSetSplitMode),
        RIGTCL("\\get_split_freq",  cbRigCTLGetSplitFreq),
        RIGTCL("\\set_split_freq",  cbRigCTLSetSplitFreq),
        RIGTCL("\xf0",              cbRigCTLChkVFO),
        RIGTCL("\xf3",              cbRigCTLGetVFOinfo),
        RIGTCL("\x88",              cbRigCTLGetPowerState),
        RIGTCL("\xa3",              cbRigCTLGetLockMode),
        RIGTCL("\xa2",              cbRigCTLSetLockMode),
        RIGTCL("f",                 cbRigCTLGetFreq),
        RIGTCL("F",                 cbRigCTLSetFreq),
        RIGTCL("v\nv",              cbRigCTLGetVFO),
        RIGTCL("v",                 cbRigCTLGetVFO),
        RIGTCL("V",                 cbRigCTLSetVFO),
        RIGTCL("m",                 cbRigCTLGetMode),
        RIGTCL("M",                 cbRigCTLSetMode),
        RIGTCL("t",                 cbRigCTLGetPTT),
        RIGTCL("T",                 cbRigCTLSetPTT),
        RIGTCL("s",                 cbRigCTLGetSplitVFO),
        RIGTCL("S",                 cbRigCTLSetSplitVFO),
        RIGTCL("x",                 cbRigCTLGetSplitMode),
        RIGTCL("X",                 cbRigCTLSetSplitMode),
        RIGTCL("i",                 cbRigCTLGetSplitFreq),
        RIGTCL("I",                 cbRigCTLSetSplitFreq),

		RIGTCL(NULL, 	NULL)
};


static char * rigctl_cvtVfo2Str(ERMAK_VFO_MODE_t vfo) {
    switch(vfo) {
    case ERMAK_VFO_MODE_A:
        return "VFOA";
    case ERMAK_VFO_MODE_B:
        return "VFOB";

    // ToDo VFO: Sub, Main, currVFO, MEM, TX, RX

    default:
        return "currVFO";
    }
}

static char * rigctl_cvtMode2Str(ERMAK_MODE_t mode) {
	switch (mode) {
	case ERMAK_MODE_LSB:
		return "LSB";
	case ERMAK_MODE_USB:
		return "USB";
	case ERMAK_MODE_CWU:
		return "CW";
	case ERMAK_MODE_FM:
		return "FM";
	case ERMAK_MODE_AM:
		return "AM";
	case ERMAK_MODE_PKTLSB:
		return "PKTLSB";
	case ERMAK_MODE_CWL:
		return "CWR";
	case ERMAK_MODE_PKTUSB:
		return "PKTUSB";

	default:
		break;
	}
    return "UNK";
}

static ERMAK_MODE_t rigctl_cvtStr2Mode(char * pStr) {
    if(strcmp(pStr, "PKTUSB") == 0) {
        return ERMAK_MODE_PKTUSB;
    }
    else if(strcmp(pStr, "PKTLSB") == 0) {
        return ERMAK_MODE_PKTLSB;
    }
    else if(strcmp(pStr, "USB") == 0) {
        return ERMAK_MODE_USB;
    }
    else if(strcmp(pStr, "LSB") == 0) {
        return ERMAK_MODE_LSB;
    }
    else if(strcmp(pStr, "CWR") == 0) {
        return ERMAK_MODE_CWL;
    }
    else if(strcmp(pStr, "CW") == 0) {
        return ERMAK_MODE_CWU;
    }
    else if(strcmp(pStr, "AM") == 0) {
        return ERMAK_MODE_AM;
    }
    else if(strcmp(pStr, "FM") == 0) {
        return ERMAK_MODE_FM;
    }

    return ERMAK_MODE_UNK;
}

/**
 *
 * @param type
 */
static void NotifyWEBRemote(ERMAK_NOTIFY_TYPE_t type){
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_NOTIFY_WEB_REMOTE;
	msg.notifyType = type;
	SEND_REQUEST_TO_ERMAK(msg);
}


/////////////////////////////////////////////////


/**
 *
 * @param pParm
 */
static void cbRigCTLChkVFO(RIGCTL_PARM_t* pParm){
    if(longReply) {
        snprintf(_str, sizeof(_str), "ChkVFO: %d\n", chkvfo);
    }
    else
        snprintf(_str, sizeof(_str), "%d\n", chkvfo);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLDumpState(RIGCTL_PARM_t* pParm){
    snprintf(_str, sizeof(_str), DUMP_STATE_ERMAK);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetVFOinfo(RIGCTL_PARM_t *pParm)
{
    ERMAK_VFO_MODE_t vfo = -1;
    bool ext_info = false;

    if(pParm->parm[0] != NULL)
    {
        if(strcmp(pParm->parm[0], "VFOA") == 0)
            vfo = ERMAK_VFO_MODE_A;
        else if(strcmp(pParm->parm[0], "VFOB") == 0)
            vfo = ERMAK_VFO_MODE_B;
        else if((strcmp(pParm->parm[0], "?") == 0))
            ext_info = true;
    }

    if (ext_info) {
        if(longReply) {
            //"get_vfo_info: currVFO ?\n"
            snprintf(_str, sizeof(_str), "VFOA VFOB\n");
        }
        else {
            snprintf(_str, sizeof(_str), "VFOA VFOB\n");
        }
    } else {
        ERMAK_MSG_t msg;
        msg.command = ERMAK_COMMAND_GET_EXTD_INFO;
        msg.extdInfo.vfo = vfo;
        SEND_REQUEST_TO_ERMAK(msg);

        if(longReply) {
            snprintf(_str,sizeof(_str), "get_vfo_info: currVFO %s\nFreq: %u\nMode: %s\nWidth: %d\nSplit: %d\nSatMode: 0\n",
                rigctl_cvtVfo2Str(msg.extdInfo.vfo), msg.extdInfo.freq, rigctl_cvtMode2Str(msg.extdInfo.mode), msg.extdInfo.passband, msg.extdInfo.split);
        }
        else {
            snprintf(_str,sizeof(_str), "%u\n%s\n%d\n%d\n0\n",
            msg.extdInfo.freq,
            rigctl_cvtMode2Str(msg.extdInfo.mode), 
            msg.extdInfo.passband, 
            msg.extdInfo.split);
        }
    }
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetPowerState(RIGCTL_PARM_t* pParm){
    // Always reply with ON
    if(longReply) {
        // "get_powerstat: currVFO"
        snprintf(_str,sizeof(_str),"Power Status: 1\n");
    } else
        snprintf(_str,sizeof(_str),"1\n");
}

/**
 *
 * @param pParm
 */
 static void cbRigCTLGetLockMode(RIGCTL_PARM_t* pParm){
    ERMAK_MSG_t msg;
    msg.command = ERMAK_COMMAND_GET_LOCK;
    SEND_REQUEST_TO_ERMAK(msg);

    if(longReply) {
        // "get_lock_mode: currVFO"
        snprintf(_str,sizeof(_str), "Locked: %d\n", msg.lock);
    }
    else
        snprintf(_str,sizeof(_str), "%d\n", msg.lock);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetLockMode(RIGCTL_PARM_t* pParm){
    ERMAK_MSG_t msg;
    msg.command = ERMAK_COMMAND_SET_LOCK;
    if((pParm->parm[0] != NULL) && (*pParm->parm[0] == '1')) {
        msg.lock = ERMAK_LOCK_ON;
    }
    else {
        msg.lock = ERMAK_LOCK_OFF;
    }
    SEND_REQUEST_TO_ERMAK(msg);
    sendRprt = RIG_OK;
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetFreq(RIGCTL_PARM_t* pParm){
    // ToDo VFOA/VFOB
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_GET_FREQ_MODE;
	SEND_REQUEST_TO_ERMAK(msg);

    if(longReply) {
        // "get_freq: %s", rigctl_cvtVfo2Str(msg.freqModeData.vfo)
        snprintf(_str,sizeof(_str), "Frequency: %u\n", msg.freqModeData.freq);
    } else
        snprintf(_str,sizeof(_str), "%u\n",msg.freqModeData.freq);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetFreq(RIGCTL_PARM_t* pParm){
    ERMAK_MSG_t msg;
    msg.command = ERMAK_COMMAND_SET_DDS;
    msg.vfoData.rx 	= ERMAK_RX_MAIN;
    msg.vfoData.vfo = -1;

    uint32_t freq;
    if(pParm->parm[1] != NULL)          // Includes VFO
    {
        freq = (uint32_t)strtof(pParm->parm[1], NULL);

        if(strcmp(pParm->parm[0], "VFOB") == 0)
            msg.vfoData.vfo = ERMAK_VFO_MODE_B;
        else if(strcmp(pParm->parm[0], "VFOA") == 0)
            msg.vfoData.vfo = ERMAK_VFO_MODE_A;
    }
    else if(pParm->parm[0] != NULL)     // Only Frequency
    {
        freq = (uint32_t)strtof(pParm->parm[0], NULL);
    }
    else {  // Parm error
        sendRprt = RIG_EINVAL;
        return;
    }

    msg.vfoData.freq = freq;
    SEND_REQUEST_TO_ERMAK(msg);

    sendRprt = RIG_OK;
    NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetVFO(RIGCTL_PARM_t* pParm){
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_GET_VFO;
	msg.vfoData.rx  = ERMAK_RX_MAIN;
	msg.vfoData.vfo = ERMAK_VFO_MODE_A;
	SEND_REQUEST_TO_ERMAK(msg);

    if(longReply) {
        // "get_vfo: currVFO"
        snprintf(_str,sizeof(_str), "VFO: %s\n", rigctl_cvtVfo2Str(msg.vfoData.vfo));
    } else
        snprintf(_str,sizeof(_str), "%s\n", rigctl_cvtVfo2Str(msg.vfoData.vfo));
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetVFO(RIGCTL_PARM_t* pParm){
    if(pParm->parm[0] == NULL) {
        sendRprt = RIG_EINVAL;
        return;
    }

    if(strcmp(pParm->parm[0], "?") == 0) {
        snprintf(_str,sizeof(_str), "set_vfo: currVFO ?\nVFOA VFOB\n");
        sendRprt = RIG_OK;
        return;
    }
    else
    {
        ERMAK_MSG_t msg;
        msg.command = ERMAK_COMMAND_SWITCH_VFO;
        if(strcmp(pParm->parm[0], "VFOB") == 0) {
            msg.vfoData.vfo = ERMAK_VFO_MODE_B;
        }
        else {
            msg.vfoData.vfo = ERMAK_VFO_MODE_A;
        }

        SEND_REQUEST_TO_ERMAK(msg);
        sendRprt = RIG_OK;
        NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
    }
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetMode(RIGCTL_PARM_t *pParm){
    // ToDo VFOA/VFOB
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_GET_FREQ_MODE;
	SEND_REQUEST_TO_ERMAK(msg);

    char* mode;
    mode = rigctl_cvtMode2Str(msg.freqModeData.mode);

   if (longReply) {
        // "get_mode: VFOA"
        snprintf(_str,sizeof(_str),"Mode: %s\nPassband: %d\n", mode, msg.freqModeData.passband);
    }
    else {
        snprintf(_str,sizeof(_str),"%s\n%d\n", mode, msg.freqModeData.passband);
    }
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetMode(RIGCTL_PARM_t* pParm){
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_SET_MODE;
    msg.freqModeData.vfo = ERMAK_VFO_MODE_A;

    ERMAK_MODE_t mode = ERMAK_MODE_UNK;

    if(pParm->parm[2] != NULL) {    // Set vfo, mode, passband
        if(strcmp(pParm->parm[0], "VFOB") == 0)
            msg.freqModeData.vfo = ERMAK_VFO_MODE_B;

        mode = rigctl_cvtStr2Mode(pParm->parm[1]);
    }
    else if(pParm->parm[0] != NULL) {   // mode, [passband]
        mode = rigctl_cvtStr2Mode(pParm->parm[0]);
    }

    if(mode == ERMAK_MODE_UNK) {
        sendRprt =  RIG_EINVAL;
        return;
    }
    msg.freqModeData.mode = mode;

    SEND_REQUEST_TO_ERMAK(msg);
    sendRprt = RIG_OK;
    NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
}

static void cbRigCTLGetPTT(RIGCTL_PARM_t* pParm){
    // ToDo VFOA/VFOB
    ERMAK_MSG_t msg;
    msg.command = ERMAK_COMMAND_GET_TX_RX;
    SEND_REQUEST_TO_ERMAK(msg);
    if (longReply) {
        // "get_ptt: None"
        snprintf(_str,sizeof(_str), "PTT: %d\n",msg.transmittRx.transmitt);
    }else
        snprintf(_str,sizeof(_str), "%d\n",msg.transmittRx.transmitt);
}

static void cbRigCTLSetPTT(RIGCTL_PARM_t* pParm){
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_SET_TRANSMITT_RX;
	msg.transmittRx.rx = ERMAK_RX_MAIN;

    if(pParm->parm[0] != NULL)
    {
        if(*pParm->parm[0] == '1')
            msg.transmittRx.transmitt = ERMAK_TRANSMIT_ON;
        else
            msg.transmittRx.transmitt = ERMAK_TRANSMIT_OFF;
    }
    else
    {
        return;
    }

    SEND_REQUEST_TO_ERMAK(msg);
    sendRprt = RIG_OK;
    NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_RXTX);
}

// Split Section

static void cbRigCTLGetSplitVFO(RIGCTL_PARM_t* pParm){
    // ToDo VFOA/VFOB
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_GET_SPLIT;
    SEND_REQUEST_TO_ERMAK(msg);

    if (longReply) {
        // "get_split_vfo: VFOA"
        snprintf(_str,sizeof(_str),"Split: %d\nTX VFO: %s\n", msg.extdInfo.split, rigctl_cvtVfo2Str(msg.extdInfo.vfo));
    } else {
        snprintf(_str,sizeof(_str),"%d\n%s\n", msg.extdInfo.split, rigctl_cvtVfo2Str(msg.extdInfo.vfo));
    }
}

static void cbRigCTLSetSplitVFO(RIGCTL_PARM_t* pParm) {
    ERMAK_MSG_t msg;
    msg.command = ERMAK_COMMAND_SET_SPLIT;

    msg.extdInfo.split = ERMAK_SPLIT_OFF;
    if((pParm->parm[0] != NULL) && (*pParm->parm[0] == '1'))
        msg.extdInfo.split = ERMAK_SPLIT_ON;

    msg.extdInfo.vfo == ERMAK_VFO_MODE_B;
    if((pParm->parm[1] != NULL) && strcmp(pParm->parm[1], "VFOA") == 0)
        msg.extdInfo.vfo == ERMAK_VFO_MODE_A;

    SEND_REQUEST_TO_ERMAK(msg);
    sendRprt = RIG_OK;
    NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_RXTX);
}


static void cbRigCTLGetSplitMode(RIGCTL_PARM_t* pParm) {
   // ToDo VFOA/VFOB
	ERMAK_MSG_t msg;
	msg.command = ERMAK_COMMAND_GET_FREQ_MODE;      // ToDO _GET_SPLIT_FREQ_MODE
    SEND_REQUEST_TO_ERMAK(msg);

    char* mode;
    mode = rigctl_cvtMode2Str(msg.freqModeData.mode);

    if (longReply) {
        snprintf(_str,sizeof(_str),"TX Mode: %s\nTX Passband: %d\n", mode, msg.freqModeData.passband);
    } else {
        snprintf(_str,sizeof(_str),"%s\n%d\n", mode, msg.freqModeData.passband);
    }
}

static void cbRigCTLSetSplitMode(RIGCTL_PARM_t* pParm) {
    // ToDo check Parm 0 - mode, 1 passband
    sendRprt = RIG_OK;
}


static void cbRigCTLGetSplitFreq(RIGCTL_PARM_t* pParm) {
    ERMAK_MSG_t msg;
    msg.command = ERMAK_COMMAND_GET_SPLIT;
    SEND_REQUEST_TO_ERMAK(msg);

    if (longReply) {
        snprintf(_str,sizeof(_str), "TX VFO: %d\n", msg.extdInfo.freq);
    } else {
        snprintf(_str,sizeof(_str), "%d\n", msg.extdInfo.freq);
    }
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetSplitFreq(RIGCTL_PARM_t* pParm) {
    ERMAK_MSG_t msg;
    msg.command = ERMAK_COMMAND_SET_SPLIT_FREQ;

    uint32_t freq;
    if(pParm->parm[0] != NULL)
    {
        freq = (uint32_t)strtof(pParm->parm[0], NULL);
    }
    else {  // Parm error
        sendRprt = RIG_EINVAL;
        return;
    }

    msg.extdInfo.freq = freq;
    SEND_REQUEST_TO_ERMAK(msg);

    sendRprt = RIG_OK;
    NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
}


static int rigctl_parse_in(char *pBuf)
{
    char ch = *pBuf;

    ansSep = '\n';
    longReply = false;

    if(ch == '+') {                                 // Long Reply
        longReply = true;
        ansSep = '\n';
        pBuf++;
    }
    else if(ch == ';' || ch == '|' || ch == ',') {  // Change delimiter
        longReply = true;
        ansSep = ch;
        pBuf++;
    }
    else if(ch == '#') {                            // Skip comment
        while(ch != '\n') {
            ch = *(++pBuf);
            if(ch == 0) return 0;
        }
    }
    else if(ch == 'q' || ch == 'Q')                 // close connection
    {
        return -1;
    }

    // split string to parm
    const char delimparm[] = " ;|,";

    pBuf = strtok(pBuf, delimparm);     // Get Command
    char * cmdin = pBuf;

    int Num = 0;
    RIGCTL_PARM_t parm = {NULL, NULL, NULL};
    pBuf = strtok(NULL, delimparm);     // Get 1'st Parameter
    while(pBuf != NULL && Num < 3)
    {
        parm.parm[Num] = pBuf;

        Num++;
        pBuf = strtok(NULL, delimparm);
    }

    int i = 0;
    RIGCTL_COMMAND_t * cmdlist;
    cmdlist = &_rigctlCommands[i++];

    while(cmdlist->command != NULL){
        if(strcmp(cmdlist->command, cmdin) == 0) {
            
            if(cmdlist->cb != NULL){
                cmdlist->cb(&parm);
            }
            return 0;
        }
        cmdlist  = &_rigctlCommands[i++];
    }

    LOG_ERRCMD(cmdin);
    sendRprt =  RIG_ENAVAIL;

    return 0;
}

int rigctl_req(char *pReq, char *pResp)
{
    size_t len = strlen(pReq);
    if(len < 2) return 0;
    if(pReq[len - 1] != '\n')  return 0;
    if(pReq[len - 2] == '\r')
        pReq[len - 2] = 0;
    else
        pReq[len - 1] = 0;

    //const char delimreq[] = "\r\n";
    //pReq = strtok(pReq, delimreq);

    sendRprt = RIG_EEND;
    _str[0] = 0;

    LOG_REQUEST(pReq);

    if(rigctl_parse_in(pReq) < 0)
        return -1;

    // Replace delimiter
    len = strlen(_str);
    if(len != 0) {
        if (ansSep != '\n') {
            int i;
            for(i = 0; i < len; i++)
                if(_str[i] == '\n') _str[i] = ansSep;
        }

        if((longReply) && (sendRprt == RIG_EEND))
            sendRprt = RIG_OK;
    }

    if(sendRprt != RIG_EEND)
    {
        char rprt[10];
        sendRprt = -sendRprt;
        if((len == 0) || (ansSep == '\n'))
            snprintf(rprt,sizeof(rprt), "RPRT %d\n", sendRprt);
        else
            snprintf(rprt,sizeof(rprt), "\nRPRT %d\n", sendRprt);
        strcat(_str, rprt);
    }

    len = strlen(_str);

    if(0 != len){
        // Send Answer
        strcpy(pResp, _str);

        LOG_RESPONSE(_str);
    }

    return len;
}
