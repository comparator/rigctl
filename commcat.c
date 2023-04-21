#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "log.h"

#include "ermak.h"
#include "commcat.h"

#define COMMRQ(c, x)        {.command = c, .cb = x}

#define BUF_TX_LEN      80

#define SEND_REQUEST_TO_ERMAK(x)    ermak_SendRequest(&x)


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
static int          parm_pwr = 100;
static char         parm_menu = 0;
static int          parm_step = 50;

static ERMAK_RX_t	m_rxActive		= ERMAK_RX_MAIN;
static ERMAK_MSG_t	msg;


static uint8_t cvtModeE2K(ERMAK_MODE_t mode);
static ERMAK_MODE_t cvtModeK2E(int mode);

static uint16_t commcat_rd_menu(uint8_t idx);

static inline void requestFullInfo(void);


static void cbReqAI(char *pStr);    // Sets or reads the Auto Information (AI) function ON/ OFF
static void cbReqAG0(char *pStr);   // Sets or reads the AF gain
static void cbReqFA(char *pStr);    // + Sets or reads the VFO A frequency
static void cbReqFB(char *pStr);    // + Sets or reads the VFO B frequency
static void cbReqFR(char *pStr);    // + Selects or reads the VFO or Memory channel
static void cbReqFT(char *pStr);    // + Selects or reads the VFO or Memory channel
// GT
static void cbReqID(char *pStr);    // Reads the transceiver ID number
static void cbReqIF(char *pStr);    // + Reads the transceiver status
static void cbReqKS(char *pStr);    // Sets and reads the Keying speed
// KY
static void cbReqMD(char *pStr);    // + Sets and reads the operating mode status
static void cbReqNB(char *pStr);    // Sets and reads the Noise Blanker function status
static void cbReqPC(char *pStr);    // Sets and reads the output power
// PT
// RC
static void cbReqRD(char *pStr);    // + Sets and reads the RIT/XIT frequency Down
static void cbReqRT(char *pStr);	// + Sets and reads the RIT function status.
static void cbReqRU(char *pStr);    // + Sets and reads the RIT/XIT frequency Up
static void cbReqRX(char *pStr);    // Sets the receiver function status
// SH
// SL
static void cbReqSM(char *pStr);    // Reads the S-meter value
static void cbReqTX(char *pStr);    // Sets the transmission mode
static void cbReqXT(char *pStr);	// + Sets and reads the XIT function status.


/* Flex Coomands */ 
static void cbReqZZAG(char *pStr);  // Sets or reads VFO A Audio Gain
static void cbReqZZFA(char *pStr);  // Sets or reads VFO A Frequency
static void cbReqZZFB(char *pStr);  // Sets or reads VFO B Frequency
/*
2023-04-02 11:20:30 WARN  commcat.c:1059: C UNK: 'ZZMU'
2023-04-02 11:20:30 WARN  commcat.c:1059: C UNK: 'ZZSW'
2023-04-02 11:20:30 WARN  commcat.c:1059: C UNK: 'ZZIF'
*/


