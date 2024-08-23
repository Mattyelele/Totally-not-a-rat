#include <discord_game_sdk/discord.h>
#include <iostream>
#include "globals.h"

struct DiscordState {
    discord::User currentUser;

    std::unique_ptr<discord::Core> core;
};

namespace {
    volatile bool interrupted{ false };
}

DiscordState state{};
discord::Core* core{};


void InitDiscord() {
    auto result = discord::Core::Create(1264994351369425109, DiscordCreateFlags_Default, &core);
    state.core.reset(core);
    if (!state.core) {
        std::cout << "Failed to instantiate discord core! (err " << static_cast<int>(result)
            << ")\n";
        std::exit(-1);
    }
}

void RunCallbacks() {
    core->RunCallbacks();
}

void UpdateDiscordPresence(const std::string& state, const std::string& details, const std::string imgId) {
    if (core) {
        discord::Activity activity{};
        activity.SetState(state.c_str());
        activity.SetDetails(details.c_str());
        activity.GetAssets().SetLargeImage(imgId.c_str());
        activity.GetAssets().SetSmallImage("20220827_102222");
        activity.GetAssets().SetSmallText("Totally not a rat overlay @ https://github.com/Mattyelele/Totally-not-a-rat");
        core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            if (result == discord::Result::Ok) {
                std::cout << "Successfully updated presence" << std::endl;
            }
            else {
                std::cerr << "Failed to update presence: " << static_cast<int>(result) << std::endl;
            }
            });
    }
    else {
        std::cerr << "Core not initialized" << std::endl;
    }
}

