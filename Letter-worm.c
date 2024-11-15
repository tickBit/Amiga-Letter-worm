/*
   Letter worm game for Amiga a'la spaghetti.
   Works best on AmiKit and is designed for it particularly.
   
   Collect enough food to get letters A-Z, then you are free.

   Can be compiled at least with VBCC and DICE.
   
   Compiling with DICE: dcc -// -lm Letter-worm.c

 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <dos/dos.h>

#include <datatypes/pictureclass.h>

#include <intuition/intuition.h>
#include <intuition/icclass.h>

#include <clib/amigaguide_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

/*
    Below should be found from include files, but just in case...
*/

#define PDTA_DestMode		(DTA_Dummy + 251)
#define PMODE_V43 (1)	/* Extended mode */

#define START_SCREEN    0
#define GAME_OVER       1
#define GAME_WON        2
#define GAME_RUNNING    3
#define GAME_PAUSED     4

int state;

struct Library *IntuitionBase = NULL;
struct Library *DataTypesBase = NULL;
struct Library *UtilityBase = NULL;
struct Library *GfxBase = NULL;
struct BitMap *bitmap = NULL;

struct Font *font;

int length = 1;

int foodX = 100;
int foodY = 200;

struct Window *win = NULL;

// the letters of the worm
char *WormLetters[] = {
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z"
};

