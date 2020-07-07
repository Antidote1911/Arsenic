#ifndef MESSAGES_H
#define MESSAGES_H

#include <QString>
#include "libexport.h"

enum : quint32 {
    CRYPT_SUCCESS,
    DECRYPT_SUCCESS,
    DECRYPT_FAIL,
    NOT_AN_ARSENIC_FILE,
    SRC_NOT_FOUND,
    SRC_CANNOT_OPEN_READ,
    DES_FILE_EXISTS,
    DES_CANNOT_OPEN_WRITE,
    SRC_HEADER_READ_ERROR,
    ABORTED_BY_USER,
    INVALID_CRYPTOBOX_IMPUT,
    BAD_CRYPTOBOX_VERSION,
    BAD_CRYPTOBOX_PEM_HEADER,
    EMPTY_PASSWORD
};

QString LIB_EXPORT errorCodeToString(quint32 error_code);

#endif // MESSAGES_H