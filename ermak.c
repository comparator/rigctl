#include <stdlib.h>
#include <stdio.h>      /* Standard input/output definitions */
#include <string.h>     /* String function definitions */
#include <stdint.h>     /* Standard int types definitions */
#include <stdbool.h>    /* bool types definitions */

#include "ermak.h"

static ERMAK_LOCK_t     ermak_cfg_lock  = ERMAK_LOCK_OFF;
static ERMAK_VFO_MODE_t ermak_cfg_vfo   = ERMAK_VFO_MODE_A;
static ERMAK_VFO_MODE_t ermak_cfg_vfoS  = ERMAK_VFO_MODE_B;
static ERMAK_TRANSMIT_t ermag_cfg_ptt   = ERMAK_TRANSMIT_OFF;
static ERMAK_SPLIT_t    ermak_cfg_split = ERMAK_SPLIT_OFF;
static ERMAK_MODE_t     ermak_cfg_mode  = ERMAK_MODE_PKTUSB;

static uint32_t         ermak_cfg_freqA = 14074000;
static uint32_t         ermak_cfg_freqB = 21074000;

//static uint16_t         ermak_cfg_passband = 2700;
static uint16_t			ermak_cfg_bp_high = 2800;
static uint16_t			ermak_cfg_bp_low  = 100;

static bool ermak_chk_vfo(ERMAK_VFO_MODE_t * pVfo)
{
    if((*pVfo != ERMAK_VFO_MODE_A) && (*pVfo != ERMAK_VFO_MODE_B))
        *pVfo = ermak_cfg_vfo;

    return (*pVfo == ERMAK_VFO_MODE_B);
}