/*
static void cbReq00(char *pStr);    // No Info
static void cbReqAC(char *pStr);    // Sets or reads the internal antenna tuner status.
static void cbReqAN(char *pStr);    // Selects the antenna connector ANT1/ ANT2
static void cbReqBC(char *pStr);    // Sets or reads the Beat Cancel function status
static void cbReqBP(char *pStr);    // Adjusts the Notch Frequency of the Manual Notch Filter.
static void cbReqBY(char *pStr);    // Reads the busy signal status - squelch closed / open
static void cbReqCA(char *pStr);    // Sets and reads the CW TUNE function status
static void cbReqCH(char *pStr);    // Operate the MULTI/CH encoder
static void cbReqCN(char *pStr);    // Sets and reads the CTCSS frequency
// CR ??
static void cbReqCT(char *pStr);    // Sets and reads the CTCSS function status
static void cbReqDA(char *pStr);    // Sets and reads the DATA mode
// ES Sets or reads the Advanced startup option, (supported from the firmware version 1.08
static void cbReqEX(char *pStr);    // Sets or reads the Menu.
static void cbReqFL(char *pStr);    // Sets and reads the IF filter
static void cbReqFS(char *pStr);    // Sets and reads the Fine Tuning function status
static void cbReqFV(char *pStr);    // Verifies the Firmware version
static void cbReqFW(char *pStr);    // Sets or reads the DSP filtering bandwidth.
static void cbReqGC(char *pStr);    // Sets or reads the AGC
static void cbReqIS(char *pStr);    // Sets and reads the DSP Filter Shift
static void cbReqLK(char *pStr);    // Sets and reads the Lock status
static void cbReqLM(char *pStr);    // Sets and reads the VGS-1 electric keyer recording status.
static void cbReqMC(char *pStr);    // Sets and reads the Memory Channel number
// ME ?
static void cbReqMF(char *pStr);    // Sets and reads Menu A or B
static void cbReqMG(char *pStr);    // Sets and reads the microphone gain
static void cbReqNL(char *pStr);    // Sets and reads the Noise Blanker level
static void cbReqNR(char *pStr);    // Sets and reads the Noise Reduction function status
static void cbReqNT(char *pStr);    // Sets and reads the Notch Filter status
static void cbReqPA(char *pStr);    // Sets and reads the Pre-amplifier function status
static void cbReqPB(char *pStr);    // Sets and reads the voice and CW message playback status
static void cbReqPR(char *pStr);    // Sets and reads the Speech Processor function ON/ OFF
static void cbReqPS(char *pStr);    // Sets and reads the Power ON/ OFF status
static void cbReqQR(char *pStr);    // Sets and reads the Quick Memory channel data
static void cbReqRA(char *pStr);    // Sets and reads the RF Attenuator status
static void cbReqRG(char *pStr);    // Sets and reads the RF Gain status
static void cbReqRM(char *pStr);    // Sets and reads the Meter function
// RS ?
static void cbReqSC(char *pStr);    // Sets and reads the Scan function status
static void cbReqSD(char *pStr);    // Sets and reads the CW break-in time delay.
static void cbReqSP(char *pStr);    // Sets and reads the split operation frequency, from the firmware version 2.00
static void cbReqSQ(char *pStr);    // Sets and reads the squelch value
static void cbReqTN(char *pStr);    // Sets and reads the Tone frequency
static void cbReqTO(char *pStr);    // Sets and reads the Tone frequency
static void cbReqTS(char *pStr);    // Sets and reads the TF-Set status
static void cbReqTY(char *pStr);    // Get radio type
static void cbReqVD(char *pStr);    // Sets and reads the VOX Delay time
static void cbReqVG(char *pStr);    // Sets and reads the VOX Gain
static void cbReqVR(char *pStr);    // Sets and reads the VOX Gain, from the firmware version 2.00.
static void cbReqVV(char *pStr);    // Performs the VFO copy (A=B) function
static void cbReqVX(char *pStr);    // Sets and reads the VOX and Break-in function status
static void cbReqXI(char *pStr);	// + Reads the transmit frequency and mode
static void cbReqXO(char *pStr);    // XO Sets and reads the offset direction and frequency for the transverter mode.
*/


static COMMRQ_CMD_t _commCmd[] = {
    /* Kenwood */
    COMMRQ("AI", cbReqAI),
    COMMRQ("AG0", cbReqAG0),
	COMMRQ("FA", cbReqFA),
	COMMRQ("FB", cbReqFB),
	COMMRQ("FR", cbReqFR),
	COMMRQ("FT", cbReqFT),
    COMMRQ("ID", cbReqID),
	COMMRQ("IF", cbReqIF),
    COMMRQ("KS", cbReqKS),
	COMMRQ("MD", cbReqMD),
    COMMRQ("NB", cbReqNB),
    COMMRQ("PC", cbReqPC),
	COMMRQ("RD", cbReqRD),
	COMMRQ("RT", cbReqRT),
	COMMRQ("RU", cbReqRU),
    COMMRQ("RX", cbReqRX),
    COMMRQ("SM", cbReqSM),
    COMMRQ("TX", cbReqTX),
	COMMRQ("XT", cbReqXT),

    /* Flex */
    COMMRQ("AG",   cbReqAG0),
    COMMRQ("ZZAG", cbReqZZAG),
	COMMRQ("ZZFA", cbReqZZFA),
	COMMRQ("ZZFB", cbReqZZFB),
/*
    COMMRQ("00", cbReq00),
    COMMRQ("AC", cbReqAC),
    COMMRQ("AN", cbReqAN),
    COMMRQ("BC", cbReqBC),
    COMMRQ("BP", cbReqBP),
    COMMRQ("BY", cbReqBY),
    COMMRQ("CA", cbReqCA),
	COMMRQ("CH", cbReqCH),
    COMMRQ("CN", cbReqCN),
    COMMRQ("CT", cbReqCT),
    COMMRQ("DA", cbReqDA),
    COMMRQ("EX", cbReqEX),
    COMMRQ("FL", cbReqFL),
    COMMRQ("FS", cbReqFS),
    COMMRQ("FV", cbReqFV),
    COMMRQ("FW", cbReqFW),
    COMMRQ("GC", cbReqGC),
    COMMRQ("IS", cbReqIS),
    COMMRQ("LK", cbReqLK),
    COMMRQ("LM", cbReqLM),
    COMMRQ("MC", cbReqMC),
    COMMRQ("MF", cbReqMF),
    COMMRQ("MG", cbReqMG),
    COMMRQ("NL", cbReqNL),
    COMMRQ("NR", cbReqNR),
    COMMRQ("NT", cbReqNT),
    COMMRQ("PA", cbReqPA),
    COMMRQ("PB", cbReqPB),
    COMMRQ("PR", cbReqPR),
    COMMRQ("PS", cbReqPS),
    COMMRQ("QR", cbReqQR),
    COMMRQ("RA", cbReqRA),
    COMMRQ("RG", cbReqRG),
    COMMRQ("RM", cbReqRM),
    COMMRQ("SC", cbReqSC),
    COMMRQ("SD", cbReqSD),
    COMMRQ("SP", cbReqSP),
    COMMRQ("SQ", cbReqSQ),
    COMMRQ("TN", cbReqTN),
    COMMRQ("TO", cbReqTO),
    COMMRQ("TS", cbReqTS),
    COMMRQ("TY", cbReqTY),
    COMMRQ("VD", cbReqVD),
    COMMRQ("VG", cbReqVG),
    COMMRQ("VR", cbReqVR),
    COMMRQ("VV", cbReqVV),
    COMMRQ("VX", cbReqVX),
	COMMRQ("XI", cbReqXI),
    COMMRQ("XO", cbReqXO),
*/
    COMMRQ(NULL, NULL)
};

