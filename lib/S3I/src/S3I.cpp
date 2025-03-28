#include "S3I.h"

String S3I::CreateMessageIdentifier() {
    String ident = "s3i:";
    const char lookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    int i;
    int j;
    for (i = 0; i < 8; i++) {
        ident += lookup[random(0, 16)];
    }
    ident += "-";

    for (j = 0; j < 3; j++) {
        for (i = 0; i < 4; i++) {
            ident += lookup[random(0, 16)];
        }
        ident += "-";
    }

    for (i = 0; i < 12; i++) {
        ident += lookup[random(0, 16)];
    }

    return ident;
}