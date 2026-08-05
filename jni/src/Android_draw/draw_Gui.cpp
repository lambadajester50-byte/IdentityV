#include "draw.h"
#include <thread>
#include <cstdint>
#include <stdio.h>
#include "My_font/wrg_font.h"
#include "kerneldriver-qxqd.hpp"
#include "DrawTool.h"
#include "Name.h"
#include "SoHookIntegration.h"
#include <linux/input.h>
#include <sstream>
#include <iomanip>
std::string 过滤类名,类名;
char gwd1[25];
char gwd2[25];
float 距离比例=11.886;
float 红夫人X, 红夫人Y, 红夫人Z;
float 红夫人镜像X, 红夫人镜像Y, 红夫人镜像Z;
typedef struct {
    uintptr_t obj;
    uintptr_t objcoor;
    int 阵营;
    char str[256];//翻译名
    char 类名[256];//类名
}DataStruct;
DataStruct data[1000];

bool permeate_record = false;
bool permeate_record_ini = false;
struct Last_ImRect LastCoordinate = {0, 0, 0, 0};
static uint32_t orientation = -1;
ANativeWindow *window; 
// 屏幕信息
android::ANativeWindowCreator::DisplayInfo displayInfo;
// 窗口信息
ImGuiWindow *g_window;
// 绝对屏幕X _ Y
int abs_ScreenX, abs_ScreenY;
int native_window_screen_x, native_window_screen_y;
std::unique_ptr<AndroidImgui>  graphics;
ImFont* zh_font = NULL;
bool niexi;
float 矩阵视野距离;
float 孽蜥距离,孽蜥按住距离;
/*定义*/
bool DrawIo[50];
float 孽蜥触摸X,孽蜥触摸Y;
bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO &io = ImGui::GetIO();
    
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.OversampleH = 1;
    config.SizePixels = SizePixels;
    ::zh_font = io.Fonts->AddFontFromMemoryTTF((void *)WRG_Font, WRG_Font_size, SizePixels, &config, io.Fonts->GetGlyphRangesChineseFull());

    return zh_font != nullptr;
}
void init_My_drawdata() {
    M_Android_LoadFont(25.0f); //加载内存字体(含中文TTF+图标)
}


void screen_config() {
    ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin() {
    if (::permeate_record_ini) {
        LastCoordinate.Pos_x = ::g_window->Pos.x;
        LastCoordinate.Pos_y = ::g_window->Pos.y;
        LastCoordinate.Size_x = ::g_window->Size.x;
        LastCoordinate.Size_y = ::g_window->Size.y;

        graphics->Shutdown();
        android::ANativeWindowCreator::Destroy(::window);
        ::window = android::ANativeWindowCreator::Create("逆天改命", native_window_screen_x, native_window_screen_y, permeate_record);
        graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y);
        ::init_My_drawdata(); //初始化绘制数据
    } 

    screen_config();
    if (::orientation != displayInfo.orientation) {
        ::orientation = displayInfo.orientation;
        Touch::setOrientation(displayInfo.orientation);
        if (g_window != NULL) {
            g_window->Pos.x = 100;
            g_window->Pos.y = 125;        
        }        
        //cout << " width:" << displayInfo.width << " height:" << displayInfo.height << " orientation:" << displayInfo.orientation << endl;
    }
}

struct Vector3A
{
	float X;
	float Y;
	float Z;

	  Vector3A()
	{
		this->X = 0;
		this->Y = 0;
		this->Z = 0;
	}

	Vector3A(float x, float y, float z)
	{
		this->X = x;
		this->Y = y;
		this->Z = z;
	}

};

float xs_prime_mirror, ys_prime_mirror;
float xs_final, ys_final;
void calculate_mirror_reflection(float x1, float y1, float x2, float y2, float xs, float ys, float *xs_prime, float *ys_prime) {
    float xm = (x1 + x2) / 2.0;
    float ym = (y1 + y2) / 2.0;

    *xs_prime = 2.0 * xm - xs;
    *ys_prime = 2.0 * ym - ys;
}

void calculate_line_reflection(float x1, float y1, float x2, float y2, float xs, float ys, float *xs_prime, float *ys_prime) {
    float A = y2 - y1;
    float B = x1 - x2;
    float C = x2 * y1 - x1 * y2;

    float D = A * xs + B * ys + C;
    float denom = A * A + B * B;

    *xs_prime = xs - 2.0 * A * D / denom;
    *ys_prime = ys - 2.0 * B * D / denom;
}


uintptr_t libbase;
uintptr_t Arrayaddr, Count, Matrix;
uintptr_t 对象,对象阵营,自身,自身阵营,namezfcz,namezfc;
uintptr_t 红夫人,红夫人镜像,镜子,捏镜子;
int 数量,zfcz,zfc;
float 过滤矩阵[17];
float matrix[16];
float angle;

static bool show_draw_Rect = true;//方框
static bool show_draw_Line = true;//射线
static bool show_draw_Camera = false;//相机
static bool show_draw_Door = false;//门
static bool show_draw_Box = false;//盒子
static bool show_draw_Name = true;//名字
static bool show_draw_Distance = true;//距离
static bool show_draw_Cellar = true;//地窖
static bool show_draw_Chair = false;//椅子
static bool show_draw_Prop = true;//道具
static bool show_draw_prophet = true;//预知监管者
static bool redqueenmod = false;//红夫人模式
static bool show_draw_sender = true;//密码机进度
static bool show_draw_secret_mechine = false;//密码机位置
static bool show_draw_Role = false;//角色
static bool show_draw_touch = false;//孽蜥
static bool show_draw_ClassName = false;//类名
static bool Debugging = false;//调试
static bool mirror = false;//镜子状态
static bool show_demo_window = false;
static bool show_another_window = false;
static bool show_window = true;  // 音量键控制：音量下=隐藏，音量上=显示
static bool voice = true;
static bool inform_ghost = false; // 显示鬼魂
static bool show_sohook = false;  // 骨骼与进度覆盖层