/* Полнофункциональные */
/*
static void cbReqCH(char *pStr) {   // Operate the MULTI/CH encoder
	if (pStr == NULL) return;

	requestFullInfo();
	ERMAK_VFO_MODE_t vfoActive = msg.fullInfo.vfoRX;
	int32_t freq;

	if (vfoActive == ERMAK_VFO_MODE_A) {
		freq = msg.fullInfo.vfoFreq.vfoA;
	} else {
		freq = msg.fullInfo.vfoFreq.vfoB;
	}

	if (*pStr == '1') {
		freq -= parm_step;
		log_info("CH1, %d", freq);
	} else {
		freq += parm_step;
		log_info("CH0, %d", freq);
	}

	msg.command = ERMAK_COMMAND_SET_VFO;
	msg.vfoData.rx = m_rxActive;
	msg.vfoData.vfo = vfoActive;
	msg.vfoData.freq = freq;
	SEND_REQUEST_TO_ERMAK(msg);
}
*/
static void cbReqFA(char *pStr) {   // Sets or reads the VFO A frequency
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "FA%011d;", msg.fullInfo.vfoFreq.vfoA);
	} else {
		msg.command = ERMAK_COMMAND_SET_VFO;
		msg.vfoData.rx = m_rxActive;
		msg.vfoData.vfo = ERMAK_VFO_MODE_A;
		msg.vfoData.freq = atoi(pStr);
		log_info("FA: %d", msg.vfoData.freq);
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqZZFA(char *pStr) {   // Sets or reads the VFO A frequency
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "ZZFA%011d;", msg.fullInfo.vfoFreq.vfoA);
	} else {
		msg.command = ERMAK_COMMAND_SET_VFO;
		msg.vfoData.rx = m_rxActive;
		msg.vfoData.vfo = ERMAK_VFO_MODE_A;
		msg.vfoData.freq = atoi(pStr);
		log_info("ZZFA: %d", msg.vfoData.freq);
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqFB(char *pStr) {   // Sets or reads the VFO B frequency
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "FB%011d;", msg.fullInfo.vfoFreq.vfoB);
	} else {
		msg.command = ERMAK_COMMAND_SET_VFO;
		msg.vfoData.rx = m_rxActive;
		msg.vfoData.vfo = ERMAK_VFO_MODE_B;
		msg.vfoData.freq = atoi(pStr);
		log_info("FB: %d", msg.vfoData.freq);
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqZZFB(char *pStr) {   // Sets or reads the VFO B frequency
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "ZZFB%011d;", msg.fullInfo.vfoFreq.vfoB);
	} else {
		msg.command = ERMAK_COMMAND_SET_VFO;
		msg.vfoData.rx = m_rxActive;
		msg.vfoData.vfo = ERMAK_VFO_MODE_B;
		msg.vfoData.freq = atoi(pStr);
		log_info("ZZFB: %d", msg.vfoData.freq);
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqFR(char *pStr) {   // Selects or reads the VFO or Memory channel Simplex
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "FR%d;", msg.fullInfo.vfoRX);
	} else {
		msg.command = ERMAK_COMMAND_SWITCH_VFO;
		char ch = *pStr;
		if (ch == '0') {
			msg.vfoData.vfo = ERMAK_VFO_MODE_A;
		} else if (ch == '1') {
			msg.vfoData.vfo = ERMAK_VFO_MODE_B;
		} else return;

		log_info("FR: %c", ch);
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqFT(char *pStr) {   // Selects or reads the VFO or Memory channel Split
	// Проситься отдельная команда для Kenwood FT
	requestFullInfo();
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "FT%d;", msg.fullInfo.vfoTX);
	} else {
		msg.command = ERMAK_COMMAND_SET_SPLIT;
		ERMAK_VFO_MODE_t vfoTx = -1;
		char ch = *pStr;
		if (ch == '0') {
			vfoTx = ERMAK_VFO_MODE_A;
		} else if (ch == '1') {
			vfoTx = ERMAK_VFO_MODE_B;
		} else return;

		log_info("FT: %d", vfoTx);

		msg.kenwoodFT.split = (msg.fullInfo.vfoRX != vfoTx) ? ERMAK_SPLIT_ON : ERMAK_SPLIT_OFF;
		msg.kenwoodFT.vfoRx = vfoTx;
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqIF(char *pStr) {   // Reads the transceiver status
	requestFullInfo();

	char rit_ena = (msg.fullInfo.ritXit.rit == ERMAK_RIT_OFF) ? '0' : '1';
	char xit_ena = (msg.fullInfo.ritXit.xit == ERMAK_RIT_OFF) ? '0' : '1';
	char rit_dir;

	int32_t rit_freq;
	if ((xit_ena == '1') && (rit_ena == '0')) {
		rit_freq = msg.fullInfo.ritXit.xitFreq;
	} else {
		rit_freq = msg.fullInfo.ritXit.ritFreq;
	}

	if (rit_freq < 0) {
		rit_dir = '-';
		rit_freq = -rit_freq;
	} else {
		rit_dir = '+';
	}

	char vfo;
	if ((msg.fullInfo.split == ERMAK_SPLIT_OFF) || (msg.fullInfo.ptt == ERMAK_TRANSMIT_OFF)) {
		vfo = (msg.fullInfo.vfoRX == ERMAK_VFO_MODE_A) ? '0' : '1';
	} else {
		vfo = (msg.fullInfo.vfoTX == ERMAK_VFO_MODE_A) ? '0' : '1';
	}

	snprintf(_str,sizeof(_str), "IF%011d00000%c%04d%c%c000%c%d%c0%c0000;",
			msg.fullInfo.freq,
			rit_dir, rit_freq, rit_ena, xit_ena,
			((msg.fullInfo.ptt == ERMAK_TRANSMIT_OFF) ? '0' : '1'),
			cvtModeE2K(msg.fullInfo.mode),
			vfo,
			((msg.fullInfo.split == ERMAK_SPLIT_OFF) ? '0' : '1'));
}
/*
IF %011d       00000  %c %04d %c %c 000 %c %d %c 0 %c 0000;
.. ........... .....  .  .... 0  .  ... .  .  .  . .  ...0; RitOff
.. ........... .....  .  .... 1  .  ... .  .  .  . .  ...0; RitOn
.. ........... .....  .  .... .  0  ... .  .  .  . .  ...0; XitOff
.. ........... .....  .  .... .  1  ... .  .  .  . .  ...0; XitOn
.. ........... .....  .  .... .  .  ... 0  .  .  . .  ...0; Rx
.. ........... .....  .  .... .  .  ... 1  .  .  . .  ...0; Tx
.. ........... .....  .  .... .  .  ... .  1  .  . .  ...0; SSB_L
.. ........... .....  .  .... .  .  ... .  2  .  . .  ...0; SSB_U
.. ........... .....  .  .... .  .  ... .  3  .  . .  ...0; CW_U
.. ........... .....  .  .... .  .  ... .  4  .  . .  ...0; FM
.. ........... .....  .  .... .  .  ... .  5  .  . .  ...0; AM
.. ........... .....  .  .... .  .  ... .  6  .  . .  ...0; DIG_L
.. ........... .....  .  .... .  .  ... .  7  .  . .  ...0; CW_L
.. ........... .....  .  .... .  .  ... .  9  .  . .  ...0; DIG_U
.. ........... .....  .  .... .  .  ... .  .  0  . 0  ...0; VfoAA
.. ........... .....  .  .... .  .  ... .  .  1  . 0  ...0; VfoBB
.. ........... .....  .  .... .  .  ... 0  .  0  . 1  ...0; VfoAB
.. ........... .....  .  .... .  .  ... 0  .  1  . 1  ...0; VfoBA
.. ........... .....  .  .... .  .  ... 1  .  0  . 1  ...0; VfoBA
.. ........... .....  .  .... .  .  ... 1  .  1  . 1  ...0; VfoAB
.. ........... .....  .  .... .  .  ... .  .  .  . 1  ...0; SplitOn
.. ........... .....  .  .... .  .  ... .  .  .  . 0  ...0; SplitOff
.. ........... .....  .  .... .  .  NMM .  .  .  . .  ...0; Memory Chn
.. ........... .....  .  .... .  .  ... .  .  .  S .  ...0; Scan Status
.. ........... .....  .  .... .  .  ... .  .  .  . .  CTN0; CTCSS
*/

static void cbReqMD(char *pStr) {   // Sets and reads the operating mode status
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "MD%d;", cvtModeE2K(msg.fullInfo.mode));
	} else {
		msg.command = ERMAK_COMMAND_SET_MODE;
		int mode = atoi(pStr);
		log_info("MD: %d", mode);
		msg.freqModeData.mode = cvtModeK2E(mode);
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqRD(char *pStr) {	// Sets and reads the RIT/XIT frequency Down
	requestFullInfo();
	int32_t freq = msg.fullInfo.ritXit.ritFreq;
	int32_t step = 1;
	if ((pStr != NULL) && (strlen(pStr) == 5)) {			// ToDo Scan Speed
		step = atoi(pStr);
	}
	log_info("RD: %d", step);

	if ((freq - step) > -9999) {		// ToDo RIT min
		freq -= step;
		msg.command = ERMAK_COMMAND_SET_RIT_FREQ;
		msg.ritXit.ritFreq = freq;
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqRT(char *pStr) {	// Sets and reads the RIT function status.
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "RT%d;", (msg.fullInfo.ritXit.rit == ERMAK_RIT_OFF) ? 0 : 1);
	}
	else
	{
		ERMAK_RIT_t rit = ERMAK_RIT_OFF;
		if (*pStr == '1')
			rit = ERMAK_RIT_ON;

		log_info("RT: %d", rit);
		msg.command = ERMAK_COMMAND_SET_RIT;
		msg.ritXit.rit = rit;
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqRU(char *pStr) {	// Sets and reads the RIT/XIT frequency Down
	requestFullInfo();
	int32_t freq = msg.fullInfo.ritXit.ritFreq;
	int32_t step = 1;
	if ((pStr != NULL) && (strlen(pStr) == 5)) {					// ToDo Scan Speed
		step = atoi(pStr);
	}
	log_info("RU: %d", step);

	if ((freq + step) < 9999) {		// ToDo RIT max
		freq += step;
		msg.command = ERMAK_COMMAND_SET_RIT_FREQ;
		msg.ritXit.ritFreq = freq;
		SEND_REQUEST_TO_ERMAK(msg);
	}
}

static void cbReqRX(char *pStr) {   // Sets the receiver function status.
	msg.command = ERMAK_COMMAND_SET_TRANSMITT_RX;
	msg.transmittRx.rx = m_rxActive;
	msg.transmittRx.transmitt = ERMAK_TRANSMIT_OFF;
	SEND_REQUEST_TO_ERMAK(msg);
	snprintf(_str,sizeof(_str), "RX;");
	log_info("RX");
}

static void cbReqTX(char *pStr) {   // Sets the transmission mode.
	char ch = '0';
	if (pStr != NULL) ch = *pStr;
	snprintf(_str,sizeof(_str), "TX%c;", ch);
	log_info("TX: %c", ch);

	msg.command = ERMAK_COMMAND_SET_TRANSMITT_RX;
	msg.transmittRx.rx = m_rxActive;
	msg.transmittRx.transmitt = ERMAK_TRANSMIT_ON;
	SEND_REQUEST_TO_ERMAK(msg);
}

/*
static void cbReqVV(char *pStr) {   // Performs the VFO copy (A=B) function.
	// здесь проситься команда A=B/B=A
	requestFullInfo();
	uint32_t freq = msg.fullInfo.vfoFreq.vfoA;
	msg.command = ERMAK_COMMAND_SET_VFO;
	msg.vfoData.rx = m_rxActive;
	msg.vfoData.vfo = ERMAK_VFO_MODE_B;
	msg.vfoData.freq = freq;
	SEND_REQUEST_TO_ERMAK(msg);
	log_info("VV");
}

static void cbReqXI(char *pStr) {   // Reads the transmit frequency and mode
	requestFullInfo();
	uint32_t freq;
	if (msg.fullInfo.vfoTX == ERMAK_VFO_MODE_A)
		freq = msg.fullInfo.vfoFreq.vfoA;
	else
		freq = msg.fullInfo.vfoFreq.vfoB;

	char datamode = '0';
	snprintf(_str,sizeof(_str), "XI%011d%c00;", freq, datamode);
}
*/
static void cbReqXT(char *pStr) {	// Sets and reads the XIT function status.
	if (pStr == NULL) {
		requestFullInfo();
		snprintf(_str,sizeof(_str), "XT%d;", (msg.fullInfo.ritXit.xit == ERMAK_XIT_OFF) ? 0 : 1);
	}
	else
	{
		ERMAK_RIT_t xit = ERMAK_XIT_OFF;
		if (*pStr == '1')
			xit = ERMAK_XIT_ON;

		log_info("XT: %d", xit);
		msg.command = ERMAK_COMMAND_SET_XIT;
		msg.ritXit.xit = xit;
		SEND_REQUEST_TO_ERMAK(msg);
	}
}


/* Не связаны с базой */

static void cbReqAG0(char *pStr) {   // Sets or reads the AF gain
	if (pStr != NULL)
		parm_af = atoi(pStr);
		log_info("AG0: %d", parm_af);
	} else {
		snprintf(_str,sizeof(_str), "AG0%03d;", parm_af);
	}
}

static void  cbReqZZAG(char *pStr) {    // Sets or reads VFO A Audio Gain
	if (pStr != NULL)
		parm_af = atoi(pStr);
		log_info("ZZAG: %d", parm_af);
	} else {
		snprintf(_str,sizeof(_str), "ZZAG%03d;", parm_af);
	}
}

static void cbReqAI(char *pStr) {   // Sets or reads the Auto Information (AI) function ON/ OFF
	if (pStr != NULL) {
		parm_ai = *pStr - '0';
		log_info("AI: %d", parm_ai);
	}
	snprintf(_str,sizeof(_str), "AI%d;", parm_ai);
}

static void cbReqPC(char *pStr) {   // Sets and reads the output power
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "PC%03d;", parm_pwr);
	} else {
		int pwr = atoi(pStr);
		if (pwr < 5) pwr = 5;
		else if (pwr > 100) pwr = 100;
		parm_pwr = pwr;
		log_info("PC: %d", pwr);
	}
}

