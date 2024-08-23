#ifdef DISCORDGAMECORE_H

void InitDiscord();
void UpdateDiscordPresence(const std::string& state, const std::string& details, const std::string imgId);
void RunCallbacks();

#endif