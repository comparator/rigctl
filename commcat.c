#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "ermak.h"
#include "commcat.h"

#define COMMRQ(c, x)        {.command = c, .cb = x}

#define BUF_TX_LEN      80

typedef void(*cbCommRq)(char *pStr);

typedef struct COMMRQ_CMD_t{
	char*	command;
	cbCommRq cb;
}COMMRQ_CMD_t;


static char _str[BUF_TX_LEN] = {0};

static char         parm_ai = 0;
static char         parm_agc = 1;
static int          parm_af = 128;
static int          parm_rf = 128;
static int          parm_sql = 128;
static int          parm_freqA = 14074000;
static int          parm_freqB = 21074000;
static int          parm_pwr = 100;
static char         parm_tx = 0;
static char         parm_mode = 1;
static char         parm_menu = 0;
static char         parm_RxVFO = 0;
static char         parm_TxVFO = 1;

static ERMAK_MSG_t	msg;


static inline void requestFullInfo(void);

static void cbReqAC(char *pStr);    // Sets or reads the internal antenna tuner status.
static void cbReqAG(char *pStr);    // Sets or reads the AF gain
static void cbReqAI(char *pStr);    // Sets or reads the Auto Information (AI) function ON/ OFF
static void cbReqAN(char *pStr);    // Selects the antenna connector ANT1/ ANT2
static void cbReqBC(char *pStr);    // Sets or reads the Beat Cancel function status
static void cbReqBY(char *pStr);    // Reads the busy signal status
static void cbReqCA(char *pStr);    // Sets and reads the CW TUNE function status
static void cbReqCN(char *pStr);    // Sets and reads the CTCSS frequency
static void cbReqCT(char *pStr);    // Sets and reads the CTCSS function status
static void cbReqDA(char *pStr);    // Sets and reads the DATA mode
static void cbReqFA(char *pStr);    // Sets or reads the VFO A frequency
static void cbReqFB(char *pStr);    // Sets or reads the VFO B frequency
static void cbReqFL(char *pStr);    // Sets and reads the IF filter
static void cbReqFR(char *pStr);    // Selects or reads the VFO or Memory channel
static void cbReqFS(char *pStr);    // Sets and reads the Fine Tuning function status
static void cbReqFT(char *pStr);    // Selects or reads the VFO or Memory channel
static void cbReqFV(char *pStr);    // Verifies the Firmware version
static void cbReqGC(char *pStr);    // Sets or reads the AGC
static void cbReqID(char *pStr);    // Reads the transceiver ID number
static void cbReqIF(char *pStr);    // Reads the transceiver status
static void cbReqIS(char *pStr);    // Sets and reads the DSP Filter Shift
static void cbReqKS(char *pStr);    // Sets and reads the Keying speed
static void cbReqLK(char *pStr);    // Sets and reads the Lock status
static void cbReqMD(char *pStr);    // Sets and reads the operating mode status
static void cbReqMF(char *pStr);    // Sets and reads Menu A or B
static void cbReqMG(char *pStr);    // Sets and reads the microphone gain
static void cbReqNB(char *pStr);    // Sets and reads the Noise Blanker function status
static void cbReqNL(char *pStr);    // Sets and reads the Noise Blanker level
static void cbReqNR(char *pStr);    // Sets and reads the Noise Reduction function status
static void cbReqNT(char *pStr);    // Sets and reads the Notch Filter status
static void cbReqPA(char *pStr);    // Sets and reads the Pre-amplifier function status
static void cbReqPC(char *pStr);    // Sets and reads the output power
static void cbReqPR(char *pStr);    // Sets and reads the Speech Processor function ON/ OFF
static void cbReqPS(char *pStr);    // Sets and reads the Power ON/ OFF status
static void cbReqQR(char *pStr);    // Sets and reads the Quick Memory channel data
static void cbReqRA(char *pStr);    // Sets and reads the RF Attenuator status
static void cbReqRG(char *pStr);    // Sets and reads the RF Gain status
static void cbReqRM(char *pStr);    // Sets and reads the Meter function
static void cbReqRX(char *pStr);    // Sets the receiver function status
static void cbReqSD(char *pStr);    // Sets and reads the CW break-in time delay.
static void cbReqSM(char *pStr);    // Reads the S-meter value
static void cbReqSQ(char *pStr);    // Sets and reads the squelch value
static void cbReqTN(char *pStr);    // Sets and reads the Tone frequency
static void cbReqTO(char *pStr);    // Sets and reads the Tone frequency
static void cbReqTS(char *pStr);    // Sets and reads the TF-Set status
static void cbReqTX(char *pStr);    // Sets the transmission mode
static void cbReqVD(char *pStr);    // Sets and reads the VOX Delay time
static void cbReqVG(char *pStr);    // Sets and reads the VOX Gain
static void cbReqVV(char *pStr);    // Performs the VFO copy (A=B) function
static void cbReqVX(char *pStr);    // Sets and reads the VOX and Break-in function status


