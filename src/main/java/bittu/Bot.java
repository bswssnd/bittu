package bittu;

import net.dv8tion.jda.api.events.interaction.SlashCommandEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;

public class Bot extends ListenerAdapter {
	private static int joinInvokerChannel(SlashCommandEvent event) {
		var vc = event.getMember().getVoiceState().getChannel();
		if (vc == null) {
			event.reply("age thik kore voice chenle join kora shek").queue();
			return -1;
		}

		var manager = event.getGuild().getAudioManager();

		if (manager.getConnectedChannel() == null) {
			manager.openAudioConnection(vc);
			manager.setSelfDeafened(true);
			manager.setSendingHandler(new AudioPlayerWrapper());

			event.reply("Joined " + vc.getAsMention()).queue();
		} else {
			event.reply("Already joined" + vc.getAsMention()).queue();
		}

		return 0;
	}

	@Override
	public void onSlashCommand(SlashCommandEvent event) {
		switch (event.getCommandPath()) {
		case "play":
			if (joinInvokerChannel(event) == -1)
				return; // ensure we've joined a voice channel.

			var player = (AudioPlayerWrapper) event.getGuild().getAudioManager().getSendingHandler();

			try {
				player.stream(event.getOption("url").getAsString());
			} catch (Exception e) {
				e.printStackTrace();
			}
			break;

//		case "leave":
//			var vc = event.getGuild().getAudioManager().getConnectedChannel();
//			if (vc == null)
//				event.reply("age thik kore chok diye dekha shek").queue();
//
//			event.getGuild().getAudioManager().closeAudioConnection();
//			event.reply("Left " + vc.getAsMention()).queue();
//			break;
		}
	}
}
