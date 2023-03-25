#ifndef ERMAK_H
#define ERMAK_H

/************************************************************
 * Definitions                            					*
 ************************************************************/

#define ERMAK_MAX_RX_FILTER_LIST				(6)

/************************************************************
 * Typedef                            						*
 ************************************************************/

typedef enum{
	ERMAK_NOTIFY_TYPE_WEB_ALL 		= 0,
	//ERMAK_NOTIFY_TYPE_TCI_ALL 		= 1,
	//ERMAK_NOTIFY_TYPE_BOTH_ALL 		= 2,

	//ERMAK_NOTIFY_TYPE_WEB_FREQ  	= 3,
	//ERMAK_NOTIFY_TYPE_TCI_FREQ 		= 4,
	//ERMAK_NOTIFY_TYPE_BOTH_FREQ 	= 5,

	//ERMAK_NOTIFY_TYPE_WEB_MODE  	= 6,
	//ERMAK_NOTIFY_TYPE_TCI_MODE  	= 7,
	//ERMAK_NOTIFY_TYPE_BOTH_MODE  	= 8,

	//ERMAK_NOTIFY_TYPE_WEB_VFO  		= 9,
	//ERMAK_NOTIFY_TYPE_TCI_VFO  		= 10,
	//ERMAK_NOTIFY_TYPE_BOTH_VFO  	= 11,

	ERMAK_NOTIFY_TYPE_WEB_RXTX  	= 12,
	//ERMAK_NOTIFY_TYPE_TCI_RXTX 		= 13,
	//ERMAK_NOTIFY_TYPE_BOTH_RXTX 	= 14,

	//ERMAK_NOTIFY_TYPE_WEB_NB 		= 15,
	//ERMAK_NOTIFY_TYPE_TCI_NB 		= 16,
	//ERMAK_NOTIFY_TYPE_BOTH_NB 		= 17,

	//ERMAK_NOTIFY_TYPE_WEB_NR 		= 18,
	//ERMAK_NOTIFY_TYPE_TCI_NR 		= 19,
	//ERMAK_NOTIFY_TYPE_BOTH_NR 		= 20,

	//ERMAK_NOTIFY_TYPE_WEB_ANF 		= 21,
	//ERMAK_NOTIFY_TYPE_TCI_ANF 		= 22,
	//ERMAK_NOTIFY_TYPE_BOTH_ANF 		= 23,

	//ERMAK_NOTIFY_TYPE_WEB_TX_POWER 	= 24,
	//ERMAK_NOTIFY_TYPE_TCI_TX_POWER 	= 25,
	//ERMAK_NOTIFY_TYPE_BOTH_TX_POWER = 26,

}ERMAK_NOTIFY_TYPE_t;

typedef enum{
	ERMAK_MODE_AM 	= 0x30, // 5
	ERMAK_MODE_USB 	= 0x31, // 2
	ERMAK_MODE_LSB 	= 0x32, // 1
	ERMAK_MODE_CWU 	= 0x33, // 3
	ERMAK_MODE_FM 	= 0x34, // 4
	ERMAK_MODE_CWL 	= 0x35, // 7
	ERMAK_MODE_FSK 	= 0x36,     // Legacy
	ERMAK_MODE_PKTUSB = 0x39   // 9
}ERMAK_MODE_t;



typedef enum{
	ERMAK_TRANSMIT_OFF = 0,
	ERMAK_TRANSMIT_ON = 1
}ERMAK_TRANSMIT_t;

typedef enum{
	ERMAK_SPLIT_OFF = 0,
	ERMAK_SPLIT_ON = 1
}ERMAK_SPLIT_t;

/* 	RX   */
typedef enum{
	ERMAK_RX_MAIN = 0,
	ERMAK_RX_SUB = 1,
}ERMAK_RX_t;

/* 	VFO   */
typedef enum{
	ERMAK_VFO_MODE_A = 0,
	ERMAK_VFO_MODE_B = 1
}ERMAK_VFO_MODE_t;

typedef enum{
    ERMAK_LOCK_OFF = 0,
    ERMAK_LOCK_ON = 1
}ERMAK_LOCK_t;