void startPrg()
{
	
    int winWidth, winHeight;
    struct RastPort rastport;

    struct IntuiMessage *Message; 

	UWORD wormX[26*8], wormY[26*8];
    UWORD wormDX = 2;
    UWORD wormDY = 0;

    int i;

    char Name[] = "gfx/AI-generated-640x640.jpg";

    UWORD fontWidth;
    UWORD fontHeight;
    UWORD fontBaseline;
    UWORD size = 0;

    struct Screen *scr = LockPubScreen("Workbench");

    Object *Item = NewDTObject (Name,
	    DTA_GroupID, GID_PICTURE,
	    PDTA_Remap, TRUE,
        PDTA_Screen, scr,
		PDTA_DestMode, PMODE_V43,
	    TAG_END);

		if (Item)
			{
			struct BitMapHeader *bmhd = NULL;
			struct BitMap *bm = NULL;

			DoMethod (Item, DTM_PROCLAYOUT, NULL, TRUE);

			GetDTAttrs (Item,
				PDTA_BitMapHeader, &bmhd,
				PDTA_DestBitMap, &bm,
				TAG_END);

			if (bmhd && bm)
				{
				win = OpenWindowTags (NULL,
					WA_Title, "Letter worm",
					WA_InnerWidth, bmhd->bmh_Width,
					WA_InnerHeight, bmhd->bmh_Height,
					WA_Flags, WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_ACTIVATE | WFLG_NOCAREREFRESH,
					WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_RAWKEY, TAG_DONE);

				if (win)
					{

			        ULONG MsgClass;
			        UWORD MsgCode;
			        BOOL Done;

                    Done = FALSE;

                    winWidth = bmhd->bmh_Width + win->BorderLeft + win->BorderRight;
                    winHeight = bmhd->bmh_Width + win->BorderBottom + win->BorderTop;
                    
                    bitmap = (struct BitMap *)AllocBitMap(bmhd->bmh_Width,bmhd->bmh_Height,bmhd->bmh_Depth,BMF_CLEAR | BMF_DISPLAYABLE,win->RPort->BitMap);
                    InitRastPort(&rastport);
                    rastport.BitMap = bitmap;
            
                    fontWidth = win->RPort->TxWidth;
                    fontHeight = win->RPort->TxHeight;
                    fontBaseline = win->RPort->TxBaseline;

                    state = START_SCREEN;
                    
                    /*
                    *  Main event loop
                    */
                    do
                    {

                        

                        while((Message = (struct IntuiMessage *)GetMsg(win->UserPort)) != NULL)
                        {
                            
                            MsgClass = Message->Class;
                            MsgCode = Message->Code;

                            switch(MsgClass)
                            {

                                case IDCMP_RAWKEY:

                                    switch(MsgCode) {
                                        
                                        // p key
                                        case 25:

                                            if (state == GAME_PAUSED) state = GAME_RUNNING; else state = GAME_PAUSED;
                                            break;

                                        // enter key
                                        case 68:
                                            
                                            
                                            if (state == GAME_PAUSED) {
                                                state = GAME_RUNNING;
                                                break;
                                            }

                                            if (state == GAME_RUNNING) break;
                                            
                                            if (state == GAME_OVER) {
                                                state = START_SCREEN;
                                                break;
                                            }

                                            if (state == GAME_WON) {
                                                state = START_SCREEN;
                                                break;
                                            }

                                            state = GAME_RUNNING;
                                            length = 1;
                                            foodX = rand() % 570 + 32;
                                            foodY = rand() % 550 + 64;

                                            for (i = 1; i < 26*8; i++) {
                                                wormX[i] = 26*8 + win->BorderLeft + 128;
                                                wormY[i] = win->BorderTop + 128;        
                                            }

                                            wormX[0] = win->BorderLeft + 128;
                                            wormY[0] = win->BorderTop + 128;

                                            break;

                                        // escape key
                                        case 27:
                                            
                                            Done = TRUE;
                                            break;

                                        // cursor left
                                        case 79:
                                            wormDX = -2;
                                            wormDY = 0;
                                            break;

                                        // cursor right
                                        case 78:
                                            wormDX = 2;
                                            wormDY = 0;
                                            break;

                                        // cursor up
                                        case 76:
                                            wormDY = -2;
                                            wormDX = 0;
                                            break;

                                        // cursor down
                                        case 77:
                                            wormDY = 2;
                                            wormDX = 0;
                                            break;


                                    }							
                                    break;
                                
                                case IDCMP_CLOSEWINDOW:

                                    Done = TRUE;
                                    break;
                            
                                
                            }

                            if (Message != NULL)
                                ReplyMsg((struct Message *)Message);
                        }

                        BltBitMapRastPort(bm, 0, 0, &rastport, 0, 0, bmhd->bmh_Width, bmhd->bmh_Height, 192);
                        
                        switch (state) {
                        
                            case GAME_RUNNING:
                        
                                for (i = length*8 - 1; i > 0; i--) {
                                    wormX[i] = wormX[i-1]; wormY[i] = wormY[i-1];
                                    if (i > 8) {
                                        if (abs(wormX[0] - wormX[i]) == 0 && abs(wormY[0] - wormY[i]) == 0) state = GAME_OVER;
                                    }
                                }
                        
                                SetAPen(&rastport, rand() % 255);
                                Move(&rastport, foodX, foodY);
                                DrawEllipse(&rastport, foodX, foodY, 8,8);


                                // set colors for letters
                                SetBPen(&rastport, 100);
                                SetAPen(&rastport, 128);

                                if (length > 26) length = 26;

                                for (i = 0; i < length*8; i++) {
                                    if (i % 8 == 0) {
                                        Move(&rastport, wormX[i], wormY[i]);
                                        Text(&rastport, WormLetters[i/8], 1);
                                    }
                                }

                                wormX[0] += wormDX;
                                wormY[0] += wormDY;
                        
                                if (wormX[0] < 2 || wormX[0] > winWidth - win->BorderRight - win->BorderLeft - fontWidth || wormY[0] < 2 || wormY[0] > win->Height - win->BorderBottom - fontHeight - win->BorderTop) {
                                    state = GAME_OVER;
                                    break;
                                }
                                
                                // check if food is collected
                                if (abs(foodX - (wormX[0] + fontWidth / 2)) < 9 + fontWidth / 2 && abs(foodY - (wormY[0] - fontHeight / 2)) < 9 + fontHeight / 2) {
                                    length++;
                                    foodX = rand() % 570 + 32;
                                    foodY = rand() % 550 + 64;
                                    if (length == 26) state = GAME_WON;
                                }
                        
                                break;

                            case START_SCREEN:

                                SetBPen(&rastport, 7);
                                SetAPen(&rastport, 134);
                                Move(&rastport, (winWidth - TextLength(&rastport, "Press return to start", 21)) / 2, win->BorderTop + 64);
                                Text(&rastport,"Press return to start", 21);

                                Move(&rastport, (winWidth - TextLength(&rastport, "Press p key to pause and return or p again to continue", 54)) / 2, win->BorderTop + 128);
                                Text(&rastport,"Press p key to pause and return or p again to continue", 54);


                                break;

                            case GAME_OVER:

                                SetBPen(&rastport, 96);
                                SetAPen(&rastport, 129);
                                Move(&rastport, (winWidth - TextLength(&rastport, "GAME OVER", 9)) / 2,160);
                                Text(&rastport, "GAME OVER", 9);

                                break;

                            case GAME_WON:

                                SetBPen(&rastport, 36);
                                SetAPen(&rastport, 72);
                                Move(&rastport, (winWidth - TextLength(&rastport, "YOU WON", 7)) / 2,160);
                                Text(&rastport, "YOU WON", 7);

                                Move(&rastport, (winWidth - TextLength(&rastport, "Perhaps you'll fly away as butterfly...", 39)) / 2,192);
                                Text(&rastport, "Perhaps you'll fly away as butterfly...", 39);

                                break;

                            case GAME_PAUSED:

                                SetBPen(&rastport, 101);
                                SetAPen(&rastport, 222);
                                Move(&rastport, (winWidth - TextLength(&rastport, "PAUSED", 6)) / 2,160);
                                Text(&rastport, "PAUSED", 6);

                                break;
                        }
                        
                        ClipBlit(&rastport, 0, 0, win->RPort, win->BorderLeft, win->BorderTop, bmhd->bmh_Width, bmhd->bmh_Height, 192);
                        WaitTOF();
                        
                        

                    } while(!Done);
                    }

                }
            
			

		}
        
        if (Item) DisposeDTObject(Item);
		cleanup();
	}


