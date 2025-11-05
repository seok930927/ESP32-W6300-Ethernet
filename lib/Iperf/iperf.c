#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "iperf.h"

// 함수 포인터 선언 (정적 변수로 안전하게 관리)
static uint32_t (*get_time_us_ptr)(void) = NULL;

// 라이브러리 초기화 함수 - 외부에서 시간 함수를 등록
int iperf_init(uint32_t (*time_func)(void)) {
    if (time_func == NULL) {
        printf("Error: time function pointer is NULL\n");
        return -1;
    }
    
    get_time_us_ptr = time_func;
    printf("iPerf library initialized with custom timer function\n");
    return 0;
}

// 내부에서 사용할 시간 함수 (함수 포인터 래퍼)
static uint32_t get_time_us(void) {
    if (get_time_us_ptr == NULL) {
        printf("Error: Timer function not initialized! Call iperf_init() first\n");
        return 0;
    }
    return get_time_us_ptr();
}

void iperf_stats_init(Stats *stats, uint32_t pacing_timer_ms) {
    stats->pacing_timer_us = pacing_timer_ms * 1000;
    stats->running = false;
    stats->t0 = 0;
    stats->t1 = 0;
    stats->t3 = 0;
    stats->nb0 = 0;
    stats->nb1 = 0;
    stats->np0 = 0;
    stats->np1 = 0;
}

void iperf_stats_start(Stats *stats) {
    stats->running = true;
    stats->t0 = stats->t1 = get_time_us();
    stats->nb0 = stats->nb1 = 0;
    stats->np0 = stats->np1 = 0;
    printf("Interval           Transfer     Bitrate\n");
}

void iperf_stats_update(Stats *stats, bool final) {
    if (!stats->running) {
        return;
    }

    uint32_t t2 = get_time_us();
    uint32_t dt = t2 - stats->t1;  // Elapsed time since last update

    if (final || dt > stats->pacing_timer_us) {
        double ta = (stats->t1 - stats->t0) / 1e6;  // Start time of the previous interval
        double tb = (t2 - stats->t0) / 1e6;         // End time of the current interval
        double transfer_mbits = (stats->nb1 * 8) / 1e6 / (dt / 1e6);  // Calculate Mbps

#ifdef IPERF_DEBUG
        printf("%5.2f-%-5.2f sec %8u Bytes  %5.2f Mbits/sec\n",
               ta, tb, stats->nb1, transfer_mbits);
#endif

        stats->t1 = t2;  // Update the timer
        stats->nb1 = 0;  // Reset byte count per interval
        stats->np1 = 0;  // Reset packet count per interval
    }
}

void iperf_stats_stop(Stats *stats) {
    stats->running = false;

    stats->t3 = get_time_us();
    uint32_t total_time_us = stats->t3 - stats->t0;
    double total_time_s = total_time_us / 1e6;
    double transfer_mbits = (stats->nb0 * 8) / 1e6 / total_time_s;

    printf("------------------------------------------------------------\n");
    printf("Total: %5.2f sec %8lu Bytes  %5.2f Mbits/sec\n",
           total_time_s, stats->nb0, transfer_mbits);
}

void iperf_stats_add_bytes(Stats *stats, uint32_t n) {
    if (!stats->running) {
        return;
    }

    stats->nb0 += n;  // Increase total byte count
    stats->nb1 += n;  // Increase byte count per interval
    stats->np0 += 1;  // Increase total packet count
    stats->np1 += 1;  // Increase packet count per interval

}