typedef enum{
	//ERMAK_COMMAND_GET_FREQ_MODE = 0,
	//ERMAK_COMMAND_SET_FREQ 		= 1,
	ERMAK_COMMAND_SET_MODE 		= 2,
	//ERMAK_COMMAND_SET_RX_FILTER = 3,
	//ERMAK_COMMAND_SET_TX_RX 	= 4,
	//ERMAK_COMMAND_SET_TX 		= 5,

	//ERMAK_COMMAND_CLR_TX 		= 6,
	//ERMAK_COMMAND_GET_TX_RX 	= 7,

	//ERMAK_COMMAND_SET_BAND 		= 8,
	//ERMAK_COMMAND_SET_AF_MUTE 	= 9,

	//ERMAK_COMMAND_SET_AF_GAIN 	= 10,
	//ERMAK_COMMAND_SET_RF_GAIN 	= 11,

	//ERMAK_COMMAND_GET_AF_GAIN 	= 12,
	//ERMAK_COMMAND_GET_RF_GAIN 	= 13,

	//ERMAK_COMMAND_SET_NB 		= 14,
	//ERMAK_COMMAND_SET_ANF 		= 15,
	//ERMAK_COMMAND_SET_NR 		= 16,

	//ERMAK_COMMAND_GET_NB 		= 17,
	//ERMAK_COMMAND_GET_ANF 		= 18,
	//ERMAK_COMMAND_GET_NR 		= 20,

	//ERMAK_COMMAND_SET_AGC 		= 21,
	//ERMAK_COMMAND_GET_AGC 		= 22,

	//ERMAK_COMMAND_SET_VAL_MODE 	= 23,
	//ERMAK_COMMAND_GET_VAL_MODE 	= 24,

	//ERMAK_COMMAND_SET_LNA_ATT 	= 25,
	//ERMAK_COMMAND_GET_LNA_ATT 	= 26,

	//ERMAK_COMMAND_SET_MUTE 		= 27,
	//ERMAK_COMMAND_GET_MUTE 		= 28,

	//ERMAK_COMMAND_SET_SPECTRUM_FREQ 	= 29,
	//ERMAK_COMMAND_GET_SPECTRUM_FREQ 	= 30,

	//ERMAK_COMMAND_SET_SPECTRUM_SPAN 	= 31,
	//ERMAK_COMMAND_GET_SPECTRUM_SPAN 	= 32,

	//ERMAK_COMMAND_SET_HETERODINE 		= 33,

	//ERMAK_COMMAND_ENTER_WEB_ACCESS  	= 34,
	//ERMAK_COMMAND_QUIT_WEB_ACCESS  		= 35,

	//ERMAK_COMMAND_WEB_CONNECTION_CHANGED  	= 36,

	//ERMAK_COMMAND_WEB_AUTH  				= 37,
	//ERMAK_COMMAND_GET_WEB_SETTINGS  		= 38,

	//ERMAK_COMMAND_GET_RX_FILTER				= 39,
	//ERMAK_COMMAND_GET_RX_FILTER_LIST		= 40,

	ERMAK_COMMAND_SET_VFO					= 41,
	//ERMAK_COMMAND_GET_VFO					= 42,

	//ERMAK_COMMAND_GET_MODE 					= 43,
	//ERMAK_COMMAND_SET_DDS					= 44,
	//ERMAK_COMMAND_GET_DDS					= 45,

	//ERMAK_COMMAND_SET_MODE_RX 				= 46,
	//ERMAK_COMMAND_GET_MODE_RX 				= 47,

	ERMAK_COMMAND_SET_TRANSMITT_RX 			= 48,
	//ERMAK_COMMAND_SUBCSRIBE					= 49,
	//ERMAK_COMMAND_NOTIFY					= 50,

	//ERMAK_COMMAND_CW_MACROS_STOP			= 51,
	//ERMAK_COMMAND_CW_MACROS_SEND			= 52,
	//ERMAK_COMMAND_SET_CW_MACROS_SPEED		= 53,
	//ERMAK_COMMAND_GET_CW_MACROS_SPEED		= 54,
	//ERMAK_COMMAND_CW_MACROS_SPEED_UP		= 55,
	//ERMAK_COMMAND_CW_MACROS_SPEED_DOWN		= 56,
	//ERMAK_COMMAND_GET_OPERATOR_CALLSIGN		= 57,
	//ERMAK_COMMAND_GET_CW_REMOTE				= 58,
	//ERMAK_COMMAND_SET_CW_REMOTE				= 59,
	//ERMAK_COMMAND_SET_CW_MACROS				= 60,
	//ERMAK_COMMAND_GET_CW_MACROS				= 61,
	//ERMAK_COMMAND_CW_MACROS_STR_SEND		= 62,
	//ERMAK_COMMAND_SET_TX_DRIVE				= 63,
	//ERMAK_COMMAND_GET_TX_DRIVE				= 64,
	//ERMAK_COMMAND_GET_VERSION				= 65,
	//ERMAK_COMMAND_GET_KENWOOD_IF			= 66,
	ERMAK_COMMAND_SWITCH_VFO				= 67,
	ERMAK_COMMAND_SET_SPLIT					= 68,
	ERMAK_COMMAND_NOTIFY_WEB_REMOTE			= 69,

	//ERMAK_COMMAND_NOTIFY_USB_BRIDGE_ON		= 70,
	//ERMAK_COMMAND_NOTIFY_USB_BRIDGE_OFF		= 71,

	//ERMAK_COMMAND_CARRIER_TX				= 72,

	//ERMAK_COMMAND_SERVER_MSG				= 73,
	//ERMAK_COMMAND_GET_VFO_FREQ				= 74,
	//ERMAK_COMMAND_UPDATE_VFO_FREQS			= 75,

    //ERMAK_COMMAND_GET_LOCK                  = 80,
	ERMAK_COMMAND_SET_LOCK                  = 81,
    //ERMAK_COMMAND_GET_SPLIT                 = 82,
    //ERMAK_COMMAND_GET_EXTD_INFO				= 83,
    //ERMAK_COMMAND_SET_SPLIT_FREQ            = 84,
	ERMAK_COMMAND_SET_RIT					= 85,
	ERMAK_COMMAND_SET_RIT_FREQ				= 86,

	//ERMAK_COMMAND_UNSUBCSRIBE				= 125,
	//ERMAK_COMMAND_SAVE_SETTINGS				= 126,
	//ERMAK_COMMAND_UNFREEZE_APP  = 127,
	//ERMAK_COMMAND_GET_BAND = 128

	ERMAK_COMMAND_GET_FULL_INFO			= 200,
	
	ERMAK_COMMAND_LAST_REC = 255
}ERMAK_COMMAND_t;