float z_x, z_y, z_z, d_x, d_y, d_z, camera, r_x, r_y, r_w;
float X1,Y1,X2,Y2,W,H,MIDDLE,TOP,BOTTOM;
int 距离;	
char objtext[256];
//char content[1024];
char Team[1024];
char Name[1024];
char 监管者预知[1024];

float px,py;
Vector3A D,Z,M;


void AimBotAuto()
{   
    bool 触摸状态 = false;
    // 是否按下触摸


    float SpeedMin = 2.0f;
    // 临时触摸速度

    double w = 0.0f, h = 0.0f, cmp = 0.0f;
    // 宽度 高度 正切

    /*double ScreenX = displayInfo.width, ScreenY = displayInfo.height;
    const Vector2 P;
    P.x=displayInfo.width;
    P.y=displayInfo.height;
    const Vector2 P(ScreenX, ScreenY); */

    //const ImVec2 P(ScreenX, ScreenY); 
    double ScreenX , ScreenY;
    if (displayInfo.width>displayInfo.height)
    {
    ScreenX = displayInfo.height;
    ScreenY = displayInfo.width;
    }
    else
    {
    
    ScreenX = displayInfo.width;
    ScreenY = displayInfo.height;
    }
    const Vector2 P(ScreenX, ScreenY);
    Touch::Init(P,false);	//初始化触摸

    // 分辨率(竖屏)PS:滑屏用的坐标是竖屏状态下的

    double ScrXH = ScreenX / 2.0f;
    // 一半屏幕X

    double ScrYH = ScreenY / 2.0f;
    // 一半屏幕X

    static float TargetX = 0;
    static float TargetY = 0;
    // 触摸目标位置
    //Vector3A obj;   
   
	
	
    while (1)
    {
    /*if (niexi)
    {
    Touch::Down(450,2500);
    触摸状态 = true;
    usleep(1000*100);
    niexi=false;
    }
    if (触摸状态)
    {
    Touch::Up();
    触摸状态 = false;
    usleep(1000*100);
    }*/
    Touch::Down(孽蜥触摸X,孽蜥触摸Y);
    usleep(1000*10);
    for (int i = 0; i < 50; i++)
    {
    Touch::Move(孽蜥触摸X+i*5,孽蜥触摸Y+i*5);
    usleep(1000*10);
    }
    //usleep(1000*10);
    Touch::Up();
    //Touch::Close;
    usleep(1000*1000);
    usleep(1000*100);
    }
}
ImColor 红色 = ImColor(255,0,0,255);
ImColor 绿色 = ImColor(0,255,0,255);
ImColor 蓝色 = ImColor(0,0,255,255);
ImColor 黄色 = ImColor(255,255,0,255);
ImColor 紫色 = ImColor(255,0,255,255);
ImColor 黑色 = ImColor(0,0,0,255);
ImColor BoneColor = ImColor(255,0,0,255);
ImColor BotBoneColor = ImColor(255,255,255,255);
int 状态 = 0;
int 数据获取状态 = 0;
int 遍历次数=0;
bool 首帧打印 = false;
bool 首帧矩阵 = false;
char extractedString[64];
long int MatrixOffset = 0,ArrayaddrOffset = 0;
typedef struct {
    unsigned long addr;
    unsigned long taddr;
} ModuleBssInfo;


ModuleBssInfo get_module_bss(int pid, const char *module_name) {
    FILE *fp;
    ModuleBssInfo info = {0, 0};
    char filename[64];
    char line[1024];

    // 生成文件名
    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);

    // 打开文件
    fp = fopen(filename, "r");

    bool found_module = false;

    if (fp!= NULL) {
        while (fgets(line, sizeof(line), fp)) {
            // 先判断是否包含模块名
            if (strstr(line, module_name)!= NULL) {
                found_module = true;
            }

            if (found_module) {
                // 检查是否满足rw权限且行长度符合要求
                long addr,taddr;
                sscanf(line, "%lx-%lx", &addr, &taddr);
                if (strstr(line, "rw")!= NULL && strlen(line) < 86 &&(taddr-addr)/4096>=2800) {
                //printf("%d", (taddr-addr)/4096);
                
                    // 将行按空格分割成字符串数组（这里简单示意，实际可能需要更完善的分割函数）
                    char *words[10];
                    int numWords = 0;
                    char *token = strtok(line, " ");
                    while (token!= NULL && numWords < 10) {
                        words[numWords++] = token;
                        token = strtok(NULL, " ");
                    }

                    // 遍历分割后的字符串数组，查找地址范围并转换
                    for (int i = 0; i < numWords; i++) {
                        if (sscanf(words[i], "%lx-%lx", &info.addr, &info.taddr) == 2) {
                            fclose(fp);
                            return info;
                        }
                    }

                    // 如果未找到正确格式的地址范围，设置为0并返回
                    info.addr = 0;
                    info.taddr = 0;
                    fclose(fp);
                    return info;
                }
            }
        }

        fclose(fp);
    }

    return info;
}

