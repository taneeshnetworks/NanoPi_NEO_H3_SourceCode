/*
* \file        tvtester.c
* \brief
*
* \version     1.0.0
* \date        2014年04月15日
* \author      hezuyao <hezuyao@allwinnertech.com>
*
* Copyright (c) 2014 Allwinner Technology. All Rights Reserved.
*
*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <asoundlib.h>

#include "asm/arch/drv_display.h"
#include "dragonboard_inc.h"

#define COLOR_WHITE                     0xffffffff
#define COLOR_YELLOW                    0xffffff00
#define COLOR_GREEN                     0xff00ff00
#define COLOR_CYAN                      0xff00ffff
#define COLOR_MAGENTA                   0xffff00ff
#define COLOR_RED                       0xffff0000
#define COLOR_BLUE                      0xff0000ff
#define COLOR_BLACK                     0xff000000

static unsigned int colorbar[8] =
{
	COLOR_WHITE,
	COLOR_YELLOW,
	COLOR_CYAN,
	COLOR_GREEN,
	COLOR_MAGENTA,
	COLOR_RED,
	COLOR_BLUE,
	COLOR_BLACK
};

struct hdmi_output_type
{
	int mode;
	int width;
	int height;
	char name[16];
};

#define TYPE_COUNT                      11

static struct hdmi_output_type type_list[TYPE_COUNT] = {
	{DISP_TV_MOD_480I,       720,  480,  "480I"},       /* 0 */
	{DISP_TV_MOD_576I,       720,  576,  "576I"},       /* 1 */
	{DISP_TV_MOD_480P,       720,  480,  "480P"},       /* 2 */
	{DISP_TV_MOD_576P,       720,  576,  "576P"},       /* 3 */
	{DISP_TV_MOD_720P_50HZ,  1280, 720,  "720P 50HZ"},  /* 4 */
	{DISP_TV_MOD_720P_60HZ,  1280, 720,  "720P 60HZ"},  /* 5 */
	{DISP_TV_MOD_1080I_50HZ, 1920, 1080, "1080I 50HZ"}, /* 6 */
	{DISP_TV_MOD_1080I_60HZ, 1920, 1080, "1080I 60HZ"}, /* 7 */
	{DISP_TV_MOD_1080P_24HZ, 1920, 1080, "1080P 24HZ"}, /* 8 */
	{DISP_TV_MOD_1080P_50HZ, 1920, 1080, "1080P 50HZ"}, /* 9 */
	{DISP_TV_MOD_1080P_60HZ, 1920, 1080, "1080P 60HZ"}  /* 10 */
};

static int disp;
static int output_mode = 0;
static int screen_width = 0;
static int screen_height = 0;
static unsigned char *buff = NULL;
static int fb;

static int sound_play_stop;

#define BUF_LEN                         4096
#define FB_DEV                          "/dev/graphics/fb1"
char *buf[BUF_LEN];

static int  disp_gettvwidth(int format)
{
    switch (format)
    {
        case DISP_TV_MOD_480I:                      return 720;
        case DISP_TV_MOD_480P:                      return 720;
        case DISP_TV_MOD_576I:                      return 720;
        case DISP_TV_MOD_576P:                      return 720;
        case DISP_TV_MOD_720P_50HZ:                 return 1280;
        case DISP_TV_MOD_720P_60HZ:                 return 1280;
        case DISP_TV_MOD_1080I_50HZ:                return 1920;
        case DISP_TV_MOD_1080I_60HZ:                return 1920;
        case DISP_TV_MOD_1080P_50HZ:                return 1920;
        case DISP_TV_MOD_1080P_60HZ:                return 1920;
        case DISP_TV_MOD_1080P_24HZ:                return 1920;
        case DISP_TV_MOD_NTSC:                      return 720;
        case DISP_TV_MOD_PAL:                       return 720;
        case DISP_TV_MOD_PAL_M:                     return 720;
        case DISP_TV_MOD_PAL_NC:                    return 720;
        default:                                    break;
    }

    return -1;
}

static int  disp_gettvheight(int format)
{
    switch (format)
    {
        case DISP_TV_MOD_480I:                      return 480;
        case DISP_TV_MOD_480P:                      return 480;
        case DISP_TV_MOD_576I:                      return 576;
        case DISP_TV_MOD_576P:                      return 576;
        case DISP_TV_MOD_720P_50HZ:                 return 720;
        case DISP_TV_MOD_720P_60HZ:                 return 720;
        case DISP_TV_MOD_1080I_50HZ:                return 1080;
        case DISP_TV_MOD_1080I_60HZ:                return 1080;
        case DISP_TV_MOD_1080P_50HZ:                return 1080;
        case DISP_TV_MOD_1080P_60HZ:                return 1080;
        case DISP_TV_MOD_1080P_24HZ:                return 1080;
        case DISP_TV_MOD_NTSC:                      return 480;
        case DISP_TV_MOD_PAL:                       return 576;
        case DISP_TV_MOD_PAL_M:                     return 576;
        case DISP_TV_MOD_PAL_NC:                    return 576;
        default:                                    break;
    }

    return  -1;
}