/* Заглушки */
static void cbReqID(char *pStr) {   // Reads the transceiver ID number
	snprintf(_str,sizeof(_str), "ID021;");  /* 021 - TS590S,     023 - TS590SG
                                               904 - Flex-6700,  905 - Flex-6500, 
                                               906 - Flex-6700R, 907 - Flex-6300 */
}

static void cbReqKS(char *pStr) {   // Sets and reads the Keying speed
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "KS025;");
	} else {
		log_info("KS: %s", pStr);
	}
}

static void cbReqNB(char *pStr) {   // Sets and reads the Noise Blanker function status.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "NB0;");
	} else {
		log_info("NB: %s", pStr);
	}
}

static void cbReqSM(char *pStr) {   // Reads the S-meter value
	snprintf(_str,sizeof(_str), "SM00010;");
}

/*
static void cbReqEX(char *pStr) {   // Sets or reads the Menu.
	if (pStr == NULL) return;

	size_t len = strlen(pStr);
	if (len < 7)
		return;

	uint8_t idx = (pStr[1] - '0') * 10 + (pStr[2] - '0');

	if (len == 7) {		// Request
		uint16_t resp = commcat_rd_menu(idx);
		snprintf(_str,sizeof(_str), "EX%03d0000%d;", resp);
	} else {
		log_info("EX Set: %d, %s", idx, &pStr[7]);
	}
}

static void cbReqGC(char *pStr) {   // Sets or reads the AGC.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "GC%d;", parm_agc);
	} else {
		char ch = *pStr;
		if (ch == '1' || ch == '3')  parm_agc = 1;
		else if (ch == '0') parm_agc = 0;
		else if (ch == '2') parm_agc = 2;
		log_info("GC: %d", parm_agc);
	}
}

static void cbReqMF(char *pStr) {   // Sets and reads Menu A or B
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "MF%d;", parm_menu);
	} else {
		char ch = *pStr;
		if ( ch == '0' ) parm_menu = 0;
		else if (ch == '1') parm_menu = 1;
		log_info("MF: %d", parm_menu);
	}
}

static void cbReqRG(char *pStr) {   // Sets and reads the RF Gain status.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "RG%03d;", parm_rf);
	} else {
		int rf = atoi(pStr);
		if (rf < 0) rf = 0;
		else if (rf > 255) rf = 255;
		parm_rf = rf;
		log_info("RG: %d", parm_rf);
	}
}

static void cbReqSQ(char *pStr) {   // Sets and reads the squelch value
	if (pStr != NULL) {
		int sql = atoi(pStr);
		if (sql < 0) sql = 0;
		else if (sql > 255) sql = 255;
		parm_sql = sql;
		log_info("SQ: %d", parm_sql);
	}
	snprintf(_str,sizeof(_str), "SQ0%03d;", parm_sql);
}
*/

