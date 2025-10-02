#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

class c_driver {
	private:
	int has_upper = 0;
	int has_lower = 0;
	int has_symbol = 0;
	int has_digit = 0;
	int fd;
	pid_t pid;

	typedef struct _COPY_MEMORY {
		pid_t pid;
		uintptr_t addr;
		void* buffer;
		size_t size;
	} COPY_MEMORY, *PCOPY_MEMORY;

	typedef struct _MODULE_BASE {
		pid_t pid;
		char* name;
		uintptr_t base;
	} MODULE_BASE, *PMODULE_BASE;

	enum OPERATIONS {
		OP_INIT_KEY = 0x800,
		OP_READ_MEM = 0x801,
		OP_WRITE_MEM = 0x802,
		OP_MODULE_BASE = 0x803,
	};
	
	int symbol_file(const char *filename) {
		//判断文件名是否含小写并且不含大写不含数字不含符号
		int length = strlen(filename);
		for (int i = 0; i < length; i++) {
			if (islower(filename[i])) {
				has_lower = 1;
			} else if (isupper(filename[i])) {
				has_upper = 1;
			} else if (ispunct(filename[i])) {
				has_symbol = 1;
			} else if (isdigit(filename[i])) {
				has_digit = 1;
			}
		}
		return has_lower && !has_upper && !has_symbol && !has_digit;
	}
	
	char *driver_path() {
	// 打开目录
		const char *dev_path = "/dev";
		DIR *dir = opendir(dev_path);
		if (dir == NULL){
			printf("无法打开/dev目录\n");
			return NULL;
		}

		struct dirent *entry;
		static char file_path[256];
		while ((entry = readdir(dir)) != NULL) {
			// 跳过当前目录和上级目录
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
			sprintf(file_path, "%s/%s", dev_path, entry->d_name);
			// 获取文件stat结构
			struct stat file_info;
			if (stat(file_path, &file_info) < 0) {
				continue;
			}
			
			// 跳过gpio接口
			if (strstr(entry->d_name, "gpiochip") != NULL) {
				continue;
			}

			// 检查是否为驱动文件
			if ((S_ISCHR(file_info.st_mode) || S_ISBLK(file_info.st_mode))
				&& strchr(entry->d_name, '_') == NULL
				&& strchr(entry->d_name, '-') == NULL
				&& strchr(entry->d_name, ':') == NULL) {
				// 过滤标准输入输出
				if (strcmp(entry->d_name, "stdin") == 0
				|| strcmp(entry->d_name, "stdout") == 0
				|| strcmp(entry->d_name, "stderr") == 0) {
					continue;
				}
				
				size_t file_name_length = strlen(entry->d_name);
				time_t current_time;
				time(&current_time);
				int current_year = localtime(&current_time)->tm_year + 1900;
				int file_year = localtime(&file_info.st_ctime)->tm_year + 1900;
				//跳过1980年前的文件
				if (file_year <= 1980) {
					continue;
				}
				
				time_t atime = file_info.st_atime;
				time_t ctime = file_info.st_ctime;
				// 检查最近访问时间和修改时间是否一致并且文件名是否是symbol文件
				if (atime == ctime) {
					//检查mode权限类型是否为S_IFREG(普通文件)和大小还有gid和uid是否为0(root)并且文件名称长度在7位或7位以下
					if ((file_info.st_mode & S_IFMT) == 8192
					&& file_info.st_size == 0
					&& file_info.st_gid == 0
					&& file_info.st_uid == 0
					&& file_name_length <= 6) {
						closedir(dir);
						return file_path;
					}
				}
			}
		}
		closedir(dir);
		return NULL;
	}
	
