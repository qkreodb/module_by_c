#include <pthread.h>
#include <stdio.h>
#include "vital_module.h"

int main() {
    pthread_t watch_thread;

    printf("메인 시스템 가동 중...\n");

    // 워치 수집 모듈을 개별 스레드로 실행 (문맥 교환 대상)
    if (pthread_create(&watch_thread, NULL, watch_collector_run, NULL) != 0) {
        perror("스레드 생성 실패");
        return 1;
    }

    // 메인은 여기서 다른 작업을 수행하거나 대기합니다.
    pthread_join(watch_thread, NULL);

    return 0;
}
