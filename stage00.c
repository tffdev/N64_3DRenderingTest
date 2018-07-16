#include <assert.h>
#include <nusys.h>
#include "graphic.h"
#include "Celebi.h"
#include <string.h>

/* Prototype all funcs before calling*/
void draw_cube( Dynamic* dynamicp, float t);
void SetViewMtx( Dynamic* );
void debug_console_int(char *name, int variable, int pos);
void debug_console_float(char *name, float variable, int pos);
int lim(u32 input);

int t = 1;
char conbuf[20];

// stuff about the cube
static float cubescale;
float cubepan;
float cubeyaw;

typedef struct {
  float x;
  float y;
  float z;
} Vec3;

static Vec3 campos;
static Vec3 camrot;

static Mtx squidmatrix;

NUContData contdata[1]; /* Read data of 1 controller  */

float View;

Mtx cameraRotation;
Mtx cameraPosition;

void SetViewMtx( Dynamic* dp)
{
    u16 perspNorm;

    /* The calculation and set-up of the projection-matrix  */
    guPerspective(
      &dp->projection,                      //Mtx *m
      &perspNorm,                           //u16 *perspNorm,
      50,                                   //FOV
      (float)SCREEN_WD/(float)SCREEN_HT,    //ASPECT
      10,                                   //near plane clipping
      10000,                                //far plane clipping
      1.0F                                  //matrix object scaling
    );

    
    guLookAt(
      &dp->viewing, 
      // xyz pos of camera
      campos.x,
      campos.y,
      campos.z,

      // XYZ of "looking at"
      sin(camrot.y) + campos.x, 
      -sin(camrot.x)*cos(camrot.y) + campos.y, 
      cos(camrot.x)*cos(camrot.y) + campos.z,

      // "up" direction, never change
      0, 1, 0 
    );

        
    gSPPerspNormalize(glistp++, perspNorm);
    gSPLookAt(glistp++, &dp->viewing);

    gSPMatrix(glistp++, &(dp->projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &(dp->viewing), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);

}

void initStage00()
{
  nuDebConDisp(NU_SC_SWAPBUFFER);
  cubescale = 1;
  cubepan = 0;
  cubeyaw = 0;
  campos = (Vec3){.x = 0, .y = 0, .z = 0};
  camrot = (Vec3){.x = 0, .y = 0, .z = 0};
  View = 0;
}

void makeDL00(void)
{
  /* Specify the display list buffer  */
  glistp = gfx_glist;
  
  /*  The initialization of RCP  */
  gfxRCPInit();

  /* Clear the frame buffer and the Z-buffer  */
  gfxClearCfb();

  SetViewMtx(&gfx_dynamic);

  // guRotate(&gfx_dynamic.modeling, 0.0F, 0.0F, 0.0F, 0.0F);

  /* Draw a square  */
  draw_cube(&gfx_dynamic,t);

  /* End the construction of the display list  */
  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  /* Check if all are put in the array  */
  assert(glistp - gfx_glist < GFX_GLIST_LEN);

  /* Activate the RSP task.  Switch display buffers at the end of the task. */
  nuGfxTaskStart(&gfx_glist[0],(s32)(glistp - gfx_glist) * sizeof (Gfx), NU_GFX_UCODE_F3DEX , NU_SC_SWAPBUFFER);


  /* DEBUG CONSOLE!!!!!!!!!!!! Display characters on the frame buffer, debug console only */
  nuDebConDisp(NU_SC_SWAPBUFFER);
  debug_console_int("t   ",t,1);
  debug_console_float("camx",camrot.x,2);
  debug_console_float("camy",camrot.y,3);
  debug_console_float("camz",camrot.z,4);
  debug_console_float("camx",campos.x,5);
  debug_console_float("camy",campos.y,6);
  debug_console_float("camz",campos.z,7);
  // debug_console_float("View",View,5);
  // debug_console_float("cScl",cubescale,6);

  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);
}

/* Draw object  */
void draw_cube(Dynamic* dynamicp, float t)
{
  int i = 0;
  
  /* Create matrices for mult */
  /* CUBE IS AT CENTER OF EARTH */
  guTranslate(&dynamicp->pos,0,0,-100);
  guRotate(&dynamicp->rotx,cubepan,1,0,0);
  guRotate(&dynamicp->roty,cubeyaw,0,1,0);
  guScale(&dynamicp->scale,cubescale,cubescale,cubescale);
 
  /* apply transformation matrices, to stack */
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->pos)),   G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->scale)), G_MTX_MODELVIEW | G_MTX_MUL  | G_MTX_NOPUSH );
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->rotx)),  G_MTX_MODELVIEW | G_MTX_MUL  | G_MTX_NOPUSH );
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->roty)),  G_MTX_MODELVIEW | G_MTX_MUL  | G_MTX_NOPUSH );

  /* Rendering setup */
  gDPSetRenderMode(glistp++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPTexture(glistp++,0x8000, 0x8000, 0, 0, G_ON);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetCombineMode(glistp++,G_CC_DECALRGBA, G_CC_DECALRGBA);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);

  /* DRAW OBJECT 
  ====================================================
  ====================================================*/ 
  gSPDisplayList(glistp++, Wtx_Celebi);
  /*=================================================
  ====================================================*/


  /* Finalise and exit drawing */
  gSPTexture(glistp++,0, 0, 0, 0, G_OFF);
  gDPPipeSync(glistp++);
}

/* Just controls and changes vars for debugging */
void updateGame00()
{
  /* Data reading of controller 1 */
  nuContDataGetEx(contdata,0);

  cubeyaw += contdata->stick_x*0.05;
  cubepan -= contdata->stick_y*0.05;
  
  /* Really basic controls for debugging purposes */
  if(contdata[0].button & START_BUTTON){
    t += 1;
  }

  if(contdata[0].button & A_BUTTON){
    cubescale += 0.001;
  }
  if(contdata[0].button & B_BUTTON){
    cubescale -= 0.001;
  }

  // UP/DOWN CAM ROTATION CONTROLS
  camrot.y += 0.06 * ( lim(contdata[0].button & L_CBUTTONS) - lim(contdata[0].button & R_CBUTTONS) );
  camrot.x += 0.06 * ( lim(contdata[0].button & U_CBUTTONS) - lim(contdata[0].button & D_CBUTTONS) );
  campos.y += 0.06 * ( lim(contdata[0].button & L_JPAD) - lim(contdata[0].button & R_JPAD) );
  campos.x += 0.06 * ( lim(contdata[0].button & U_JPAD) - lim(contdata[0].button & D_JPAD) );
}

/* UTILITIES */
void debug_console_int(char *name, int variable, int pos)
{
  nuDebConTextPos(0,1,1*pos);
  sprintf(conbuf,"%s=%i",name,variable);
  nuDebConCPuts(0, conbuf);
}

void debug_console_float(char *name, float variable, int pos)
{
  nuDebConTextPos(0,1,1*pos);
  sprintf(conbuf,"%s=%f",name,variable);
  nuDebConCPuts(0, conbuf);
}

int lim(u32 input) {
  if(input == 0) {
    return 0;
  }else{
    return 1;
  }
}