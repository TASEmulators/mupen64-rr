#ifdef USE_GTK
#include <gtk/gtk.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include <string.h>
#include <limits.h>

#include "../main/winlnxdefs.h"
#include "Audio_#1.1.h"

static AUDIO_INFO AudioInfo;
static int dsp;
static int frequency;

EXPORT void CALL
AiDacrateChanged( int SystemType )
{
   int f;
   switch (SystemType)
     {
      case SYSTEM_NTSC:
	f = 48681812 / (*AudioInfo.AI_DACRATE_REG + 1);
	break;
      case SYSTEM_PAL:
	f = 49656530 / (*AudioInfo.AI_DACRATE_REG + 1);
	break;
      case SYSTEM_MPAL:
	f = 48628316 / (*AudioInfo.AI_DACRATE_REG + 1);
	break;
     }
   if (ioctl(dsp, SNDCTL_DSP_SPEED, &f) == -1)
     printf("error initializing frequency:%x\n", f);
   frequency = f;
}

void sync()
{
   static long long limit = 0;
   double secPerByte = (1/(double)frequency)/4.0;
   struct timeval tv;
   gettimeofday(&tv, NULL);
   //printf("new tv = %d\n", tv.tv_usec);
   if (limit && (tv.tv_sec*1000000+tv.tv_usec) < limit)
     {
	usleep(limit-(tv.tv_sec*1000000+tv.tv_usec));
     }
   //else printf("not sleep\n");
   gettimeofday(&tv, NULL);
   limit = tv.tv_sec*1000000+tv.tv_usec + (long)(*AudioInfo.AI_LEN_REG*secPerByte*1000000);
   //printf("limit:%d\n", (long)(*AudioInfo.AI_LEN_REG*secPerByte*1000000)-100000);
   //printf("f=%d, %lf\n", frequency, secPerByte);
}

static pthread_t my_thread/*, my_thread2*/;
static short *buf[2] = {NULL, NULL};
static int buf_size[2] = {0, 0};
static int cur_buf = 0;
static sem_t sem, sem2/*, semt, semt2*/;
static int closed = 0;

static void *sound_thread(void *emp)
{
   while (!closed)
     {
	sem_wait(&sem);
	write(dsp, buf[cur_buf], *AudioInfo.AI_LEN_REG);
	sem_post(&sem2);
     }
   return 0;
}

/*static void *timing_thread(void *emp)
{
   //double secPerByte;
   while (!closed)
     {
	sem_wait(&semt);
	//secPerByte = (1/(double)frequency)/4.0;
	//usleep((long)(*AudioInfo.AI_LEN_REG*secPerByte*1000000));
	usleep((long long)((long long)*AudioInfo.AI_LEN_REG*1000000LL/((long long)frequency*4LL)));
	sem_post(&semt2);
     }
   return 0;
}*/

EXPORT void CALL
AiLenChanged( void )
{
   int i;
   short *p = (short*)(AudioInfo.RDRAM + 
		       (*AudioInfo.AI_DRAM_ADDR_REG & 0xFFFFFF));
   if (buf_size[1-cur_buf] < *AudioInfo.AI_LEN_REG)
     {
	buf_size[1-cur_buf] = *AudioInfo.AI_LEN_REG;
	if (buf[1-cur_buf] != NULL) free(buf[1-cur_buf]);
	buf[1-cur_buf] = (short*)malloc(buf_size[1-cur_buf]*sizeof(char));
     }
   for (i=0; i<(buf_size[1-cur_buf]/4); i++)
     {
	buf[1-cur_buf][i*2+0] = p[i*2+1];
	buf[1-cur_buf][i*2+1] = p[i*2+0];
     }
   //pthread_join(my_thread, NULL);
   cur_buf = 1 - cur_buf;
   //pthread_create(&my_thread, NULL, sound_thread, NULL);
   sem_post(&sem);
   sem_wait(&sem2);
   //sem_post(&semt);
   //sem_wait(&semt2);
   //sync();
     /*{
	static int sample_count = 0;
	count_info info;
	while (info.bytes<sample_count)
	  {
	     if (ioctl(dsp, SNDCTL_DSP_GETOPTR, &info) == -1)
	       printf("error getting sample count\n");
//	     printf("i:%d<%d\n", info.bytes, sample_count);
	  }
	sample_count += *AudioInfo.AI_LEN_REG;
     }*/
}

EXPORT DWORD CALL
AiReadLength( void )
{
   return 0;
}

EXPORT void CALL
CloseDLL( void )
{
}

EXPORT void CALL
DllAbout( HWND hParent )
{
#ifdef USE_GTK
   char tMsg[256];
   GtkWidget *dialog, *label, *okay_button;
   
   dialog = gtk_dialog_new();
   sprintf(tMsg,"Mupen64 Audio Plugin v0.2 by Hacktarux");
   label = gtk_label_new(tMsg);
   okay_button = gtk_button_new_with_label("OK");
   
   gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     GTK_OBJECT(dialog));
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
		     okay_button);
   
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
		     label);
   gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
   gtk_widget_show_all(dialog);
#else
   char tMsg[256];
   sprintf(tMsg,"Mupen64 Audio Plugin v0.2 by Hacktarux");
   fprintf(stderr, "About\n%s\n", tMsg);
#endif
}

static void GetPluginDir(char *Directory)
{
   int n = readlink("/proc/self/exe", Directory, PATH_MAX);
   if (n == -1)
     strcpy(Directory, "./");
   else
     {
	Directory[n] = '\0';
	while(Directory[strlen(Directory)-1] != '/')
	  Directory[strlen(Directory)-1] = '\0';
     }
   strcat(Directory, "plugins/");
}

