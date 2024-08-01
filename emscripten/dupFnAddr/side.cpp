#include <string>
#include <stdio.h>
#include <emscripten.h>


#include "inc.h"

extern "C" {
    void EMSCRIPTEN_KEEPALIVE world(VOID_FNPTR ptr) {
        if (ptr == fnA) 
        {
            fnA();
        }
        else if (ptr == fnB)
        {
            fnB();
        } 
        else {
            printf("fn addr %p does not match any: %p or %p\n", ptr, fnA, fnB);
            ptr();
        }
     }
}