#include "AudioPlayer.hpp"
#include "SoundLibrary.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>

namespace audio {

namespace {
    void play(const char* path) {
        PlaySoundA(path, NULL, SND_FILENAME | SND_ASYNC);
    }
}

void AudioPlayer::playMove() {
    play(SoundLibrary::MOVE_SOUND);
}

void AudioPlayer::playCapture() {
    play(SoundLibrary::CAPTURE_SOUND);
}

void AudioPlayer::playGameOver() {
    play(SoundLibrary::GAME_OVER_SOUND);
}

}
