#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <conio.h>

using namespace std;

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable:4996)

#define PORT 5500
#define BUFFERSIZE 100
#define NAMESIZE 30
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77
#define SPACE 32

HANDLE  mutex;
int clientNumber = 0;
unsigned  threadID;
int allClientSocket[3];

int num[10][10] = { NULL };

class map {
public:
    void create_hurdle(int y, int x);  //장애물 생성 함수(2*2)형성
    void print_map(); //맵을 표시하는 함수
    void moving_map(); //맵의 요소들의 이동 처리 함수
    virtual int main_func() = 0; //맵의 요소를 구동할 추상메소드 지정 
};

void map::create_hurdle(int y, int x) { //장애물 생성 함수(2*2)형성
    num[y][x] = 5;
    num[y][x + 1] = 5;
    num[y + 1][x] = 5;
    num[y + 1][x + 1] = 5;
}

void map::print_map() { //맵을 표시하는 함수
    for (int i = 0; i < 10; i++) { 
        for (int j = 0; j < 10; j++) {
            int temp = num[i][j] - (num[i][j] / 100) * 100;
            if (temp / 10 == 1) cout << "▲";
            else if (temp / 10 == 2) cout << "▼";
            else if (temp / 10 == 3) cout << "◀";
            else if (temp / 10 == 4) cout << "▶";
            else if (temp == 5) cout << "■";
            else if (temp == 6) cout << "▣";
            else cout << "□";
        }
        cout << endl;
    }
}

void map::moving_map() { //맵의 요소들의 이동을 처리한다.
    int movecount = 1;
    while (movecount <= clientNumber) {
        int temp = 0;
        int player = 0;
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (num[i][j] / 100 == movecount) {
                    int tens_temp = (num[i][j] - (num[i][j] / 100) * 100); //tens_temp에 10 + 1의자리 대입
                    int ones_temp = tens_temp - (tens_temp / 10) * 10; //ones_temp에 1의 자리 대입
                    tens_temp = (tens_temp / 10) * 10; //tens_temp에 10의자리 대입

                    if (ones_temp == 0) { //1의 자리가 0일 경우 이동
                        if (tens_temp == 10) { //UP
                            if (i != 0) {
                                if (num[i - 1][j] == 0) {
                                    num[i - 1][j] = num[i][j];
                                    num[i][j] = 0;
                                }
                            }
                        }
                        else if (tens_temp == 20) { //DOWN
                            if (i != 9) {
                                if (num[i + 1][j] == 0) {
                                    num[i + 1][j] = num[i][j];
                                    num[i][j] = 0;
                                }
                            }
                        }
                        else if (tens_temp == 30) { //LEFT
                            if (j != 0) {
                                if (num[i][j - 1] == 0) {
                                    num[i][j - 1] = num[i][j];
                                    num[i][j] = 0;
                                }
                            }
                        }
                        else if (tens_temp == 40) { //RIGHT
                            if (j != 9) {
                                if (num[i][j + 1] == 0) {
                                    num[i][j + 1] = num[i][j];
                                    num[i][j] = 0;
                                }
                            }
                        }
                    }
                    movecount++;
                    temp++;
                }
            }
        }
        player += temp;
        if (temp == 0)movecount++; //해당 플레이어가 없을시 패스
    }
}

class THD{
public:
    int start_server(int playernum); //서버 시작 함수
    void join_client(); //클라이언트 연결 함수
    static unsigned __stdcall receive(void* arg); //각 클라이언트로부터 입력을 받음
    virtual int main_func() = 0; //서버 연결을 구동할 추상메소드 지정
protected:
    SOCKET serverSocket;
};

int THD::start_server(int playernum) { //서버 시작 함수
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata); //서버 소켓 생성

    mutex = CreateMutex(NULL, FALSE, NULL); //뮤텍스 생성

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    cout << "서버 소켓을 생성 했습니다." << endl;

    SOCKADDR_IN  serverAddress; //서버 구조체 선언
    ZeroMemory(&serverAddress, sizeof(serverAddress)); //서버 구조체 초기화

    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);//localhost 입력
    serverAddress.sin_family = AF_INET; // TCP/IP선택
    serverAddress.sin_port = htons(PORT); //포트번호

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) { //서버 소켓 바인딩
        cout << "서버 소켓을 바인드 하는데 실패했습니다." << endl;
        return 0;
    }

    if (listen(serverSocket, playernum) == -1) { //서버 listen(대기모드)
        cout << "서버 소켓을 listen 모드로 설정하는데 실패했습니다" << endl;
        return 0;
    }
    cout << "서버가 실행 되었습니다." << endl;
    return 1;
}

