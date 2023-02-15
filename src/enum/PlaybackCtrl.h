#ifndef CTUNE_ENUM_PLAYBACKCTRL_H
#define CTUNE_ENUM_PLAYBACKCTRL_H

#include <stdbool.h>

typedef enum CTUNE_PLAYBACK_CTRL {
    CTUNE_PLAYBACK_CTRL_OFF             = 0b00000000, //  0
    CTUNE_PLAYBACK_CTRL_PLAY            = 0b00000001, //  1
    CTUNE_PLAYBACK_CTRL_REC             = 0b00000010, //  2
    CTUNE_PLAYBACK_CTRL_ACTIVE_MASK     = 0b00001111, // 15
    CTUNE_PLAYBACK_CTRL_SWITCH_PLAY_REQ = 0b00010000, // 16,
    CTUNE_PLAYBACK_CTRL_SWITCH_REC_REQ  = 0b00100000, // 32,
    CTUNE_PLAYBACK_CTRL_STATE_REQ       = 0b10000000, //128,
} ctune_PlaybackCtrl_e;

/**
 * PlayerState namespace
 */
extern const struct ctune_PlayerState_Namespace {
    /**
     * Checks state mask for ON status
     * @param state Playback Control state
     * @return 'ON' state
     */
    bool (* isOn)( ctune_PlaybackCtrl_e state );

    /**
     * Checks state mask for RECORDING status
     * @param state Playback Control state
     * @return 'RECORDING' state
     */
    bool (* isRecording)( ctune_PlaybackCtrl_e state );

    /**
     * Checks state mask for OFF status
     * @param state Playback Control state
     * @return 'OFF' state
     */
    bool (* isOff)( ctune_PlaybackCtrl_e state );

    /**
     * Gets the string representation of the enum
     * @param state Player ctrl state enum
     * @return String representation
     */
    const char * (* str)( ctune_PlaybackCtrl_e state );

} ctune_PlaybackCtrl;

#endif //CTUNE_PLAYBACKCTRL_H
