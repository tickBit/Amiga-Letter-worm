/* 
 * My first AmigaOS 4 app.
 * 
 * compiling with VBCC:
 * vc Letter-worm-OS4.c -o Letter-worm-OS4 -lauto
 */

#include <stdio.h>
#include <math.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/icclass.h>
#include <exec/libraries.h>
#include <graphics/text.h>
#include <graphics/rastport.h>
#include <diskfont/diskfont.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/datatypes.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/intuition.h>
#include <proto/utility.h>


 
#define START_SCREEN    0
#define GAME_OVER       1
#define GAME_WON        2
#define GAME_RUNNING    3
#define GAME_PAUSED     4

int main(void)
{

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

    UWORD wormX[26*12], wormY[26*12];
    UWORD wormDX = 1;
    UWORD wormDY = 0;
    UWORD x, foodX, foodY;

    UWORD length = 26;  // old test value, is initialized, when the game begins..

    ULONG state;
    state = START_SCREEN;

    struct IntuitionIFace *IIntuition = NULL;
    struct GraphicsIFace *IGraphics = NULL;
    struct DiskfontIFace *IDiskfont = NULL;
    
    /* Opening the libraries could fail. No fail check here... */

    struct Library *GfxBase = IExec->OpenLibrary("graphics.library", 50);
    IGraphics = (struct GraphicsIFace*)IExec->GetInterface(GfxBase, "main", 1, NULL);
 
    struct Library *IntuitionBase = IExec->OpenLibrary("intuition.library", 50);
    IIntuition = (struct IntuitionIFace*)IExec->GetInterface(IntuitionBase, "main", 1, NULL);

    struct Library *DiskfontBase = IExec->OpenLibrary("diskfont.library", 50);
    IDiskfont = (struct DiskfontIFace*)IExec->GetInterface(DiskfontBase, "main", 1, NULL);
    
    // Font
    struct TextAttr myta = {
      (STRPTR)"topaz.font",
      11, 0, 0
   };
 
    
    struct TextFont *myfont = NULL, *oldfont = NULL;
    struct RastPort rastport = {0};
    
    struct Window *win = NULL;
    
    Object *dto = NULL;

    if ((dto = IDataTypes->NewDTObject("gfx/AI-Fantasy-forest-512x512.jpg",
                                      DTA_SourceType, DTST_FILE,
                                      PDTA_Remap,  TRUE,
                                      DTA_GroupID,    GID_PICTURE,
                                      PDTA_DestMode,  PMODE_V43,   /* 24 bit pic */
				                              GA_Immediate,		TRUE,
				                              GA_RelVerify,		TRUE,
                                      TAG_END)))
    {
      
        struct BitMapHeader *bmh = NULL;
        struct BitMap *backBuffer = NULL;
        struct BitMap *drawBuffer = NULL;

        struct dtFrameBox mydtFrameBox;       /* Use this with DTM_FRAMEBOX method   */
        struct FrameInfo myFrameInfo;         /* For info returned from DTM_FRAMEBOX */
        struct gpLayout mygpLayout;           /* Use this with DTM_PROCLAYOUT method */

        mydtFrameBox.MethodID         = DTM_FRAMEBOX;
        mydtFrameBox.dtf_GInfo        = NULL;
        mydtFrameBox.dtf_ContentsInfo = NULL;
        mydtFrameBox.dtf_FrameInfo    = &myFrameInfo;
        mydtFrameBox.dtf_SizeFrameInfo= sizeof (struct FrameInfo);
        mydtFrameBox.dtf_FrameFlags   = 0L;

        // this has return value, operation could fail, no fail check here..
        IIntuition->IDoMethodA(dto,&mydtFrameBox);
 
        // layout (remap) of the object on the process
        mygpLayout.MethodID   = DTM_PROCLAYOUT;
        mygpLayout.gpl_GInfo  = NULL;
        mygpLayout.gpl_Initial= 1L;

        // this has return value, operation could fail, no fail check here..
        IIntuition->IDoMethodA(dto, &mygpLayout);

        /* The bitmap of the image is pointed to backBuffer */
        IDataTypes->GetDTAttrs(dto, PDTA_BitMapHeader, &bmh,  PDTA_BitMap, &backBuffer, TAG_END);
     
        if((win = IIntuition->OpenWindowTags(NULL,
                 WA_InnerWidth,    bmh->bmh_Width,
                 WA_InnerHeight,    bmh->bmh_Height,
                 WA_Title,         "Worm game for AmigaOS 4",
                 WA_IDCMP,         IDCMP_CLOSEWINDOW | IDCMP_RAWKEY,
                 WA_DragBar,       TRUE,
                 WA_DepthGadget,   TRUE,
                 WA_CloseGadget,   TRUE,
                 WA_AutoAdjust,    TRUE,
                 WA_SimpleRefresh, TRUE,
                 WA_BusyPointer,   TRUE,
                 WA_Activate,      TRUE,
                 TAG_END)))
        {
        
        // drawing is done to this buffer
        drawBuffer = IGraphics->AllocBitMapTags(bmh->bmh_Width, bmh->bmh_Height, bmh->bmh_Depth,
                                                    BMATags_Clear, TRUE,
                                                    BMATags_Displayable, TRUE,
                                                    BMATags_Friend, win->RPort->BitMap,
                                                    TAG_DONE);
        
        // init RastPort to drawing buffer
        IGraphics->InitRastPort(&rastport);
        rastport.BitMap = drawBuffer;

        // these should be read from some structure...
        // If these are in RastPort structure in OS4 too,
        // they're with different field names than in classic Amiga OS..
        UWORD fontWidth = 11;
        UWORD fontHeight = 11;
        UWORD fontBaseline = 8;
        
        if (myfont = IDiskfont->OpenDiskFont(&myta))
        {
            IGraphics->SetFont(&rastport, myfont);
            IGraphics->SetDrMd(&rastport, JAM2 );
        }
        
        BOOL going = TRUE;

        struct IntuiMessage *imsg;

        while(going)
        {
          
          // Intuition message loop
          while((imsg = (struct IntuiMessage *) IExec->GetMsg(win->UserPort)))
          {
            ULONG MsgClass = imsg->Class;
					  ULONG MsgCode = imsg->Code;

            switch(MsgClass)
            {
              case IDCMP_CLOSEWINDOW:
                   going = FALSE;
                   break;                 
                   
              case IDCMP_RAWKEY:

                switch(MsgCode) {
                  
                  // Esc
                  case 69:
                    going = FALSE;
                    break;
            
                  // p key
                  case 25:
                    if (state == GAME_PAUSED) {
                      state = GAME_RUNNING;
                      break;
                    }
                    
                    if (state == GAME_RUNNING) {
                      state = GAME_PAUSED;
                      break;
                    }

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
                    foodX = rand() % 500;
                    foodY = rand() % 500;

                    for (UWORD i = 1; i < 26*12; i++) {
                      wormX[i] = 26*12 + win->BorderLeft + 128;
                      wormY[i] = win->BorderTop + 128;        
                    }

                    wormX[0] = win->BorderLeft + 128;
                    wormY[0] = win->BorderTop + 128;

                    break;

								// cursor left
								case 79:
                  wormDX = -1;
                  wormDY = 0;
                  break;

                // cursor right
                case 78:
                  wormDX = 1;
                  wormDY = 0;
                  break;

								// cursor up
								case 76:
									wormDY = -1;
                  wormDX = 0;
									break;

							  // cursor down
								case 77:
									wormDY = 1;
                  wormDX = 0;
								  break;
                }

                break;
              }
          }
          
          // clear the drawing buffer with background image
          IGraphics->BltBitMapRastPort(backBuffer, 0, 0, &rastport, 0, 0, bmh->bmh_Width, bmh->bmh_Height, 192);
          
          switch (state) {

                    case GAME_RUNNING:
                
                        for (UWORD i = length*12 - 1; i > 0; i--) {
                              wormX[i] = wormX[i-1]; wormY[i] = wormY[i-1];
                              if (i > 12) {
                                  if (abs(wormX[0] - wormX[i]) == 0 && abs(wormY[0] - wormY[i]) == 0) state = GAME_OVER;
                              }
                        }

                        IGraphics->SetAPen(&rastport, rand() % 255);
                        IGraphics->Move(&rastport, foodX, foodY);
                        IGraphics->DrawEllipse(&rastport, foodX, foodY, 8,8);

                        // set colors for letters
                        IGraphics->SetBPen(&rastport, 88);
                        IGraphics->SetAPen(&rastport, 68);

                        if (length > 26) length = 26;

                        for (UWORD i = 0; i < length*12; i++) {
                              if (i % 12 == 0) {
                                IGraphics->Move(&rastport, wormX[i], wormY[i]);
                                IGraphics->Text(&rastport, WormLetters[i/12], 1);
                              }
                        }
                        

                        wormX[0] += wormDX;
                        wormY[0] += wormDY;
				
                        if (wormX[0] < win->BorderLeft || wormX[0] > bmh->bmh_Width - win->BorderRight - win->BorderLeft || wormY[0] < 5 || wormY[0] > win->Height - fontHeight - win->BorderTop) {
                            state = GAME_OVER;
                            break;
                        }

                        // check if food is collected
                        if (abs(foodX - (wormX[0] + fontWidth / 2)) < 9 + fontWidth / 2 && abs(foodY - (wormY[0] - fontHeight / 2)) < 9 + fontHeight / 2) {
                            length++;
                            foodX = rand() % 500;
                            foodY = rand() % 500;
                            if (length == 26) state = GAME_WON;
                        }

                        break;

                    case START_SCREEN:

                        IGraphics->SetBPen(&rastport, 77);
                        IGraphics->SetAPen(&rastport, 133);
                        IGraphics->Move(&rastport, (bmh->bmh_Width - IGraphics->TextLength(&rastport, "Press return to start", 21)) / 2, win->BorderTop + 64);
                        IGraphics->Text(&rastport,"Press return to start", 21);

                        IGraphics->Move(&rastport, (bmh->bmh_Width - IGraphics->TextLength(&rastport, "Press p key to pause and return or p again to continue", 54)) / 2, win->BorderTop + 128);
                        IGraphics->Text(&rastport,"Press p key to pause and return or p again to continue", 54);

                        break;

                    case GAME_OVER:

                        IGraphics->SetBPen(&rastport, 76);
                        IGraphics->SetAPen(&rastport, 133);
                        IGraphics->Move(&rastport, (bmh->bmh_Width - IGraphics->TextLength(&rastport, "GAME OVER", 9)) / 2,160);
                        IGraphics->Text(&rastport, "GAME OVER", 9);

                        break;

                    case GAME_WON:

                        IGraphics->SetBPen(&rastport, 69);
                        IGraphics->SetAPen(&rastport, 132);
                        IGraphics->Move(&rastport, (bmh->bmh_Width - IGraphics->TextLength(&rastport, "YOU WON", 7)) / 2,160);
                        IGraphics->Text(&rastport, "YOU WON", 7);

                        IGraphics->Move(&rastport, (bmh->bmh_Width - IGraphics->TextLength(&rastport, "Perhaps you'll fly away as butterfly...", 39)) / 2,192);
                        IGraphics->Text(&rastport, "Perhaps you'll fly away as butterfly...", 39);

                        break;

                    case GAME_PAUSED:

                        IGraphics->SetBPen(&rastport, 77);
                        IGraphics->SetAPen(&rastport, 133);
                        IGraphics->Move(&rastport, (bmh->bmh_Width - IGraphics->TextLength(&rastport, "PAUSED", 6)) / 2,160);
                        IGraphics->Text(&rastport, "PAUSED", 6);

                        break;
            }

            // draw the drawing buffer to window
            IGraphics->ClipBlit(&rastport, 0, 0, win->RPort, win->BorderLeft, win->BorderTop, bmh->bmh_Width, bmh->bmh_Height, 192);
            IGraphics->WaitTOF();
          
        }
        

        IGraphics->FreeBitMap(drawBuffer);

        IGraphics->CloseFont(myfont);

        IIntuition->CloseWindow(win);

        IDataTypes->DisposeDTObject(dto);
        
      } else printf("Could not open window\n");
    
    }

    IExec->DropInterface((struct Interface*)IDiskfont);
    IExec->CloseLibrary(DiskfontBase);
    
    IExec->DropInterface((struct Interface*)IIntuition);
    IExec->CloseLibrary(IntuitionBase);
 
    IExec->DropInterface((struct Interface*)IGraphics);
    IExec->CloseLibrary(GfxBase);

  return 0;
}