/*
typedef struct{
  ERMAK_RX_t          rx;
  ERMAK_STACK_MEM_t      stackMem;
	ERMAK_TRANSMIT_t		ptt;
  ERMAK_BAND_t        band;
  ERMAK_BAND_EDGE_t      edges;
	ERMAK_MODE_t			mode;
	uint32_t				freq;
	ERMAK_VFO_FREQS_t		vfoFreq;
	ERMAK_VFO_MODE_t		vfoMode;
	ERMAK_VFO_MODE_t		vfoRX;
	ERMAK_VFO_MODE_t		vfoTX;
	ERMAK_SPLIT_t			split;
  ERMAK_RIT_HIT_INFO_t     ritXitInfo;
  ERMAK_LNA_ATT_t        lnaAtt;
	ERMAK_RX_FILTER_SETTINGS_t	filter;
  int16_t            rfGain;
  int16_t            afGain;
  ERMAK_AF_MUTE_t        afMute;
  ERMAK_NOB_t          nob;
  ERMAK_NR_t          nor;
  ERMAK_ANF_t          anf;
  ERMAK_AGC_TYPE_t      agc;
  bool            agcOn;
	ERMAK_LOCK_t				lock;
  ERMAK_VOX_t          vox;
  ERMAK_ANT_t          rxAnt;
  ERMAK_ANT_t          txAnt;
}ERMAK_FULL_INFO_t;
*/
void ermak_SendRequest(ERMAK_MSG_t * pMsg)
{
	switch(pMsg->command)
	{
		case ERMAK_COMMAND_GET_FULL_INFO:
			pMsg->fullInfo.ptt			= ermag_cfg_ptt;
			pMsg->fullInfo.mode			= ermak_cfg_mode;
			pMsg->fullInfo.freq = (ermak_cfg_vfo == ERMAK_VFO_MODE_A) ? ermak_cfg_freqA : ermak_cfg_freqB;
			pMsg->fullInfo.vfoFreq.vfoA = ermak_cfg_freqA;
			pMsg->fullInfo.vfoFreq.vfoB = ermak_cfg_freqB;
			pMsg->fullInfo.vfoMode		= ermak_cfg_vfo;
			pMsg->fullInfo.vfoRX		= ermak_cfg_vfo;
			pMsg->fullInfo.vfoTX		= ermak_cfg_vfoS;
			pMsg->fullInfo.split		= ermak_cfg_split;
			pMsg->fullInfo.filter.bandPassHigh = ermak_cfg_bp_high;
			pMsg->fullInfo.filter.bandPassLow = ermak_cfg_bp_low;
			pMsg->fullInfo.lock			= ermak_cfg_lock;
			break;

		case ERMAK_COMMAND_SET_LOCK:
			if(pMsg->lockOn)
				ermak_cfg_lock = ERMAK_LOCK_ON;
			else
				ermak_cfg_lock = ERMAK_LOCK_OFF;
			break;

		case ERMAK_COMMAND_SWITCH_VFO:
			ermak_cfg_vfo = pMsg->vfoData.vfo;
			break;

		case ERMAK_COMMAND_SET_VFO:
			if(ermak_chk_vfo(&pMsg->vfoData.vfo))
				ermak_cfg_freqB = pMsg->vfoData.freq;
			else
				ermak_cfg_freqA = pMsg->vfoData.freq;
			break;

		case ERMAK_COMMAND_SET_MODE:
			ermak_cfg_mode = pMsg->freqModeData.mode;
			break;

		case ERMAK_COMMAND_SET_TRANSMITT_RX:
			// ToDo pMsg->transmittRx.rx - Main/Sub
			ermag_cfg_ptt = pMsg->transmittRx.transmitt;
			break;

		case ERMAK_COMMAND_SET_SPLIT:
			ermak_cfg_split = pMsg->kenwoodFT.split;
			ermak_cfg_vfoS = pMsg->kenwoodFT.vfoRx;
			break;
/*
        case ERMAK_COMMAND_GET_FREQ_MODE:
            if(ermak_cfg_vfo == ERMAK_VFO_MODE_A)
                pMsg->freqModeData.freq = ermak_cfg_freqA;
            else
                pMsg->freqModeData.freq = ermak_cfg_freqB;
            pMsg->freqModeData.mode = ermak_cfg_mode;
            pMsg->freqModeData.passband = ermak_cfg_passband;
            break;

        case ERMAK_COMMAND_GET_VFO:
            if(ermak_chk_vfo(&pMsg->vfoData.vfo))
                pMsg->vfoData.freq = ermak_cfg_freqB;
            else
                pMsg->vfoData.freq = ermak_cfg_freqA;
            break;

        case ERMAK_COMMAND_GET_TX_RX:
            pMsg->transmittRx.transmitt = ermag_cfg_ptt;
            break;



        case ERMAK_COMMAND_GET_SPLIT:
            pMsg->extdInfo.split = ermak_cfg_split;
            pMsg->extdInfo.mode = ermak_cfg_mode;      // ToDo mode for split ??
            pMsg->extdInfo.vfo = ermak_cfg_vfoS;
            if(ermak_cfg_vfoS == ERMAK_VFO_MODE_A)
            {
                pMsg->extdInfo.freq = ermak_cfg_freqA;
            }
            else
            {
                pMsg->extdInfo.freq = ermak_cfg_freqB;
            }
            break;



        case ERMAK_COMMAND_SET_SPLIT_FREQ:
            if(ermak_cfg_vfoS == ERMAK_VFO_MODE_A)
            {
                ermak_cfg_freqA = pMsg->extdInfo.freq;
            }
            else
            {
                ermak_cfg_freqB = pMsg->extdInfo.freq;
            }
            break;

        case ERMAK_COMMAND_GET_EXTD_INFO:
            if(ermak_chk_vfo(&pMsg->extdInfo.vfo))
                pMsg->extdInfo.freq = ermak_cfg_freqB;
            else
                pMsg->extdInfo.freq = ermak_cfg_freqA;

            pMsg->extdInfo.passband = ermak_cfg_passband;
            pMsg->extdInfo.split = ermak_cfg_split;
            pMsg->extdInfo.mode = ermak_cfg_mode;
            break;
*/
        default:
            break;
    }
}