/* Заглушки */
/*
static void cbReq00(char *pStr) {   // No Info
	snprintf(_str,sizeof(_str), "000;");
}

static void cbReqAC(char *pStr) {   // Sets or reads the internal antenna tuner status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "AC000;");
	} else {
		log_info("AC: %s", pStr);
	}
}

static void cbReqAN(char *pStr) {   // Selects the antenna connector ANT1/ ANT2
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "AN011;");
	} else {
		log_info("AN: %s", pStr);
	}
}

static void cbReqBC(char *pStr) {   // Sets or reads the Beat Cancel function status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "BC0;");
	} else {
		log_info("BC: %s", pStr);
	}
}

static void cbReqBP(char *pStr) {   // Adjusts the Notch Frequency of the Manual Notch Filter.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "BP080;");
	} else {
		log_info("BP: %s", pStr);
	}
}

static void cbReqBY(char *pStr) {   // Reads the busy signal status
	snprintf(_str,sizeof(_str), "BY10;");
}

static void cbReqCA(char *pStr) {   // Sets and reads the CW TUNE function status.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "CA0;");
	} else {
		log_info("CA: %s", pStr);
	}
}

static void cbReqCN(char *pStr) {   // Sets and reads the CTCSS frequency.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "CN00;");
	} else {
		log_info("CN: %s", pStr);
	}
}

static void cbReqCT(char *pStr) {   // Sets and reads the CTCSS function status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "CT0;");
	} else {
		log_info("CT: %s", pStr);
	}
}

static void cbReqDA(char *pStr) {   // Sets and reads the DATA mode
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "DA0;");
	} else {
		log_info("DA: %s", pStr);
	}
}

static void cbReqFL(char *pStr) {   // Sets and reads the IF filter
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "FL1;");
	} else {
		log_info("FL: %s", pStr);
	}
}

static void cbReqFS(char *pStr) {   // Sets and reads the Fine Tuning function status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "FS0;");
	} else {
		log_info("FS: %s", pStr);
	}
}

static void cbReqFV(char *pStr) {   // Verifies the Firmware version.
	snprintf(_str,sizeof(_str), "FV2.05;");		// Last 2.05
}

static void cbReqFW(char *pStr) {   // Sets or reads the DSP filtering bandwidth.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "FW2700;");
	} else {
		log_info("FW: %s", pStr);
	}
}


static void cbReqIS(char *pStr) {   // Sets and reads the DSP Filter Shift.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "IS 0800;");
	} else {
		log_info("IS: %s", pStr);
	}
}


static void cbReqLK(char *pStr) {   // Sets and reads the Lock status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "LK0;");
	} else {
		log_info("LK: %s", pStr);
	}
}

static void cbReqLM(char *pStr) {   // Sets and reads the VGS-1 electric keyer recording status.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "LM00000;");
	} else {
		log_info("LM: %s", pStr);
	}
}

static void cbReqMC(char *pStr) {   // Sets and reads the Memory Channel number
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "MC000;");
	} else {
		log_info("MC: %s", pStr);
	}
}

static void cbReqMG(char *pStr) {   // Sets and reads the microphone gain.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "MG050;");
	} else {
		log_info("MG: %s", pStr);
	}
}


static void cbReqNL(char *pStr) {   // Sets and reads the Noise Blanker level
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "NL005;");
	} else {
		log_info("NL: %s", pStr);
	}
}

static void cbReqNR(char *pStr) {   // Sets and reads the Noise Reduction function status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "NR0;");
	} else {
		log_info("NR: %s", pStr);
	}
}

static void cbReqNT(char *pStr) {   // Sets and reads the Notch Filter status.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "NT00;");
	} else {
		log_info("NT: %s", pStr);
	}
}

static void cbReqPA(char *pStr) {   // Sets and reads the Pre-amplifier function status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "PA00;");
	} else {
		log_info("PA: %s", pStr);
	}
}

static void cbReqPB(char *pStr) {   // Sets and reads the voice and CW message playback status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "PB0000;");
	} else {
		log_info("PB: %s", pStr);
	}
}

static void cbReqPR(char *pStr) {   // Sets and reads the Speech Processor function ON/ OFF.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "PR0;");
	} else {
		log_info("PR: %s", pStr);
	}
}

static void cbReqPS(char *pStr) {   // Sets and reads the Power ON/ OFF status
	if ((pStr == NULL) || (parm_ai != 0)) {
		snprintf(_str,sizeof(_str), "PS1;");
	}
}

static void cbReqQR(char *pStr) {   // Sets and reads the Quick Memory channel data
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "QR00;");
	} else {
		log_info("QR: %s", pStr);
	}
}

static void cbReqRA(char *pStr) {   // Sets and reads the RF Attenuator status.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "RA0000;");
	} else {
		log_info("RA: %s", pStr);
	}
}

static void cbReqRM(char *pStr) {   // Sets and reads the Meter function
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "RM00000;");
	} else {
		log_info("RM: %s", pStr);
	}
}

static void cbReqSC(char *pStr) {   // Sets and reads the Scan function status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "SC00;");
	} else {
		log_info("SC: %s", pStr);
	}
}

static void cbReqSD(char *pStr) {   // Sets and reads the CW break-in time delay.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "SD0250;");
	} else {
		log_info("SD: %s", pStr);
	}
}


static void cbReqSP(char *pStr) {   // Sets and reads the split operation frequency
	// from the firmware version 2.00
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "SP0;");
	} else {
		log_info("SP: %s", pStr);
	}
}

static void cbReqTN(char *pStr) {   // Sets and reads the Tone frequency.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "TN00;");
	} else {
		log_info("TN: %s", pStr);
	}
}

static void cbReqTO(char *pStr) {   // Sets and reads the Tone frequency.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "TO0;");
	} else {
		log_info("TO: %s", pStr);
	}
}

static void cbReqTS(char *pStr) {   // Sets and reads the TF-Set status
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "TS0;");
	} else {
		log_info("TS: %s", pStr);
	}
}

static void cbReqTY(char *pStr) {   // Get radio type
	snprintf(_str,sizeof(_str), "TYA6;");	// нет описания
}

static void cbReqVD(char *pStr) {   // Sets and reads the VOX Delay time
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "VD0600;");
	} else {
		log_info("VD: %s", pStr);
	}
}

static void cbReqVG(char *pStr) {   // Sets and reads the VOX Gain
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "VG005;");
	} else {
		log_info("VG: %s", pStr);
	}
}

static void cbReqVR(char *pStr) {   // Sets and reads the VOX Gain
	// supported from the firmware version 2.00.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "VR0;");
	} else {
		log_info("VR: %s", pStr);
	}
}

static void cbReqVX(char *pStr) {   // Sets and reads the VOX and Break-in function status.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "VX0;");
	} else {
		log_info("VX: %s", pStr);
	}
}

static void cbReqXO(char *pStr) {   // XO Sets and reads the offset direction and frequency for the transverter mode.
	if (pStr == NULL) {
		snprintf(_str,sizeof(_str), "XO000000000000;");
	} else {
		log_info("XO: %s", pStr);
	}
}
*/

