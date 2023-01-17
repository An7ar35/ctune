#ifndef CTUNE_ENUM_FLAG_H
#define CTUNE_ENUM_FLAG_H

typedef enum {
    FLAG_SET_OFF   = 0,
    FLAG_SET_ON    = 1,
    FLAG_GET_VALUE = 2,
} ctune_Flag_e;


extern const struct ctune_Flag_Namespace {
    /**
     * Gets corresponding string for a Flag
     * @param flag Flag enum type
     * @return String
     */
    const char * (* str)( ctune_Flag_e flag );

} ctune_Flag;

#endif //CTUNE_ENUM_FLAG_H