/* 	Freq + Mode  */
typedef struct{
	//uint32_t			freq;   // ToDo Type int64
    //int32_t             passband;
	
    //ERMAK_VFO_MODE_t	vfo;
	uint16_t			bw;
	ERMAK_MODE_t		mode;
}ERMAK_FREQ_MODE_DATA_t;

/* 	VFO data  */
typedef struct{
	uint32_t 			freq;
	ERMAK_RX_t			rx;
	ERMAK_VFO_MODE_t	vfo;
}ERMAK_VFO_DATA_t;

/* 	TRANSMITT RX   */
typedef struct{
	ERMAK_TRANSMIT_t transmitt;
	ERMAK_RX_t rx;
}ERMAK_TRANSMIT_RX_t;

/* Extended Info */
typedef struct{
    uint32_t 			freq;
    int32_t             passband;
    ERMAK_SPLIT_t 		split;
    ERMAK_MODE_t		mode;
    ERMAK_VFO_MODE_t	vfo;
}ERMAK_EXTD_INFO_t;

/* PRINTF("Stackmem:\t%d\n",((uint32_t)msg.fullInfo.stackMem) + 1);*/
typedef enum {
	ERMAK_STACK_MEM_1 = 0,
	ERMAK_STACK_MEM_2,
	ERMAK_STACK_MEM_3
}ERMAK_STACK_MEM_t;

/* 	Bands  */
typedef enum{
		ERMAK_BAND_160 = 0,
		ERMAK_BAND_80,
		ERMAK_BAND_40,
		ERMAK_BAND_30,
		ERMAK_BAND_20,
		ERMAK_BAND_16,
		ERMAK_BAND_15,
		ERMAK_BAND_12,
		ERMAK_BAND_10,
		ERMAK_BAND_ALL,
		ERMAK_BAND_6,
		ERMAK_BAND_60,
		ERMAK_BAND_CB,
		ERMAK_BAND_UNDEF

}ERMAK_BAND_t;