ModuleBssInfo get_module_bssgjf(int pid, const char *module_name) {
    FILE *fp;
    ModuleBssInfo info = {0, 0};
    long addr,taddr;
    char *pch;
    char filename[64];
    char line[1024];
    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    fp = fopen(filename, "r");
    bool is = false;
    if (fp!= NULL) {
        while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%lx-%lx", &addr, &taddr);
            if (strstr(line, module_name) &&strstr(line, "r-xp")!= NULL &&(taddr-addr)== 114982912) {
                is = true;
            }
            if (is) {
                if (strstr(line, "rw")!= NULL &&!feof(fp) && (strlen(line) < 86)) {
                long addr,taddr;
                sscanf(line, "%lx-%lx", &addr, &taddr);
                if ((taddr-addr)/4096<=3000)
                continue;
                    if (sscanf(line, "%lx-%lx", &info.addr, &info.taddr)!= 2) {
                        // 处理转换失败的情况
                        info.addr = 0;
                        info.taddr = 0;
                        break;
                    }
                    break;
                }
            }
        }
        fclose(fp);
    }
    return info;
}
int get_name_pid1(const char *packageName) {
    int id = -1;
    DIR *dir;
    FILE *fp;
    char filename[64];
    char cmdline[64] = {};
    struct dirent *entry;
    dir = opendir("/proc");
    if (dir == NULL) {
        return -1;
    }
    while ((entry = readdir(dir))!= NULL) {
        id = atoi(entry->d_name);
        if (id!= 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                char *readResult = fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                if (readResult != NULL &&
                    (strstr(cmdline, packageName) != NULL || strstr(cmdline, "com.netease.idv") != NULL) &&
                    strstr(cmdline, "com") != NULL && strstr(cmdline, "PushService") == NULL &&
                    strstr(cmdline, "gcsdk") == NULL) {
                    sprintf(extractedString, "%s", cmdline);
                    closedir(dir);
                    return id;
                }
            }
        }
    }
    closedir(dir);
    return -1;
}
long getModuleBasegjf(int pid, const char *module_name) {
    FILE *fp;
    long addr,taddr;
    char *pch;
    char filename[64];
    char line[1024];
    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    fp = fopen(filename, "r");
    bool is = false;
    if (fp!= NULL) {
        while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "r-xp")!= NULL &&!feof(fp) && strstr(line, module_name)) {
                sscanf(line, "%lx-%lx", &addr, &taddr);
                    if ((taddr-addr)== 114982912) {
                        // 处理转换失败的情况
                        fclose(fp);
                        return addr;
                        break;
                    }
                    //break;
                }
            
        }
        fclose(fp);
    }
    return 0;
}

int c;
char libso[256] = {"libclient.so"};

// ==================== 过滤表 ====================
static const char* g_filter_keywords[] = {
    "creature",
    "dm65_survivor_girl_page",
    "skill_hudie",
    "h55_joseph_camera",
    "burke_console",
    "redqueen_e_heijin_yizi",
    "qiutu_box",
    "weapon",
    "nvyao"
};
static constexpr int g_filter_count = sizeof(g_filter_keywords) / sizeof(g_filter_keywords[0]);
inline bool should_filter(const std::string& name) {
    for (int i = 0; i < g_filter_count; ++i) {
        if (name.find(g_filter_keywords[i]) != std::string::npos)
            return true;
    }
    return false;
}

// ================== 幽灵/隐身状态判定(合并原有特殊场景排除) ==================
// +0x70 == 0x1000000 且 +0x1a0 == 450.0 才是"正常在场"的真实角色/道具;
// 其余取值(包括65150鬼魂视角等)统一视为幽灵态, 由 inform_ghost 决定是否仍然显示。
bool ShouldSkipEntity(const DataStruct& obj) {
    // 这几类名字命中即直接跳过, 与幽灵判定无关(原本散落在渲染循环里, 现在收拢到一处)
    if (strstr(obj.类名, "h55_joseph_camera") != NULL) return true;   // 约瑟夫相机
    if (strstr(obj.类名, "redqueen_mirror") != NULL) return true;      // 红夫人镜子
    if (strstr(obj.类名, "burke_console") != NULL) return true;        // 疯眼场景
    if (strstr(obj.类名, "chr\\guajian") != NULL) return true;
    if (strstr(obj.类名, "girl_e_sj_zuoyi") != NULL) return true;
    if (strstr(obj.类名, "h55_survivor_w_shangren_tiaoban") != NULL) return true; // 商人跳板

    int checkVal = getDword(obj.obj + 0x70);
    float checkFloat = getFloat(obj.obj + 0x1a0);
    bool is_ghost_obj = (checkVal != 0x1000000 || checkFloat != 450.0f);

    if (is_ghost_obj) {
        if (!inform_ghost) return true; // 不显示幽灵/隐身角色, 直接过滤
        // 即便开启显示幽灵, 这几个特殊角色的隐身态数据不可靠(坐标/朝向失真), 依然过滤
        if (strstr(obj.str, "红蝶") || strstr(obj.str, "无常") ||
            strstr(obj.str, "歌剧") || strstr(obj.str, "破轮") ||
            strstr(obj.str, "木偶") || strstr(obj.str, "冒险家")) {
            return true;
        }
    }
    return false;
}