static COMMRQ_CMD_t _commCmd[] = {
    COMMRQ("AC", cbReqAC),
    COMMRQ("AG", cbReqAG),
    COMMRQ("AI", cbReqAI),
    COMMRQ("AN", cbReqAN),
    COMMRQ("BC", cbReqBC),
    COMMRQ("BY", cbReqBY),
    COMMRQ("CA", cbReqCA),
    COMMRQ("CN", cbReqCN),
    COMMRQ("CT", cbReqCT),
    COMMRQ("DA", cbReqDA),
    COMMRQ("FA", cbReqFA),
    COMMRQ("FB", cbReqFB),
    COMMRQ("FL", cbReqFL),
    COMMRQ("FR", cbReqFR),
    COMMRQ("FS", cbReqFS),
    COMMRQ("FT", cbReqFT),
    COMMRQ("FV", cbReqFV),
    COMMRQ("GC", cbReqGC),
    COMMRQ("ID", cbReqID),
    COMMRQ("IF", cbReqIF),
    COMMRQ("IS", cbReqIS),
    COMMRQ("KS", cbReqKS),
    COMMRQ("LK", cbReqLK),
    COMMRQ("MD", cbReqMD),
    COMMRQ("MF", cbReqMF),
    COMMRQ("MG", cbReqMG),
    COMMRQ("NB", cbReqNB),
    COMMRQ("NL", cbReqNL),
    COMMRQ("NR", cbReqNR),
    COMMRQ("NT", cbReqNT),
    COMMRQ("PA", cbReqPA),
    COMMRQ("PC", cbReqPC),
    COMMRQ("PR", cbReqPR),
    COMMRQ("PS", cbReqPS),
    COMMRQ("QR", cbReqQR),
    COMMRQ("RA", cbReqRA),
    COMMRQ("RG", cbReqRG),
    COMMRQ("RM", cbReqRM),
    COMMRQ("RX", cbReqRX),
    COMMRQ("SD", cbReqSD),
    COMMRQ("SM", cbReqSM),
    COMMRQ("SQ", cbReqSQ),
    COMMRQ("TN", cbReqTN),
    COMMRQ("TO", cbReqTO),
    COMMRQ("TS", cbReqTS),
    COMMRQ("TX", cbReqTX),
    COMMRQ("VD", cbReqVD),
    COMMRQ("VG", cbReqVG),
    COMMRQ("VV", cbReqVV),
    COMMRQ("VX", cbReqVX),

    COMMRQ(NULL, NULL)
};


/**
 * Request full info from server
 * @return
 */
static inline void requestFullInfo(void){
	msg.command = ERMAK_COMMAND_GET_FULL_INFO;
	msg.fullInfo.rx = m_rxActive;
	SEND_REQUEST_TO_ERMAK(msg);
}



static void cbReqAC(char *pStr) {   // Sets or reads the internal antenna tuner status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "AC000;", parm_freqA);
    }
    else
    {
        printf("AC: %s\n", pStr);
    }
}

static void cbReqAG(char *pStr) {   // Sets or reads the AF gain
    if(pStr[1] != 0)
    {
        parm_af = atoi(pStr);
        printf("AG: %d\n", parm_af);
    }
    else
    {
        snprintf(_str,sizeof(_str), "AG0%03d;", parm_af);
    }
}