#ifdef USE_GTK
typedef struct
{
   GtkWidget *dialog;
   GtkObject *adj;
} ConfigDialog;

static void okButtonCallback(GtkWidget *widget, void *data)
{
   ConfigDialog *configDialog = (ConfigDialog*)data;
   
   int val = (int)(GTK_ADJUSTMENT(configDialog->adj)->value);
   char path[PATH_MAX];
   FILE *f;
   
   GetPluginDir(path);
   strcat(path, "mupen64_sound.cfg");
   f = fopen(path, "wb");
   fprintf(f, "%d\n", val);
   fclose(f);
   
   gtk_widget_hide(configDialog->dialog);
}

static ConfigDialog *CreateConfigDialog()
{
   ConfigDialog *configDialog = malloc(sizeof(ConfigDialog));
   GtkWidget *dialog;
   GtkWidget *okButton;
   GtkWidget *cancelButton;
   GtkWidget *hScale;
   GtkWidget *frame;
   GtkObject *adj;
   
   dialog = gtk_dialog_new();
   gtk_window_set_title(GTK_WINDOW(dialog), "Mupen64 sound plugin configuration");
   
   okButton = gtk_button_new_with_label("OK");
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), okButton);
   
   cancelButton = gtk_button_new_with_label("Cancel");
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), cancelButton);
   
   frame = gtk_frame_new("Buffer size");
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), frame);
   
   adj = gtk_adjustment_new(0x10, 0x2, 0x32, 1, 1, 1);
   
   hScale = gtk_hscale_new(GTK_ADJUSTMENT(adj));
   gtk_scale_set_digits(GTK_SCALE(hScale), 0);
   gtk_container_add(GTK_CONTAINER(frame), hScale);
   
   gtk_signal_connect_object(GTK_OBJECT(dialog), "delete-event",
			     GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete),
			     GTK_OBJECT(dialog));
   gtk_signal_connect(GTK_OBJECT(okButton), "clicked",
		      GTK_SIGNAL_FUNC(okButtonCallback),
		      (void*)configDialog);
   gtk_signal_connect_object(GTK_OBJECT(cancelButton), "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_hide),
			     GTK_OBJECT(dialog));
   
   configDialog->dialog = dialog;
   configDialog->adj = adj;
   return configDialog;
}
#endif

EXPORT void CALL
DllConfig ( HWND hParent )
{
#ifdef USE_GTK
   static ConfigDialog *configDialog = NULL;
   if (configDialog == NULL) configDialog = CreateConfigDialog();
   
     {
	char path[PATH_MAX];
	FILE *f;
	int val;
	
	GetPluginDir(path);
	strcat(path, "mupen64_sound.cfg");
	f = fopen(path, "rb");
	if (f)
	  {
	     fscanf(f, "%d", &val);
	     fclose(f);
	     gtk_adjustment_set_value(GTK_ADJUSTMENT(configDialog->adj), val);
	  }
     }
   
   gtk_widget_show_all(configDialog->dialog);
#endif
}

EXPORT void CALL
DllTest ( HWND hParent )
{
}

EXPORT void CALL
GetDllInfo( PLUGIN_INFO * PluginInfo )
{
   PluginInfo->Version = 0x0101;
   PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
   sprintf(PluginInfo->Name,"mupen64 audio plugin");
   PluginInfo->NormalMemory  = TRUE;
   PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL
InitiateAudio( AUDIO_INFO Audio_Info )
{
   AudioInfo = Audio_Info;
   return TRUE;
}

EXPORT void CALL RomOpen()
{
   int channel=1, format, f;
   int val = 0x20010;
   char path[PATH_MAX];
   FILE *file;
	
   GetPluginDir(path);
   strcat(path, "mupen64_sound.cfg");
   file = fopen(path, "rb");
   if (file)
     {
	fscanf(file, "%d", &val);
	fclose(file);
	val = 0x20000 | val;
     }
   dsp = open("/dev/dsp", O_WRONLY);
   if (dsp == -1) printf("error opening /dev/dsp\n");
   if (ioctl(dsp, SNDCTL_DSP_RESET) == -1)
     printf("error resetting sound card\n");
   f = 0x20010;
   if (ioctl(dsp, SNDCTL_DSP_SETFRAGMENT, &val) == -1)
     printf("error setting fragment size\n");
   if (ioctl(dsp, SNDCTL_DSP_STEREO, &channel) == -1)
     printf("error setting stereo mode\n");
   if (!channel)
     printf("error setting stereo mode\n");
   format = AFMT_S16_LE;
   if (ioctl(dsp, SNDCTL_DSP_SAMPLESIZE, &format) == -1)
     printf("error initializing format\n");
   f = 32000;
   if (ioctl(dsp, SNDCTL_DSP_SPEED, &f) == -1)
     printf("error initializing frequency:%d\n", f);
   sem_init(&sem, 0, 0);
   sem_init(&sem2,0, 1);
   //sem_init(&semt,0, 0);
   //sem_init(&semt2,0,1);
   closed = 0;
   pthread_create(&my_thread, NULL, sound_thread, NULL);
   //pthread_create(&my_thread2,NULL, timing_thread,NULL);
}

EXPORT void CALL
RomClosed( void )
{

   ioctl(dsp, SNDCTL_DSP_SYNC);
   closed = 1;
   sem_post(&sem);
   sem_post(&sem);
   //sem_post(&semt);
   //sem_post(&semt);
   pthread_join(my_thread, NULL);
   //pthread_join(my_thread2,NULL);
   close(dsp);
}

EXPORT void CALL
ProcessAlist( void )
{
}