	char *qx8() {
		// 打开目录
		char name[128], path[256];
		static char file_path[256];
		DIR *dir = opendir("/sys/devices/virtual/");
		if (dir == NULL)
			return NULL;
		struct dirent *Neko1;
		while ((Neko1 = readdir(dir)) != NULL) {
			// 跳过当前目录和上级目录
			if (strcmp(Neko1->d_name, ".") == 0 || strcmp(Neko1->d_name, "..") == 0)
				continue;

			if (strcmp(Neko1->d_name, "usbmon") == 0)
				continue;

			int ret=0;
			sprintf(path, "/sys/devices/virtual/%s/", Neko1->d_name);
			DIR *subdir = opendir(path);
			if (subdir == NULL)
				continue;
			struct dirent *Neko2;
			while ((Neko2 = readdir(subdir)) != NULL) {
				// 跳过当前目录和上级目录
				if (strcmp(Neko2->d_name, ".") == 0 || strcmp(Neko2->d_name, "..") == 0)
					continue;
				ret++;
				if (ret == 1)
					sprintf(name, "%s", Neko2->d_name);
			}
			closedir(subdir);
			if (ret > 1)
				continue;
	
			sprintf(file_path, "/sys/devices/virtual/%s/%s/dev", Neko1->d_name, name);
			if (access(file_path,F_OK) != 0)
				continue;
			
			int neko, zero;
			FILE *file = fopen(file_path, "r");
			if (file == NULL)
				continue;
			char line[256];
			while (fgets(line, sizeof(line), file))
			{
				if (sscanf(line, "%d:%d", &neko, &zero) == 2)
					break;
			}
			fclose(file);
			if (zero != 0)
				continue;
			
			sprintf(file_path, "/dev/%s", name);
			if (access(file_path,F_OK) != 0)
			{
				mode_t mode = S_IFCHR | 0600;
				int ret = mknod(file_path, mode, (neko << 8) | 0);
				if (ret == 0) {
					return file_path;
				}
			}
		}
		return NULL;
	}
	
	char* qx10()
	{
    	const char* command = "dir=$(ls -l /proc/*/exe 2>/dev/null | grep -E '/data/[^/]* \\(deleted\\)' | sed 's/ /\\n/g' | grep '/proc' | sed 's/\\/[^/]*$//g');if [[ \"$dir\" ]]; then sbwj=$(head -n 1 \"$dir/comm\");open_file=\"\";for file in \"$dir\"/fd/*; do link=$(readlink \"$file\");if [[ \"$link\" == \"/dev/$sbwj (deleted)\" ]]; then open_file=\"$file\";break;fi;done;if [[ -n \"$open_file\" ]]; then nhjd=$(echo \"$open_file\");sbid=$(ls -L -l \"$nhjd\" | sed 's/\\([^,]*\\).*/\\1/' | sed 's/.*root //');echo \"/dev/$sbwj\";rm -rf \"/dev/$sbwj\";mknod \"/dev/$sbwj\" c \"$sbid\" 0;fi;fi;";
    	FILE* file = popen(command, "r");
    	if (file == NULL) {
        	return NULL;
    	}
    	static char result[512];
    	if (fgets(result, sizeof(result), file) == NULL) {
			return NULL;
		}
    	pclose(file);
        result[strlen(result)-1] = '\0';
		return result;
    }
	
	int open_driver() {
		char *dev_path3 = qx10();
		if (dev_path3 != NULL) {
			fd = open(dev_path3, O_RDWR);
			if (fd>0){
				printf("隐藏驱动：%s\n", dev_path3);
				unlink(dev_path3);
				return 1;
			}
		}
		char *dev_path1 = driver_path();
		if (dev_path1 != NULL) {
			fd = open(dev_path1, O_RDWR);
			if (fd>0){
				printf("must驱动文件：%s\n", dev_path1);
				unlink(dev_path1);
				return 1;
			}
		}
		char *dev_path2 = qx8();
		if (dev_path2 != NULL) {
			fd = open(dev_path2, O_RDWR);
			if (fd>0){
				printf("隐藏驱动：%s\n", dev_path2);
				unlink(dev_path2);
				return 1;
			}
		}
		return 0;
	}
	
	public:
	c_driver() {
		open_driver();
		if (fd <= 0) {
			printf("[-] open driver failed\n");
			exit(0);
		}
	}

	~c_driver() {
		//wont be called
		if (fd > 0)
			close(fd);
	}

	void initialize(pid_t pid) {
		this->pid = pid;
	}

