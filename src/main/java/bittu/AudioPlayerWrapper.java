package bittu;

import java.nio.ByteBuffer;

import bittu.jni.BittuPlayerException;
import bittu.jni.BittuplayerJNI;
import net.dv8tion.jda.api.audio.AudioSendHandler;

public class AudioPlayerWrapper implements AudioSendHandler {
	protected long ptr; // pointer from JNI
	protected boolean isPlaying;
	protected ByteBuffer jdaPacket = ByteBuffer.allocate(65536); // 8KiB is enough

	public void stream(String url) throws Exception {
		if (isPlaying)
			BittuplayerJNI.finalize(ptr);
		ptr = BittuplayerJNI.stream(url);

		if (!BittuplayerJNI.isStreamOpened(ptr))
			BittuPlayerException.throwIfError(ptr);

		isPlaying = true;
	}

	@Override
	public boolean canProvide() {
		return isPlaying;
	}

//	public static ByteBuffer cloneBuffer(ByteBuffer original) {
//		ByteBuffer clone = ByteBuffer.allocate(original.capacity());
//		original.rewind();// copy from the beginning
//		clone.put(original);
//		original.rewind();
//		clone.flip();
//		return clone;
//	}

	@Override
	public ByteBuffer provide20MsAudio() {
		var packet = BittuplayerJNI.read(ptr);
		if (packet == null) {
			BittuplayerJNI.finalize(ptr);

			isPlaying = false;
		} else {
			packet.rewind();

			jdaPacket.rewind();
			jdaPacket.limit(jdaPacket.capacity());
			jdaPacket.put(packet);
			jdaPacket.flip();

			BittuplayerJNI.readFinalize(ptr);
		}

		return jdaPacket;
	}

	@Override
	public boolean isOpus() {
		return true;
	}
}
