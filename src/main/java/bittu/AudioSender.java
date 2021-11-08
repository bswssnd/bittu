package bittu;

import java.net.DatagramSocket;
import java.util.concurrent.ConcurrentMap;

import javax.annotation.CheckForNull;

import org.slf4j.MDC;

import net.dv8tion.jda.api.audio.factory.IAudioSendSystem;
import net.dv8tion.jda.api.audio.factory.IPacketProvider;

public class AudioSender implements IAudioSendSystem {
	private IPacketProvider packetProvider;
	private Thread sender;
	private ConcurrentMap<String, String> contextMap;

	public AudioSender(IPacketProvider packetProvider) {
		this.packetProvider = packetProvider;
	}

	@Override
	public void setContextMap(@CheckForNull ConcurrentMap<String, String> contextMap) {
		this.contextMap = contextMap;
	}

	@Override
	public void start() {
		final DatagramSocket socket = packetProvider.getUdpSocket();

		if (contextMap != null)
			MDC.setContextMap(contextMap);

		sender = new Thread(() -> {
			long lastFrameSent = 0;
		});

	}

	@Override
	public void shutdown() {
		// TODO Auto-generated method stub

	}
}