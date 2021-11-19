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
int x[3] = {};
int y[3] = {};

int num[10][10] = { NULL };


class map {
public:
    void create_hurdle(int y, int x);
    void print_map();
protected:
private:
};
void map::create_hurdle(int y, int x) {
    num[y][x] = 5;
    num[y][x+1] = 5;
    num[y + 1][x] = 5;
    num[y + 1][x + 1] = 5;
}

void map::print_map() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (num[i][j] == 0) cout << "��";
        }
        cout << endl;
    }
}

class THD:public map{
public:
    int Soc_Ser();
    static unsigned __stdcall receive(void* arg);
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

    if (listen(serverSocket, 3) == -1) { //���� listen(�����)
        cout << "���� ������ listen ���� �����ϴµ� �����߽��ϴ�" << endl;
        return 0;
    }
    cout << "������ ���� �Ǿ����ϴ�." << endl;
    cout << "**********��ü ��ȭ ����***********" << endl;

    while (clientNumber < 2) {

        SOCKADDR_IN clientAddress; //Ŭ���̾�Ʈ ����ü ����

        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        WaitForSingleObject(mutex, INFINITE); //���ؽ� ����
        allClientSocket[clientNumber] = clientSocket;
        clientNumber++;
        ReleaseMutex(mutex); //���ؽ� ����

        /*Ŭ���̾�Ʈ���� ������ ȯ�� �޽����� ������ �迭�� �����߽��ϴ�.*/
        char greetMessage[BUFFERSIZE]; //�޽��� ���� �迭 ����
        sprintf(greetMessage, "[����]ȯ���մϴ�. ��ȭ���� �Է��� �ּ���\n"); //�޽��� ����
        send(clientSocket, greetMessage, sizeof(greetMessage), 0);

        unsigned long thread; 
        thread = _beginthreadex(NULL, 0, receive, (void*)clientSocket, 0, &threadID);

    }

    int in = 0;
    while (1) {
        cout << "Y�� ������ ������ �����մϴ�." << endl;
        in = _getch();
        if (in == 89 || in == 121) break;
    }

    int snum[10][10];
    while (1) { //���� ��ε� ĳ����
        WaitForSingleObject(mutex, INFINITE);
        int movecount = 1;       
        while (movecount <= clientNumber) {
            int temp = 0;
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    if (num[i][j] / 100 == movecount) { 
                        int tens_temp = (num[i][j] - (num[i][j] / 100) * 100); //tens_temp�� 10 + 1���ڸ� ����
                        int ones_temp = tens_temp - (tens_temp / 10) * 10; //ones_temp�� 1�� �ڸ� ����
                        tens_temp = (tens_temp / 10) * 10; //tens_temp�� 10���ڸ� ����

                        if (ones_temp == 0) {
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
            if (temp == 0)movecount++; //�ش� �÷��̾ ������ �н�
        }
        
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                snum[i][j] = htonl(num[i][j]);
                if (num[i][j] == 7 || num[i][j] == 6) num[i][j] = 0;
            }
        }
        for (int i = 0; i < clientNumber; i++) {
            send(allClientSocket[i], (char*)snum, sizeof(snum), 0);
        }
        ReleaseMutex(mutex);
        Sleep(300);
        
    }
    
    return 0;
}

unsigned __stdcall THD::receive(void* arg) {
    SOCKET myClientSocket = (SOCKET)arg;

    int Clientnum[1] = { 0 };
    Clientnum[0] = clientNumber;
    cout << Clientnum[0] << "��° �÷��̾� ����" << endl;

    Clientnum[0] = htonl(Clientnum[0]);
    send(myClientSocket, (char*)Clientnum, sizeof(Clientnum), 0); //Ŭ���̾�Ʈ �ڽ��� ���� ����
    Clientnum[0] = htonl(Clientnum[0]);
    cout << Clientnum[0] << "��° �÷��̾� ����" << endl;

    int fromClient[1]; //Ű���� �Է� �� ���� �迭

    while (1) {
        int readlen = recv(myClientSocket, (char*)fromClient, sizeof(fromClient), 0); //Ű���� �Է� ����
        fromClient[0] = htonl(fromClient[0]);

        WaitForSingleObject(mutex, INFINITE); 
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (num[i][j] / 100 == Clientnum[0])
                {
                    int tens_temp = (num[i][j] - (num[i][j] / 100) * 100); //tens_temp�� 10 + 1���ڸ� ����
                    int ones_temp = tens_temp - (tens_temp / 10) * 10; //ones_temp�� 1�� �ڸ� ����
                    tens_temp = (tens_temp / 10) * 10; //tens_temp�� 10���ڸ� ����

                    if (fromClient[0] == SPACE) { //���� �߻�
                        if (tens_temp == 10) { //��
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
                        else if (tens_temp == 20) { //�Ʒ�
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
                        else if (tens_temp == 30) { //����
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
                        else if (tens_temp == 40) { //�Ʒ�
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
                    else if (fromClient[0] == 83 || fromClient[0] == 115) { // ����
                        if (ones_temp == 0)  num[i][j]++;
                        else  num[i][j]--;
                    }
                    else if (fromClient[0] == UP) { //���� �̵�
                        num[i][j] -= tens_temp;
                        num[i][j] += 10;
                    }
                    else if (fromClient[0] == DOWN) { //�Ʒ��� �̵�
                        num[i][j] -= tens_temp;
                        num[i][j] += 20;
                    }
                    else if (fromClient[0] == LEFT) { //�������� �̵�
                        num[i][j] -= tens_temp;
                        num[i][j] += 30;
                    }
                    else if (fromClient[0] == RIGHT) { //�����ʷ� �̵�
                        num[i][j] -= tens_temp;
                        num[i][j] += 40;
                    }
                }
            }
        }
        ReleaseMutex(mutex);
    }
    return 0;
}



int main() {
    THD cla;

    cla.create_hurdle(3, 3);
    cla.create_hurdle(2, 6);
    cla.create_hurdle(6, 1);
    cla.create_hurdle(7, 6);

    num[0][0] = 120;
    num[9][0] = 230;
    num[0][9] = 310;

    cla.Soc_Ser();



	return 0;
}