void THD::join_client() { //클라이언트 연결 함수
    SOCKADDR_IN clientAddress; //클라이언트 구조체 선언

    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

    WaitForSingleObject(mutex, INFINITE); //뮤텍스 시작
    allClientSocket[clientNumber] = clientSocket;
    clientNumber++;
    ReleaseMutex(mutex); //뮤텍스 해제

    char greetMessage[BUFFERSIZE]; //메시지 저장 배열 선언
    sprintf(greetMessage, "[서버]환영합니다\n"); //메시지 전달
    send(clientSocket, greetMessage, sizeof(greetMessage), 0);

    unsigned long thread;
    thread = _beginthreadex(NULL, 0, receive, (void*)clientSocket, 0, &threadID); //각 클라이언트부터 메시지를 받는 쓰레드 실행
}

unsigned __stdcall THD::receive(void* arg) {
    SOCKET myClientSocket = (SOCKET)arg;

    int Clientnum[1] = { 0 };
    Clientnum[0] = clientNumber;
    cout << Clientnum[0] << "번째 플레이어 입장" << endl;

    Clientnum[0] = htonl(Clientnum[0]);
    send(myClientSocket, (char*)Clientnum, sizeof(Clientnum), 0); //클라이언트 자신의 정보 전송
    Clientnum[0] = htonl(Clientnum[0]);

    int fromClient[1]; //키보드 입력 값 저장 배열

    while (1) {
        int readlen = recv(myClientSocket, (char*)fromClient, sizeof(fromClient), 0); //키보드 입력 받음
        if (readlen < 0) break; //플레이어가 나갈경우 반복문 종료
        fromClient[0] = htonl(fromClient[0]);

        //키보드 입력값에 따라 쓰레드가 담당하는 플레이어의 상태값을 바꾼다.
        WaitForSingleObject(mutex, INFINITE);
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (num[i][j] / 100 == Clientnum[0])
                {
                    int tens_temp = (num[i][j] - (num[i][j] / 100) * 100); //tens_temp에 10 + 1의자리 대입
                    int ones_temp = tens_temp - (tens_temp / 10) * 10; //ones_temp에 1의 자리 대입
                    tens_temp = (tens_temp / 10) * 10; //tens_temp에 10의자리 대입

                    if (fromClient[0] == SPACE) { //대포 발사
                        if (tens_temp == 10) { //위
                            for (int k = 1; k < 4; k++) {
                                if (i - k >= 0) {
                                    if (num[i - k][j] != 5) {
                                        num[i - k][j] = 6;
                                    }
                                    else break;
                                }
                                else break;
                            }
                        }
                        else if (tens_temp == 20) { //아래
                            for (int k = 1; k < 4; k++) {
                                if (i + k <= 9) {
                                    if (num[i + k][j] != 5) {
                                        num[i + k][j] = 6;
                                    }
                                    else break;
                                }
                                else break;
                            }
                        }
                        else if (tens_temp == 30) { //왼쪽
                            for (int k = 1; k < 4; k++) {
                                if (j - k >= 0) {
                                    if (num[i][j - k] != 5) {
                                        num[i][j - k] = 6;
                                    }
                                    else break;
                                }
                                else break;
                            }
                        }
                        else if (tens_temp == 40) { //아래
                            for (int k = 1; k < 4; k++) {
                                if (j + k <= 9) {
                                    if (num[i][j + k] != 5) {
                                        num[i][j + k] = 6;
                                    }
                                    else break;
                                }
                                else break;
                            }
                        }
                    }
                    else if (fromClient[0] == 83 || fromClient[0] == 115) { // 멈춤
                        if (ones_temp == 0)  num[i][j]++;
                        else  num[i][j]--;
                    }
                    else if (fromClient[0] == UP) { //위로 이동
                        num[i][j] -= tens_temp;
                        num[i][j] += 10;
                    }
                    else if (fromClient[0] == DOWN) { //아래로 이동
                        num[i][j] -= tens_temp;
                        num[i][j] += 20;
                    }
                    else if (fromClient[0] == LEFT) { //왼쪽으로 이동
                        num[i][j] -= tens_temp;
                        num[i][j] += 30;
                    }
                    else if (fromClient[0] == RIGHT) { //오른쪽로 이동
                        num[i][j] -= tens_temp;
                        num[i][j] += 40;
                    }
                }
            }
        }
        ReleaseMutex(mutex);
    }
    //종료한 소켓 제거 부분
    WaitForSingleObject(mutex, INFINITE);//뮤텍스 동기화 시작
    int i, j;
    for (i = 0; i < clientNumber; i++) { //제거할 소켓을 찾음
        if (allClientSocket[i] == myClientSocket) { //제거할 소켓 일 경우
            j = i;
            while (j < clientNumber - 1) {
                allClientSocket[j] = allClientSocket[j + 1];
                j++;
            }
            break;
        }
    }
    clientNumber--;//전체 클라이언트의 수를 1감소 시킴
    ReleaseMutex(mutex);//뮤텍스 동기화 종료
    closesocket(myClientSocket);//현재 클라이언트와 연결된 소켓을 종료함
    return 0;
}

