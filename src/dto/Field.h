#ifndef CTUNE_DTO_FIELD_H
#define CTUNE_DTO_FIELD_H

typedef enum ctune_FieldType {
    CTUNE_FIELD_UNKNOWN = 0,
    CTUNE_FIELD_BOOLEAN,
    CTUNE_FIELD_SIGNED_LONG,
    CTUNE_FIELD_UNSIGNED_LONG,
    CTUNE_FIELD_DOUBLE,
    CTUNE_FIELD_CHAR_PTR,
    CTUNE_FIELD_STRING,
    CTUNE_FIELD_STRLIST,
    CTUNE_FIELD_ENUM_STATIONSRC,
} ctune_FieldType_e;

typedef struct ctune_Field {
    void *            _field;
    ctune_FieldType_e _type;
} ctune_Field_t;

#endif //CTUNE_DTO_FIELD_H
