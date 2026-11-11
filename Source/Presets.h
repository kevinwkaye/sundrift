#pragma once
#include <array>

namespace sundrift {
// Artist-inspired interpretations, not artist-endorsed tone captures.
struct BandInfo {
    const char* name;
    const char* shortName;
    const char* descriptor;
    unsigned int colour;
};
inline constexpr std::array<BandInfo, 4> bands {{
    {"BALTHVS", "BALTHVS", "SUNLIT PSYCHEDELIA", 0xffe9ad54},
    {"KHRUANGBIN", "KHRUANGBIN", "SILK & REVERB", 0xff72bca9},
    {"GLASS BEAMS", "GLASS BEAMS", "MIRAGE GROOVES", 0xffab99dc},
    {"YUUF", "YUUF", "SLOW-MOTION COLOUR", 0xff85bee0}
}};
struct PresetRecipe {
    const char* name;
    const char* character;
    std::array<float, 18> values;
};
// Param order: input,compress,focus,drive,edge,body,tone,phaseRate,phase,chorus,wobble,
// echoTime,feedback,echoMix,space,decay,width,output.
inline constexpr std::array<PresetRecipe, 12> presets {{
    {"Sunday Forecast", "Hazy clean lead | elastic echoes",    {1.f, 0.31f, 0.46f, 0.28f, 0.60f, 0.57f, 0.54f, 0.22f, 0.13f, 0.32f, 0.20f, 420.f, 0.22f, 0.22f, 0.46f, 0.74f, 0.82f, -2.f}},
    {"Golden Cumbia", "Warm, bouncy melodic lines",            {1.f, 0.48f, 0.58f, 0.32f, 0.67f, 0.62f, 0.59f, 0.32f, 0.27f, 0.19f, 0.12f, 330.f, 0.18f, 0.12f, 0.31f, 0.52f, 0.74f, -2.f}},
    {"Lunar Coast", "Slow psychedelic drift",                  {0.f, 0.25f, 0.34f, 0.29f, 0.42f, 0.65f, 0.43f, 0.15f, 0.36f, 0.51f, 0.40f, 540.f, 0.38f, 0.30f, 0.66f, 0.79f, 0.92f, -3.f}},
    {"Silk Lines", "Expressive compressed single notes",       {0.f, 0.53f, 0.78f, 0.22f, 0.34f, 0.47f, 0.59f, 0.16f, 0.11f, 0.07f, 0.04f, 360.f, 0.19f, 0.08f, 0.42f, 0.56f, 0.54f, -2.f}},
    {"Desert Dub", "Percussive picking, restrained dub echo",   {-1.f, 0.37f, 0.66f, 0.28f, 0.56f, 0.52f, 0.63f, 0.11f, 0.12f, 0.05f, 0.08f, 470.f, 0.45f, 0.29f, 0.30f, 0.50f, 0.65f, -2.f}},
    {"Texas Velvet", "Sweet, dry, intimate guitar",           {-2.f, 0.62f, 0.71f, 0.13f, 0.24f, 0.53f, 0.68f, 0.08f, 0.03f, 0.04f, 0.03f, 290.f, 0.10f, 0.05f, 0.22f, 0.41f, 0.44f, -1.f}},
    {"Mirage Pulse", "Resonant desert phaser and dark repeats",{-2.f, 0.20f, 0.59f, 0.53f, 0.65f, 0.50f, 0.62f, 0.39f, 0.68f, 0.25f, 0.28f, 480.f, 0.38f, 0.31f, 0.43f, 0.65f, 0.94f, -3.50f}},
    {"Temple Steps", "Rhythmic phase haze and bite",           {-1.f, 0.34f, 0.60f, 0.50f, 0.73f, 0.57f, 0.67f, 0.53f, 0.82f, 0.12f, 0.16f, 300.f, 0.37f, 0.22f, 0.31f, 0.50f, 0.73f, -3.50f}},
    {"Copper Serpent", "Hypnotic melodic sustain",              {0.f, 0.41f, 0.75f, 0.41f, 0.53f, 0.62f, 0.47f, 0.28f, 0.58f, 0.27f, 0.22f, 500.f, 0.31f, 0.25f, 0.48f, 0.74f, 0.88f, -3.f}},
    {"Afterglow", "Open chorus and late-night trails",         {0.f, 0.19f, 0.37f, 0.16f, 0.19f, 0.62f, 0.43f, 0.12f, 0.24f, 0.59f, 0.47f, 620.f, 0.43f, 0.34f, 0.73f, 0.89f, 0.99f, -3.f}},
    {"Coastal Float", "Airy, glassy and softly modulated",      {-1.f, 0.15f, 0.39f, 0.11f, 0.16f, 0.52f, 0.61f, 0.17f, 0.16f, 0.64f, 0.35f, 740.f, 0.35f, 0.30f, 0.64f, 0.80f, 0.97f, -3.f}},
    {"Late Drift", "Dark atmospheric echoes",                  {-2.f, 0.19f, 0.27f, 0.23f, 0.30f, 0.69f, 0.35f, 0.10f, 0.28f, 0.43f, 0.61f, 680.f, 0.51f, 0.46f, 0.79f, 0.91f, 0.95f, -4.f}}
}};
inline constexpr int presetsPerBand = 3;
inline constexpr int bandForPreset(int preset) { return preset / presetsPerBand; }
}