class main_CLA :public map, THD {
public:
    main_CLA();
    int main_func();
protected:
private:
};

main_CLA::main_CLA() {
    create_hurdle(3, 3);
    create_hurdle(2, 6);
    create_hurdle(6, 1);
    create_hurdle(7, 6);

    num[0][0] = 120;
    num[9][0] = 230;
    num[0][9] = 310;
}

int main_CLA::main_func() {
    int playernum = 0;
    cout << "플레이할 인원을 입력해 주세요 : ";
    cin >> playernum;

    if (start_server(playernum) == 0) return 0; //서버함수(실패하면 종료)

    while (clientNumber < playernum) { //플레이 인원만큼 client와 연결
        join_client();

    }

    Sleep(100); //마지막 플레이어가 접속할때까지 Sleep
    cout << "모든 플레이어가 접속하였습니다." << endl;

    cout << "탱크의 이동속도를 입력해주세요" << endl; //탱크 이동속도 조절
    cout << "탱크의 이동속도 : ";
    int gamespeed;
    cin >> gamespeed;

    while (1) { //Y를 누를때 까지 대기
        cout << "Y를 누르면 게임을 시작합니다." << endl;
        int in = _getch();
        if (in == 89 || in == 121) break;
    }

    int fog = 0; //안개모드 설정
    while (1) { //맵을 브로드 캐스팅
        WaitForSingleObject(mutex, INFINITE); //뮤텍스 잠금

        moving_map(); //맵 플레이어들의 위치를 이동시킴

        if (_kbhit()) { //안개모드 번경
            int in = _getch();
            if (in == 77 || in == 109) {
                if (fog == 1) fog = 0;
                else fog = 1;
            }
        }

        system("cls");
        print_map(); //서버에 맵 표시

        cout << "m을 누르면 안개모드를 변경합니다. 현재모드 : ";
        if (fog == 1) cout << "안개모드" << endl;
        else cout << "비안개모드" << endl;

        int snum[101]; //전송용 배열
        for (int i = 0; i < 10; i++) { //빅엔디안 정렬
            for (int j = 0; j < 10; j++) {
                snum[i * 10 + j] = htonl(num[i][j]);
                if (num[i][j] == 6) num[i][j] = 0;
            }
        }
        snum[100] = htonl(fog); //배열의 마지막에 안개/비안개 모드 구별 코드입력

        for (int i = 0; i < clientNumber; i++) { // 전체 플레이어에게 전송
            send(allClientSocket[i], (char*)snum, sizeof(snum), 0);
        }
        ReleaseMutex(mutex); //뮤텍스 해제
        Sleep(gamespeed);
    }
    return 0;
}

int main() {
    main_CLA cla;

    cla.main_func();

	return 0;
}