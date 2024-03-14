/*
   Letter worm game for Amiga a'la spaghetti.
   Works best on AmiKit and is designed for it particularly.
   
   Collect enough food to get letters A-Z, then you are free.

   Can be compiled at least with GCC and DICE.
   Remeber to use -// switch with DICE, you'll get error code 5, but also the executable :-)

 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <exec/memory.h>
#include <dos/dos.h>

#include <datatypes/pictureclass.h>

#include <utility/tagitem.h>

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


#define PDTA_WhichPicture	(DTA_Dummy + 219)
#define PDTA_GetNumPictures	(DTA_Dummy + 220)

#define DTA_SourceAddress	(DTA_Dummy+39)

#define DTA_SourceSize		(DTA_Dummy+40)
#define	PDTA_ModeID		(DTA_Dummy + 200)
#define PDTA_SourceMode		(DTA_Dummy + 250)
#define PMODE_V43 (1)	/* Extended mode */

#define	DTST_MEMORY		5

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
struct BitMap *bgBitMap = NULL;

struct Font *font;

int length = 1;

int foodX = 100;
int foodY = 200;

struct Window *Window = NULL;

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
	
    int width, height, newWidth, newHeight;
    struct RastPort rastport;

    struct IntuiMessage *Message; 

	Object *Item = NULL;
	APTR memory = NULL;

	UWORD wormX[26*8], wormY[26*8];
    UWORD wormDX = 2;
    UWORD wormDY = 0;

    BPTR fileHandle;

    int i;

    char Name[] = "gfx/AI-generated-640x640.jpg";

    UWORD fontWidth;
    UWORD fontHeight;
    UWORD fontBaseline;
    UWORD size = 0;

	fileHandle = Open(Name,MODE_OLDFILE);
    
    if (fileHandle) {

			BOOL good = FALSE;

            Seek(fileHandle,0,OFFSET_END);
            size = Seek(fileHandle, 0, OFFSET_BEGINNING);
			if (size > 0)
			{
				memory = AllocVec(size,MEMF_ANY|MEMF_PUBLIC);
				if(memory != NULL)
				{
				    if (Read(fileHandle,memory,size) == size) good = TRUE;
                    Close(fileHandle);
				}
			}

			if(!good)
			{
                Close(fileHandle);
				FreeVec(memory);
				memory = NULL;

				size = 0;

                printf("File not recognized.\n");
				cleanup(memory, Window);
			}
		} else {
            printf("File not found!\n");
            cleanup(memory, Window);
        }

        Window = OpenWindowTags(NULL,
		WA_Title,			"Letter worm",
		WA_InnerWidth,		400,
		WA_InnerHeight,		400,
		WA_CloseGadget,		TRUE,
		WA_DepthGadget,		TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,		FALSE,
		WA_RMBTrap,			TRUE,
		WA_Activate,		TRUE,
        WA_SimpleRefresh,	TRUE,
		WA_IDCMP,			IDCMP_CLOSEWINDOW |
							IDCMP_RAWKEY,
	                        TAG_DONE);

    if (Window == NULL) cleanup(memory);

    WindowLimits(Window,
			Window->BorderLeft + 100 + Window->BorderRight,Window->BorderTop + 100 + Window->BorderBottom,
			Window->WScreen->Width,Window->WScreen->Height);


	if(memory != NULL)
		{
			
			Item = NewDTObject(Name,
				//PDTA_WhichPicture,	0,
				//PDTA_GetNumPictures, 1,
                DTA_SourceType,      DTST_MEMORY,
                PDTA_SourceMode,    PMODE_V43,
				DTA_SourceAddress,	memory,
				DTA_SourceSize,		size,
                PDTA_Remap,         TRUE,
				GA_Immediate,		TRUE,
				GA_RelVerify,		TRUE,
				DTA_TextAttr,		Window->WScreen->Font,
				GA_Left,			Window->BorderLeft,
				GA_Top,				Window->BorderTop,
				GA_RelWidth,		-(Window->BorderLeft + Window->BorderRight),
				GA_RelHeight,		-(Window->BorderTop + Window->BorderBottom),
				ICA_TARGET,			ICTARGET_IDCMP,
			TAG_DONE);

		}
		else
		{
			Item = NewDTObject(Name,
				GA_Immediate,	TRUE,
				GA_RelVerify,	TRUE,
                PDTA_SourceMode,    PMODE_V43,
                PDTA_Remap,     TRUE,
				DTA_TextAttr,	Window->WScreen->Font,
				GA_Left,		Window->BorderLeft,
				GA_Top,			Window->BorderTop,
				GA_RelWidth,	-(Window->BorderLeft + Window->BorderRight),
				GA_RelHeight,	-(Window->BorderTop + Window->BorderBottom),
                
				ICA_TARGET,		ICTARGET_IDCMP,
			TAG_DONE);

            
		}


		if(Item != NULL)
		{
			/* struct IntuiMessage *Message; */
			ULONG MsgClass;
			UWORD MsgCode;
			struct TagItem *MsgTags;
            struct BitMapHeader *bmh;
			BOOL Done;

            
            struct dtFrameBox mydtFrameBox;       // Use this with DTM_FRAMEBOX method
            struct FrameInfo myFrameInfo;         // For info returned from DTM_FRAMEBOX
            struct gpLayout mygpLayout;           // Use this with DTM_PROCLAYOUT method/

            mydtFrameBox.MethodID         = DTM_FRAMEBOX;
            mydtFrameBox.dtf_GInfo        = NULL;
            mydtFrameBox.dtf_ContentsInfo = NULL;
            mydtFrameBox.dtf_FrameInfo    = &myFrameInfo;
            mydtFrameBox.dtf_SizeFrameInfo= sizeof (struct FrameInfo);
            mydtFrameBox.dtf_FrameFlags   = 0L;

            // this has return value, operation could fail, no fail check here..
            DoMethodA(Item,&mydtFrameBox);
 
            // layout (remap) of the object on the process
            mygpLayout.MethodID   = DTM_PROCLAYOUT;
            mygpLayout.gpl_GInfo  = NULL;
            mygpLayout.gpl_Initial= 1L;

            // this has return value, operation could fail, no fail check here..
            DoMethodA(Item, &mygpLayout);
            

        	GetDTAttrs(Item, PDTA_BitMapHeader, &bmh, PDTA_BitMap, &bgBitMap, TAG_END);
        	width = bmh->bmh_Width;
        	height = bmh->bmh_Height;
        	newWidth = width + Window->BorderLeft + Window->BorderRight;
        	newHeight = height + Window->BorderTop + Window->BorderBottom;
            
            Done = FALSE;

            /* set new window dimensions */ 
            ChangeWindowBox(Window, Window->LeftEdge, Window->TopEdge, newWidth, newHeight);    
            
			// rastport from window
    
            fontWidth = Window->RPort->TxWidth;
            fontHeight = Window->RPort->TxHeight;
            fontBaseline = Window->RPort->TxBaseline;

            state = START_SCREEN;

            bitmap = (struct BitMap *)AllocBitMap(bmh->bmh_Width,bmh->bmh_Height,bmh->bmh_Depth,BMF_CLEAR | BMF_DISPLAYABLE,Window->RPort->BitMap);
            InitRastPort(&rastport);
            rastport.BitMap = bitmap;
            

            /*
             *  Main event loop
             */
			do
			{
               
				while((Message = (struct IntuiMessage *)GetMsg(Window->UserPort)) != NULL)
				{
                    
					MsgClass = Message->Class;
					MsgCode = Message->Code;
					MsgTags = Message->IAddress;

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
                                        wormX[i] = 26*8 + Window->BorderLeft + 128;
                                        wormY[i] = Window->BorderTop + 128;        
                                    }

                                    wormX[0] = Window->BorderLeft + 128;
                                    wormY[0] = Window->BorderTop + 128;

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

                BltBitMapRastPort(bgBitMap, 0, 0, &rastport, 0, 0, bmh->bmh_Width, bmh->bmh_Height, 192);

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
				
                        if (wormX[0] < 2 || wormX[0] > newWidth - Window->BorderRight - Window->BorderLeft - fontWidth || wormY[0] < 2 || wormY[0] > Window->Height - Window->BorderBottom - fontHeight - Window->BorderTop) {
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
                        Move(&rastport, (newWidth - TextLength(&rastport, "Press return to start", 21)) / 2, Window->BorderTop + 64);
                        Text(&rastport,"Press return to start", 21);

                        Move(&rastport, (newWidth - TextLength(&rastport, "Press p key to pause and return or p again to continue", 54)) / 2, Window->BorderTop + 128);
                        Text(&rastport,"Press p key to pause and return or p again to continue", 54);


                        break;

                    case GAME_OVER:

                        SetBPen(&rastport, 96);
                        SetAPen(&rastport, 129);
                        Move(&rastport, (newWidth - TextLength(&rastport, "GAME OVER", 9)) / 2,160);
                        Text(&rastport, "GAME OVER", 9);

                        break;

                    case GAME_WON:

                        SetBPen(&rastport, 36);
                        SetAPen(&rastport, 72);
                        Move(&rastport, (newWidth - TextLength(&rastport, "YOU WON", 7)) / 2,160);
                        Text(&rastport, "YOU WON", 7);

                        Move(&rastport, (newWidth - TextLength(&rastport, "Perhaps you'll fly away as butterfly...", 39)) / 2,192);
                        Text(&rastport, "Perhaps you'll fly away as butterfly...", 39);

                        break;

                    case GAME_PAUSED:

                        SetBPen(&rastport, 101);
                        SetAPen(&rastport, 222);
                        Move(&rastport, (newWidth - TextLength(&rastport, "PAUSED", 6)) / 2,160);
                        Text(&rastport, "PAUSED", 6);

                        break;
                }

                ClipBlit(&rastport, 0, 0, Window->RPort, Window->BorderLeft, Window->BorderTop, bmh->bmh_Width, bmh->bmh_Height, 192);
                WaitTOF();

			} while(!Done);

			DisposeDTObject(Item);

		}
        
		cleanup(memory, Window);
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

int cleanup(memory, Window) {

    if (bitmap) FreeBitMap((struct BitMap *)bitmap);
    if (DataTypesBase) CloseLibrary(DataTypesBase);
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (UtilityBase) CloseLibrary(UtilityBase);
	if (GfxBase) CloseLibrary(GfxBase);

    if (memory) FreeVec((APTR)memory);

	if (Window) CloseWindow((struct Window *)Window);
	
	exit(0);

}
