#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "ermak.h"
#include "dump_state.h"
#include "rigctl.h"
#include "log.h"


#define LOG_REQUEST(x)      log_trace("RQ: %s", x)
#define LOG_RESPONSE(x)     log_trace("RA: %s", x)
#define LOG_ERRCMD(x)       log_error("cmd '%s' not found\n", x)

#define BUF_TX_LEN      (1426)       // TCP Header 54 Bytes, Paket Size = 1480, Ok for All network

#define SEND_REQUEST_TO_ERMAK(x)    ermak_SendRequest(&x)

#define RIGTCL(c, x)        {.command = c, .cb = x}
#define RIGFUNC(c, g, s)	{.cmd = c, .cbGet = g, .cbSet = s}

/************************************************************
 * Typedef                            						*
 ************************************************************/
typedef struct RIGCTL_PARM_t {
	char * parm[3];
} RIGCTL_PARM_t;

typedef void(*cbRigCtl)(RIGCTL_PARM_t*);

typedef struct RIGCTL_COMMAND_t {
	char* command;
	cbRigCtl cb;
} RIGCTL_COMMAND_t;

/* Функции */

typedef void(*cbRigFuncGet_t)(void);
typedef void(*cbRigFuncSet_t)(uint8_t parm);

typedef struct {
	char*			cmd;
	cbRigFuncGet_t	cbGet;
	cbRigFuncSet_t	cbSet;
}RIGCTL_FUNC_t;


/************************************************************
 * Explicit External Declarations                            *
 ************************************************************/

/************************************************************
 * Static variable Declaration                              *
 ************************************************************/
static char 	strReply[BUF_TX_LEN] 	= { 0 };
static bool 	longReply 			= false;
static char 	ansSep 				= '\n';
static int 		sendRprt 			= -1;
static char 	chkvfo 				= 0;	 /* if chkvfo == 1 need a VFOA/VFOB */
static int 		srvCoId 			= 0;
static ERMAK_RX_t	m_rxActive		= ERMAK_RX_MAIN;
static ERMAK_MSG_t	msg;
/************************************************************
 * Static Function Declaration                              *
 ************************************************************/
static int RigctlParseIn(char *pBuf);
static char * rigctl_cvtMode2Str(ERMAK_MODE_t mode);
static void NotifyWEBRemote(ERMAK_NOTIFY_TYPE_t type);
static int SendRequestToErmak(ERMAK_MSG_t *serverMsg);

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

