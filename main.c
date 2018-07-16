#include <nusys.h>
#include "main.h"

/* Declaration of the prototype, STAGE 0 */
void stage00(int);
/*extern prototypes*/
void makeDL00(void);
void initStage00(void);
void updateGame00(void);


/* The global variable  */
NUContData contdata[1]; /* Read data of 1 controller  */
u8 contPattern;        /* The pattern connected to the controller  */

/*------------------------
	Main
--------------------------*/
void mainproc(void)
{
  /* The initialization of graphic  */
  nuGfxInit();

  contPattern = nuContInit();

  /* Init scene */
  initStage00();

  /* Set call-back  */
  nuGfxFuncSet((NUGfxFunc)stage00);

  /* The screen display ON */
  nuGfxDisplayOn();

  while(1){}
}

/*-----------------------------------------------------------------------------
  The call-back function 

  pendingGfx which is passed from Nusystem as the argument of the call-back 
  function is the total number of RCP tasks that are currently processing 
  and waiting for the process. 
-----------------------------------------------------------------------------*/
void stage00(int pendingGfx)
{
  /* It provides the display process if there is no RCP task that is processing. */
  if(pendingGfx < 2){
    makeDL00();	
  }
  updateGame00();
}