/*
static void exit_layer(void)
{
#if 1
	unsigned int args[4];

	munmap(buf, screen_width * screen_height * 4);

	args[0] = 1;
	ioctl(disp, DISP_CMD_FB_RELEASE, args);
#endif
}

static int detect_output_mode(void)
{
	unsigned int args[4];
	int ret;
	int support_mode;

	args[0] = 1;
	args[1] = DISP_TV_MOD_1080P_50HZ;
	ret = ioctl(disp, DISP_CMD_HDMI_SUPPORT_MODE, args);
	if (ret == 1) {
		db_msg("hdmitester: your device support 1080p 50Hz\n");
		output_mode = DISP_TV_MOD_1080P_50HZ;
		screen_width  = 1920;
		screen_height = 1080;
	}
	else {
		args[0] = 1;
		args[1] = DISP_TV_MOD_720P_50HZ;
		ret = ioctl(disp, DISP_CMD_HDMI_SUPPORT_MODE, args);
		if (ret == 1) {
			db_msg("hdmitester: your device support 720p 50Hz\n");
			output_mode = DISP_TV_MOD_720P_50HZ;
			screen_width  = 1280;
			screen_height = 720;
		}
		else {
			db_msg("hdmitester: your device do not support neither 1080p nor 720p (50Hz)\n");
			if (script_fetch("hdmi", "support_mode", &support_mode, 1)) {
				support_mode = 2;
				db_msg("hdmitester: can't fetch user config mode, use default mode: %s\n",
					type_list[support_mode].name);
			}
			else if (support_mode < 0 || support_mode >= TYPE_COUNT) {
				support_mode = 2;
				db_msg("hdmitester: user config mode invalid, use default mode: %s\n",
					type_list[support_mode].name);
			}
			db_msg("hdmitester: use user config mode: %s\n", type_list[support_mode].name);
			args[0] = 1;
			args[1] = type_list[support_mode].mode;
			ret = ioctl(disp, DISP_CMD_HDMI_SUPPORT_MODE, args);
			if (ret == 1) {
				db_msg("tvtester: you device support %s\n", type_list[support_mode].name);
				output_mode = type_list[support_mode].mode;
				screen_width  = type_list[support_mode].width;
				screen_height = type_list[support_mode].height;
			}
			else {
				db_msg("tvtester: you device do not support %s\n",
					type_list[support_mode].name);
				return -1;
			}
		}
	}

	return 0;
}
*/

static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) {
		err = snd_pcm_prepare(handle);
	}

	if (err < 0) {
		db_warn("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
	}
	else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
			sleep(1);

			if (err < 0) {
				err = snd_pcm_prepare(handle);
			}
			if (err < 0) {
				db_warn("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
			}
		}

		return 0;
	}

	return err;
}