static void cbReqAI(char *pStr) {   // Sets or reads the Auto Information (AI) function ON/ OFF
    if(pStr != NULL) {
        parm_ai = *pStr - '0';
    }
    snprintf(_str,sizeof(_str), "AI%d;", parm_ai);
}

static void cbReqAN(char *pStr) {   // Selects the antenna connector ANT1/ ANT2
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "AN011;");
    }
    else
    {
        printf("AN: %s\n", pStr);
    }
}

static void cbReqBC(char *pStr) {   // Sets or reads the Beat Cancel function status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "BC0;");
    }
    else
    {
        printf("BC: %s\n", pStr);
    }
}

static void cbReqBY(char *pStr) {   // Reads the busy signal status
    snprintf(_str,sizeof(_str), "BY00;");
}

static void cbReqCA(char *pStr) {   // Sets and reads the CW TUNE function status.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "CA0;");
    }
    else
    {
        printf("CA: %s\n", pStr);
    }
}

static void cbReqCN(char *pStr) {   // Sets and reads the CTCSS frequency.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "CN00;");
    }
    else
    {
        printf("CN: %s\n", pStr);
    }
}

static void cbReqCT(char *pStr) {   // Sets and reads the CTCSS function status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "CT0;");
    }
    else
    {
        printf("CT: %s\n", pStr);
    }
}

static void cbReqDA(char *pStr) {   // Sets and reads the DATA mode
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "DA0;");
    }
    else
    {
        printf("DA: %s\n", pStr);
    }
}

static void cbReqFA(char *pStr) {   // Sets or reads the VFO A frequency
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "FA%011d;", parm_freqA);
    }
    else
    {
        parm_freqA = atoi(pStr);
        printf("FA: %d\n", parm_freqA);
    }
}

static void cbReqFB(char *pStr) {   // Sets or reads the VFO B frequency
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "FB%011d;", parm_freqB);
    }
    else
    {
        parm_freqB = atoi(pStr);
        printf("FB: %d\n", parm_freqB);
    }
}

static void cbReqFL(char *pStr) {   // Sets and reads the IF filter
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "FL1;");
    }
    else
    {
        printf("FL: %s\n", pStr);
    }
}

static void cbReqFR(char *pStr) {   // Selects or reads the VFO or Memory channel Simplex
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "FR%d;", parm_RxVFO);
    }
    else
    {
        char ch = *pStr;
        if(ch == '0') parm_RxVFO = 0;
        else if(ch == '1') parm_RxVFO = 1;
        printf("FR; %d\n", parm_RxVFO);
    }
}

static void cbReqFS(char *pStr) {   // Sets and reads the Fine Tuning function status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "FS0;");
    }
    else
    {
        printf("FS: %s\n", pStr);
    }
}

static void cbReqFT(char *pStr) {   // Selects or reads the VFO or Memory channel Split
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "FT%d;", parm_TxVFO);
    }
    else
    {
        char ch = *pStr;
        if(ch == '0') parm_TxVFO = 0;
        else if(ch == '1') parm_TxVFO = 1;
        printf("FR; %d\n", parm_RxVFO);
    }
}

static void cbReqFV(char *pStr) {   // Verifies the Firmware version.
    snprintf(_str,sizeof(_str), "FV1.02;");
}

static void cbReqGC(char *pStr) {   // Sets or reads the AGC.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "GC%d;", parm_agc);
    }
    else
    {
        char ch = *pStr;
        if(ch == '1' || ch == '3')  parm_agc = 1;
        else if(ch == '0') parm_agc = 0;
        else if(ch == '2') parm_agc = 2;
        printf("GC: %d\n", parm_agc);
    }
}

static void cbReqID(char *pStr) {   // Reads the transceiver ID number
    snprintf(_str,sizeof(_str), "ID021;");
}

