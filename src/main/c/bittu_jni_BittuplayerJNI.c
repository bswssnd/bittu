#include "bittu_jni_BittuplayerJNI.h"

#include <libavformat/avformat.h>
#include <libavutil/error.h>

#include <stdatomic.h>

#include <opus/opus.h>

struct bittuplayer {
    AVFormatContext*    format_context;
    AVPacket*           demuxed_packet;

    int                 audio_stream_index;
    atomic_int_fast64_t position;

    int  error_state;
    char error_message[AV_ERROR_MAX_STRING_SIZE];

    // as the sender runs in a different thread, it can clutter up other
    // single-threaded operations due to it being called repeteadly.
    //
    // NOTE: do we need this atomic? or we are just paranoid?
    int  read_error_state;
    char read_error_message[AV_ERROR_MAX_STRING_SIZE];
};

JNIEXPORT jlong JNICALL Java_bittu_jni_BittuplayerJNI_stream
  (JNIEnv * env, jclass cls, jstring url)
{
    int ret;

    struct bittuplayer* player = calloc(1, sizeof(player));
    if(!player) {
        return (long)NULL;
    }

    // TODO: restrict protocol coverage for security concerns.

    const char* c_url = (*env)->GetStringUTFChars(env, url, NULL);
    if(!c_url) {
        player->error_state = bittu_jni_BittuplayerJNI_INTERNAL_ERROR;
        strcpy(player->error_message, "No non-null URL was supplied");

        return (long)player;
    }

#define SET_DEMUXING_ERROR(IF_FREE) \
    switch(ret) { \
    case 0: break; \
\
        av_strerror(ret, player->error_message, AV_ERROR_MAX_STRING_SIZE); \
        if(IF_FREE) \
            avformat_close_input(&player->format_context); \
\
    case AVERROR_HTTP_BAD_REQUEST: \
    case AVERROR_HTTP_FORBIDDEN: \
    case AVERROR_HTTP_NOT_FOUND: \
    case AVERROR_HTTP_OTHER_4XX: \
    case AVERROR_HTTP_SERVER_ERROR: \
    case AVERROR_HTTP_UNAUTHORIZED: \
        player->error_state = bittu_jni_BittuplayerJNI_NETWORK_ERROR; \
        return (long)player; \
    default: \
        player->error_state = bittu_jni_BittuplayerJNI_DEMUXING_ERROR; \
        return (long)player; \
    }

    ret = avformat_open_input(&player->format_context, strdup(c_url), NULL, NULL);
    (*env)->ReleaseStringUTFChars(env, url, c_url);

    SET_DEMUXING_ERROR(0);

    player->demuxed_packet = av_packet_alloc();
    if(!player->demuxed_packet) {
        avformat_close_input(&player->format_context);

        player->error_state = bittu_jni_BittuplayerJNI_MEMORY_ERROR;
        strcpy(player->error_message, "Failed to allocate AVPacket");

        return (long)player;
    }

    ret = avformat_find_stream_info(player->format_context, NULL);
    SET_DEMUXING_ERROR(1);

    atomic_init(&player->position, player->format_context->start_time);

    AVCodec* stream_codec;

    player->audio_stream_index = av_find_best_stream(player->format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &stream_codec, 0);
    if(player->audio_stream_index < 0) {
        avformat_close_input(&player->format_context);

        player->error_state = bittu_jni_BittuplayerJNI_NO_AUDIO_STREAMS;
        return (long)player;
    }

    if(stream_codec->id != AV_CODEC_ID_OPUS) {
        avformat_close_input(&player->format_context);

        // TODO: add handling for non-OPUS files.
        player->error_state = bittu_jni_BittuplayerJNI_INTERNAL_ERROR;
        return (long)player;
    }

    player->error_state = bittu_jni_BittuplayerJNI_NO_ERROR;
    return (long)player;
}

JNIEXPORT jboolean JNICALL Java_bittu_jni_BittuplayerJNI_isStreamOpened
  (JNIEnv * env, jclass cls, jlong ptr)
{
    struct bittuplayer* player = (void*)ptr;

    return  player != NULL ||
            player->error_state == bittu_jni_BittuplayerJNI_NO_ERROR;
}

