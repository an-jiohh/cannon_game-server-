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
    mutex = CreateMutex(NULL, FALSE, NULL); //���ؽ� ����

    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata); //���� ���� ����

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    cout << "���� ������ ���� �߽��ϴ�." << endl;

    SOCKADDR_IN  serverAddress; //���� ����ü ����
    ZeroMemory(&serverAddress, sizeof(serverAddress)); //���� ����ü �ʱ�ȭ

    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);//localhost �Է�
    serverAddress.sin_family = AF_INET; // TCP/IP����
    serverAddress.sin_port = htons(PORT); //��Ʈ��ȣ

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) { //���� ���� ���ε�
        cout << "���� ������ ���ε� �ϴµ� �����߽��ϴ�." << endl;
        return 0;
    }

    if (listen(serverSocket, 5) == -1) { //���� listen(�����)
        cout << "���� ������ listen ���� �����ϴµ� �����߽��ϴ�" << endl;
        return 0;
    }
    cout << "������ ���� �Ǿ����ϴ�." << endl;
    cout << "**********��ü ��ȭ ����***********" << endl;

    while (1) {

        SOCKADDR_IN clientAddress; //Ŭ���̾�Ʈ ����ü ����

        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        WaitForSingleObject(mutex, INFINITE); //���ؽ� ����

        clientNumber++;
        allClientSocket[clientNumber - 1] = clientSocket;

        ReleaseMutex(mutex); //���ؽ� ����
        /*Ŭ���̾�Ʈ���� ������ ȯ�� �޽����� ������ �迭�� �����߽��ϴ�.*/
        char greetMessage[BUFFERSIZE]; //�޽��� ���� �迭 ����
        sprintf(greetMessage, "[����]ȯ���մϴ�. ��ȭ���� �Է��� �ּ���\n"); //�޽��� ����
        send(clientSocket, greetMessage, sizeof(greetMessage), 0);
        /*Ŭ���̾�Ʈ���� ������ �޽����� �о� �鿩�� ��� Ŭ���̾�Ʈ�� �����ϴ� broadcastAllClient �Լ��� ȣ���մϴ�.*/
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
			if (num[i][j] == 0) cout << "��";
		}
		cout << endl;
	}
}

int main() {
    THD cla;
    cla.Soc_Ser();

	return 0;
}