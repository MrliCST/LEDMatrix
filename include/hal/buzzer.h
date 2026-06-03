#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
public:
    static Buzzer& instance();  // 获取单例

    void begin();   // 初始化
    void playSong(int index, bool loop = false);  // 播放指定铃声
    void stop();    // 停止
    
    // 是否正在播放
    bool isPlaying() const { 
        return _playing; 
    }

private:
    Buzzer() = default;
    bool _playing = false;
};

#endif
