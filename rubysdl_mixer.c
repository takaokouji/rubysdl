#ifdef HAVE_SDL_MIXER

#include "rubysdl.h"
#include <SDL_mixer.h>

static int mix_opened=0;

static VALUE mix_openAudio(VALUE mod,VALUE frequency,VALUE format,
			   VALUE channels,VALUE chunksize)
{
  if( mix_opened ){
    rb_raise(eSDLError,"already initialize SDL::Mixer");
  }
  if( Mix_OpenAudio( NUM2INT(frequency),NUM2UINT(format),NUM2INT(channels),
		     NUM2INT(chunksize) ) != 1){
    rb_raise(eSDLError,"Couldn't open audio: %s",SDL_GetError());
  }
  return Qnil;
}

static VALUE mix_querySpec(VALUE mod)
{
  int rate;
  Uint16 format;
  int channels;

  if( !Mix_QuerySpec(&rate,&format,&channels) )
    rb_raise(eSDLError,"audio have not been opened yet");
  return rb_ary_new3( 3,INT2NUM(rate),UINT2NUM(title),INT2NUM(channels) );
}
  
/* Returns which channel was used to play the sound. */
static VALUE mix_playChannel(VALUE mod,VALUE channel,VALUE wave,VALUE looping)
{
  Mix_Chunk *chunk;
  if( ! rb_obj_is_kind_of(cWave,wave) )
    rb_raise(rb_eArgError,"type mismatch");
  Data_Get_Struct(wave,Mix_Chunk,chunk);
  return INT2FIX( Mix_PlayChannel(NUM2INT(channel),chunk,(looping)? -1 : 1 ) );
}

/*  need changeing */
static VALUE mix_playing(VALUE mod,VALUE channel)
{
  if( Mix_Playing( NUM2INT(channel) ) ){
    return Qtrue;
  }else{
    return Qfalse;
  }
}
  
static VALUE mix_loadWav(VALUE class,VALUE filename)
{
  Mix_Chunk *wave;
  Check_Type(filename,T_STRING);
  wave = Mix_LoadWAV( STR2CSTR(filename) );
  if( wave == NULL ){
    rb_raise( eSDLError,"Couldn't load wave file %s: %s",
	      STR2CSTR(filename),SDL_GetError() );
  }
  return Data_Wrap_Struct(class,0,Mix_FreeChunk,wave);
}

/* Volume setting functions and methods : volume in 0..128 */
static VALUE mix_volume(VALUE mod,VALUE channel,VALUE volume)
{
  return INT2FIX( Mix_Volume( NUM2INT(channel),NUM2INT(volume) ) );
}
static VALUE mix_wave_volume(VALUE obj,VALUE volume)
{
  Mix_Chunk *chunk;
  Data_Get_Struct(obj,Mix_Chunk,chunk);
  return INT2FIX( Mix_VolumeChunk( chunk,NUM2INT(volume) ) );
}

/* Halt,Pause function */
static VALUE mix_halt(VALUE mod,VALUE channel)
{
  Mix_HaltChannel(NUM2INT(channel));
  return Qnil;
}
static VALUE mix_pause(VALUE mod,VALUE channel)
{
  Mix_Pause(NUM2INT(channel));
  return Qnil;
}
static VALUE mix_resume(VALUE mod,VALUE channel)
{
  Mix_Resume(NUM2INT(channel));
  return Qnil;
}
static VALUE mix_paused(VALUE mod,VALUE channel)
{
  return Mix_Paused(NUM2INT(channel));
}

static void defineConstForAudio()
{
  rb_define_const(mMixer,"FORMAT_U8",UINT2NUM(AUDIO_U8));
  rb_define_const(mMixer,"FORMAT_S8",UINT2NUM(AUDIO_S8));
  rb_define_cosnt(mMixer,"FORMAT_U16LSB",UINT2NUM(AUDIO_U16LSB));
  rb_define_cosnt(mMixer,"FORMAT_S16LSB",UINT2NUM(AUDIO_S16LSB));
  rb_define_cosnt(mMixer,"FORMAT_U16MSB",UINT2NUM(AUDIO_U16MSB));
  rb_define_cosnt(mMixer,"FORMAT_S16MSB",UINT2NUM(AUDIO_S16MSB));
  rb_define_cosnt(mMixer,"FORMAT_U16",UINT2NUM(AUDIO_U16));
  rb_define_cosnt(mMixer,"FORMAT_S16",UINT2NUM(AUDIO_S16));

  rb_define_cosnt(mMixer,"FORMAT_U16SYS",UINT2NUM(AUDIO_U16SYS));
  rb_define_cosnt(mMixer,"FORMAT_S16SYS",UINT2NUM(AUDIO_S16SYS));
}

void init_mixer()
{
  mMixer = rb_define_module_under(mSDL,"Mixer");
  rb_define_module_function(mMixer,"open",mix_openAudio,4);
  rb_define_module_function(mMixer,"spec",mix_querySpec,0);
  rb_define_module_function(mMixer,"playChannel",mix_playChannel,3);
  rb_define_module_function(mMixer,"play?",mix_playing,1);
  rb_define_module_function(mMixer,"setVolume",mix_volume,2);
  rb_define_module_function(mMixer,"halt",mix_halt,1);
  rb_define_module_function(mMixer,"pause",mix_pause,1);
  rb_define_module_function(mMixer,"resume"mix_resume,1);
  rb_define_module_function(mMixer,"pause?",mix_paused,1);
  
  cWave = rb_define_class_under(mMixer,"Wave",rb_cObject);
  rb_define_singleton_method(cWave,"load",mix_loadWav,1);
  rb_define_method(cWave,"setVolume",mix_wave_volume,1);

  defineConstForAudio();
  return ;
}

void quit_mixer()
{
  if( mix_opened ){
    Mix_CloseAudio();
  }
}
#endif  /* HAVE_SDL_MIXER */