JNIEXPORT jint JNICALL Java_bittu_jni_BittuplayerJNI_getErrorState
  (JNIEnv * env, jclass cls, jlong ptr)
{
    return ((struct bittuplayer*)ptr)->error_state;
}

JNIEXPORT jstring JNICALL Java_bittu_jni_BittuplayerJNI_getErrorMessage
  (JNIEnv * env, jclass cls, jlong ptr)
{
    return (*env)->NewStringUTF(env, ((struct bittuplayer*)ptr)->error_message);
}

JNIEXPORT jlong JNICALL Java_bittu_jni_BittuplayerJNI_getDuration
  (JNIEnv * env, jclass cls, jlong ptr)
{
    return ((struct bittuplayer*)ptr)->format_context->duration;
}

JNIEXPORT jlong JNICALL Java_bittu_jni_BittuplayerJNI_getPosition
  (JNIEnv * env, jclass cls, jlong ptr)
{
    return atomic_load(&((struct bittuplayer*)ptr)->position);
}

JNIEXPORT void JNICALL Java_bittu_jni_BittuplayerJNI_pause
  (JNIEnv * env, jclass cls, jlong ptr)
{
    av_read_pause(((struct bittuplayer*)ptr)->format_context);
}

JNIEXPORT void JNICALL Java_bittu_jni_BittuplayerJNI_unpause
  (JNIEnv * env, jclass cls, jlong ptr)
{
    av_read_play(((struct bittuplayer*)ptr)->format_context);
}

JNIEXPORT jboolean JNICALL Java_bittu_jni_BittuplayerJNI_isSeekable
  (JNIEnv * env, jclass cls, jlong ptr)
{
    return ((struct bittuplayer*)ptr)->format_context->pb->seekable != 0;
}

JNIEXPORT void JNICALL Java_bittu_jni_BittuplayerJNI_seek
  (JNIEnv * env, jclass cls, jlong ptr, jlong ts)
{
    int ret;

    struct bittuplayer* player = (void*)ptr;

    // seeking needs not to be accurate.
    // https://ffmpeg.org/doxygen/trunk/ffplay_8c_source.html#l02853
    ret = avformat_seek_file(player->format_context,
                             player->audio_stream_index,
                             INT64_MIN,
                             ts,
                             INT64_MAX,
                             0);

    if(ret < 0)
        player->error_state = bittu_jni_BittuplayerJNI_SEEKING_ERROR;
    else
        player->error_state = bittu_jni_BittuplayerJNI_NO_ERROR;
}

JNIEXPORT jobject JNICALL Java_bittu_jni_BittuplayerJNI_read
  (JNIEnv * env, jclass cls, jlong ptr)
{
    int ret;

    struct bittuplayer* player = (void*)ptr;

    // at this moment it just demuxes OPUS packets, wraps the AVPacket::data
    // in a ByteBuffer doing .allocateDirect().
    do {
        ret = av_read_frame(player->format_context, player->demuxed_packet);
        if(ret < 0) {
            player->read_error_state = bittu_jni_BittuplayerJNI_STREAM_EOF;
            av_strerror(ret, player->read_error_message, AV_ERROR_MAX_STRING_SIZE);
            return NULL;
        }
    } while(player->demuxed_packet->stream_index != player->audio_stream_index);

    atomic_store(&player->position, player->demuxed_packet->dts);

    // assume the ByteBuffer will not be written.
    // this is potentially dangeours move, at the cost of performance.
    return (*env)->NewDirectByteBuffer(env,
                                       player->demuxed_packet->data,
                                       player->demuxed_packet->size);
}

JNIEXPORT void JNICALL Java_bittu_jni_BittuplayerJNI_readFinalize
  (JNIEnv * env, jclass cls, jlong ptr)
{
    // the ByteBuffer will be dangling after this option.
    // this is very dangerous, though its never used again
    // thus will be automatically collected by GC.
    av_packet_unref(((struct bittuplayer*)ptr)->demuxed_packet);
}

JNIEXPORT void JNICALL Java_bittu_jni_BittuplayerJNI_finalize
  (JNIEnv * env, jclass cls, jlong ptr)
{
    struct bittuplayer* player = (void*)ptr;

    av_packet_free(&player->demuxed_packet);
    avformat_close_input(&player->format_context);
}
