#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_PORT "/dev/ttyUSB0"

void serial_send(int fd)
{
    printf("[serial_send]: start sending!\n");

    // 送信用
    int err;
    char send[1];

    for(int i = 0; i < 23; i++){
        send[0] = '\0';
        switch(i){
            case 0: sprintf(send, "\x03"); break;
            case 1: sprintf(send, "\x03"); break;
            case 2: sprintf(send, "\xE8"); break;
            case 3: sprintf(send, "\x07"); break;
            case 4: sprintf(send, "\xD0"); break;
            case 5: sprintf(send, "\x02"); break;
            case 6: sprintf(send, "\x23"); break;
            case 7: sprintf(send, "\x14"); break;
            case 8: sprintf(send, "\x32"); break;
            case 9: sprintf(send, "\x00"); break;
            case 10: sprintf(send, "\x64"); break;
            case 11: sprintf(send, "\x45"); break;
            case 12: sprintf(send, "\xBB"); break;
            case 13: sprintf(send, "\x80"); break;
            case 14: sprintf(send, "\x00"); break;
            case 15: sprintf(send, "\x45"); break;
            case 16: sprintf(send, "\x3B"); break;
            case 17: sprintf(send, "\x80"); break;
            case 18: sprintf(send, "\x00"); break;
            case 19: sprintf(send, "\x44"); break;
            case 20: sprintf(send, "\xFA"); break;
            case 21: sprintf(send, "\x00"); break;
            case 22: sprintf(send, "\x00"); break;
        }
        err = write(fd, send, 1);
        if (err < 0){
            printf("write error\n");
            exit(1);
        }
        printf("%d(size: %d):  %x\n", i, sizeof(send[i]), send[0]);
    }

    printf("[serial_send]: finish sending!\n");
    //sprintf(send, "\x03\x03\xE8\x07\xD0\x02\x23\x14\x32\x00\x64\x45\xBB\x80\x00\x45\x3B\x80\x00\x44\xFA\x00\x00");
}

void serial_receive(int fd)
{
    printf("[serial_receive]: start recieving!\n");

    // 受信用
    int len;
    unsigned char resv[512] = {'\0'};

    // デバイスからの受信
    len = read(fd, resv, sizeof(resv));
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
    int fd;                             // ファイルディスクリプタ
    struct termios tio;                 // シリアル通信設定
    int baudRate = B9600;
    int len;
    int err;
    int size;

    // デバイスのオープン
    fd = open(SERIAL_PORT, O_RDWR);
    if (fd < 0){
        //perror(argv[1]);
        printf("open error\n");
        return -1;
    }

    // 初期設定
    memset(&tio, 0, sizeof(tio));        // 初期化
    tio.c_cflag = CS8 | CLOCAL | CREAD;  // データビット：8bit, ローカルライン（モデム制御なし）, 受信有効
    cfsetispeed(&tio, baudRate);         // 入力ボーレート設定
    cfsetospeed(&tio, baudRate);         // 出力ボーレート設定
    cfmakeraw(&tio);                     // RAWモード
    tcsetattr(fd, TCSANOW, &tio);        // デバイスに設定を行う
    ioctl(fd, TCSETS, &tio);             // ポートの設定を有効にする

    //printf("check1\n");
 
    // デバイスへの送信   
    serial_send(fd);

    sleep(3);                             // readの前に待機

    // デバイスからの受信
    serial_receive(fd);

    // デバイスのクローズ
    close(fd);

    return 0;
}