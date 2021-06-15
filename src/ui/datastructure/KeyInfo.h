#ifndef CTUNE_UI_DATASTRUCTURE_KEYINFO_H
#define CTUNE_UI_DATASTRUCTURE_KEYINFO_H

#include "../enum/TextID.h"
#include "../enum/ActionID.h"
#include "../enum/ContextID.h"

typedef struct ctune_UI_KeyInfo {
    ctune_UI_Context_e  ctx_id;
    ctune_UI_ActionID_e action_id;
    ctune_UI_TextID_e   description_id;

} ctune_UI_KeyInfo_t;

#endif //CTUNE_UI_DATASTRUCTURE_KEYINFO_H