static int check_audio_route()
{
	int err, idx, i;
	snd_ctl_t *handle;
	const char *route_ctrl_name = "Speaker Function";
	snd_ctl_elem_value_t * elem_value;
	snd_ctl_elem_info_t *info;

	snd_ctl_elem_list_t elist;
	snd_ctl_elem_id_t *eid;

	snd_ctl_elem_value_alloca(&elem_value);
	snd_ctl_elem_info_alloca(&info);
	//snd_ctl_elem_value_set_numid(elem_value, 54);

	if ((err = snd_ctl_open(&handle, "hw:0", 0)) < 0) {
		  db_msg("Open control error: %s\n", snd_strerror(err));
		  goto check_audio_route_err;
	}

	db_msg("card=%d\n", handle->card);

	memset(&elist, 0, sizeof(elist));
	if (snd_ctl_elem_list(handle, &elist) < 0) {
		db_msg("snd_ctl_elem_list 1 failed\n");
		goto check_audio_route_err;
	}

	eid = calloc(elist.count, sizeof(snd_ctl_elem_id_t));
	elist.space = elist.count;
	elist.pids = eid;
	if (snd_ctl_elem_list(handle, &elist) < 0) {
		db_msg("snd_ctl_elem_list 2 failed\n");
		goto check_audio_route_err;
	}

	for (i = 0; i < elist.count; ++i) {
		info->id.numid = eid[i].numid;
		if ((err = snd_ctl_elem_info(handle, info)) < 0) {
			db_msg("Cannot find the given element from control\n");
			goto check_audio_route_err;
		}
		//db_msg("name[%d]=%s\n", i, snd_ctl_elem_info_get_name(info));

		if (!strcmp(snd_ctl_elem_info_get_name(info), route_ctrl_name)) {
			db_msg("route ctrl found!!!\n");
			break;
		}
	}

	snd_ctl_elem_value_set_numid(elem_value, info->id.numid);
	if ((err = snd_ctl_elem_read(handle,elem_value)) < 0) {
		  db_msg("snd_ctl_elem_read error: %s\n", snd_strerror(err));
		  goto check_audio_route_err;
	}

	db_msg("numid=%d\n", snd_ctl_elem_value_get_numid(elem_value));
	db_msg("name=%s\n", snd_ctl_elem_value_get_name(elem_value));

	//to set the new value
	snd_ctl_elem_value_set_enumerated(elem_value,0, 1);
	if ((err = snd_ctl_elem_write(handle,elem_value)) < 0) {
		  db_msg("snd_ctl_elem_write error: %s\n", snd_strerror(err));
		  goto check_audio_route_err;
	}

	//read it out again to check if we did set the registers.
	if ((err = snd_ctl_elem_read(handle,elem_value)) < 0) {
		  db_msg("snd_ctl_elem_read error: %s\n", snd_strerror(err));
		  goto check_audio_route_err;
	}

	db_msg("after: %d\n", snd_ctl_elem_value_get_enumerated(elem_value, 0));

	snd_ctl_close(handle);
	return 0;

check_audio_route_err:
	snd_ctl_close(handle);
	return -1;
}

