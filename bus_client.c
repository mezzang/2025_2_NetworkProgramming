#define MAX_CLNT 3
#define ROWS 2
#define COLS 10
#define MAX_SEATS (ROWS *COLS)
// cmd 값
#define RESERVE_START 0 // 예약 시작: Server -> Client
#define RESERVE_REQ 1 // 예약 요청: Client -> Server
#define RESERVE_RES 2 // 예약 결과: Server -> Client
#define RESERVE_END 3 // 예약 종료: 모든 자리가 예약된 경우 Server -> Client
// result 값
#define SUCCESS 0 // 빈 좌석을 예약한 경우
#define OCCUPIED 1 // 다른 클라이언트가 이미 예약한 경우
#define CANCEL_BROKER_SEAT 2 // 암표상이 예약한 좌석을 사이버 수사대가 변경
#define PASS 3 // 사이버 수사대는 빈 좌석은 건너뜀

typedef struct
{
    int cmd;
    int seatno; 
    int seats[ROWS][COLS];
    int result;
} PACKET;