static void cbReqIF(char *pStr) {   // Reads the transceiver status
    int freq = (parm_RxVFO == 0) ? parm_freqA : parm_freqB;

    snprintf(_str,sizeof(_str), "IF%011d0000+0000000000%d%d%d000000;", freq, parm_tx, parm_mode, parm_RxVFO);
}
/*
IF 00007074000 0100+00110 00000 0  9  0  000000;  RIT

IF 00007074000 0100+00000 00000 1  9  0  000000;  TX

IF 00007074000 0100+00000 00000 0  9  0  000000;
IF %011d       0000+00000 00000 %d %d %d 000000;
.. ........... .......... ..... 1  .  .  ....... pmTx
.. ........... .......... ..... .  9  .  ....... pmMode
.. ........... .......... ..... .  .  1  .0..... pmVfoBB
.. ........... .......... ..... .  .  .  .1..... pmSplitOn
*/

static void cbReqIS(char *pStr) {   // Sets and reads the DSP Filter Shift.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "IS 0800;");
    }
    else
    {
        printf("IS: %s\n", pStr);
    }
}

static void cbReqKS(char *pStr) {   // Sets and reads the Keying speed
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "KS025;");
    }
    else
    {
        printf("KS: %s\n", pStr);
    }
}

static void cbReqLK(char *pStr) {   // Sets and reads the Lock status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "LK0;");
    }
    else
    {
        printf("LK: %s\n", pStr);
    }
}

static void cbReqMD(char *pStr) {   // Sets and reads the operating mode status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "MD%d;", parm_mode);
    }
    else
    {
        parm_mode = atoi(pStr);
        printf("MD: %d\n", parm_mode);
    }
}

static void cbReqMF(char *pStr) {   // Sets and reads Menu A or B
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "MF%d;", parm_menu);
    }
    else
    {
        char ch = *pStr;
        if( ch == '0' ) parm_menu = 0;
        else if(ch == '1') parm_menu = 1;
        printf("MF: %d\n", parm_menu);
    }
}

static void cbReqMG(char *pStr) {   // Sets and reads the microphone gain.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "MG050;");
    }
    else
    {
        printf("MG: %s\n", pStr);
    }
}

static void cbReqNB(char *pStr) {   // Sets and reads the Noise Blanker function status.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "NB0;");
    }
    else
    {
        printf("NB: %s\n", pStr);
    }
}

static void cbReqNL(char *pStr) {   // Sets and reads the Noise Blanker level
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "NL005;");
    }
    else
    {
        printf("NL: %s\n", pStr);
    }
}

static void cbReqNR(char *pStr) {   // Sets and reads the Noise Reduction function status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "NR0;");
    }
    else
    {
        printf("NR: %s\n", pStr);
    }
}

static void cbReqNT(char *pStr) {   // Sets and reads the Notch Filter status.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "NT00;");
    }
    else
    {
        printf("NT: %s\n", pStr);
    }
}

static void cbReqPA(char *pStr) {   // Sets and reads the Pre-amplifier function status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "PA00;");
    }
    else
    {
        printf("PA: %s\n", pStr);
    }
}

static void cbReqPC(char *pStr) {   // Sets and reads the output power
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "PC%03d;", parm_pwr);
    }
    else
    {
        int pwr = atoi(pStr);
        if(pwr < 5) pwr = 5;
        else if(pwr > 100) pwr = 100;
        parm_pwr = pwr;
        printf("PC: %d\n", pwr);
    }
}

static void cbReqPR(char *pStr) {   // Sets and reads the Speech Processor function ON/ OFF.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "PR0;");
    }
    else
    {
        printf("PR: %s\n", pStr);
    }
}

static void cbReqPS(char *pStr) {   // Sets and reads the Power ON/ OFF status
    if((pStr == NULL) || (parm_ai != 0)) {
        snprintf(_str,sizeof(_str), "PS1;");
    }
}

static void cbReqQR(char *pStr) {   // Sets and reads the Quick Memory channel data
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "QR00;");
    }
    else
    {
        printf("QR: %s\n", pStr);
    }
}

static void cbReqRA(char *pStr) {   // Sets and reads the RF Attenuator status.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "RA0000;");
    }
    else
    {
        printf("RA: %s\n", pStr);
    }
}