void read_thread(long int PD1,long int PD2,long int PD3)
{
    bool waitingLogged = false;
    while (pid <= 0) {
        pid = get_name_pid1("dwrg");
        if (pid > 0) break;
        if (!waitingLogged) {
            printf("[进程] 等待游戏进程启动\n");
            waitingLogged = true;
        }
        sleep(1);
    }
    driver->initialize(pid);
    printf("[进程] 已获取游戏进程 PID=%d 包名=%s\n", pid, extractedString);

    ModuleBssInfo result;
    // libbase 统一从maps取, 不用内核ioctl
    while (libbase == 0) {
        char mappath[64];
        snprintf(mappath, sizeof(mappath), "/proc/%d/maps", pid);
        FILE *fp = fopen(mappath, "r");
        if (fp == NULL) {
            printf("[进程] 无法打开 %s，重新等待游戏进程\n", mappath);
            pid = -1;
            while (pid <= 0) {
                sleep(1);
                pid = get_name_pid1("dwrg");
            }
            driver->initialize(pid);
            continue;
        }
        char line[1024];
        bool 官方 = strstr(extractedString, "com.netease.idv") != NULL;
        while (fgets(line, sizeof(line), fp)){
            long a, t;
            if (sscanf(line, "%lx-%lx", &a, &t) != 2) continue;
            if (libbase == 0 && strstr(line, "r-xp")){
                if (官方 && strstr(line, "."))    libbase = a;
                if (!官方 && strstr(line, libso)) libbase = a;
            }
        }
        fclose(fp);
        if (libbase == 0) {
            printf("[基址] 暂未找到游戏模块，1秒后重试\n");
            sleep(1);
        }
    }
    if (strstr(extractedString, "com.netease.idv") != NULL)
        result = get_module_bssgjf(pid, ".");
    else
        result = get_module_bss(pid, libso);
    printf("[基址] libbase=0x%llX BSS=0x%llX-0x%llX\n",
           (unsigned long long)libbase, (unsigned long long)result.addr, (unsigned long long)result.taddr);
    if (libbase < 0x5000000000){
        printf("[错误] libbase无效, 无法继续\n");
        sleep(9999);
    }
    c = (result.taddr-result.addr)/4096;
    long buff[512];
    while (MatrixOffset==0||ArrayaddrOffset==0)
    {
    	for (int i = 0; i < c; i++){
        	vm_readv(result.addr+(i*4096), &buff, 0x1000);
        	for (int ii=0;ii<512;ii+=1){

        	    if (MatrixOffset == 0 && *(long long*)(&buff[ii]) == 0x656A624F72655028LL){
        	        uint64_t 矩阵根 = result.addr + i*4096 + ii*8 + 0x430;
        	        uint64_t p1 = getPtr64(矩阵根);
        	        if (p1 > 0x5000000000){
        	            MatrixOffset = 矩阵根 - libbase;
        	            printf("[矩阵命中] 偏移=0x%llX   %lx   %lx\n", (unsigned long long)矩阵根 ,libbase,MatrixOffset);
        	        }
        	    }

        	    if (ArrayaddrOffset == 0 && buff[ii] == 16384){
                    if (getDword(result.addr + 4096*i + 8*ii - 0x8) == 257 &&
                        getFloat(result.addr + 4096*i + 8*ii - 16) == 1.0f){
                        ArrayaddrOffset = result.addr - libbase + i*4096 + ii*8 + 56;
                        printf("[数组命中] 偏移=0x%llX\n", (unsigned long long)ArrayaddrOffset);
                    }
                }
    	    }
        }
        if (MatrixOffset!=0 && ArrayaddrOffset!=0){
            uint64_t tmpArr = getPtr64(libbase + ArrayaddrOffset);
            uint64_t tmpEnd = getPtr64(libbase + ArrayaddrOffset + 8);
            if (tmpArr > 0x5000000000 && tmpEnd > tmpArr) break;
            printf("[数组无效] 重新扫描 Array=0x%llX End=0x%llX\n", (unsigned long long)tmpArr, (unsigned long long)tmpEnd);
            ArrayaddrOffset = 0;
        }
        sleep(5);
    }
    状态 = 2;
	
    Arrayaddr = getPtr64(libbase + ArrayaddrOffset);
    uint64_t ArrayEnd = getPtr64(libbase + ArrayaddrOffset + 8);
    Count = (ArrayEnd - Arrayaddr) / 8;
    if (Count <= 0 || Count > 10000) Count = 3000;
    printf("[数组] Arrayaddr=0x%llX End=0x%llX Count=%d\n", (unsigned long long)Arrayaddr, (unsigned long long)ArrayEnd, (int)Count);

    while (true)
    {        
    	uint64_t curArray = getPtr64(libbase + ArrayaddrOffset);
        uint64_t curArrayEnd = getPtr64(libbase + ArrayaddrOffset + 8);
        uint64_t curCount = curArrayEnd > curArray ? (curArrayEnd - curArray) / 8 : 0;
	    if (curArray < 0x5000000000 || curCount == 0 || curCount > 10000) {
            数量 = 0;
            状态 = 1;
            sleep(1);
            continue;
        }
        Arrayaddr = curArray;
        Count = curCount;
        状态 = 2;
    	int 指针数量=0;
        红夫人 = 0;      // 每轮清零，防止跨局/跨帧残留
        红夫人镜像 = 0;
        镜子 = 0;
        for (int ii = 0; ii < Count && 指针数量 < 1000; ii++){
            对象 = getPtr64(curArray+0x8 * ii);	// 遍历数量次数            
                
    		if (对象 == 0)   			
        		continue;    			    			
    		
    	    uint64_t 类名对象 = getPtr64(getPtr64(getPtr64(getPtr64(getPtr64(对象 + 0xf8)+0x0)+0x8)+0x20)+0x20)+0x0;
            int len = getDword(类名对象 + 0x10);
            if (len >= 256 || len == 0 || len < 0)
                continue;

            过滤类名.resize(len);
            vm_readv(getPtr64(类名对象 + 0x8), &过滤类名[0], len);
        		
			int 过滤重复指针=0;
			float pd1 = getFloat(对象 + 0x1a0);
			float pd2 = getFloat(对象 + 0x298);
			for (int i = 0; i < 指针数量; i++){
        		if(对象 == data[i].obj){
        		    过滤重复指针=1;
        		}        		    
        	}
        	if (过滤重复指针 == 1){
    		    continue;
    		}
        	if (should_filter(过滤类名)) {
        		continue;//过滤随从等无关对象
        	}
        	std::string s;
        	//预知监管者
            if (show_draw_prophet){//预知开始
                if (strstr(过滤类名.c_str(), "burke_console") == NULL&&strstr(过滤类名.c_str(), "h55_joseph_camera") == NULL&&strstr(过滤类名.c_str(), "redqueen_e_heijin_yizi") == NULL&&strstr(过滤类名.c_str(), "_lod") == NULL){
                    if (strstr(过滤类名.c_str(), "boss") != NULL){
                        s += getboss(过滤类名.c_str());
                        sprintf(监管者预知, "%s", s.c_str());
                    }       
                }
            }//预知结束
    		
			if (strstr(过滤类名.c_str(), "player") != NULL||strstr(过滤类名.c_str(), "boss") != NULL || pd1 == 450 || strstr(过滤类名.c_str(), "scene") != NULL || strstr(过滤类名.c_str(), "prop") != NULL || strstr(过滤类名.c_str(), "mirror") != NULL || Debugging )
			{
    			data[指针数量].obj = 对象;
    			if (strstr(过滤类名.c_str(), "boss") != NULL){
    			//data[指针数量].str=getboss(过滤类名.c_str());
    			strcpy(data[指针数量].str, getboss(过滤类名.c_str()));
    			data[指针数量].阵营=1;
    			}
    			else if (strstr(过滤类名.c_str(), "player") != NULL||strstr(类名.c_str(), "npc_deluosi_dress_ghost") != NULL||strstr(类名.c_str(), "h55_pendant_huojian") != NULL){
    			//data[指针数量].str=getplayer(过滤类名.c_str());
    			strcpy(data[指针数量].str, getplayer(过滤类名.c_str()));
    			data[指针数量].阵营=2;
    			}
    			else if (strstr(过滤类名.c_str(), "scene") != NULL){
    			const char* scene_result = getscene(过滤类名.c_str());
    			if (scene_result == NULL) continue;
    			strcpy(data[指针数量].str, scene_result);
    			data[指针数量].阵营=3;
    			}
    			else if (strstr(过滤类名.c_str(), "prop") != NULL){
    			const char* prop_result = getprop(过滤类名.c_str());
    			if (prop_result == NULL) continue;
    			strcpy(data[指针数量].str, prop_result);
    			data[指针数量].阵营=4;
    			}

    			else if (strstr(过滤类名.c_str(), "redqueen") != NULL&&strstr(过滤类名.c_str(), "mirror") != NULL&&strstr(过滤类名.c_str(), "model") != NULL){
    			data[指针数量].阵营=5;
    			}
    			//sprintf(data[指针数量].类名, "%s", 过滤类名.c_str());
    			strcpy(data[指针数量].类名, 过滤类名.c_str());
    			data[指针数量].objcoor=getPtr64(对象+0x28);
    			if (!首帧打印){
        			printf("[实体] %s 地址=0x%llX 坐标地址=0x%llX 坐标=(%.1f,%.1f,%.1f) 类名=%s\n",
        			       data[指针数量].str, (unsigned long long)对象,
        			       (unsigned long long)data[指针数量].objcoor,
        			       getFloat(data[指针数量].objcoor + 0xa0),
        			       getFloat(data[指针数量].objcoor + 0xa4),
        			       getFloat(data[指针数量].objcoor + 0xa8),
        			       data[指针数量].类名);
        		}
    			指针数量++;
			}
    			
			//红夫人模式（不依赖遍历顺序，直接用jxpd区分本体/镜像）
			if (pd1==450){
			    int jxpd = getDword(对象 + 0x70);
			    uintptr_t coorPtr = getPtr64(对象 + 0x28);
			    if (strstr(过滤类名.c_str(), "boss") != NULL && strstr(过滤类名.c_str(), "redqueen") != NULL && strstr(过滤类名.c_str(), "mirror") == NULL
			        && getFloat(coorPtr + 0xa0) != 0 && getFloat(coorPtr + 0xa8) != 0) {
			        if (jxpd != 65150 && getFloat(coorPtr + 0xa4) >= -300) {
			            红夫人 = 对象;      // 本体：非鬼魂且在地面
			        } else {
			            红夫人镜像 = 对象;  // 镜像：鬼魂状态或在地下
			        }
			    }
    			if (strstr(过滤类名.c_str(), "boss") != NULL && strstr(过滤类名.c_str(), "mirror") != NULL
    			    && getFloat(coorPtr + 0xa0) != 0 && getFloat(coorPtr + 0xa8) != 0)
    			{
    		    	镜子=对象;
    			}    			
			}    						
        }
        if (!首帧打印){
            首帧打印 = true;
            printf("[首帧调试] 矩阵16值已打印 实体列表已打印\n");
        }
        数量 = 指针数量;
        sleep(3);
    }
}






