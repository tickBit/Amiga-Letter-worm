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
   I guess my system's GCC's includes are older than the ones, that are needed for this.
   This is why I've pasted these from later include files...
*/


#define PDTA_WhichPicture	(DTA_Dummy + 219)
#define PDTA_GetNumPictures	(DTA_Dummy + 220)


/* New for V44. Address of a DTST_MEMORY source type
 * object (APTR).
 */


#define DTA_SourceAddress	(DTA_Dummy+39)


/* New for V44. Size of a DTST_MEMORY source type
 * object (ULONG).
 */

#define DTA_SourceSize		(DTA_Dummy+40)

#define	DTST_MEMORY		5

#define START_SCREEN    0
#define GAME_OVER       1
#define GAME_WON        2
#define GAME_RUNNING    3
#define GAME_PAUSED     4

int state;

struct Library *IntuitionBase;
struct Library *DataTypesBase;
struct Library *UtilityBase;
struct Library *GfxBase;

struct RastPort *rastport;

struct DataType *dtn;

struct TagItem *TagList;

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
    struct BitMapHeader  *bmhd;

    struct IntuiMessage *Message; 

	Object *Item;
	APTR memory = NULL;

	UWORD wormX[26*8], wormY[26*8];
    UWORD wormDX = 2;
    UWORD wormDY = 0;

    BPTR fileHandle = NULL;

    int i;

    char Name[] = "gfx/AI-generated-640x640.jpg";

    UWORD fontWidth;
    UWORD fontHeight;
    UWORD fontBaseline;
    UWORD size = 0;

	fileHandle = Open(Name,MODE_OLDFILE);
    
    if (fileHandle != NULL) {

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
		WA_SizeBRight,		TRUE,
		WA_SizeBBottom,		TRUE,
		WA_CloseGadget,		TRUE,
		WA_DepthGadget,		TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,		FALSE,
		WA_RMBTrap,			TRUE,
		WA_Activate,		TRUE,
        WA_SimpleRefresh,	TRUE,
		WA_IDCMP,			IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_NEWSIZE |
							IDCMP_GADGETDOWN | IDCMP_IDCMPUPDATE |
							IDCMP_RAWKEY,
	                        TAG_DONE);

    if (Window == NULL) cleanup(memory);

    WindowLimits(Window,
			Window->BorderLeft + 100 + Window->BorderRight,Window->BorderTop + 100 + Window->BorderBottom,
			Window->WScreen->Width,Window->WScreen->Height);

	if(memory != NULL)
		{
               
			
			Item = NewDTObject(Name,
				PDTA_WhichPicture,	0,
				PDTA_GetNumPictures, 1,
                DTA_SourceType,      DTST_MEMORY,
				DTA_SourceAddress,	memory,
				DTA_SourceSize,		size,
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
			struct TagItem *MsgTags,*List,*This;
			BOOL Done;

        	GetDTAttrs(Item, PDTA_BitMapHeader, &bmhd, TAG_DONE);
        	width = bmhd->bmh_Width;
        	height = bmhd->bmh_Height;
        	newWidth = width + Window->BorderLeft + Window->BorderRight;
        	newHeight = height + Window->BorderTop + Window->BorderBottom;

            Done = FALSE;

            /* add the item to the Window */
            AddDTObject(Window,NULL,Item,-1);
			RefreshDTObjects(Item,Window,NULL,NULL);

            /* set new window dimensions */
             
            ChangeWindowBox(Window, Window->LeftEdge, Window->TopEdge, newWidth, newHeight);
            
			// rastport from window
			rastport=Window->RPort;
    
            fontWidth = rastport->TxWidth;
            fontHeight = rastport->TxHeight;
            fontBaseline = rastport->TxBaseline;

            state = START_SCREEN;

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
						

                        case IDCMP_IDCMPUPDATE:

							List = MsgTags;

							while((This = NextTagItem(&List)) != NULL)
							{
								switch(This->ti_Tag)
								{
									case DTA_Busy:

										if(This->ti_Data)
										{
											SetWindowPointer(Window,
												WA_BusyPointer,	TRUE,
											TAG_DONE);
										}
										else
										{
											SetWindowPointerA(Window,NULL);
										}

										break;

									case DTA_Sync:

										RefreshDTObjects(Item,Window,NULL,NULL);
										break;

								}
							}

							break;
						

						case IDCMP_REFRESHWINDOW:

							ReplyMsg((struct Message *)Message);

							BeginRefresh(Window);
							EndRefresh(Window,TRUE);

							Message = NULL;

							break;

                        
					}

					if (Message != NULL)
						ReplyMsg((struct Message *)Message);
				}
            
                RefreshDTObjectA(Item,Window,NULL,NULL);
                

                switch (state) {

                    case GAME_RUNNING:
                 
                        for (i = length*8; i > 0; i--) {
                            wormX[i] = wormX[i-1]; wormY[i] = wormY[i-1];
                            if (i > 8) {
                                if (abs(wormX[0] - wormX[i]) == 0 && abs(wormY[0] - wormY[i]) == 0) state = GAME_OVER;
                            }
                        }
                
                        SetAPen(rastport, rand() % 255);
                        Move(rastport, foodX, foodY);
                        DrawEllipse(rastport, foodX, foodY, 8,8);


                        // set colors for letters
                        SetBPen(rastport, 100);
                        SetAPen(rastport, 128);

                        if (length > 25) length = 25;

                        for (i = 0; i < length*8; i++) {
                            if (i % 8 == 0) {
                                Move(rastport, wormX[i], wormY[i]);
                                Text(rastport, WormLetters[i/8], 1);
                            }
                        }

                        wormX[0] += wormDX;
                        wormY[0] += wormDY;
				
                        if (wormX[0] < Window->BorderLeft + 2 || wormX[0] > newWidth - Window->BorderRight - 2 - Window->BorderLeft || wormY[0] < Window->TopEdge + fontBaseline || wormY[0] > Window->Height - Window->BorderBottom - 4) {
                            state = GAME_OVER;
                            break;
                        }
                        
                        // check if food is collected
                        if (abs(foodX - (wormX[0] + fontWidth / 2)) < 9 + fontWidth / 2 && abs(foodY - (wormY[0] - fontHeight / 2)) < 9 + fontHeight / 2) {
                            length++;
                            foodX = rand() % 570 + 32;
                            foodY = rand() % 550 + 64;
                            if (length == 25) state = GAME_WON;
                        }
                
                        break;

                    case START_SCREEN:

                        SetBPen(rastport, 7);
                        SetAPen(rastport, 134);
                        Move(rastport, (newWidth - TextLength(rastport, "Press return to start", 21)) / 2, Window->BorderTop + 64);
                        Text(rastport,"Press return to start", 21);

                        Move(rastport, (newWidth - TextLength(rastport, "Press p key to pause and return or p again to continue", 54)) / 2, Window->BorderTop + 128);
                        Text(rastport,"Press p key to pause and return or p again to continue", 54);

                        WaitTOF();

                        break;

                    case GAME_OVER:

                        SetBPen(rastport, 96);
                        SetAPen(rastport, 129);
                        Move(rastport, (newWidth - TextLength(rastport, "GAME OVER", 9)) / 2,160);
                        Text(rastport, "GAME OVER", 9);

                        WaitTOF();
                        break;

                    case GAME_WON:

                        SetBPen(rastport, 36);
                        SetAPen(rastport, 72);
                        Move(rastport, (newWidth - TextLength(rastport, "YOU WON", 7)) / 2,160);
                        Text(rastport, "YOU WON", 7);

                        Move(rastport, (newWidth - TextLength(rastport, "Perhaps you'll fly away as butterfly...", 39)) / 2,192);
                        Text(rastport, "Perhaps you'll fly away as butterfly...", 39);

                        WaitTOF();

                        break;

                    case GAME_PAUSED:

                        SetBPen(rastport, 101);
                        SetAPen(rastport, 222);
                        Move(rastport, (newWidth - TextLength(rastport, "PAUSED", 6)) / 2,160);
                        Text(rastport, "PAUSED", 6);

                        WaitTOF();

                        break;
                }

                

                WaitTOF();

			} while(!Done);


			RemoveDTObject(Window,Item);

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

    if (DataTypesBase != NULL) CloseLibrary(DataTypesBase);
    if (IntuitionBase != NULL) CloseLibrary(IntuitionBase);
    if (UtilityBase != NULL) CloseLibrary(UtilityBase);
	if (GfxBase != NULL) CloseLibrary(GfxBase);

    if (memory != NULL) FreeVec((APTR)memory);

	if (Window != NULL) CloseWindow((struct Window *)Window);
	
	exit(0);

}
