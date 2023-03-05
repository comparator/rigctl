#ifndef RIGCTL_H
#define RIGCTL_H

/**
 * \brief Hamlib error codes
 * Error code definition that can be returned by the Hamlib functions.
 * Unless stated otherwise, Hamlib functions return the negative value
 * of rig_errcode_e definitions in case of error, or 0 when successful.
 */
 /**
  * @{ \name Hamlib error codes
  */

enum rig_errcode_e {
    RIG_OK = 0,     /*!< 0 No error, operation completed successfully */
    RIG_EINVAL,     /*!< 1 invalid parameter */
    RIG_ECONF,      /*!< 2 invalid configuration (serial,..) */
    RIG_ENOMEM,     /*!< 3 memory shortage */
    RIG_ENIMPL,     /*!< 4 function not implemented, but will be */
    RIG_ETIMEOUT,   /*!< 5 communication timed out */
    RIG_EIO,        /*!< 6 IO error, including open failed */
    RIG_EINTERNAL,  /*!< 7 Internal Hamlib error, huh! */
    RIG_EPROTO,     /*!< 8 Protocol error */
    RIG_ERJCTED,    /*!< 9 Command rejected by the rig */
    RIG_ETRUNC,     /*!< 10 Command performed, but arg truncated */
    RIG_ENAVAIL,    /*!< 11 Function not available */
    RIG_ENTARGET,   /*!< 12 VFO not targetable */
    RIG_BUSERROR,   /*!< 13 Error talking on the bus */
    RIG_BUSBUSY,    /*!< 14 Collision on the bus */
    RIG_EARG,       /*!< 15 NULL RIG handle or any invalid pointer parameter in get arg */
    RIG_EVFO,       /*!< 16 Invalid VFO */
    RIG_EDOM,       /*!< 17 Argument out of domain of func */
    RIG_EDEPRECATED,/*!< 18 Function deprecated */
    RIG_ESECURITY,  /*!< 19 Security error */
    RIG_EPOWER,     /*!, 20 Rig not powered on */
    RIG_EEND        // MUST BE LAST ITEM IN LAST
};



int rigctl_req(peer_t * peer);

#endif /* RIGCTL_H */