	bool init_key(char* key) {
		char buf[0x100];
		strcpy(buf,key);
		if (ioctl(fd, OP_INIT_KEY, buf) != 0) {
			return false;
		}
		return true;
	}

	bool read(uintptr_t addr, void *buffer, size_t size) {
		COPY_MEMORY cm;

		cm.pid = this->pid;
		cm.addr = addr;
		cm.buffer = buffer;
		cm.size = size;

		if (ioctl(fd, OP_READ_MEM, &cm) != 0) {
			return false;
		}
		return true;
	}

	bool write(uintptr_t addr, void *buffer, size_t size) {
		COPY_MEMORY cm;

		cm.pid = this->pid;
		cm.addr = addr;
		cm.buffer = buffer;
		cm.size = size;

		if (ioctl(fd, OP_WRITE_MEM, &cm) != 0) {
			return false;
		}
		return true;
	}

	template <typename T>
	T read(uintptr_t addr) {
		T res;
		if (this->read(addr, &res, sizeof(T)))
			return res;
		return {};
	}

	template <typename T>
	bool write(uintptr_t addr,T value) {
		return this->write(addr, &value, sizeof(T));
	}

	uintptr_t get_module_base(char* name) {
		MODULE_BASE mb;
		char buf[0x100];
		strcpy(buf,name);
		mb.pid = this->pid;
		mb.name = buf;

		if (ioctl(fd, OP_MODULE_BASE, &mb) != 0) {
			return 0;
		}
		return mb.base;
	}
};

static c_driver *driver = new c_driver();

/*--------------------------------------------------------------------------------------------------------*/

typedef char PACKAGENAME;	// 包名
pid_t pid;	// 进程ID
/*
int get_name_pid(const std::string& PackageName)
{
    FILE* fp;
    std::string cmd = "pidof " + PackageName;
    fp = popen(cmd.c_str(), "r");
    fscanf(fp, "%d", &pid);
    pclose(fp);
    if (pid > 0)
    {
        driver->initialize(pid);
    }
    return pid;
}
*/
int get_name_pid(char* PackageName)
{
	FILE* fp;
    char cmd[0x100] = "pidof ";
    strcat(cmd, PackageName);
    fp = popen(cmd,"r");
    fscanf(fp,"%d", &pid);
    pclose(fp);
	if (pid > 0)
	{
		driver->initialize(pid);
	}
    return pid;
}

bool PidExamIne()
{
	char path[128];
	sprintf(path, "/proc/%d",pid);
	if (access(path,F_OK) != 0)
	{
		printf("\033[31;1m");
		puts("获取进程PID失败!");
		exit(1);
	}
	return true;
}
/*
void* getModuleBase(char* module_name)
{
    uintptr_t base = 0;
    base = driver->get_module_base(module_name);
    return reinterpret_cast<void*>(base);
}
*/
long getModuleBase(char* module_name)
{
	uintptr_t base=0;
	base = driver->get_module_base(module_name);
	return base;
}

// 读取内存
bool vm_readv(unsigned long address, void *buffer, size_t size)
{
	return driver->read(address, buffer, size);
}

// 写入内存
bool vm_writev(unsigned long address, void *buffer, size_t size)
{
	return driver->write(address, buffer, size);
}

// 获取F类内存
float getfloat(unsigned long addr)
{
	float var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取F类内存
float getFloat(unsigned long addr)
{
	float var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取D类内存
int getdword(unsigned long addr)
{
	int var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取D类内存
int getDword(unsigned long addr)
{
	int var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取指针(32位游戏)
unsigned int getPtr32(unsigned int addr)
{
	unsigned int var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取指针(64位游戏)
unsigned long getPtr64(unsigned long addr)
{
	addr=addr& 0xffffffffffffff;
	long var = 0;
    driver->read(addr, &var, 8);
	return var& 0xffffffffffffff;
}


   // 写入D类内存 
void writedword(unsigned long addr, int data)
{
   vm_writev(addr, &data, 4);
} 
    


void writefloat(unsigned long addr, float data)
{
	vm_writev(addr, &data, 4);
}