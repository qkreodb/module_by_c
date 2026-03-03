#include <modbus/modbus.h>
#include <errno.h>
#include <unistd.h>
#include "shared.h"

// 센서 설정 (기존 설정 유지)
static const int SLAVE_ID = 1;
static const int REG_ADDR = 0;
static const int REG_CNT  = 2;
static modbus_t *g_ctx = NULL;

// 센서 고유 정보 (DB/식별용)
static const int TH_SEN_ID = 201; // 온습도 센서 ID 예시
static const int TH_WP_ID  = 1;   // 1번 작업장

// 내부 복구 로직 함수들 (기존 코드와 동일하므로 구현 생략, 로직 내에서 호출됨)
static int _soft_reconnect(void);
static int _hard_recreate(const char* ip, int port);

// 메인에서 스레드로 실행할 함수
void* th_module(void* arg) {
    printf("[TH_Module] 모듈 가동 시작 (Modbus TCP)\n");

    // 1. 초기 연결 (센서 IP는 실제 환경에 맞춰 수정하세요)
    const char* sensor_ip = "192.168.0.20"; 
    int sensor_port = 502;

    g_ctx = modbus_new_tcp(sensor_ip, sensor_port);
    modbus_set_slave(g_ctx, SLAVE_ID);
    
    if (modbus_connect(g_ctx) == -1) {
        printf("⚠️ [TH] 초기 연결 실패, 복구 루틴 대기\n");
    }

    uint16_t reg[REG_CNT];

    while (1) {
        // 2. Modbus 데이터 읽기
        int rc = modbus_read_input_registers(g_ctx, REG_ADDR, REG_CNT, reg);

        if (rc != REG_CNT) {
            printf("⚠️ [TH] 읽기 실패, 복구 시도...\n");
            _soft_reconnect(); // 기존의 복구 로직 호출
            sleep(1);
            continue;
        }

        // 3. 스케일링 및 데이터 확정
        float t = reg[0] / 10.0f;
        float h = reg[1] / 10.0f;

        // 4. 통합 구조체(SensorPacket) 생성 및 큐 삽입
        SensorPacket *packet = (SensorPacket*)malloc(sizeof(SensorPacket));
        packet->type = TYPE_TH;
        packet->payload.th.sen_id = TH_SEN_ID;
        packet->payload.th.wp_id  = TH_WP_ID;
        packet->payload.th.temp   = t;
        packet->payload.th.humd   = h;
        packet->payload.th.time   = time(NULL);

        q_push(&q_th, packet); // shared.h에 정의된 q_th로 전송

        printf("🌡️ [TH] Temp: %.1f, Humd: %.1f (Queue 전송 완료)\n", t, h);

        // 수집 주기 조절 (예: 5초)
        sleep(5);
    }

    if (g_ctx) {
        modbus_close(g_ctx);
        modbus_free(g_ctx);
    }
    return NULL;
}
