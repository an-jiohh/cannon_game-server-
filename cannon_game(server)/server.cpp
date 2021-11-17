#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable:4996)

#define  PORT 5500
#define BUFFERSIZE 100
#define NAMESIZE 30
HANDLE  mutex;
int clientNumber = 0;
unsigned  threadID;
int allClientSocket[100];

class THD {
public:
    int Soc_Ser();
};
int THD::Soc_Ser()
{
    mutex = CreateMutex(NULL, FALSE, NULL); //뮤텍스 생성

    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata); //서버 소켓 생성

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    if (listen(serverSocket, 5) == -1) { //서버 listen(대기모드)
        cout << "서버 소켓을 listen 모드로 설정하는데 실패했습니다" << endl;
        return 0;
    }
    cout << "서버가 실행 되었습니다." << endl;
    cout << "**********전체 대화 내용***********" << endl;

    while (1) {

        SOCKADDR_IN clientAddress; //클라이언트 구조체 선언

        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        WaitForSingleObject(mutex, INFINITE); //뮤텍스 시작

        clientNumber++;
        allClientSocket[clientNumber - 1] = clientSocket;

        ReleaseMutex(mutex); //뮤텍스 해제
        /*클라이언트에게 전달할 환영 메시지를 저장할 배열을 선언했습니다.*/
        char greetMessage[BUFFERSIZE]; //메시지 저장 배열 선언
        sprintf(greetMessage, "[서버]환영합니다. 대화명을 입력해 주세요\n"); //메시지 전달
        send(clientSocket, greetMessage, sizeof(greetMessage), 0);
        /*클라이언트에서 전송한 메시지를 읽어 들여서 모든 클라이언트로 전송하는 broadcastAllClient 함수를 호출합니다.*/
    }
    return 0;
}

using namespace std;

class map {
public:
	void print_map();
protected:
	int num[10][10] = { NULL };
private:
};

void map::print_map() {
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			if (num[i][j] == 0) cout << "□";
		}
		cout << endl;
	}
}

int main() {
    THD cla;
    cla.Soc_Ser();

	return 0;
}