/* Конверторы и прочее */

static uint8_t cvtModeE2K(ERMAK_MODE_t mode)
{
	switch(mode) {
		case ERMAK_MODE_LSB:
			return 1;
		case ERMAK_MODE_USB:
			return 2;
		case ERMAK_MODE_CWU:
			return 3;
		case ERMAK_MODE_FM:
			return 4;
		case ERMAK_MODE_AM:
			return 5;
		case ERMAK_MODE_FSK:	/* DIG_L */
			return 6;
		case ERMAK_MODE_CWL:
			return 7;
		case ERMAK_MODE_PKTUSB:	/* DIG_U */
			return 9;
		default:
			return 0;
	}
}

static ERMAK_MODE_t cvtModeK2E(int mode)
{
	switch(mode) {
		case 1:
			return ERMAK_MODE_LSB;
		case 2:
			return ERMAK_MODE_USB;
		case 3:
			return ERMAK_MODE_CWU;
		case 4:
			return ERMAK_MODE_FM;
		case 5:
			return ERMAK_MODE_AM;
		case 6:
			return ERMAK_MODE_FSK;
		case 7:
			return ERMAK_MODE_CWL;
		case 9:
			return ERMAK_MODE_PKTUSB;
		default:
			return -1;
	}
}

static uint16_t commcat_rd_menu(uint8_t idx)
{
	switch(idx) {
		case 14:	// MULTI/CH control step change for SSB/CW/FSK
			return 0;
		case 15:	// MULTI/CH control step change for AM
			return 0;
		case 16:	// MULTI/CH control step change for FM
			return 0;
		case 17:	// Maximum number of Quick Memory channels
			return 0;
		case 23:	// Auto mode change
			return 0;
		case 30:	// Transmit equalizer
			return 0;
		case 31:	// Receive equalizer
			return 0;
		case 48:	// Power fine
			return 0;
		case 50:	// Configuring the Transverter function and power down
			return 0;
		case 55:	// Constant recording
			return 0;
		case 63:	// DATA modulation line
			return 0;

		default:
			log_debug("EX Req Unknown IDX: %d", idx);
			break;
	}
	return 0;
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


/* API */

int commcat_req(char *pReq, char *pResp)
{
    int i = 0;
    COMMRQ_CMD_t * cmdlist;
    cmdlist = &_commCmd[i++];

    _str[0] = 0;

    log_trace("CR: '%s' ", pReq);

    size_t req_len = strlen(pReq);
    while(cmdlist->command != NULL){
        size_t cmd_len = strlen(cmdlist->command);

        if ((cmd_len <= req_len) && (strncmp(cmdlist->command, pReq, cmd_len) == 0)) {
            char *pParm;
            if (req_len > cmd_len)
                pParm = &pReq[cmd_len];
            else
                pParm = NULL;

            cmdlist->cb(pParm);

            size_t len = 0;
            if (_str[0] != 0)
            {
                len = strlen(_str);
                // Send Answer
                strcpy(pResp, _str);
                log_trace("CA: '%s'", _str);
            }
            return len;
        }

        cmdlist  = &_commCmd[i++];
    }


	snprintf(_str,sizeof(_str), "?;");
	strcpy(pResp, _str);

	log_warn("C UNK: '%s'", pReq);
	return strlen(_str);
}