void Draw_Main(ImDrawList *Draw){
    if (libbase == 0 || 状态 == 0) return;  // 数据未就绪，跳过本帧绘制
    int 内核人物数量 = 0;
    const bool 模仿者绘制中 = false; // 发布版本: 注入功能已停用, SoHook::IsCopycatDrawingActive() 不再调用
    
    Matrix = getPtr64(getPtr64(libbase + MatrixOffset) + 0xa58) + 0x2c0; //矩阵
    M.X = getFloat(Matrix - 0x290);
    M.Z = getFloat(Matrix - 0x290+4);
    M.Y = getFloat(Matrix - 0x290+8);
    
    // 红夫人坐标读取（加保护检查，防止地址为0时崩溃）
    if (红夫人 != 0) {
        uintptr_t 红夫人坐标指针 = getPtr64(红夫人 + 0x28);
        if (红夫人坐标指针 != 0) {
            红夫人X = getFloat(红夫人坐标指针 + 0xa0);
            红夫人Z = getFloat(红夫人坐标指针 + 0xa4);
            红夫人Y = getFloat(红夫人坐标指针 + 0xa8);
        }
    }
    if (红夫人镜像 != 0) {
        uintptr_t 镜像坐标指针 = getPtr64(红夫人镜像 + 0x28);
        if (镜像坐标指针 != 0) {
            红夫人镜像X = getFloat(镜像坐标指针 + 0xa0);
            红夫人镜像Z = getFloat(镜像坐标指针 + 0xa4);
            红夫人镜像Y = getFloat(镜像坐标指针 + 0xa8);
        }
    }
    if (红夫人Z >= -300 && 红夫人镜像Z >= -300&&红夫人X != 0 && 红夫人Y != 0 && 红夫人镜像X != 0 && 红夫人镜像Y != 0) {
        mirror=true;
    }else{
        mirror=false;
    }
    vm_readv(Matrix, matrix, 64);
    if (!首帧矩阵){
        首帧矩阵 = true;
        printf("[矩阵]");
        for (int i = 0; i < 16; i++) printf(" %.4f", matrix[i]);
        printf("\n");
    }  // 直接从Matrix读16个float
    if (show_draw_prophet){
        auto textSize = ImGui::CalcTextSize(监管者预知, 0, 25);
        Draw->AddText({px-(textSize.x/2),130}, 红色, 监管者预知);
    }

    for (int i = 0; i < 数量; i++){
    
        if (strstr(data[i].类名, "buzz") != NULL)
            continue;//跳过不知所谓的东西
        if (strstr(data[i].类名, "nvyao.gim") != NULL)
            continue;//跳过女妖蜡烛
        D.X = getFloat(data[i].objcoor + 0xa0);
        D.Z = getFloat(data[i].objcoor + 0xa4);
        D.Y = getFloat(data[i].objcoor + 0xa8);

        // 已锁定的自身: 只负责判断"还活着没"+持续更新坐标, 不重新参与后面的识别/绘制逻辑
        if (自身 != 0 && data[i].obj == 自身) {
            if (!ShouldSkipEntity(data[i]) && !(D.X==0 && D.Y==0) && D.Z>-300) {
                Z.X = D.X; Z.Z = D.Z; Z.Y = D.Y;
            }
            continue; // 不管有效无效, 自身都不需要再走下面的常规实体流程
        }

        if (D.X==0 || D.Y==0){
		    continue;//跳过xy0
		}
		if (D.Z<=-300){
		    continue;//跳过地下
		}
		if (data[i].阵营 == 1 || data[i].阵营 == 2) 内核人物数量++;
		int jxpd = getDword(data[i].obj + 0x70);
		camera = matrix[3] * D.X + matrix[7] * D.Z + matrix[11] * D.Y + matrix[15];
        距离 = sqrt(pow(D.X - Z.X, 2) + pow(D.Y - Z.Y, 2) + pow(D.Z - Z.Z, 2)) / 距离比例;
        矩阵视野距离 = sqrt(pow(D.X - M.X, 2) + pow(D.Y - M.Y, 2) + pow(D.Z - M.Z, 2)) / 距离比例;
		孽蜥距离 = sqrt(pow(D.X - Z.X, 2) + pow(D.Y - Z.Y, 2)) / 距离比例;
		r_x = px + (matrix[0] * D.X + matrix[4] * D.Z + matrix[8] * D.Y + matrix[12]) / camera * px;
        r_y = py - (matrix[1] * D.X + matrix[5] * (D.Z+ 8.5) + matrix[9] * (D.Y) + matrix[13]) / camera * py;
        r_w = py - (matrix[1] * D.X + matrix[5] * (D.Z+ 28.5) + matrix[9] * (D.Y) + matrix[13]) / camera * py;
												
		W = (r_y - r_w) / 2;	// 宽度
		H = r_y - r_w;		// 高度
		X1 = r_x - (r_y - r_w) / 4;	// X1
		Y1 = r_y - H / 2;	// Y1
		X2 = X1 + W;		// X2
		Y2 = Y1 + H;		// Y2
		if (距离>=300){
            continue;
        }
        if (W>0){
            if (Debugging){                
                std::string test;
                sprintf(objtext, "%lx", data[i].obj);
                test += " [";
                test += std::to_string((int) 距离);    
                test += " 米]  0x";
                test += objtext;    
                test += " [类名] ";
                test += data[i].类名;
                auto textSize = ImGui::CalcTextSize(test.c_str(), 0, 25);
                Draw->AddText({r_x-(textSize.x/2),r_y}, ImColor(255,200,0,255), test.c_str());
            }
        
            if (strstr(data[i].类名, "camera") != NULL && 距离 < 38){
                if (getDword(data[i].obj + 0xa8)==256){
		            continue;//跳过使用过的椅子
		        }
                std::string s;
			    if (show_draw_Camera){                          
                    s += "[摄影机]";
                }
                auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
                Draw->AddText({r_x-(textSize.x/2),r_y}, ImColor(255,200,0,255), s.c_str());
            }
            

		
    		if (data[i].阵营==3)
    		{
    		    if (strstr(data[i].类名, "dm65_scene_prop_30") != NULL){
    			    std::string s;
    		        if (show_draw_Door){
                        s += "[大门]";
                    }
                    auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
                    Draw->AddText({r_x-(textSize.x/2),r_y}, ImColor(255,200,0,255), s.c_str());
    		    }
    		
    		    else if (strstr(data[i].类名, "dm65_scene_prop_01") != NULL&&距离<38){
    			    std::string s;
    		        if (show_draw_Box){                          
    		            if (getDword(data[i].obj + 0x148)==0){
    		                continue;//跳过使用过的箱子
    		            }
                        s += "[道具箱]";
                    }    
                    auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
                    Draw->AddText({r_x-(textSize.x/2),r_y}, 红色, s.c_str());
    		    }

    		    else if (strstr(data[i].类名, "dm65_scene_gallow") != NULL&&strstr(data[i].类名, "bashou") == NULL&&距离<38){
    			    std::string s;
    		        if (show_draw_Chair){                          
    		            if (getDword(data[i].obj + 0xa8)==256){
    		                continue;//跳过使用过的椅子
    		            }
                        s += "[狂欢之椅]";
                    }
                    auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
                    Draw->AddText({r_x-(textSize.x/2),r_y}, 红色, s.c_str());
    		    }
    		
    		    else if (strstr(data[i].类名, "dm65_scene_prop_76") != NULL){
    			    std::string s;
    		        if (show_draw_Cellar){                                                  
                            s += "[地窖] ";
                            s += std::to_string((int) 距离);    
                            s += " 米 ";
                    }
                    auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
                    Draw->AddText({r_x-(textSize.x/2),r_y}, 紫色, s.c_str());
    		    }

    	    else if (strstr(data[i].类名, "sender") != NULL){
    	        if (show_draw_secret_mechine){
    	            std::string s;
    	            std::ostringstream oss;
    	            oss << std::fixed << std::setprecision(1) << 距离;
    	            s += "[密码机] " + oss.str() + " 米";
    	            auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
    	            Draw->AddText({r_x-(textSize.x/2), r_y},
    	                (距离 >= 61 && 距离 <= 63) ? 绿色 : ImColor(255, 255, 255, 255),
    	                s.c_str());
    	        }
    	    }
    		}
	
		    if (show_draw_Prop&&data[i].阵营==4){
                const char* propName = getprop(data[i].类名);
                if (propName) {
                    std::string s = propName;
                    s += std::to_string((int) 距离);
                    s += " 米";
                    auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
                    Draw->AddText({r_x-(textSize.x/2),r_y}, ImColor(255,200,0,255), s.c_str());
                }
            }

            int zy;//=getbool(data[i].obj + 0xaa);
            vm_readv(data[i].obj + 0xaa, &zy, 1);

            if (!ShouldSkipEntity(data[i])){
                if (show_draw_Role&&strstr(data[i].类名, "chr") != NULL){
                    std::string test;
                    test += " [";
                    test += std::to_string((int) 距离);    
                    test += " 米]  0x";
                    test += objtext;    
                    test += " [类名] ";
                    test += data[i].类名;
                    auto textSize = ImGui::CalcTextSize(test.c_str(), 0, 25);
                    Draw->AddText({r_x-(textSize.x/2),r_y}, ImColor(255,200,0,255), test.c_str());
                }
                			            
                if (camera < 40 && camera > 10 && zy&&(data[i].阵营==1||data[i].阵营==2)){
                    自身 = data[i].obj;
                    Z.X = D.X;
                    Z.Z = D.Z;
                    Z.Y = D.Y;
                    自身阵营=对象阵营;
                       continue;
                }
               
                std::string s;


                if (!模仿者绘制中 && (data[i].阵营==1||data[i].阵营==2)){
                    s+=data[i].str;
                    auto textSize = ImGui::CalcTextSize(s.c_str(), 0, 25);
                    Draw->AddText({X1 + W/2-(textSize.x/2),Y1-45}, ImColor(255,200,0,255), s.c_str());
                    if (show_draw_Rect){
        			    if (jxpd==65150)
        			        ImGui::GetForegroundDrawList()->AddRect({X1, Y1},{X2, Y2}, BotBoneColor,3, 0,1.8);			        	        
        			    else if (data[i].阵营==1)
        			        ImGui::GetForegroundDrawList()->AddRect({X1, Y1},{X2, Y2}, BoneColor,3, 0,1.8f);
        			    else if (data[i].阵营==2)
        			        ImGui::GetForegroundDrawList()->AddRect({X1, Y1},{X2, Y2}, 绿色,3, 0,1.8f);
        			}
                    if (show_draw_Distance){
                        std::string 人物距离;
                        人物距离 += std::to_string((int) 距离);
                        人物距离 += " 米";
                        auto textSize = ImGui::CalcTextSize(人物距离.c_str(), 0, 25);
                        Draw->AddText({X1 + W/2-(textSize.x/2),Y2+10}, ImColor(255,200,0,255), 人物距离.c_str());
                    }
                    
                    if (show_draw_Line){
                        ImGui::GetForegroundDrawList()->AddLine({px, 160},{X1 + W/2, Y1}, ImColor(255, 255, 255),2);
                    }                                          
                }
            }                                                          			
    	}//判断w    
	     
	                
	   //红夫人镜像                                   
	    if (mirror&&redqueenmod){
            if (getFloat(data[i].obj+0x1a0)==450&&data[i].阵营==2){
                std::string ss;
                calculate_mirror_reflection(红夫人X, 红夫人Y, 红夫人镜像X, 红夫人镜像Y, D.X, D.Y, &xs_prime_mirror, &ys_prime_mirror);
                calculate_line_reflection(红夫人X, 红夫人Y, 红夫人镜像X,  红夫人镜像Y,xs_prime_mirror, ys_prime_mirror, &D.X, &D.Y);
                camera = matrix[3] * D.X + matrix[7] * D.Z + matrix[11] * D.Y + matrix[15];
                距离 = sqrt(pow(D.X - Z.X, 2) + pow(D.Y - Z.Y, 2) + pow(D.Z - Z.Z, 2)) / 距离比例;
        		r_x = px + (matrix[0] * D.X + matrix[4] * D.Z + matrix[8] * D.Y + matrix[12]) / camera * px;
                r_y = py - (matrix[1] * D.X + matrix[5] * (D.Z+ 8.5) + matrix[9] * (D.Y) + matrix[13]) / camera * py;
                r_w = py - (matrix[1] * D.X + matrix[5] * (D.Z+ 28.5) + matrix[9] * (D.Y) + matrix[13]) / camera * py;
												
        		W = (r_y - r_w) / 2;	// 宽度
        		H = r_y - r_w;		// 高度
        		X1 = r_x - (r_y - r_w) / 4;	// X1
        		Y1 = r_y - H / 2;	// Y1
        		X2 = X1 + W;		// X2
        		Y2 = Y1 + H;		// Y2
        
                if (W>0){

                    ss += data[i].str;
                    auto textSize = ImGui::CalcTextSize(ss.c_str(), 0, 25);
                    Draw->AddText({X1 + W/2-(textSize.x/2),Y1-45}, BotBoneColor, ss.c_str());
                        
                    if (show_draw_Rect){
                        ImGui::GetForegroundDrawList()->AddRect({X1, Y1},{X2, Y2}, BotBoneColor,3, 0,1.8f);
    			    }
    			        
                    if (show_draw_Distance){
                        std::string 镜像距离;
                        镜像距离 += std::to_string((int) 距离);
                        镜像距离 += " 米";
                        auto textSize = ImGui::CalcTextSize(镜像距离.c_str(), 0, 25);
                        Draw->AddText({X1 + W/2-(textSize.x/2),Y2+10}, BotBoneColor, 镜像距离.c_str());
                    }
                        
                    if (show_draw_Line){
                        ImGui::GetForegroundDrawList()->AddLine({px, 160},{X1 + W/2, Y1}, ImColor(255, 255, 255),2);
                    }            
                }//判断W
            }                
        }
    }
    // 发布版本: 注入功能已停用
    // if (show_sohook)
    //     SoHook::DrawOverlay(Draw, matrix, px, py, 内核人物数量,
    //                         Z.X, Z.Z, Z.Y, 距离比例);
    show_draw_ClassName = 0;
}
        