static void cbRigCTLSetRit(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetRit(RIGCTL_PARM_t* pParm);
static void cbRigCTLSetXit(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetXit(RIGCTL_PARM_t* pParm);

static void cbRigCTLSetFunc(RIGCTL_PARM_t* pParm);
static void cbRigCTLGetFunc(RIGCTL_PARM_t* pParm);

static void cbRigCTLGetFullInfo(RIGCTL_PARM_t* pParm);
static inline void requestFullInfo(void);

/* Коллбэки для функций */

static void cbRigFuncGetRit(void);
static void cbRigFuncSetRit(uint8_t parm);
static void cbRigFuncGetXit(void);
static void cbRigFuncSetXit(uint8_t parm);



static RIGCTL_COMMAND_t _rigctlCommands[] = {
		RIGTCL("\\chk_vfo",			cbRigCTLChkVFO),
		//RIGTCL("\set_vfo_opt",	cbRigCTLSetVFOopt),
		RIGTCL("\\dump_state",		cbRigCTLDumpState),
		RIGTCL("\\get_vfo_info",	cbRigCTLGetVFOinfo),

		RIGTCL("\\get_powerstat",	cbRigCTLGetPowerState),
		RIGTCL("\\get_lock_mode",	cbRigCTLGetLockMode),
		RIGTCL("\\set_lock_mode",	cbRigCTLSetLockMode),
		RIGTCL("\\get_freq",		cbRigCTLGetFreq),
		RIGTCL("\\set_freq",		cbRigCTLSetFreq),
		RIGTCL("\\get_vfo",			cbRigCTLGetVFO),
		RIGTCL("\\set_vfo",			cbRigCTLSetVFO),
		RIGTCL("\\get_mode",		cbRigCTLGetMode),
		RIGTCL("\\set_mode",		cbRigCTLSetMode),
		RIGTCL("\\get_ptt",			cbRigCTLGetPTT),
		RIGTCL("\\set_ptt",			cbRigCTLSetPTT),
		RIGTCL("\\get_split_vfo",	cbRigCTLGetSplitVFO),
		RIGTCL("\\set_split_vfo",	cbRigCTLSetSplitVFO),
		RIGTCL("\\get_split_mode",	cbRigCTLGetSplitMode),
		RIGTCL("\\set_split_mode",	cbRigCTLSetSplitMode),
		RIGTCL("\\get_split_freq",	cbRigCTLGetSplitFreq),
		RIGTCL("\\set_split_freq",	cbRigCTLSetSplitFreq),
		RIGTCL("\\set_rit",			cbRigCTLSetRit),
		RIGTCL("\\get_rit",			cbRigCTLGetRit),
		RIGTCL("\\set_xit",			cbRigCTLSetXit),
		RIGTCL("\\get_xit",			cbRigCTLGetXit),
		RIGTCL("\\set_func",		cbRigCTLSetFunc),
		RIGTCL("\\get_func",		cbRigCTLGetFunc),
		//RIGTCL("\\get_dcd",			cbRigCTLGetDCD),

		RIGTCL("\\f",				cbRigCTLGetFullInfo),
		
		RIGTCL("\xf0",				cbRigCTLChkVFO),
		RIGTCL("\xf3",				cbRigCTLGetVFOinfo),
		RIGTCL("\x88",				cbRigCTLGetPowerState),
		RIGTCL("\xa3",				cbRigCTLGetLockMode),
		RIGTCL("\xa2",				cbRigCTLSetLockMode),
		RIGTCL("f",					cbRigCTLGetFreq),
		RIGTCL("F",					cbRigCTLSetFreq),
		RIGTCL("v",					cbRigCTLGetVFO),
		RIGTCL("V",					cbRigCTLSetVFO),
		RIGTCL("m",					cbRigCTLGetMode),
		RIGTCL("M",					cbRigCTLSetMode),
		RIGTCL("t",					cbRigCTLGetPTT),
		RIGTCL("T",					cbRigCTLSetPTT),
		RIGTCL("s",					cbRigCTLGetSplitVFO),
		RIGTCL("S",					cbRigCTLSetSplitVFO),
		RIGTCL("x",					cbRigCTLGetSplitMode),
		RIGTCL("X",					cbRigCTLSetSplitMode),
		RIGTCL("i",					cbRigCTLGetSplitFreq),
		RIGTCL("I",					cbRigCTLSetSplitFreq),
		RIGTCL("J",					cbRigCTLSetRit),
		RIGTCL("j",					cbRigCTLGetRit),
		RIGTCL("Z",					cbRigCTLSetXit),
		RIGTCL("z",					cbRigCTLGetXit),
		RIGTCL("U",					cbRigCTLSetFunc),
		RIGTCL("u",					cbRigCTLGetFunc),
		//RIGTCL("\x0b",				cbRigCTLGetDCD),

		RIGTCL(NULL, 	NULL)
};

static RIGCTL_FUNC_t _rigctlFunc[] = {
	RIGFUNC("FAGC",			NULL, NULL),
	RIGFUNC("NB",			NULL, NULL),
	RIGFUNC("COMP",			NULL, NULL),
	RIGFUNC("VOX",			NULL, NULL),
	RIGFUNC("TONE",			NULL, NULL),
	RIGFUNC("TSQL",			NULL, NULL),
	RIGFUNC("SBKIN",		NULL, NULL),
	RIGFUNC("FBKIN",		NULL, NULL),
	RIGFUNC("ANF",			NULL, NULL),
	RIGFUNC("NR",			NULL, NULL),
	RIGFUNC("AIP",			NULL, NULL),
	RIGFUNC("APF",			NULL, NULL),
	RIGFUNC("MON",			NULL, NULL),
	RIGFUNC("MN",			NULL, NULL),
	RIGFUNC("RF",			NULL, NULL),
	RIGFUNC("ARO",			NULL, NULL),
	RIGFUNC("LOCK",			NULL, NULL),
	RIGFUNC("MUTE",			NULL, NULL),
	RIGFUNC("VSC",			NULL, NULL),
	RIGFUNC("REV",			NULL, NULL),
	RIGFUNC("SQL",			NULL, NULL),
	RIGFUNC("ABM",			NULL, NULL),
	RIGFUNC("BC",			NULL, NULL),
	RIGFUNC("MBC",			NULL, NULL),
	RIGFUNC("RIT",			cbRigFuncGetRit, cbRigFuncSetRit),
	RIGFUNC("AFC",			NULL, NULL),
	RIGFUNC("SATMODE",		NULL, NULL),
	RIGFUNC("SCOPE",		NULL, NULL),
	RIGFUNC("RESUME",		NULL, NULL),
	RIGFUNC("TBURST",		NULL, NULL),
	RIGFUNC("TUNER",		NULL, NULL),
	RIGFUNC("XIT",			cbRigFuncGetXit, cbRigFuncSetXit),

	RIGFUNC(NULL, NULL, NULL)
};


/**
 * Convert  VFO value  to rigctl VFO string
 * @param mode
 * @return
 */
/* Эта функция может выводить не только VFOA/VFOB,а так-же при соответствующей поддержке из ToDo
	Мне кажется это не очень хорошая идея менять ее везде на  (vfoActive == ERMAK_VFO_MODE_A ? "VFOA" : "VFOB")

	ToDo VFO: Sub, Main, currVFO, MEM, TX, RX
  */
static char * rigctl_cvtVfo2Str(ERMAK_VFO_MODE_t vfo) {
	switch(vfo) {
		case ERMAK_VFO_MODE_A:
			return "VFOA";
		case ERMAK_VFO_MODE_B:
			return "VFOB";

		default:
			return "currVFO";
	}
}

/**
 * Convert  mode value  to rigctl mode string
 * @param mode
 * @return
 */
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
	case ERMAK_MODE_FSK:
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

/**
 * Send request to ERMAK internal server
 * @param serverMsg message
 * @return int result code
 */
/*
static int SendRequestToErmak(ERMAK_MSG_t *serverMsg){
	ERMAK_MSG_t msgsend;
	memcpy(&msgsend,serverMsg,sizeof(ERMAK_MSG_t));
	if(-1 == MsgSend(srvCoId, &msgsend, sizeof(msgsend), serverMsg, sizeof(ERMAK_MSG_t))){
		return EINVAL;
	}
	return EOK;
}
*/

/**
 * Convert  rigctl mode string to mode value
 * @param pStr
 * @return
 */
static ERMAK_MODE_t rigctl_cvtStr2Mode(char * pStr) {
	if (strcmp(pStr, "PKTUSB") == 0) {
		return ERMAK_MODE_PKTUSB;
	} else if (strcmp(pStr, "PKTLSB") == 0) {
		return ERMAK_MODE_FSK;
	} else if (strcmp(pStr, "USB") == 0) {
		return ERMAK_MODE_USB;
	} else if (strcmp(pStr, "LSB") == 0) {
		return ERMAK_MODE_LSB;
	} else if (strcmp(pStr, "CWR") == 0) {
		return ERMAK_MODE_CWL;
	} else if (strcmp(pStr, "CW") == 0) {
		return ERMAK_MODE_CWU;
	} else if (strcmp(pStr, "AM") == 0) {
		return ERMAK_MODE_AM;
	} else if (strcmp(pStr, "FM") == 0) {
		return ERMAK_MODE_FM;
	}

	return -1;
}

/**
 * Request full info from server
 * @return
 */
static inline void requestFullInfo(void){
	msg.command = ERMAK_COMMAND_GET_FULL_INFO;
	msg.fullInfo.rx = m_rxActive;
	SEND_REQUEST_TO_ERMAK(msg);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLChkVFO(RIGCTL_PARM_t* pParm) {
	if (longReply) {
		snprintf(strReply, sizeof(strReply), "ChkVFO: %d\n", chkvfo);
	} else
		snprintf(strReply, sizeof(strReply), "%d\n", chkvfo);
}

/**
 * Get DUMP state
 * @param pParm
 */

static void cbRigCTLDumpState(RIGCTL_PARM_t* pParm) {
	snprintf(strReply, sizeof(strReply), DUMP_STATE_ERMAK);
}

/**
 *	Get full VFO info
 * @param pParm
 */
static void cbRigCTLGetVFOinfo(RIGCTL_PARM_t *pParm) {
	bool ext_info = false;
	uint32_t freq = 0;
	uint32_t bw = 0;
	bool split;

	requestFullInfo();
	ERMAK_VFO_MODE_t vfoActive = msg.fullInfo.vfoRX;

	if (pParm->parm[0] != NULL) {
		if (strcmp(pParm->parm[0], "VFOA") == 0)
			vfoActive = ERMAK_VFO_MODE_A;
		else if (strcmp(pParm->parm[0], "VFOB") == 0)
			vfoActive = ERMAK_VFO_MODE_B;
		else if ((strcmp(pParm->parm[0], "?") == 0))
			ext_info = true;
	}

	if (ext_info) {
		if (longReply)
			/* "get_vfo_info: currVFO ?\n"  */
			snprintf(strReply, sizeof(strReply), "VFOA VFOB\n");
		else

			snprintf(strReply, sizeof(strReply), "VFOA VFOB\n");

	} else {

		if (vfoActive == ERMAK_VFO_MODE_A)
			freq = msg.fullInfo.vfoFreq.vfoA;
		else
			freq = msg.fullInfo.vfoFreq.vfoB;

		bw = msg.fullInfo.filter.bandPassHigh - msg.fullInfo.filter.bandPassLow;

		split = false;
		if (msg.fullInfo.split == ERMAK_SPLIT_ON)
			split = true;

		if (longReply) {
			snprintf(
					strReply,
					sizeof(strReply),
					"get_vfo_info: currVFO %s\nFreq: %u\nMode: %s\nWidth: %d\nSplit: %d\nSatMode: 0\n",
					(vfoActive == ERMAK_VFO_MODE_A ? "VFOA" : "VFOB"),
					freq,
					rigctl_cvtMode2Str(msg.fullInfo.mode),
					bw,
					split);
		} else {
			// В сокращенном варианте VFO не выводится
			snprintf(strReply, sizeof(strReply), "%u\n%s\n%d\n%d\n0\n",
					freq,
					rigctl_cvtMode2Str(msg.fullInfo.mode),
					bw,
					split);
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
		snprintf(strReply,sizeof(strReply),"Power Status: 1\n");
	} else
		snprintf(strReply,sizeof(strReply),"1\n");
}

/**
 * Get Lock Mode
 * @param pParm
 */
static void cbRigCTLGetLockMode(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	if (longReply) {
		// "get_lock_mode: currVFO"
		snprintf(strReply, sizeof(strReply), "Locked: %d\n", msg.fullInfo.lock == ERMAK_LOCK_ON ? 1:0);
	} else
		snprintf(strReply, sizeof(strReply), "%d\n", msg.fullInfo.lock == ERMAK_LOCK_ON ? 1:0);
}

/**
 * Set LOCK mode
 * @param pParm
 */
static void cbRigCTLSetLockMode(RIGCTL_PARM_t* pParm) {
	msg.command = ERMAK_COMMAND_SET_LOCK;
	if ((pParm->parm[0] != NULL) && (*pParm->parm[0] == '1'))
		msg.lockOn = true;
	else
		msg.lockOn = false;

	SEND_REQUEST_TO_ERMAK(msg);
	sendRprt = RIG_OK;
}

/**
 * Get current frequency
 * @param pParm
 */
static void cbRigCTLGetFreq(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	if (longReply)
		snprintf(strReply, sizeof(strReply), "Frequency: %u\n", msg.fullInfo.freq);
	else
		snprintf(strReply, sizeof(strReply), "%u\n", msg.fullInfo.freq);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetFreq(RIGCTL_PARM_t* pParm) {
	msg.command = ERMAK_COMMAND_SET_VFO;
	msg.vfoData.rx = m_rxActive;
	msg.vfoData.vfo = -1;

	uint32_t freq;
	if (pParm->parm[1] != NULL) {

		/* Includes VFO */
		freq = (uint32_t) strtof(pParm->parm[1], NULL);

		if (strcmp(pParm->parm[0], "VFOB") == 0)
			msg.vfoData.vfo = ERMAK_VFO_MODE_B;
		else if (strcmp(pParm->parm[0], "VFOA") == 0)
			msg.vfoData.vfo = ERMAK_VFO_MODE_A;

	} else if (pParm->parm[0] != NULL){

		freq = (uint32_t) strtof(pParm->parm[0], NULL);

	} else {

		sendRprt = RIG_EINVAL;
		return;
	}

	msg.vfoData.freq = freq;
	SEND_REQUEST_TO_ERMAK(msg);


	sendRprt = RIG_OK;
	NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
}

/**
 * Get full info
 * @param pParm
 */
static void cbRigCTLGetFullInfo(RIGCTL_PARM_t* pParm){
	requestFullInfo();
	if ((pParm->parm[0] != NULL) && (*pParm->parm[0] == '1'))	/* Prevent Memory Fault  wo Parm*/
			msg.fullInfo.rx = ERMAK_RX_SUB;

	SEND_REQUEST_TO_ERMAK(msg);

#define FULL_INFO_STR \
"Full Info\n" \
"  Rx:\t\t%s\n" \
"  Ptt:\t\t%s\n" \
"  Stackmem:\t%d\n" \
"  Band:\t\t%d\n" \
"  Freq:\t\t%dHz\n" \
"  Edges:\t%d - %d Hz\n" \
"  Mode:\t\t%s\n" \
"  VFOA:\t\t%dHz\n" \
"  VFOB:\t\t%dHz\n" \
"  VFO mode:\t%s\n" \
"  Split:\t%s\n" \
"  Rx:\t%s\n" \
"  Tx:\t%s\n" \
"  RIT:\t\t%s\n" \
"  XIT:\t\t%s\n" \
"  LNA:\t\t%s\n" \
"  ATT:\t\t%ddB\n" \
"  Filter:\t%d\n" \
"  BW:\t\t%d\n" \
"  APF:\t\t%s\n" \
"  RF gain:\t%ddB\n" \
"  AF gain:\t%ddB\n" \
"  AF mute:\t%s\n" \
"  NB:\t\t%s\n" \
"  NR:\t\t%s\n" \
"  ANF:\t\t%s\n" \
"  AGC:\t\t%d\n" \
"  AGC type:\t%s\n" \
"  Lock:\t\t%s\n" \
"  VOX:\t\t%s\n" \
"  RX ant:\t%s\n" \
"  TX ant:\t%s\n"

    snprintf(strReply, sizeof(strReply), FULL_INFO_STR,
        msg.fullInfo.rx == ERMAK_RX_MAIN ? "RX_MAIN":"RX_SUB",
        msg.fullInfo.ptt == ERMAK_TRANSMIT_ON ? "TX_MODE":"RX_MODE",
        ((uint32_t)msg.fullInfo.stackMem) + 1,
        (uint32_t)msg.fullInfo.band,
        (uint32_t)msg.fullInfo.freq,
        msg.fullInfo.edges.low,msg.fullInfo.edges.high,
        rigctl_cvtMode2Str(msg.fullInfo.mode),
        msg.fullInfo.vfoFreq.vfoA,
        msg.fullInfo.vfoFreq.vfoB,
        msg.fullInfo.vfoMode == ERMAK_VFO_MODE_A ? "VFOA":"VFOB",
        msg.fullInfo.split == ERMAK_SPLIT_ON ? "ON":"OFF",
        msg.fullInfo.vfoRX == ERMAK_VFO_MODE_A ? "VFOA":"VFOB",
        msg.fullInfo.vfoTX == ERMAK_VFO_MODE_A ? "VFOA":"VFOB",
        msg.fullInfo.ritXit.rit == ERMAK_RIT_ON ? "ON":"OFF",
        msg.fullInfo.ritXit.xit == ERMAK_XIT_ON ? "ON":"OFF",
        msg.fullInfo.lnaAtt == ERMAK_LNA_ON ? "ON":"OFF",
        msg.fullInfo.lnaAtt == ERMAK_LNA_ON ? 0:(-10 * (msg.fullInfo.lnaAtt - 1)),
        msg.fullInfo.filter.filterType,
        msg.fullInfo.filter.bandPassHigh - msg.fullInfo.filter.bandPassLow,
        msg.fullInfo.filter.apf == ERMAK_APF_ON ? "ON":"OFF",
        msg.fullInfo.rfGain,
        msg.fullInfo.afGain,
        msg.fullInfo.afMute == ERMAK_AF_MUTE_ON ? "MUTED":"UNMUTED",
        msg.fullInfo.nob.run == true ? "ON":"OFF",
        msg.fullInfo.nor.run == true ? "ON":"OFF",
        msg.fullInfo.anf == ERMAK_ANF_ON ? "ON":"OFF",
        msg.fullInfo.agc,
        msg.fullInfo.agcOn == true ? "ON":"OFF",
        msg.fullInfo.lock == ERMAK_LOCK_ON  ? "ON":"OFF",
        msg.fullInfo.vox == ERMAK_VOX_ON  ? "ON":"OFF",
        msg.fullInfo.rxAnt == ERMAK_ANT_1  ? "1":"2",
        msg.fullInfo.txAnt == ERMAK_ANT_1  ? "1":"2");
}

/**
 * Get current VFO
 * @param pParm
 */
static void cbRigCTLGetVFO(RIGCTL_PARM_t* pParm){
	requestFullInfo();
	// Почему здесь используется 'vfoMode' а не 'vfoRX', и чем они отличаются
	if(longReply) {
		// "get_vfo: currVFO"
		snprintf(strReply,sizeof(strReply), "VFO: %s\n", 
			rigctl_cvtVfo2Str(msg.fullInfo.vfoMode));
	} else
		snprintf(strReply,sizeof(strReply), "%s\n", 
			rigctl_cvtVfo2Str(msg.fullInfo.vfoMode));
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetVFO(RIGCTL_PARM_t* pParm) {
	if (pParm->parm[0] == NULL) {
		sendRprt = RIG_EINVAL;
		return;
	}

	if (strcmp(pParm->parm[0], "?") == 0) {
		snprintf(strReply, sizeof(strReply), "set_vfo: currVFO ?\nVFOA VFOB\n");
		sendRprt = RIG_OK;
		return;
	} else {
		msg.command = ERMAK_COMMAND_SWITCH_VFO;
		// ToDo convert VFO str to number
		if (strcmp(pParm->parm[0], "VFOB") == 0) {
			msg.vfoData.vfo = ERMAK_VFO_MODE_B;
		} else {
			msg.vfoData.vfo = ERMAK_VFO_MODE_A;
		}

		SEND_REQUEST_TO_ERMAK(msg);
		sendRprt = RIG_OK;
		NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
	}
}

/**
 * Get current mode
 * @param pParm
 */
static void cbRigCTLGetMode(RIGCTL_PARM_t *pParm) {
	char* mode;
	requestFullInfo();
	mode = rigctl_cvtMode2Str(msg.fullInfo.mode);

	if (longReply) {
		// "get_mode: VFOA"
		snprintf(strReply, sizeof(strReply), "Mode: %s\nPassband: %d\n", mode,
				msg.freqModeData.bw);
	} else
		snprintf(strReply, sizeof(strReply), "%s\n%d\n", mode,
				msg.freqModeData.bw);
}

/**
 * Set mode
 * @param pParm
 */

static void cbRigCTLSetMode(RIGCTL_PARM_t* pParm) {
	msg.command = ERMAK_COMMAND_SET_MODE;
	ERMAK_MODE_t mode = -1;

	if(pParm->parm[2] != NULL) {    /* Set vfo, mode, passband */
		/*
		// Номер VFO игнорируем
		if(strcmp(pParm->parm[0], "VFOB") == 0)
		    msg.freqModeData.vfo = ERMAK_VFO_MODE_B;
		// !! команда не должна менять VFO
		msg.command = ERMAK_COMMAND_SWITCH_VFO;
		if (strcmp(pParm->parm[0], "VFOB") == 0)
			msg.vfoData.vfo = ERMAK_VFO_MODE_B;
		else
			msg.vfoData.vfo = ERMAK_VFO_MODE_A;
		SEND_REQUEST_TO_ERMAK(msg);
		*/
		mode = rigctl_cvtStr2Mode(pParm->parm[1]);
	}
	else if(pParm->parm[0] != NULL) {
		mode = rigctl_cvtStr2Mode(pParm->parm[0]);
	}

	if(mode == -1) {
		sendRprt =  RIG_EINVAL;
		return;
	}

	msg.freqModeData.mode = mode;
	SEND_REQUEST_TO_ERMAK(msg);
	sendRprt = RIG_OK;
	NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetPTT(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	if (longReply)
		snprintf(strReply, sizeof(strReply), "PTT: %d\n", msg.fullInfo.ptt);
	else
		snprintf(strReply, sizeof(strReply), "%d\n", msg.fullInfo.ptt);
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetPTT(RIGCTL_PARM_t* pParm) {
	msg.command = ERMAK_COMMAND_SET_TRANSMITT_RX;
	msg.transmittRx.rx = m_rxActive;

	if (pParm->parm[0] != NULL) {
		if (*pParm->parm[0] == '1')
			msg.transmittRx.transmitt = ERMAK_TRANSMIT_ON;
		else
			msg.transmittRx.transmitt = ERMAK_TRANSMIT_OFF;
	} else {
        sendRprt =  RIG_EINVAL;
		return;
	}

	SEND_REQUEST_TO_ERMAK(msg);
	sendRprt = RIG_OK;
	NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_RXTX);
}

/* Split Section */

/**
 * Get split mode
 * @param pParm
 */
static void cbRigCTLGetSplitVFO(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	if (longReply)
		snprintf(strReply, sizeof(strReply), "Split: %d\nTX VFO: %s\n",
						 msg.fullInfo.split == ERMAK_SPLIT_OFF 	? 	0:1,
						 msg.fullInfo.vfoTX == ERMAK_VFO_MODE_A 	? 	"VFOA":"VFOB");
	else
		snprintf(strReply, sizeof(strReply), "%d\n%s\n",
						 msg.fullInfo.split == ERMAK_SPLIT_OFF 	? 	0:1,
						 msg.fullInfo.vfoTX == ERMAK_VFO_MODE_A 	? 	"VFOA":"VFOB");

}

/**
 * Set split mode
 * @param pParm
 */
 
// Управляет переключением VFO для TX в режиме split, и включением режима
// не должен менять VFO
 
static void cbRigCTLSetSplitVFO(RIGCTL_PARM_t* pParm) {
	msg.command = ERMAK_COMMAND_SET_SPLIT;

	msg.kenwoodFT.split = ERMAK_SPLIT_OFF;
	if ((pParm->parm[0] != NULL) && (*pParm->parm[0] == '1'))
		msg.kenwoodFT.split = ERMAK_SPLIT_ON;

	msg.kenwoodFT.vfoRx = ERMAK_VFO_MODE_B;
	if ((pParm->parm[1] != NULL) && strcmp(pParm->parm[1], "VFOA") == 0)
		msg.kenwoodFT.vfoRx = ERMAK_VFO_MODE_A;		// здесь выставляем какой VFO используется для TX в режиме split
													// RX VFO здесь не должен изменяться
	SEND_REQUEST_TO_ERMAK(msg);
	sendRprt = RIG_OK;
	NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_RXTX);	// Какой должен быть notify ?
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetSplitMode(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	char* mode;
	mode = rigctl_cvtMode2Str(msg.fullInfo.mode);

	if (longReply) {
		snprintf(strReply, sizeof(strReply), "TX Mode: %s\nTX Passband: %d\n", mode,
				msg.fullInfo.filter.bandPassHigh - msg.fullInfo.filter.bandPassLow);
	} else {
		snprintf(strReply, sizeof(strReply), "%s\n%d\n", mode,
				msg.fullInfo.filter.bandPassHigh - msg.fullInfo.filter.bandPassLow);
	}
}

/**
 *
 * @param pParm
 */
static void cbRigCTLSetSplitMode(RIGCTL_PARM_t* pParm) {
	// ToDo check Parm 0 - mode, 1 passband
	sendRprt = RIG_OK;
}

/**
 *
 * @param pParm
 */
static void cbRigCTLGetSplitFreq(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	uint32_t freq = 0;

	freq = (msg.fullInfo.vfoTX == ERMAK_VFO_MODE_A ? msg.fullInfo.vfoFreq.vfoA:msg.fullInfo.vfoFreq.vfoB);

	if (longReply)
		snprintf(strReply, sizeof(strReply), "TX VFO: %d\n", freq);
	else
		snprintf(strReply, sizeof(strReply), "%d\n", freq);
}

/**
 *
 * @param pParm
 */
// Это команда изменяет частоту в TX VFO и не должна менять активный VFO.
static void cbRigCTLSetSplitFreq(RIGCTL_PARM_t* pParm) {
	ERMAK_VFO_MODE_t vfoTX;

	/* GET active VFO */
	requestFullInfo();

	vfoTX = msg.fullInfo.vfoTX;

	/* Prepare freq to TX VFO  */
	msg.command = ERMAK_COMMAND_SET_VFO;
	msg.vfoData.rx = m_rxActive;
	msg.vfoData.vfo = vfoTX;

	uint32_t freq;
	if (pParm->parm[0] != NULL) {
		freq = (uint32_t) strtof(pParm->parm[0], NULL);
	} else {
		sendRprt = RIG_EINVAL;
		return;
	}

	msg.vfoData.freq = freq;
	SEND_REQUEST_TO_ERMAK(msg);

	sendRprt = RIG_OK;
	NotifyWEBRemote(ERMAK_NOTIFY_TYPE_WEB_ALL);
}

// Только устанавливает RIT частоту, не активирует
static void cbRigCTLSetRit(RIGCTL_PARM_t* pParm) {
	int32_t freq;
	if (pParm->parm[0] == NULL) {
		sendRprt = RIG_EINVAL;
		return;
	} else {
		// ToDo Check min - max
		freq = atoi(pParm->parm[0]);
	}

	msg.command = ERMAK_COMMAND_SET_RIT_FREQ;
	msg.ritXit.ritFreq = freq;
	SEND_REQUEST_TO_ERMAK(msg);

	sendRprt = RIG_OK;
}

static void cbRigCTLGetRit(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	if (longReply)
		snprintf(strReply, sizeof(strReply), "RIT: %d\n", msg.fullInfo.ritXit.ritFreq);
	else
		snprintf(strReply, sizeof(strReply), "%d\n", msg.fullInfo.ritXit.ritFreq);
}

// Только устанавливает XIT частоту, не активирует
static void cbRigCTLSetXit(RIGCTL_PARM_t* pParm) {
	int32_t freq;
	if (pParm->parm[0] == NULL) {
		sendRprt = RIG_EINVAL;
		return;
	} else {
		// ToDo Check min - max
		freq = atoi(pParm->parm[0]);
	}

	msg.command = ERMAK_COMMAND_SET_XIT_FREQ;
	msg.ritXit.xitFreq = freq;
	SEND_REQUEST_TO_ERMAK(msg);

	sendRprt = RIG_OK;
}

static void cbRigCTLGetXit(RIGCTL_PARM_t* pParm) {
	requestFullInfo();
	if (longReply)
		snprintf(strReply, sizeof(strReply), "XIT: %d\n", msg.fullInfo.ritXit.xitFreq);
	else
		snprintf(strReply, sizeof(strReply), "%d\n", msg.fullInfo.ritXit.xitFreq);
}


/* Функции */

static void cbRigFuncGetRit(void) {
	requestFullInfo();
	char rit = (msg.fullInfo.ritXit.rit == ERMAK_RIT_OFF) ? '0' : '1';
	if (longReply) {
		snprintf(strReply, sizeof(strReply), "Func Status: %c\n", rit);
	} else {
		snprintf(strReply, sizeof(strReply), "%c\n", rit);
	}
}

static void cbRigFuncSetRit(uint8_t parm) {
	ERMAK_RIT_t rit = ERMAK_RIT_OFF;
	if (parm == 1)
		rit = ERMAK_RIT_ON;

	msg.command = ERMAK_COMMAND_SET_RIT;
	msg.ritXit.rit = rit;
	SEND_REQUEST_TO_ERMAK(msg);

	sendRprt = RIG_OK;
}

static void cbRigFuncGetXit(void) {
	requestFullInfo();
	char xit = (msg.fullInfo.ritXit.xit == ERMAK_XIT_OFF) ? '0' : '1';
	if (longReply) {
		snprintf(strReply, sizeof(strReply), "Func Status: %c\n", xit);
	} else {
		snprintf(strReply, sizeof(strReply), "%c\n", xit);
	}
}

static void cbRigFuncSetXit(uint8_t parm) {
	ERMAK_XIT_t xit = ERMAK_XIT_OFF;
	if (parm == 1)
		xit = ERMAK_XIT_ON;

	msg.command = ERMAK_COMMAND_SET_XIT;
	msg.ritXit.xit = xit;
	SEND_REQUEST_TO_ERMAK(msg);

	sendRprt = RIG_OK;
}



static void cbRigCTLSetFunc(RIGCTL_PARM_t* pParm) {
	if (pParm->parm[0] == NULL) {
		sendRprt = RIG_EINVAL;
		return;
	} else if (*pParm->parm[0] == '?') {		/* Get Implemented Functions List */
		RIGCTL_FUNC_t * funkList;
		int i = 0;
		do {
			funkList = &_rigctlFunc[i++];
			if (funkList->cbSet != NULL) {
				strcat(strReply, " ");
				strcat(strReply, funkList->cmd);
			}
		}while (funkList->cmd != NULL);
		strcat(strReply, "\n");
	} else {
		RIGCTL_FUNC_t * funkList;
		int i = 0;
		funkList = &_rigctlFunc[i++];
		while (funkList->cmd != NULL) {
			if (strcmp(funkList->cmd, pParm->parm[0]) == 0) {
				if (funkList->cbSet != NULL){
					uint8_t parm = 0;
					if(pParm->parm[1] != NULL)
						parm = atoi(pParm->parm[1]);
					funkList->cbSet(parm);
				}
				return;
			}
			funkList = &_rigctlFunc[i++];
		}

		LOG_ERRCMD(pParm->parm[0]);
		sendRprt =  RIG_ENAVAIL;
	}
}

static void cbRigCTLGetFunc(RIGCTL_PARM_t* pParm) {
	if (pParm->parm[0] == NULL) {
		sendRprt = RIG_EINVAL;
		return;
	} else if (*pParm->parm[0] == '?') {		/* Get Implemented Functions List */
		if (longReply) {
			strcat(strReply, "Func Status: ");
		}

		RIGCTL_FUNC_t * funkList;
		int i = 0;
		do {
			funkList = &_rigctlFunc[i++];
			if (funkList->cbGet != NULL) {
				strcat(strReply, " ");
				strcat(strReply, funkList->cmd);
			}
		}while (funkList->cmd != NULL);
		strcat(strReply, "\n");
	} else {
		RIGCTL_FUNC_t * funkList;
		int i = 0;
		funkList = &_rigctlFunc[i++];
		while (funkList->cmd != NULL) {
			if (strcmp(funkList->cmd, pParm->parm[0]) == 0) {
				if (funkList->cbGet != NULL){
					funkList->cbGet();
				}
				return;
			}
			funkList = &_rigctlFunc[i++];
		}

		LOG_ERRCMD(pParm->parm[0]);
		sendRprt =  RIG_ENAVAIL;
	}
}



/* API */

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
        //while(ch != '\n') {
        //    ch = *(++pBuf);
        //    if(ch == 0) return 0;
        //}
        return 0;
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

/**
 *
 * @param type
 */
static void NotifyWEBRemote(ERMAK_NOTIFY_TYPE_t type) {
	msg.command = ERMAK_COMMAND_NOTIFY_WEB_REMOTE;
	msg.notifyType = type;
	SEND_REQUEST_TO_ERMAK(msg);
}

int rigctl_req(char *pReq, char *pResp)
{
    size_t len = strlen(pReq);

    sendRprt = RIG_EEND;
    strReply[0] = 0;

    LOG_REQUEST(pReq);

    if(rigctl_parse_in(pReq) < 0)
        return -1;

    // Replace delimiter
    len = strlen(strReply);
    if(len != 0) {
        if (ansSep != '\n') {
            int i;
            for(i = 0; i < len; i++)
                if(strReply[i] == '\n') strReply[i] = ansSep;
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
        strcat(strReply, rprt);
    }

    len = strlen(strReply);

    if(0 != len){
        // Send Answer
        strcpy(pResp, strReply);

        LOG_RESPONSE(strReply);
    }

    return len;
}