typedef struct {
	uint32_t low;
	uint32_t high;
}ERMAK_BAND_EDGE_t;


typedef struct{
	uint32_t vfoA;
	uint32_t vfoB;
}ERMAK_VFO_FREQS_t;

typedef enum {
	ERMAK_RIT_OFF = 0,
	ERMAK_RIT_ON = 1
}ERMAK_RIT_t;

typedef enum {
	ERMAK_XIT_OFF = 0,
	ERMAK_XIT_ON = 1
}ERMAK_XIT_t;

typedef struct{
	int32_t		ritFreq;
	ERMAK_RIT_t rit;
	int32_t		xitFreq;
	ERMAK_XIT_t xit;
}ERMAK_RIT_XIT_t;

/* 	LNA att  */
typedef enum{
	ERMAK_LNA_ATT_OFF = 0,
	ERMAK_LNA_ON,
	ERMAK_LNA_ATT_ON_10DB,
	ERMAK_LNA_ATT_ON_20DB,
	ERMAK_LNA_ATT_ON_30DB,
	ERMAK_LNA_ATT_NONE
}ERMAK_LNA_ATT_t;

typedef enum{
	ERMAK_RX_FILTER_NONE = 0,
	ERMAK_RX_FILTER_SSB_N,
	ERMAK_RX_FILTER_SSB_W,
	ERMAK_RX_FILTER_CW_N,
	ERMAK_RX_FILTER_CW_W,
	ERMAK_RX_FILTER_AMN,
	ERMAK_RX_FILTER_NFM,
	ERMAK_RX_FILTER_ADJ,
	ERMAK_RX_FILTER_CW_ADJ
}ERMAK_RX_FILTER_t;

typedef enum {
	ERMAK_APF_OFF = 0,
	ERMAK_APF_ON = 1
}ERMAK_APF_t;

/* 	RX filter   */
typedef struct{
	uint16_t			bandPassLow;
	uint16_t			bandPassHigh;
	ERMAK_RX_FILTER_t	filterType;
	ERMAK_APF_t			apf;
}ERMAK_RX_FILTER_SETTINGS_t;

/* 	Mute  */
typedef enum{
	ERMAK_AF_MUTE_OFF = 0,
	ERMAK_AF_MUTE_ON
}ERMAK_AF_MUTE_t;

/* 	NB */
typedef struct{
    uint32_t            run;
    float				tau;
    float               hangtime;
    float               advtime;
    float               backtau;
    float               threshold;
}ERMAK_NOB_t;

/* 	NR */
typedef struct{
    int32_t          run;
    uint32_t         nReduceValue;
    int32_t          VAD;
}ERMAK_NR_t;

typedef enum{
	ERMAK_ANF_OFF = 0,
	ERMAK_ANF_ON = 1
}ERMAK_ANF_t;

/* 	AGSW type */
typedef enum{
	ERMAK_AGC_TYPE_SLOW = 0,
	ERMAK_AGC_TYPE_MEDIUM,
	ERMAK_AGC_TYPE_FAST,
	ERMAK_AGC_TYPE_USER_1,
	ERMAK_AGC_TYPE_USER_2,
	ERMAK_AGC_TYPE_NONE
}ERMAK_AGC_TYPE_t;

typedef enum{
	ERMAK_VOX_OFF = 0,
	ERMAK_VOX_ON = 1
}ERMAK_VOX_t;

typedef enum{
	ERMAK_ANT_1 = 0,
	ERMAK_ANT_2 = 1
}ERMAK_ANT_t;

/* 	KENWOOD FT command  */
typedef struct{
	ERMAK_SPLIT_t 		split;
	ERMAK_VFO_MODE_t	vfoRx;
}ERMAK_KENWOOD_FT_t;