int main(int argc,char **argv)
{

	/* The program might need to ask for later versions of these libraries... */

   	if(IntuitionBase == NULL)
    {
        IntuitionBase = OpenLibrary("intuition.library",39);
        if(IntuitionBase == NULL)
            return -1;
    }

    if (DataTypesBase == NULL)
    {
        DataTypesBase = OpenLibrary("datatypes.library",39);
        if(DataTypesBase == NULL) {
            
            if (IntuitionBase != NULL) CloseLibrary(IntuitionBase);

            return -1;
        }
    }

    if (UtilityBase == NULL)
    {
        UtilityBase = OpenLibrary("utility.library",39);
        if(UtilityBase == NULL) {
            
            if (DataTypesBase != NULL) CloseLibrary(DataTypesBase);
            if (IntuitionBase != NULL) CloseLibrary(IntuitionBase);

            return -1;
        }
    }

	if (GfxBase == NULL) {
		GfxBase = OpenLibrary("graphics.library",39);
		if(GfxBase == NULL) {

            if (UtilityBase!= NULL) CloseLibrary(UtilityBase);
            if (DataTypesBase!= NULL) CloseLibrary(DataTypesBase);
            if (IntuitionBase!= NULL) CloseLibrary(IntuitionBase);

            return -1;
        }

	}

	startPrg();


	return 0;
}

int cleanup() {

    if (bitmap) FreeBitMap((struct BitMap *)bitmap);
    if (DataTypesBase) CloseLibrary(DataTypesBase);
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (UtilityBase) CloseLibrary(UtilityBase);
	if (GfxBase) CloseLibrary(GfxBase);

	if (win) CloseWindow((struct Window *)win);
	
	return;

}
