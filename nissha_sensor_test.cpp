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
#define BAUDRATE B115200 // ボーレート
//#define MC 439

void serial_init(int* fd)
{
    printf("[serial_init]: start initialize at %d!\n", *fd);

    struct termios oldtio, newtio;          // シリアル通信設定

    // 初期設定
    tcgetattr(*fd, &oldtio);                // 現在のシリアルポート設定を退避  
    bzero(&newtio, sizeof(newtio));         // 新しいポートの設定の構造体をクリア
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;  // データビット：8bit, ローカルライン（モデム制御なし）, 受信有効
    //newtio.c_iflag = IGNPAR;                // バリティエラーの文字は無視
    newtio.c_oflag = 0;                     // rawモード
    newtio.c_lflag = 0;                     // 非カノニカル入力
    newtio.c_cc[VTIME] = 1;                 // 0: キャラクタ間タイマは未使用
    newtio.c_cc[VMIN] = 1;                 // MC文字受け取るまでブロック

    tcflush(*fd, TCIFLUSH);                 // ポートのクリア
    tcsetattr(*fd, TCSANOW, &newtio);       // ポートの設定を有効にする
}

void serial_send(int fd)
{
    printf("[serial_send]: start sending at %d!\n", fd);

    // 送信用
    int err;
    /*
    unsigned char send[] = {0x03,0x03,0xE8,0x07,0xD0,0x02,0x03,0x14,0x32,0x00,0x64,
                   0x45,0xBB,0x80,0x00,0x45,0x3B,0x80,0x00,0x44,0xFA,0x00,0x00};
    err = write(fd, send, 23);
    */

   // 送信データ
    char command = 0x03;
    uint16_t tconv1 = 1000;
    uint16_t tconv2 = 2000;
    char rsv1 = 0x02;
    char rsv2 = 0x03;
    uint8_t trashmm = 20;
    uint8_t nrepeat = 50;
    uint16_t sps = 300;
    float kx = 6000.0f;
    float ky = 3000.0f;
    float kz = 2000.0f;

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

    printf("[serial_send]: finish sending!\n");
   }

void serial_receive(int fd)
{
    printf("[serial_receive]: start recieving at %d!\n", fd);

    // 受信用
    int len;
    unsigned char resv[512] = {'\0'};

    // デバイスからの受信
    len = read(fd, resv, sizeof(resv));
    printf("[serial_receive]: len = %d\n", len);
    if(len==0){
        // read()が0を返したら、end of file
        // 通常は正常終了するのだが今回は無限ループ
        printf("waiting...");
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
    unsigned char msg[256] = "serial port open...\n";
    int fd_a, fd_b, fd_c, fd_d;               // ファイルディスクリプタ, COM_A〜Cに対応
    int len;
    int err;
    int size;

    // デバイスのオープン
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
    fd_d = open(SERIAL_PORT_D, O_RDWR);
    if (fd_d < 0){
        printf("open error\n");
        return -1;
    }

    /*
    int fd_test;
    fd_test = open("text.txt", O_RDWR | O_TRUNC);
    */

    // デバイスの初期化
    serial_init(&fd_a);
    serial_init(&fd_b);
    serial_init(&fd_c);
    serial_init(&fd_d);
    
    //printf("check1\n");
 
    // デバイスへの送信   
    serial_send(fd_a);

    sleep(3);                             // readの前に待機

    // デバイスからの受信
    serial_receive(fd_a);
    //serial_receive(fd_b);
    //serial_receive(fd_c);
    serial_receive(fd_d);

    // デバイスのクローズ
    close(fd_a);
    close(fd_b);
    close(fd_c);
    close(fd_d);

    return 0;
}