static void cbReqRG(char *pStr) {   // Sets and reads the RF Gain status.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "RG%03d;", parm_rf);
    }
    else
    {
        int rf = atoi(pStr);
        if(rf < 0) rf = 0;
        else if(rf > 255) rf = 255;
        parm_rf = rf;
        printf("RG: %d\n", parm_rf);
    }
}

static void cbReqRM(char *pStr) {   // Sets and reads the Meter function
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "RM00000;");
    }
    else
    {
        printf("RM: %s\n", pStr);
    }
}

static void cbReqRX(char *pStr) {   // Sets the receiver function status.
    parm_tx = 0;
    snprintf(_str,sizeof(_str), "RX;");
    printf("RX\n");
}

static void cbReqSD(char *pStr) {   // Sets and reads the CW break-in time delay.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "SD0250;");
    }
    else
    {
        printf("SD: %s\n", pStr);
    }
}

static void cbReqSM(char *pStr) {   // Reads the S-meter value
    snprintf(_str,sizeof(_str), "SM00010;");
}

static void cbReqSQ(char *pStr) {   // Sets and reads the squelch value
    if(pStr != NULL)
    {
        int sql = atoi(pStr);
        if(sql < 0) sql = 0;
        else if(sql > 255) sql = 255;
        parm_sql = sql;
        printf("SQ: %d\n", parm_sql);
    }
    snprintf(_str,sizeof(_str), "SQ0%03d;", parm_sql);
}

static void cbReqTN(char *pStr) {   // Sets and reads the Tone frequency.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "TN00;");
    }
    else
    {
        printf("TN: %s\n", pStr);
    }
}

static void cbReqTO(char *pStr) {   // Sets and reads the Tone frequency.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "TO0;");
    }
    else
    {
        printf("TO: %s\n", pStr);
    }
}

static void cbReqTS(char *pStr) {   // Sets and reads the TF-Set status
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "TS0;");
    }
    else
    {
        printf("TS: %s\n", pStr);
    }
}

static void cbReqTX(char *pStr) {   // Sets the transmission mode.
    char ch = '0';
    if(pStr != NULL)
        ch = *pStr;
    parm_tx = 1;
    snprintf(_str,sizeof(_str), "TX%c;", ch);
    printf("TX: %c\n", ch);
}

static void cbReqVD(char *pStr) {   // Sets and reads the VOX Delay time
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "VD0600;");
    }
    else
    {
        printf("VD: %s\n", pStr);
    }
}

static void cbReqVG(char *pStr) {   // Sets and reads the VOX Gain
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "VG005;");
    }
    else
    {
        printf("VG: %s\n", pStr);
    }
}

static void cbReqVV(char *pStr) {   // Performs the VFO copy (A=B) function.
   parm_freqB = parm_freqA;
   printf("VV\n");
}

static void cbReqVX(char *pStr) {   // Sets and reads the VOX and Break-in function status.
    if(pStr == NULL)
    {
        snprintf(_str,sizeof(_str), "VX0;");
    }
    else
    {
        printf("VX: %s\n", pStr);
    }
}


int commcat_config(CONFIG_t * pCfg)
{
    return 0;
}

int commcat_req(char *pReq, char *pResp)
{
    int i = 0;
    COMMRQ_CMD_t * cmdlist;
    cmdlist = &_commCmd[i++];
    
    _str[0] = 0;

    //printf("CR: '%s' ", pReq);

    while(cmdlist->command != NULL){
        if(memcmp(cmdlist->command, pReq, 2) == 0) {
            char *pParm;
            if(strlen(pReq) > 2)
                pParm = &pReq[2];
            else
                pParm = NULL;

            cmdlist->cb(pParm);
            
            size_t len = 0;
            if(_str[0] != 0)
            {
                len = strlen(_str);
                // Send Answer
                strcpy(pResp, _str);
                //printf("CA: '%s'\n", _str);
            }
            return len;
        }

        cmdlist  = &_commCmd[i++];
    }


    printf("C UNK: '%s'\n", pReq);
    return 0;
}
