#include <stdbool.h>
#include <stdint.h>
void app_diag_report_event(uint16_t dtc_id, bool failed)
{
    // 在其它模組的測試中，我們不關心診斷機制的具體行為
    (void)dtc_id;
    (void)failed;
}