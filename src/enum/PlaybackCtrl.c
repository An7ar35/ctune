#include "PlaybackCtrl.h"

/**
 * Checks state mask for ON status
 * @param state Playback Control state
 * @return 'ON' state
 */
static bool ctune_PlaybackCtrl_isOn( ctune_PlaybackCtrl_e state ) {
    return ( state & CTUNE_PLAYBACK_CTRL_ACTIVE_MASK );
}

/**
 * Checks state mask for RECORDING status
 * @param state Playback Control state
 * @return 'RECORDING' state
 */
static bool ctune_PlaybackCtrl_isRecording( ctune_PlaybackCtrl_e state ) {
    return ( state & CTUNE_PLAYBACK_CTRL_REC );
}

/**
 * Checks state mask for OFF status
 * @param state Playback Control state
 * @return 'OFF' state
 */
static bool ctune_PlaybackCtrl_isOff( ctune_PlaybackCtrl_e state ) {
    return !( ctune_PlaybackCtrl_isOn( state ) );
}

/**
 * Gets the string representation of the enum
 * @param state Player state enum
 * @return String representation
 */
static const char * ctune_PlaybackCtrl_str( enum CTUNE_PLAYBACK_CTRL state ) {
    switch( state ) {
        case CTUNE_PLAYBACK_CTRL_OFF            : return "OFF";
        case CTUNE_PLAYBACK_CTRL_PLAY           : return "PLAY";
        case CTUNE_PLAYBACK_CTRL_REC            : return "RECORD";
        case CTUNE_PLAYBACK_CTRL_ACTIVE_MASK    : return "ACTIVE_MASK";
        case CTUNE_PLAYBACK_CTRL_SWITCH_PLAY_REQ: return "SWITCH_PLAY_REQ";
        case CTUNE_PLAYBACK_CTRL_SWITCH_REC_REQ : return "SWITCH_REC_REQ";
        case CTUNE_PLAYBACK_CTRL_STATE_REQ      : return "STATE_REQ";
        default                                 : return "unknown";
    }
}

/**
 * Namespace declaration
 */
const struct ctune_PlayerState_Namespace ctune_PlaybackCtrl = {
    .isOn        = &ctune_PlaybackCtrl_isOn,
    .isRecording = &ctune_PlaybackCtrl_isRecording,
    .isOff       = &ctune_PlaybackCtrl_isOff,
    .str         = &ctune_PlaybackCtrl_str,
};