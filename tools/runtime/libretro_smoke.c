/* Minimal software-video libretro smoke frontend. No emulator or BIOS code.
 * Run with an external timeout; a stalled core can block retro_run().
 * The caller supplies matching ELF symbol addresses and an empty output folder.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "libretro.h"
static unsigned frame, format, captures;
static const char *output;
static struct { char key[128], value[128]; } vars[200];
static int var_count;
static bool environment(unsigned cmd, void *data) {
    switch(cmd) {
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: *(const char **)data=output; return true;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: format=*(unsigned *)data; return format<=2;
    case RETRO_ENVIRONMENT_GET_CAN_DUPE: *(bool *)data=true; return true;
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *(bool *)data=false; return true;
    case RETRO_ENVIRONMENT_SET_VARIABLES: {
        const struct retro_variable *v=data;
        for(;v->key && var_count<200;v++) {
            const char *p=strstr(v->value,"; "); if(!p) continue;
            snprintf(vars[var_count].key,128,"%s",v->key);
            snprintf(vars[var_count].value,128,"%.*s",(int)strcspn(p+2,"|"),p+2);
            var_count++;
        } return true;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE: {
        struct retro_variable *v=data; v->value=NULL;
        for(int i=0;i<var_count;i++) if(!strcmp(v->key,vars[i].key)) v->value=vars[i].value;
        return v->value!=NULL;
    }
    default:return false;
    }
}
static void video(const void *data,unsigned w,unsigned h,size_t pitch) {
    if(!data || (frame!=120 && frame!=180 && frame!=181 && frame!=182 && frame!=200 && frame!=201 && frame!=202 && frame!=240 && frame!=299))return;
    char path[1024];snprintf(path,sizeof path,"%s/frame-%03u.ppm",output,frame);
    FILE *f=fopen(path,"wb");if(!f)exit(2);fprintf(f,"P6\n%u %u\n255\n",w,h);
    for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++) {
        const uint8_t *p=(const uint8_t*)data+y*pitch;
        unsigned r,g,b;
        if(format==1) {uint32_t v;memcpy(&v,p+4*x,4);r=(v>>16)&255;g=(v>>8)&255;b=v&255;}
        else {uint16_t v;memcpy(&v,p+2*x,2);r=((v>>(format==2?11:10))&31)*255/31;g=((v>>5)&(format==2?63:31))*255/(format==2?63:31);b=(v&31)*255/31;}
        fputc(r,f);fputc(g,f);fputc(b,f);
    }fclose(f);++captures;printf("capture %u %ux%u format=%u\n",frame,w,h,format);fflush(stdout);
}
static void audio(int16_t l,int16_t r){(void)l;(void)r;}
static size_t batch(const int16_t *d,size_t n){(void)d;return n;}
static void poll(void){}
static int16_t input(unsigned port,unsigned device,unsigned index,unsigned id){
    (void)index;return port==0 && device==RETRO_DEVICE_JOYPAD && id==RETRO_DEVICE_ID_JOYPAD_B && frame>=180 && frame<200;
}
static uint32_t read32(const uint8_t *p) {
    return p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}

int main(int argc,char **argv) {
    if(argc!=5) {
        fprintf(stderr,"usage: %s CUE OUTPUT_DIRECTORY FRAME_SYMBOL BUTTON_SYMBOL\n",argv[0]);
        return 2;
    }
    output=argv[2];
    char *end;
    unsigned long frame_address=strtoul(argv[3],&end,16);
    if(*end) return 2;
    unsigned long button_address=strtoul(argv[4],&end,16);
    if(*end) return 2;
    size_t frame_offset=frame_address & 0x1fffff;
    size_t button_offset=button_address & 0x1fffff;
    retro_set_environment(environment);
    retro_set_video_refresh(video);
    retro_set_audio_sample(audio);
    retro_set_audio_sample_batch(batch);
    retro_set_input_poll(poll);
    retro_set_input_state(input);
    retro_init();
    struct retro_game_info game={argv[1],NULL,0,NULL};
    if(!retro_load_game(&game)) {
        fprintf(stderr,"load failed\n");
        retro_deinit();
        return 1;
    }
    retro_set_controller_port_device(0,RETRO_DEVICE_JOYPAD);
    uint8_t *ram=retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);
    size_t ram_size=retro_get_memory_size(RETRO_MEMORY_SYSTEM_RAM);
    if(!ram || frame_offset+4>ram_size || button_offset+2>ram_size) return 2;
    uint32_t previous=0;
    bool passed=true;
    for(frame=0;frame<300;frame++) {
        retro_run();
        uint32_t counter=read32(ram+frame_offset);
        unsigned buttons=ram[button_offset] | (unsigned)ram[button_offset+1]<<8;
        if(frame>120 && counter!=previous+1) {
            fprintf(stderr,"frame counter stalled/jumped at %u: %u -> %u\n",frame,previous,counter);
            passed=false;
        }
        previous=counter;
        if(frame==120 || frame==181 || frame==199 || frame==202 || frame==299) {
            unsigned expected=(frame==181 || frame==199)?0x10:0;
            printf("state %u game_frame=%u buttons=%04x expected=%04x\n",frame,counter,buttons,expected);
            if(buttons!=expected) passed=false;
        }
    }
    if(captures!=9) passed=false;
    retro_unload_game();
    retro_deinit();
    puts(passed?"PASS: HLE boot, 300 frames, press/hold/release state; inspect captures for rendering":"FAIL: runtime checks");
    return passed?0:1;
}
