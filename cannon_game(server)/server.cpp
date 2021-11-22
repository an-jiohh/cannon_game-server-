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
    void create_hurdle(int y, int x);  //��ֹ� ���� �Լ�(2*2)����
    void print_map(); //���� ǥ���ϴ� �Լ�
    void moving_map(); //���� ��ҵ��� �̵� ó�� �Լ�
    virtual int main_func() = 0; //���� ��Ҹ� ������ �߻�޼ҵ� ���� 
};

void map::create_hurdle(int y, int x) { //��ֹ� ���� �Լ�(2*2)����
    num[y][x] = 5;
    num[y][x + 1] = 5;
    num[y + 1][x] = 5;
    num[y + 1][x + 1] = 5;
}

void map::print_map() { //���� ǥ���ϴ� �Լ�
    for (int i = 0; i < 10; i++) { 
        for (int j = 0; j < 10; j++) {
            int temp = num[i][j] - (num[i][j] / 100) * 100;
            if (temp / 10 == 1) cout << "��";
            else if (temp / 10 == 2) cout << "��";
            else if (temp / 10 == 3) cout << "��";
            else if (temp / 10 == 4) cout << "��";
            else if (temp == 5) cout << "��";
            else if (temp == 6) cout << "��";
            else cout << "��";
        }
        cout << endl;
    }
}