/* 	Command packet  */
/*
typedef struct {
  ERMAK_COMMAND_t command;
  union{
	uint8_t   array[128];

    ERMAK_LOCK_t                    lock;

	//ERMAK_BAND_SETTINGS_t  		band;
	

	//ERMAK_TRANSMIT_t		 		transmit;
	//ERMAK_AF_MUTE_t		 		mute;
	//ERMAK_VAL_MODE_t		 		valMode;
	//int8_t				 		audioGain;
	//uint8_t   			 		rfGainDb;
	//ERMAK_AGC_TYPE_t		 		agc;
	//ERMAK_NR_t			 		nr;
	//ERMAK_NOB_t			 		nb;
	//ERMAK_LNA_ATT_t		 		lnaAtt;
	//bool 					 		anfOn;
	//uint32_t				 		heterodyne;
	//ERMAK_SPAN_t			 		scopeSpan;
	//ERMAK_WEB_CONNECT_EV_t	 	remoteWebConnect;
	//char					 		remotePassword[8];
	//ERMAK_AUTH_t			 		authCode;
	//ERMAK_WEB_SETTINGS_t	 		webSettings;
	//ERMAK_RX_FILTER_SETTINGS_t		rxFilterSettings;
	//ERMAK_RX_FILTER_t				rxFilterList[ERMAK_MAX_RX_FILTER_LIST];


    ERMAK_EXTD_INFO_t               extdInfo;
	//ERMAK_VFO_FREQ_t				vfoFreq;
	//ERMAK_MODE_RX_t				modeRx;
	
	//ERMAK_NOTIFY_TCI_t			notifyTCI;

	//uint8_t						wpm;
	//uint8_t						wpmChange;
	//ERMAK_CW_MACROS_t				cwMacros;
	//char					 		operatorCallsign[ERMAK_CALLSIGN_SIZE];
	//ERMAK_CW_REMOTE_t				cwRemote;
	//ERMAK_CW_MACROS_EDIT_t		cwMacrosEdit;
	//ERMAK_CW_MACROS_STR_t			cwMacrosStr;
	//uint8_t						txPower;
	//ERMAK_VERSION_t				version;
			//ERMAK_KENWOOD_IF_t			kenwoodIF;
	//
	//ERMAK_SUBSCRIBE_t				subscribe;
	//bool							carrierOn;
	//ERMAK_SERVER_MSG_t			serverMsg;
	//ERMAK_VFO_FREQS_t				vfoFreqs;
};
}ERMAK_MSG_t;
*/

typedef struct{
	ERMAK_RX_t				rx;
	ERMAK_STACK_MEM_t		stackMem;
	ERMAK_TRANSMIT_t		ptt;
	ERMAK_BAND_t			band;
	ERMAK_BAND_EDGE_t		edges;
	ERMAK_MODE_t			mode;
	uint32_t				freq;
	ERMAK_VFO_FREQS_t		vfoFreq;
	ERMAK_VFO_MODE_t		vfoMode;
	ERMAK_VFO_MODE_t		vfoRX;
	ERMAK_VFO_MODE_t		vfoTX;
	ERMAK_SPLIT_t			split;
	ERMAK_RIT_XIT_t			ritXit;
	ERMAK_LNA_ATT_t			lnaAtt;
	ERMAK_RX_FILTER_SETTINGS_t	filter;
	int16_t					rfGain;
	int16_t					afGain;
	ERMAK_AF_MUTE_t			afMute;
	ERMAK_NOB_t				nob;
	ERMAK_NR_t				nor;
	ERMAK_ANF_t				anf;
	ERMAK_AGC_TYPE_t		agc;
	bool					agcOn;
	ERMAK_LOCK_t			lock;
	ERMAK_VOX_t				vox;
	ERMAK_ANT_t				rxAnt;
	ERMAK_ANT_t				txAnt;
}ERMAK_FULL_INFO_t;


typedef struct {
	ERMAK_COMMAND_t command;
	bool lockOn;
	union {
		uint8_t   array[128];

		ERMAK_FULL_INFO_t			fullInfo;
		ERMAK_VFO_DATA_t			vfoData;
		ERMAK_FREQ_MODE_DATA_t		freqModeData;
		ERMAK_TRANSMIT_RX_t			transmittRx;
		ERMAK_KENWOOD_FT_t			kenwoodFT;
		ERMAK_RIT_XIT_t				ritXit;

		ERMAK_NOTIFY_TYPE_t			notifyType;
	};
}ERMAK_MSG_t;

void ermak_SendRequest(ERMAK_MSG_t * pMsg);

#endif  // ERMAK_H
