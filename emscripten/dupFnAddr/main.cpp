#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>
#include <emscripten.h>

#include    "inc.h"

using namespace std;

struct cmdStruct
{
    const char* fnname;
    VOID_FNPTR fnptr;
};

static cmdStruct* gCmdDataP = nullptr;
static VOID_FNPTR gCurFnptr = nullptr;

void fnA() { printf(">in fnA> %s, %p, %p\n", ((gCurFnptr == fnA) ? "passed" : "failed:fnptr mismatch"), gCurFnptr, fnA); }
void fnB() { printf(">in fnB> %s, %p, %p\n", ((gCurFnptr == fnB) ? "passed" : "failed:fnptr mismatch"), gCurFnptr, fnB); }

static cmdStruct cmdsArray[] = { { "fnA", fnA }, { "fnB", fnB } };


int  main()
{
    gCmdDataP = cmdsArray;
    //
    gCurFnptr = fnA;
    gCurFnptr();
    gCurFnptr = fnB;
    gCurFnptr();
    printf("--\n");

    srand (time(NULL));
    for (int i=0; i < 5; i++) {
        int index = (rand() %2);
        cmdStruct& cc = gCmdDataP[index];

        printf("call %s", cc.fnname);
        gCurFnptr = cc.fnptr;
        gCurFnptr();
    }

    return 0;
}