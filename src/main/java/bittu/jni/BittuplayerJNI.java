package bittu.jni;

import java.nio.ByteBuffer;

public class BittuplayerJNI {
	static {
		System.loadLibrary("bittuplayer");
	}

	public static final int NO_ERROR = 0;
	public static final int STREAM_EOF = -1;
	public static final int NO_AUDIO_STREAMS = -2;
	public static final int NETWORK_ERROR = -3;
	public static final int DEMUXING_ERROR = -4;
	public static final int DECODING_ERROR = -5;
	public static final int SEEKING_ERROR = -6;
	public static final int INTERNAL_ERROR = -7;
	public static final int RESAMPLER_ERROR = -8;
	public static final int OPUS_ERROR = -9;
	public static final int MEMORY_ERROR = -10;

	public static native long stream(String url);

	public static native int getErrorState(long ptr);

	public static native String getErrorMessage(long ptr);

	public static native boolean isStreamOpened(long ptr);

	public static native long getDuration(long ptr);

	public static native long getPosition(long ptr);

	public static native void pause(long ptr);

	public static native void unpause(long ptr);

	public static native boolean isSeekable(long ptr);

	public static native void seek(long ptr, long ts);

	public static native ByteBuffer read(long ptr);

	public static native void readFinalize(long ptr);

	public static native void finalize(long ptr);
}