size_t get_memory_usage_kb() {
    FILE* file = fopen("/proc/self/statm", "r");
    if (!file) return 0;
    size_t size = 0, resident = 0;
    fscanf(file, "%zu %zu", &size, &resident);
    fclose(file);
    return resident * 4;
}

int GetInputDeviceCount() {
    DIR *dir = opendir("/dev/input/");
    if (!dir) return -1;
    dirent *ptr = NULL;
    int count = 0;
    while ((ptr = readdir(dir)) != NULL) {
        if (strstr(ptr->d_name, "event"))
            count++;
    }
    closedir(dir);
    return count ? count : -1;
}

void VolumeKeyHide() {
    int EventCount = GetInputDeviceCount();
    if (EventCount <= 0) return;

    int *fdArray = (int *)malloc(EventCount * sizeof(int));
    if (!fdArray) return;

    for (int i = 0; i < EventCount; i++) {
        char temp[128];
        sprintf(temp, "/dev/input/event%d", i);
        fdArray[i] = open(temp, O_RDWR | O_NONBLOCK);
    }

    input_event ev;
    while (1) {
        for (int i = 0; i < EventCount; i++) {
            if (fdArray[i] < 0) continue;
            memset(&ev, 0, sizeof(ev));
            while (read(fdArray[i], &ev, sizeof(ev)) == sizeof(ev)) {
                if (ev.type == EV_KEY && ev.code == KEY_VOLUMEDOWN && ev.value == 1)
                    voice = false;
                if (ev.type == EV_KEY && ev.code == KEY_VOLUMEUP && ev.value == 1)
                    voice = true;
            }
        }
        show_window = voice;
        usleep(10000);
    }
    free(fdArray);
}