void map::moving_map() { //���� ��ҵ��� �̵��� ó���Ѵ�.
    int movecount = 1;
    while (movecount <= clientNumber) {
        int temp = 0;
        int player = 0;
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (num[i][j] / 100 == movecount) {
                    int tens_temp = (num[i][j] - (num[i][j] / 100) * 100); //tens_temp�� 10 + 1���ڸ� ����
                    int ones_temp = tens_temp - (tens_temp / 10) * 10; //ones_temp�� 1�� �ڸ� ����
                    tens_temp = (tens_temp / 10) * 10; //tens_temp�� 10���ڸ� ����

                    if (ones_temp == 0) { //1�� �ڸ��� 0�� ��� �̵�
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
        if (temp == 0)movecount++; //�ش� �÷��̾ ������ �н�
    }
}

class THD{
public:
    int start_server(int playernum); //���� ���� �Լ�
    void join_client(); //Ŭ���̾�Ʈ ���� �Լ�
    static unsigned __stdcall receive(void* arg); //�� Ŭ���̾�Ʈ�κ��� �Է��� ����
    virtual int main_func() = 0; //���� ������ ������ �߻�޼ҵ� ����
protected:
    SOCKET serverSocket;
};

int THD::start_server(int playernum) { //���� ���� �Լ�
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata); //���� ���� ����

    mutex = CreateMutex(NULL, FALSE, NULL); //���ؽ� ����

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    if (listen(serverSocket, playernum) == -1) { //���� listen(�����)
        cout << "���� ������ listen ���� �����ϴµ� �����߽��ϴ�" << endl;
        return 0;
    }
    cout << "������ ���� �Ǿ����ϴ�." << endl;
    return 1;
}

void THD::join_client() { //Ŭ���̾�Ʈ ���� �Լ�
    SOCKADDR_IN clientAddress; //Ŭ���̾�Ʈ ����ü ����

    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

    WaitForSingleObject(mutex, INFINITE); //���ؽ� ����
    allClientSocket[clientNumber] = clientSocket;
    clientNumber++;
    ReleaseMutex(mutex); //���ؽ� ����

    char greetMessage[BUFFERSIZE]; //�޽��� ���� �迭 ����
    sprintf(greetMessage, "[����]ȯ���մϴ�\n"); //�޽��� ����
    send(clientSocket, greetMessage, sizeof(greetMessage), 0);

    unsigned long thread;
    thread = _beginthreadex(NULL, 0, receive, (void*)clientSocket, 0, &threadID); //�� Ŭ���̾�Ʈ���� �޽����� �޴� ������ ����
}

unsigned __stdcall THD::receive(void* arg) {
    SOCKET myClientSocket = (SOCKET)arg;

    int Clientnum[1] = { 0 };
    Clientnum[0] = clientNumber;
    cout << Clientnum[0] << "��° �÷��̾� ����" << endl;

    Clientnum[0] = htonl(Clientnum[0]);
    send(myClientSocket, (char*)Clientnum, sizeof(Clientnum), 0); //Ŭ���̾�Ʈ �ڽ��� ���� ����
    Clientnum[0] = htonl(Clientnum[0]);

    int fromClient[1]; //Ű���� �Է� �� ���� �迭

    while (1) {
        int readlen = recv(myClientSocket, (char*)fromClient, sizeof(fromClient), 0); //Ű���� �Է� ����
        if (readlen < 0) break; //�÷��̾ ������� �ݺ��� ����
        fromClient[0] = htonl(fromClient[0]);

        //Ű���� �Է°��� ���� �����尡 ����ϴ� �÷��̾��� ���°��� �ٲ۴�.
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
    //������ ���� ���� �κ�
    WaitForSingleObject(mutex, INFINITE);//���ؽ� ����ȭ ����
    int i, j;
    for (i = 0; i < clientNumber; i++) { //������ ������ ã��
        if (allClientSocket[i] == myClientSocket) { //������ ���� �� ���
            j = i;
            while (j < clientNumber - 1) {
                allClientSocket[j] = allClientSocket[j + 1];
                j++;
            }
            break;
        }
    }
    clientNumber--;//��ü Ŭ���̾�Ʈ�� ���� 1���� ��Ŵ
    ReleaseMutex(mutex);//���ؽ� ����ȭ ����
    closesocket(myClientSocket);//���� Ŭ���̾�Ʈ�� ����� ������ ������
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
    cout << "�÷����� �ο��� �Է��� �ּ��� : ";
    cin >> playernum;

    if (start_server(playernum) == 0) return 0; //�����Լ�(�����ϸ� ����)

    while (clientNumber < playernum) { //�÷��� �ο���ŭ client�� ����
        join_client();

    }

    Sleep(100); //������ �÷��̾ �����Ҷ����� Sleep
    cout << "��� �÷��̾ �����Ͽ����ϴ�." << endl;

    cout << "��ũ�� �̵��ӵ��� �Է����ּ���" << endl; //��ũ �̵��ӵ� ����
    cout << "��ũ�� �̵��ӵ� : ";
    int gamespeed;
    cin >> gamespeed;

    while (1) { //Y�� ������ ���� ���
        cout << "Y�� ������ ������ �����մϴ�." << endl;
        int in = _getch();
        if (in == 89 || in == 121) break;
    }

    int fog = 0; //�Ȱ���� ����
    while (1) { //���� ��ε� ĳ����
        WaitForSingleObject(mutex, INFINITE); //���ؽ� ���

        moving_map(); //�� �÷��̾���� ��ġ�� �̵���Ŵ

        if (_kbhit()) { //�Ȱ���� ����
            int in = _getch();
            if (in == 77 || in == 109) {
                if (fog == 1) fog = 0;
                else fog = 1;
            }
        }

        system("cls");
        print_map(); //������ �� ǥ��

        cout << "m�� ������ �Ȱ���带 �����մϴ�. ������ : ";
        if (fog == 1) cout << "�Ȱ����" << endl;
        else cout << "��Ȱ����" << endl;

        int snum[101]; //���ۿ� �迭
        for (int i = 0; i < 10; i++) { //�򿣵�� ����
            for (int j = 0; j < 10; j++) {
                snum[i * 10 + j] = htonl(num[i][j]);
                if (num[i][j] == 6) num[i][j] = 0;
            }
        }
        snum[100] = htonl(fog); //�迭�� �������� �Ȱ�/��Ȱ� ��� ���� �ڵ��Է�

        for (int i = 0; i < clientNumber; i++) { // ��ü �÷��̾�� ����
            send(allClientSocket[i], (char*)snum, sizeof(snum), 0);
        }
        ReleaseMutex(mutex); //���ؽ� ����
        Sleep(gamespeed);
    }
    return 0;
}

int main() {
    main_CLA cla;

    cla.main_func();

	return 0;
}