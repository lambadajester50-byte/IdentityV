//微验网络验证//
//如果是AIDE编译jni，请将原main.cpp删除，将此注入好的文件改成main.cpp
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <fstream>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <iostream>
#include <fstream>

#include<iostream>
#include<ctime>
using namespace std;
#include "draw.h"    //绘制套
#include "AndroidImgui.h"     //创建绘制套
#include "GraphicsManager.h" //获取 当前渲染模式
#include "Android_draw/timer.h"
#include "SoHookIntegration.h"
timer DrawFPS;
float fps = 60;
long int value1,value2,value3;

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        exit(1);
    }
    if (pid > 0) {
        exit(0); // 父进程退出，子进程继续
    }

    if (setsid() < 0) {
        exit(1);
    }

    if (chdir("/") < 0) {
        exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // stdin → /dev/null
    open("/dev/null", O_RDONLY);
    // stdout → 日志文件（追加写入）
    open("/data/local/tmp/hack_stdout.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
    // stderr → 同一个日志文件（追加写入）
    open("/data/local/tmp/hack_stderr.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
}


int main(int argc, char *argv[]) {

    daemonize();
    SoHook::StartListeners();

	   
    value1 = 970061201;
    value2 = 16384;
    value3 = 257;
    
    ::graphics = GraphicsManager::getGraphicsInterface(GraphicsManager::VULKAN);//绘图方式

    //获取屏幕信息    
    ::screen_config(); 

    ::native_window_screen_x = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::native_window_screen_y = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::abs_ScreenX = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::abs_ScreenY = (::displayInfo.height < ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    
   // GetPKG();
    
    ::window = android::ANativeWindowCreator::Create("new_edition", native_window_screen_x, native_window_screen_y, permeate_record);
    graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y);
    
    Touch::Init({(float)::abs_ScreenX, (float)::abs_ScreenY}, true);
    Touch::setOrientation(displayInfo.orientation);
    
    new std::thread(read_thread,value1,value2,value3);
    
	DrawFPS.SetFps(fps);
	DrawFPS.AotuFPS_init();
	DrawFPS.setAffinity();
    
    ::init_My_drawdata(); //初始化绘制数据
    
    static bool flag = true;
    while (flag) {
        drawBegin();
        graphics->NewFrame();        
        Layout_tick_UI(&flag);
        graphics->EndFrame();
        DrawFPS.SetFps(fps);
	    DrawFPS.AotuFPS();
    }
    
    graphics->Shutdown();
    SoHook::StopListeners();
    android::ANativeWindowCreator::Destroy(::window);
    return 0;
}