void Layout_tick_UI(bool *main_thread_flag) {
    static bool volume_thread_started = false;
    if (!volume_thread_started) {
        std::thread(VolumeKeyHide).detach();
        volume_thread_started = true;
    }

    px = static_cast<float>(displayInfo.width) / 2;
    py = static_cast<float>(displayInfo.height) / 2;
    // 发布版本: 注入功能已停用
    // SoHook::Update(pid);

    Draw_Main(ImGui::GetForegroundDrawList());

    if (show_window) {
        ImGui::Begin("New_Edition", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if (::permeate_record_ini) {
            ImGui::SetWindowPos({LastCoordinate.Pos_x, LastCoordinate.Pos_y});
            ImGui::SetWindowSize({LastCoordinate.Size_x, LastCoordinate.Size_y});
            permeate_record_ini = false;
        }

        ImGui::Text("渲染模式 : %s, gui版本 : %s", graphics->RenderName, IMGUI_VERSION);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "帧率 %.1f FPS", ImGui::GetIO().Framerate);

        size_t mem_kb = get_memory_usage_kb();
        if (mem_kb >= 1024)
            ImGui::Text("内存占用: %.2f MB", mem_kb / 1024.0f);
        else
            ImGui::Text("内存占用: %zu KB", mem_kb);

        ImGui::Text("数据状态:");
        if (状态 == 2)
            ImGui::TextColored(ImVec4(0.0f, 205.0f, 0.0f, 100.0f), "已获取到游戏数据");
        else if (状态 == 1)
            ImGui::TextColored(ImVec4(255.0f, 0.0f, 0.0f, 100.0f), "正在获取游戏数据");

        if (ImGui::CollapsingHeader("基础信息")) {
            ImGui::Text("游戏进程:%d", pid);
            ImGui::Text("模块入口:%lx", libbase);
            ImGui::Text("游戏包名:%s", extractedString);
            ImGui::Text("矩阵地址:%lx", Matrix);
            ImGui::Text("数组地址:%lx", Arrayaddr);
            ImGui::Text("矩阵偏移:%lx", MatrixOffset);
            ImGui::Text("模块页数:%d", c);
            ImGui::Text("数组偏移:%lx", ArrayaddrOffset);
            ImGui::Text("数据获取状态:%d", 数据获取状态);
            ImGui::Text("监管者:%s", 监管者预知);
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("绘制设置")) {
            ImGui::Checkbox("显示鬼魂", &inform_ghost);
            ImGui::SameLine();
            ImGui::Checkbox("预知监管", &show_draw_prophet);

            ImGui::Checkbox("绘制道具", &show_draw_Prop);
            ImGui::SameLine();
            ImGui::Checkbox("夫人模式", &redqueenmod);

            ImGui::Checkbox("绘制调试", &Debugging);
            ImGui::SameLine();
            ImGui::Checkbox("显示密码机", &show_draw_secret_mechine);

            // 发布版本: 注入功能已停用
            // ImGui::Checkbox("骨骼与进度", &show_sohook);

            ImGui::Text("");
            if (ImGui::Button("结束进程"))
                exit(0);
        }

        // 发布版本: 注入功能已停用
        // if (show_sohook) {
        //     ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        //     if (ImGui::CollapsingHeader("骨骼与进度")) {
        //         SoHook::RenderPanel(pid);
        //     }
        // }

        g_window = ImGui::GetCurrentWindow();
        ImGui::End();
    }
}
