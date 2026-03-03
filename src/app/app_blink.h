#ifndef APP_BLINK_H
#define APP_BLINK_H

#ifdef __cplusplus
extern "C"
{
#endif

    // 供 main.c 註冊 Task 用
    void vAppBlinkTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLINK_H */