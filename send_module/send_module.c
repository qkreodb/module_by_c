#include "shared.h"
#include <arpa/inet.h>

void* send_module(void* arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in watch_addr;

    while (1) {
        // q_send에서 이벤트가 나올 때까지 대기(Pop)
        SensorPacket *packet = (SensorPacket*)q_pop(&q_send);
        
        if (packet->type == TYPE_EVENT) {
            memset(&watch_addr, 0, sizeof(watch_addr));
            watch_addr.sin_family = AF_INET;
            watch_addr.sin_port = htons(5006);
            
            // 중요: 실제 운영 시에는 sen_id에 매핑된 IP를 찾는 로직이 필요합니다.
            // 테스트용으로 특정 IP를 지정하거나 구조체에 IP 필드를 추가하는 걸 추천해요.
            watch_addr.sin_addr.s_addr = inet_addr("192.168.0.15"); 

            char feedback[128];
            snprintf(feedback, sizeof(feedback), "{\"state_code\": \"%s\"}", 
                     packet->payload.event.state_code);

            sendto(sock, feedback, strlen(feedback), 0, (struct sockaddr*)&watch_addr, sizeof(watch_addr));
            printf("[Send] 워치로 제어 명령 전송: %s\n", feedback);
        }
        free(packet); // 사용이 끝난 패킷 메모리 해제
    }
    return NULL;
}
