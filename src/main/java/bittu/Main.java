package bittu;

import javax.security.auth.login.LoginException;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.interactions.commands.OptionType;
import net.dv8tion.jda.api.interactions.commands.build.CommandData;
import net.dv8tion.jda.api.utils.MemberCachePolicy;

public class Main {
	private static final String[] TEST_GUILDS = { "678263205562286112", "726692780025315329", "771299961870221323",
			"852111233532428319" };

	public static void main(String[] args) throws LoginException, InterruptedException {
		var bot = JDABuilder.createDefault(args[0]).setBulkDeleteSplittingEnabled(false)
				.setMemberCachePolicy(MemberCachePolicy.VOICE).addEventListeners(new Bot()).build();

		bot.awaitReady();

		for (var guildId : TEST_GUILDS) {
			var guild = bot.getGuildById(guildId);

			if (guild != null)
				guild.updateCommands().addCommands(new CommandData("play", "Enqueue a song for decoding")
						.addOption(OptionType.STRING, "url", "URL to play", true)).queue();
		}

	}
}
