#ifndef PAGE_BASE_H
#define PAGE_BASE_H

#include "core/config.h"

class PageBase {
public:
    virtual void enter(Page from) {}  // 页面进入时调用:  from代表从那个页面来
    virtual void exit() {}            // 页表退出时调用
    virtual void update() = 0;        // 页面更新时调用

    virtual void onBtn1Short() {}     // 按钮1短按
    virtual void onBtn2Short() {}     // 按钮2短按
    virtual void onBtn3Short() {}     // 按钮3短按
    virtual void onBtn1Long()  {}     // 按钮1长按
    virtual void onBtn2Long()  {}     // 按钮2长按
    virtual void onBtn3Long()  {}     // 按钮3长按

    virtual ~PageBase() = default;    // 析构函数
};

#endif
