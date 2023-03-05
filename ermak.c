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
static uint16_t         ermak_cfg_passband = 2700;

static bool ermak_chk_vfo(ERMAK_VFO_MODE_t * pVfo)
{
    if((*pVfo != ERMAK_VFO_MODE_A) && (*pVfo != ERMAK_VFO_MODE_B))
        *pVfo = ermak_cfg_vfo;

    return (*pVfo == ERMAK_VFO_MODE_B);
}


void ermak_SendRequest(ERMAK_MSG_t * pMsg)
{
    switch(pMsg->command)
    {
        case ERMAK_COMMAND_GET_LOCK:
            pMsg->lock = ermak_cfg_lock;
            break;

        case ERMAK_COMMAND_SET_LOCK:
            ermak_cfg_lock = pMsg->lock;
            break;

        case ERMAK_COMMAND_GET_FREQ_MODE:
            if(ermak_cfg_vfo == ERMAK_VFO_MODE_A)
                pMsg->freqModeData.freq = ermak_cfg_freqA;
            else
                pMsg->freqModeData.freq = ermak_cfg_freqB;
            pMsg->freqModeData.mode = ermak_cfg_mode;
            pMsg->freqModeData.passband = ermak_cfg_passband;
            break;

        case ERMAK_COMMAND_SET_MODE:
            ermak_cfg_mode = pMsg->freqModeData.mode;
            break;

        case ERMAK_COMMAND_SET_DDS:
             if(ermak_chk_vfo(&pMsg->vfoData.vfo))
                 ermak_cfg_freqB = pMsg->vfoData.freq;
             else
                 ermak_cfg_freqA = pMsg->vfoData.freq;
            break;

        case ERMAK_COMMAND_GET_VFO:
            if(ermak_chk_vfo(&pMsg->vfoData.vfo))
                pMsg->vfoData.freq = ermak_cfg_freqB;
            else
                pMsg->vfoData.freq = ermak_cfg_freqA;
            break;

        case ERMAK_COMMAND_SWITCH_VFO:
            ermak_cfg_vfo = pMsg->vfoData.vfo;
            break;

        case ERMAK_COMMAND_GET_TX_RX:
            pMsg->transmittRx.transmitt = ermag_cfg_ptt;
            break;

        case ERMAK_COMMAND_SET_TRANSMITT_RX:
            ermag_cfg_ptt = pMsg->transmittRx.transmitt;
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

        case ERMAK_COMMAND_SET_SPLIT:
            ermak_cfg_split = pMsg->extdInfo.split;
            ermak_cfg_vfoS = pMsg->extdInfo.vfo;
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

        default:
            break;
    }
}