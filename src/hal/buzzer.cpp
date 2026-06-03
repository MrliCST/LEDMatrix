#include "hal/buzzer.h"
#include "core/config.h"
#include <songs/songs.h>

// 后台任务句柄
static TaskHandle_t _buzzerTask = nullptr;

// 给子线程的任务函数
static void _playTask(void* param) {
    int songIdx = (int)param;
    if (songIdx < 0 || songIdx >= SONG_COUNT) {
        vTaskDelete(nullptr);
        return;
    }

    const SongInfo& song = SONG_LIST[songIdx];
    for (int i = 0; i < song.length; i++) {
        int note = song.notes[i];
        int duration = song.durations[i];
        // ledcWriteTone(channel, note);   // 播放音符
        // vTaskDelay(duration * 10 / portTICK_PERIOD_MS);
        // ledcWriteTone(channel, 0);       // 停止
    }

    Buzzer::instance().stop();
    vTaskDelete(nullptr);
}

// 获取单例
Buzzer& Buzzer::instance() {
    static Buzzer b;  // 单例
    return b;
}

// 初始化
void Buzzer::begin() {
    // ledcSetup / ledcAttachPin for PWM
}

// 播放音乐
void Buzzer::playSong(int index, bool loop) {
    stop(); _playing = true;
    xTaskCreate(_playTask, "buzzerTask", 2000, (void*)index, 1, &_buzzerTask);  // 利用FREERTOS创建子进程, 设置句柄
}

// 停止播放
void Buzzer::stop() {
    if (_buzzerTask) { 
        vTaskDelete(_buzzerTask); 
        _buzzerTask = nullptr; 
    }
    _playing = false;
}