// A80 donot set route in prepare, so route it here.
static void *sound_play(void *args)
{
	char path[256];
	int samplerate;
	int err;
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;
	FILE *fp;

	db_msg("tv:prepare play sound...\n");
	if (script_fetch("tv", "sound_file", (int *)path, sizeof(path) / 4)) {
		db_warn("tv:unknown sound file, use default\n");
		strcpy(path, "/dragonboard/data/test48000.pcm");
	}
	if (script_fetch("tv", "samplerate", &samplerate, 1)) {
		db_warn("tv:unknown samplerate, use default #48000\n");
		samplerate = 48000;
	}
	db_msg("tv:samplerate #%d\n", samplerate);

	if (check_audio_route() < 0) {
		db_error("tv:check_audio_route failed\n");
		pthread_exit((void *)-1);
	}

	err = snd_pcm_open(&playback_handle, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		db_error("tv:cannot open audio device (%s)\n", snd_strerror(err));
		pthread_exit((void *)-1);
	}

	err = snd_pcm_hw_params_malloc(&hw_params);
	if (err < 0) {
		db_error("tv:cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		pthread_exit((void *)-1);
	}

	err = snd_pcm_hw_params_any(playback_handle, hw_params);
	if (err < 0) {
		db_error("tv:cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		pthread_exit((void *)-1);
	}

	err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		db_error("tv:cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		pthread_exit((void *)-1);
	}

	err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if (err < 0) {
		db_error("tv:cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		pthread_exit((void *)-1);
	}

	err = snd_pcm_hw_params_set_rate(playback_handle, hw_params, samplerate, 0);
	if (err < 0) {
		db_error("tv:cannot set sample rate (%s)\n", snd_strerror(err));
		pthread_exit((void *)-1);
	}

	err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2);
	if (err < 0) {
		db_error("tv:cannot set channel count (%s), err = %d\n", snd_strerror(err), err);
		pthread_exit((void *)-1);
	}

    err = snd_pcm_hw_params_set_raw_flag(playback_handle, hw_params, 1);
    if (err < 0) {
        db_error("tv:cannot set raw_flag (%s), err = %d\n", snd_strerror(err), err);
        pthread_exit((void *)-1);
    }

	err = snd_pcm_hw_params(playback_handle, hw_params);
	if (err < 0) {
		db_error("tv:cannot set parameters (%s)\n", snd_strerror(err));
		pthread_exit((void *)-1);
	}

	snd_pcm_hw_params_free(hw_params);

	db_msg("tv:open test pcm file: %s\n", path);
	fp = fopen(path, "r");
	if (fp == NULL) {
		db_error("tv:cannot open test pcm file(%s)\n", strerror(errno));
		pthread_exit((void *)-1);
	}

	db_msg("tv:play it...\n");
	while (1) {
		while (!feof(fp)) {
			if (sound_play_stop) {
				goto out;
			}

			err = fread(buf, 1, BUF_LEN, fp);
			if (err < 0) {
				db_warn("tv:read test pcm failed(%s)\n", strerror(errno));
			}

			err = snd_pcm_writei(playback_handle, buf, BUF_LEN/4);
			if (err < 0) {
				err = xrun_recovery(playback_handle, err);
				if (err < 0) {
					db_warn("tv:write error: %s\n", snd_strerror(err));
				}
			}

			if (err == -EBADFD) {
				db_warn("tv:PCM is not in the right state (SND_PCM_STATE_PREPARED or SND_PCM_STATE_RUNNING)\n");
			}
			if (err == -EPIPE) {
				db_warn("tv:an underrun occurred\n");
			}
			if (err == -ESTRPIPE) {
				db_warn("tv:a suspend event occurred (stream is suspended and waiting for an application recovery)\n");
			}

			if (feof(fp)) {
				fseek(fp, 0L, SEEK_SET);
			}
		}
	}

out:
	db_msg("tv:play end...\n");
	fclose(fp);
	snd_pcm_close(playback_handle);
	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int args[4] = {0};
	int test_status = 0; // 1: sucessed
	int dev_status = 0;  // 1: opended
	int retry = 0;
	int flags = 0;
	int ret;
	pthread_t tid;
    disp_output_type output_type;
	int hdmi_status;
	int mic_activated;
	INIT_CMD_PIPE();

	init_script(atoi(argv[2]));

	disp = open("/dev/disp", O_RDWR);
	if (disp == -1) {
		db_error("tvtester: open /dev/disp failed(%s)\n", strerror(errno));
		goto err;
	}

	if(script_fetch("mic", "activated", &mic_activated,1))
	{
		mic_activated = 0;
	}
	/* test main loop */
	while (1)
	{
		args[0] = 1;
		output_type = getCurrentOutputType();
		hdmi_status = isHdmiPluged();
		if (hdmi_status != 1)
		{
			if (retry < 3) 
			{
				retry++;
				sleep(1);
				continue;
			}
			
			if (test_status == 1)
			{
				sleep(1);
				db_warn("TV: HPD!\n");
				continue;
			}

			if(DISP_OUTPUT_TYPE_NONE == output_type)
			{
				db_warn("then open tv\n");
				args[0] = 0;
				args[1] = output_type;
				//ret = ioctl(disp, DISP_CMD_TV_SET_MODE, args);
				ret = 0;
				if (ret < 0) 
				{
					db_error("tvtester: set tv output mode failed(%d)\n", ret);
					goto err;
				}
				//init_layer(tv_format);
				if(dev_status == 0)
				{
					//ret = ioctl(disp, DISP_CMD_TV_ON, args);
					ret = 0;
					if (ret < 0) 
					{
						db_error("tvtester: open tv failed(%d)\n", ret);
						goto err;
					}
					dev_status = 1;
				}
			}
			else if(DISP_OUTPUT_TYPE_TV == output_type)
			{
				dev_status = 1;
			}
			else
			{
				db_error("other display device runing! ,output_type[%d]\n", output_type);
				sleep(3);
				continue;
			}

			db_warn("tvtester: mic_activated[%d]\n", mic_activated);
			if(!mic_activated)
			{
				// create sound play thread
				sound_play_stop = 0;
				ret = pthread_create(&tid, NULL, sound_play, NULL);
				if (ret != 0) 
				{
					db_error("tvtester: create sound play thread failed\n");
					//ioctl(disp, DISP_CMD_TV_OFF, args);
					goto err;
				}
				mic_activated = 1;
			}
			test_status = 1;
			SEND_CMD_PIPE_OK();
		}
		else
		{
			void *retval;  
			//db_warn("tvtester: output_type[%d] is not tv!\n", output_type);

			// reset retry to 0      
			retry = 0;
			        
			if (flags < 3)
			{   
				flags++;        
				sleep(1);        
				continue;        
			}
			
			test_status = 0;
			
			if((dev_status == 1) || (output_type == DISP_OUTPUT_TYPE_TV))
			{
				args[0] = 0;
				//ret = ioctl(disp, DISP_CMD_TV_OFF, args);
				ret = 0;
			    if (ret < 0) 
			    {
				    db_error("tvtester: close tv failed(%d)\n", ret);
			    }
				dev_status = 0;
			}
			
			if(mic_activated)
			{
				// end sound play thread 
				sound_play_stop = 1;
				db_msg("tvtester: waiting for sound play thread finish...\n");
				if (pthread_join(tid, &retval)) 
				{  
					db_error("tvtester: can't join with sound play thread\n"); 
				}        
				db_msg("tvtester: sound play thread exit code #%d\n", (int)retval);
				mic_activated = 0;
			}
		}

		// sleep 1 second
		sleep(1);
	}

err:
	SEND_CMD_PIPE_FAIL();
	//exit_layer();
	close(disp);
	deinit_script();
	return -1;
}
