#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_PORT_A "/dev/ttyUSB0"
#define SERIAL_PORT_B "/dev/ttyUSB1"
#define SERIAL_PORT_C "/dev/ttyUSB2"
#define SERIAL_PORT_D "/dev/ttyUSB3"
#define BAUDRATE B1152000 // ボーレート
//#define MC 439

void serial_init(int fd)
{
    printf("[serial_init]: start initialize at %d!\n", fd);

    struct termios tio = {0};                  // シリアル通信設定

    // 初期設定
    bzero(&tio, sizeof(tio));            // 新しいポートの設定の構造体をクリア
    tio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;  // データビット：8bit, ローカルライン（モデム制御なし）, 受信有効
    tio.c_iflag = IGNPAR;                // バリティエラーの文字は無視
    tio.c_oflag = 0;                     // rawモード
    tio.c_lflag = 0;                     // 非カノニカル入力
    tio.c_cc[VTIME] = 20;                // 0: キャラクタ間タイマは未使用, 1以上: タイマーが時間切れになるまでreadで停止，単位は1/10[sec]
    tio.c_cc[VMIN] = 0;                  // MC文字受け取るまでreadで停止

    //cfmakeraw(&tio);
    tcflush(fd, TCIFLUSH);                 // ポートのクリア, 受信したがまだ読み込んでいないすべてのデータを廃棄
    int err;
    err = tcsetattr(fd, TCSANOW, &tio);       // ポートの設定を有効にする
    if(err == -1){
        printf("tcsetattr() ERROR\n");
    }
}

void serial_send(int fd)
{
    printf("[serial_send]: start sending at %d!\n", fd);

    /*
    // 送信用
    int err;

    unsigned char send[] = {0x03,0xE8,0x03,0xD0,0x07,0x01,0x01,0x14,0x32,0x2C,0x01,
                   0x00,0x80,0xBB,0x45,0x00,0x80,0x3B,0x45,0x00,0x00,0xFA,0x44,0x0A};

    err = write(fd, send, 24);
    if(err == -1){
        printf("write ERROR\n");
    }
    */

    /**/
    // 送信データ
    char command = 0x03;
    uint16_t tconv1 = 1000;
    uint16_t tconv2 = 2000;
    char rsv1 = 0x01;
    char rsv2 = 0x01;
    uint8_t trashmm = 20;
    uint8_t nrepeat = 50;
    uint16_t sps = 300;
    float kx = 6000.0f;
    float ky = 3000.0f;
    float kz = 2000.0f;
    char bcc = 0x0A;

    
    write(fd, &command, 1);
    write(fd, &tconv1, 2);
    write(fd, &tconv2, 2);
    write(fd, &rsv1, 1);
    write(fd, &rsv2, 1);
    write(fd, &trashmm, 1);
    write(fd, &nrepeat, 1);
    write(fd, &sps, 2);
    write(fd, &kx, 4);
    write(fd, &ky, 4);
    write(fd, &kz, 4);
    write(fd, &bcc, 1);
    /**/

    printf("[serial_send]: finish sending!\n");
}

void serial_receive(int fd)
{
    printf("[serial_receive]: start recieving at %d!\n", fd);

    // 受信用
    int len;
    unsigned char resv[439] = {'\0'};      // 0x03: 439byte, 0x05: 181byte, 0x15; 183byte   

    // デバイスからの受信
    len = read(fd, resv, sizeof(resv));    // read関数：最大でresv[]のサイズまで読み込む
    printf("[serial_receive]: len = %d\n", len);
    if(len==0){
        // read()が0を返したら、end of file
        // 通常は正常終了するのだが今回は無限ループ
        printf("0 byte read\n");
    }
    if(len<0){
        printf("read ERROR\n");
        // read()が負を返したら何らかのI/Oエラー
        exit(2);
    }
    if (0 < len){
        for(int i = 0; i < len; i++){
            printf("%02X", resv[i]);
        }
        printf("\n");
    }
    printf("[serial_receive]: finish receiving!\n");
}

int main(int argc, char *argv[])
{
    // 変数宣言
    //unsigned char msg[256] = "serial port open...\n";
    int fd_a, fd_b, fd_c;               // ファイルディスクリプタ, COM_A〜Dに対応

    // デバイスのオープン, COM_Dは不使用
    fd_a = open(SERIAL_PORT_A, O_RDWR);
    if (fd_a < 0){
        printf("open error\n");
        return -1;
    }
    fd_b = open(SERIAL_PORT_B, O_RDWR);
    if (fd_b < 0){
        printf("open error\n");
        return -1;
    }
    fd_c = open(SERIAL_PORT_C, O_RDWR);
    if (fd_c < 0){
        printf("open error\n");
        return -1;
    }
    /*
    fd_d = open(SERIAL_PORT_D, O_RDWR);
    if (fd_d < 0){
        printf("open error\n");
        return -1;
    }
    
    int fd_test;
    fd_test = open("text.txt", O_RDWR);
    if (fd_test < 0){
        printf("open error\n");
        return -1;
    }
    */

    // デバイスの初期化
    serial_init(fd_a);
    serial_init(fd_b);
    serial_init(fd_c);
    //serial_init(fd_d);
    
    // デバイスへの送信, COM_Aのみに送信   
    serial_send(fd_a);
    //serial_send(fd_b);
    //serial_send(fd_c);
    //serial_send(fd_d);
    //serial_send(fd_test);

    sleep(2);                             // readの前に待機

    // デバイスからの受信, COM_A〜COM_Cから受信する
    serial_receive(fd_a);
    serial_receive(fd_b);
    serial_receive(fd_c);
    //serial_receive(fd_d);
    //serial_receive(fd_test);

    // デバイスのクローズ
    close(fd_a);
    close(fd_b);
    close(fd_c);
    //close(fd_d);
    //close(fd_test);

    return 0;
}
