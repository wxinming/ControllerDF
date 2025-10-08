#pragma once
#pragma execution_character_set("utf-8")

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <QThread>

#include <QStandardPaths>

#include <QHostAddress>

#include <QTcpSocket>

#include <Power/Power.h>
#pragma comment(lib, "Power.lib")

#include <Relay/Relay.h>
#pragma comment(lib, "Relay.lib")

#include <Voltmeter/Voltmeter.h>
#pragma comment(lib, "Voltmeter.lib")

#include <Amperemeter/Amperemeter.h>
#pragma comment(lib, "Amperemeter.lib")

#include <libcan/libcan.h>
#include <libuds/libuds.h>
#ifdef QT_DEBUG
#pragma comment(lib, "libcand.lib")
#pragma comment(lib, "libudsd.lib")
#else
#pragma comment(lib, "libcan.lib")
#pragma comment(lib, "libuds.lib")
#endif

#include <Lin/Lin.h>
#pragma comment(lib, "Lin.lib")

#include <WifiMgr/WifiMgr.h>
#pragma comment(lib, "WifiMgr.lib")

#include <TSCPrinterMgr/TSCPrinterMgr.h>
#pragma comment(lib, "TSCPrinterMgr.lib")

#include <CaptureCard/CaptureCard.h>
#pragma comment(lib, "CaptureCard.lib")

#include <TcpClient/TcpClient.h>
#pragma comment(lib, "TcpClient.lib")

#include "../Control/QLabelEx.h"

#include "../Manage/Json.h"

#include "../Algorithm/RayAxis.h"

#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "urlmon.lib")

#include <Dependent/Lib/curl/include/curl.h>
#pragma comment(lib, "../../Dependent/Lib/curl/lib/libcurl.lib")

#include <Dependent/Lib/vlc-2.2.4/include/vlc.h>
#pragma comment(lib, "../../Dependent/Lib/vlc-2.2.4/lib/libvlc.lib")
#pragma comment(lib, "../../Dependent/Lib/vlc-2.2.4/lib/libvlccore.lib")

#include <Tesseract/Tesseract.h>
#if defined(QT_DEBUG)
#pragma comment(lib, "../../Dependent/Lib/Tesseract/lib/x86/Debug/Tesseract.lib")
#else
#pragma comment(lib, "../../Dependent/Lib/Tesseract/lib/x86/Release/Tesseract.lib")
#endif

#include <process.h>

#include <wincon.h>
#include <WinInet.h>
#pragma comment(lib, "wininet.lib")

#include <Urlmon.h>
#pragma comment(lib, "Urlmon.lib")

#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")

/*
* @brief,写入日志
* @param1,格式化字符串
* @param2,可变参数
*/
#define WRITE_LOG(format, ...) [&]()\
{\
	auto s = Q_SPRINTF(format, ##__VA_ARGS__);\
	m_logList.append(s); \
}()

/*
* @brief,获取机种名称
* @return,QString
*/
#define GET_TYPE_NAME() UTIL_JSON->getDeviceConfigValue("机种名称")

/*
* @brief,启动延时
* @return,ulong
*/
#define START_DELAY static_cast<ulong>(UTIL_JSON->getParsedThresholdConfig().startDelay)

/*
* @brief,逻辑索引
* @return,int
*/
#define LOGIC_INDEX m_logicIndex

/*
* @brief,获取测试逻辑
* @return,bool
*/
#define GET_TEST_LOGIC [&]()->bool {\
	if (LOGIC_INDEX > sizeof(LogicConfig) * 8 - 1)\
		return true;\
	return static_cast<bool>(utility::getBit(*reinterpret_cast<int*>(&UTIL_JSON->getParsedDefConfig()->logic), LOGIC_INDEX));\
}

/*
* @brief,判断机种名是否为NAME
* @param1,对比的名字
* @return,bool
*/
#define TYPE_NAME_IS(NAME) (GET_TYPE_NAME() == NAME)

/*
* @brief,判断类名是否为NAME
* @param1,对比的名字
* @return,bool
*/
#define CLASS_NAME_IS(NAME) (getClassName() == NAME)

/*
* @brief,判断类名是否包含NAME
* @param1,对比的名字
* @return,bool
*/
#define CLASS_NAME_CONTAINS(NAME) (getClassName().contains(NAME))

#define FVAL val

#define FMSG msg

#define FSIZE size

#define FDATA data

#define FVALL valList

#define CAN_PROC_FNC(...) [__VA_ARGS__](quint64 FVAL, const can::Msg& FMSG)mutable->bool

#define LIN_PROC_FNC(...) [__VA_ARGS__](quint64 FVAL, const LinMsg& FMSG)mutable->bool

#define UDS_PROC_FNC(...) [__VA_ARGS__](quint64 FVAL, int FSIZE, const uchar* FDATA)mutable->bool

#define UDS_PROC_FNC_EX(...) [__VA_ARGS__](ValList FVALL, int FSIZE, const uchar* FDATA)mutable->bool

#define TEST_PASS []()->bool{ return true; }

typedef const std::initializer_list<int>& IdList;

typedef const std::initializer_list<quint64>& ValList;

typedef const std::initializer_list<can::Msg>& CanList;

typedef const std::initializer_list<uchar>& DidList;

typedef const std::initializer_list<uchar>& DataList;

typedef const std::function<bool(quint64, const can::Msg&)>& CanProc;

typedef const std::function<bool(quint64, int, const uchar*)>& UdsProc;

typedef const std::function<bool(ValList, int, const uchar*)>& UdsProcEx;

typedef const std::function<bool(quint64, const LinMsg&)>& LinProc;

struct DETECTION_DLL_EXPORT CanProcInfo
{
	int id;
	int val;
	CanProc proc;
};

struct DETECTION_DLL_EXPORT CanProcInfoEx
{
	IdList idList;
	ValList valList;
	CanProc proc;
};

enum TestSequence
{
	TS_DEBUG_MODE,
	TS_RETEST_MODE,
	TS_JUDGE_CODE,
	TS_QUERY_STATION,
	TS_SCAN_CODE = 0x1000,
	TS_PREPARE_TEST,
	TS_CHECK_STATIC_CURRENT,
	TS_CHECK_CAN_ROUSE_SLEEP,
	TS_CHECK_COMPONENT_VOLTAGE,
	TS_CHECK_COMPONENT_RESISTOR,
	TS_CHECK_WORKING_CURRENT,
	TS_CHECK_VERSION,
	TS_WRITE_SN,
	TS_CHECK_SN,
	TS_CHECK_DATE,
	TS_WRITE_DATE,
	TS_SAVE_LOG,
	TS_TRIGGER_IMAGE,
	TS_CHECK_IMAGE,
	TS_CHECK_AVM,
	TS_TRIGGER_AVM,
	TS_EXIT_AVM,
	TS_CHECK_DVR,
	TS_CHECK_FRVIEW,
	TS_CHECK_LRVIEW,
	TS_CHECK_ALLVIEW,
	TS_CLEAR_DTC,
	TS_CHECK_DTC,
	TS_FORMAT_CARD,
	TS_UMOUNT_CARD,
	TS_CHECK_CARD,
	TS_POPUP_CARD,
	TS_CHECK_RECORD,
	TS_CHECK_PHOTOGRAPH,
	TS_CHECK_RAY_AXIS_SFR,
	TS_CHECK_RAY_AXIS,
	TS_CHECK_SFR,
	TS_CHECK_PLAYBACK,
	TS_CHECK_UART,
	TS_CHECK_KEY_VOLTAGE,
	TS_CHECK_LDW,
	TS_CHANGE_PWD,
	TS_CHECK_MIN_POWER_CURRENT,
	TS_CHECK_MAX_POWER_CURRENT,
	TS_RESET_FACTORY,
	TS_WRITE_SET,
	TS_WAIT_STARTUP,
	TS_CAN_PROC0,
	TS_CAN_PROC1,
	TS_CAN_PROC2,
	TS_CAN_PROC3,
	TS_CAN_PROC4,
	TS_CAN_PROC5,
	TS_CAN_PROC6,
	TS_CAN_PROC7,
	TS_CAN_PROC8,
	TS_CAN_PROC9,
	TS_LIN_PROC0,
	TS_LIN_PROC1,
	TS_LIN_PROC2,
	TS_LIN_PROC3,
	TS_LIN_PROC4,
	TS_LIN_PROC5,
	TS_LIN_PROC6,
	TS_LIN_PROC7,
	TS_LIN_PROC8,
	TS_LIN_PROC9,
	TS_UDS_PROC0,
	TS_UDS_PROC1,
	TS_UDS_PROC2,
	TS_UDS_PROC3,
	TS_UDS_PROC4,
	TS_UDS_PROC5,
	TS_UDS_PROC6,
	TS_UDS_PROC7,
	TS_UDS_PROC8,
	TS_UDS_PROC9,
	TS_CHECK_NETWORK,
	TS_CHECK_PING,
	TS_PRINT_LABEL,
	TS_CHECK_OTHER0,
	TS_CHECK_OTHER1,
	TS_CHECK_OTHER2,
	TS_CHECK_OTHER3,
	TS_CHECK_OTHER4,
	TS_CHECK_OTHER5,
	TS_CHECK_OTHER6,
	TS_CHECK_OTHER7,
	TS_CHECK_OTHER8,
	TS_CHECK_OTHER9,
	TS_OUTER_NET_COMMUNICATE,
	TS_CHECK_J2_VERSION,
	TS_CHECK_FAST_STARTUP_IMAGE,
	TS_CHECK_NETWORK_VERSION,
	TS_CHECK_100MB_NETWORK,
	TS_CHECK_1GB_NETWORK,
	TS_CHECK_8_CHANNEL_CAN,
	TS_RECOGNIZE_IMAGE,
	TS_CHECK_CAN_ROUSE,
	TS_CHECK_CAN_SLEEP,
	TS_DOWNLOAD_VIDEO,
	TS_DOWNLOAD_PHOTO,
	TS_CHECK_ALGORITHM_STATUS,
	TS_CHECK_CAN_SLEEP_ASYNC,
	TS_WAIT_CAN_SLEEP_ASYNC,
	TS_CHECK_IMAGE_DYNAMIC,
	TS_NO
};

#define TS_SUCCESS success

#define TS_SEQUENCE m_testSequence

#define TS_QUIT m_quit

#define TS_CONNECT m_connect

#define GO_SAVE_LOG()\
TS_SUCCESS = false;\
TS_SEQUENCE = TS_SAVE_LOG;\
break

#define GO_NEXT_TEST(NEXT)\
TS_SUCCESS = true;\
TS_SEQUENCE = NEXT;\
break

#define ASSERT_TEST_PROC(...) [__VA_ARGS__]()mutable->void

/*
* @brief,断言测试
* @param1,当前测试内容
* @param2,当前测试函数
* @param3,下个测试内容
* @param4,附加函数,ASSERT_TEST_PROC(){},可选
*/
#define ASSERT_TEST(CURR, FUNC, NEXT, ...)\
case CURR:\
{\
	if(!UTIL_JSON->getEnableItem(CURR))\
	{\
		\
	}\
	else if (!GET_TEST_LOGIC() && CURR != TS_SCAN_CODE && CURR != TS_SAVE_LOG)\
	{\
		\
	}\
	else\
	{\
		if (!(FUNC))\
		{\
			if (CURR == TS_SAVE_LOG)\
			{\
				setMessageBox("保存日志失败", getLastError());\
			}\
			else if (CURR == TS_SCAN_CODE)\
			{\
				\
			}\
			else if (UTIL_JSON->getEnableItem(TS_DEBUG_MODE))\
			{\
				if (setQuestionBox("调试模式", QString("错误信息:%1,\n测试NG,是否进行复测?").arg(getLastError()), false))\
				{\
					GO_NEXT_TEST(CURR);\
				}\
			}\
			else if (UTIL_JSON->getEnableItem(TS_RETEST_MODE))\
			{\
				if (setQuestionBox("复测模式", QString("错误信息:%1,\n测试NG,是否进行复测?").arg(getLastError()), false))\
				{\
					GO_NEXT_TEST(CURR);\
				}\
				else\
				{\
					GO_SAVE_LOG();\
				}\
			}\
			else\
			{\
				GO_SAVE_LOG();\
			}\
		}\
	}\
	if (CURR != TS_SCAN_CODE && CURR != TS_SAVE_LOG)\
	{\
		LOGIC_INDEX++;\
	}\
	VARIABLE_PARAM_CALL_FUNC(__VA_ARGS__);\
	GO_NEXT_TEST(NEXT);\
}

#define TEST_THREAD(FUNC)\
bool success = true;\
while(!TS_QUIT)\
{\
	if (TS_CONNECT)\
	{\
		switch(TS_SEQUENCE)\
		{\
			FUNC\
			default:break;\
		}\
	}\
	msleep(100);\
}\
quit();

#define TEST_THREAD_EX(FUNC)\
bool success = true;\
while(!TS_QUIT)\
{\
	if (TS_CONNECT)\
	{\
		switch(TS_SEQUENCE)\
		{\
			ASSERT_TEST(TS_SCAN_CODE, setScanDlgWindow(), TS_PREPARE_TEST);\
			FUNC\
			ASSERT_TEST(TS_SAVE_LOG, saveLog(TS_SUCCESS), TS_SCAN_CODE);\
			default:break;\
		}\
	}\
	msleep(100);\
}\
quit();

//设备故障码
enum DeviceFaultCode
{
	DFC_POWER,
	DFC_REALY,
	DFC_AMPEREMETER,
	DFC_VOLTMETER,
	DFC_CAN_CARD,
	DFC_LIN_CARD,
	DFC_CAPTURE_CARD,
	DFC_ADAPTER_BOX,
	DFC_PINTER,
	DFC_NO
};

namespace BaseTypes
{
	/*
	* @brief,下载信息
	* @param1,对话框标题
	* @param2,URL链接
	* @param3,保存路径
	* @param4,文件大小
	* @param5,所用时间
	* @param6,平均速度
	* @param7,错误内容
	* @param8,结果
	*/
	struct DETECTION_DLL_EXPORT DownloadInfo
	{
		QString title;
		QString url;
		QString path;
		float size;
		ulong time;
		float speed;
		QString error;
		bool result;
	};

	/*检测类型*/
	enum DetectionType
	{
		DT_HARDWARE,
		DT_FUNCTION,
		DT_AVM,
		DT_DVR,
		DT_OMS,
		DT_AICS,
		DT_MODULE,
		DT_TEMPERATURE
	};

	/*测试结果*/
	enum TestResult
	{ 
		TR_NO,
		TR_OK, 
		TR_NG,
		TR_TS
	};

	/*检测日志*/
	enum DetectionLog
	{ 
		DL_POWER_CURRENT,
		DL_COMPONET_RESISTOR, 
		DL_COMPONET_VOLTAGE, 
		DL_VERSION,
		DL_DTC, 
		DL_ALL 
	};
}

namespace HwdTypes
{

}

namespace FncTypes
{
	/*矩形类型*/
	enum RectType
	{ 
		RT_FRONT_BIG,
		RT_REAR_BIG,
		RT_LEFT_BIG,
		RT_RIGHT_BIG,
		RT_SMALL_ALL,
		RT_OTHER,
		RT_BIG_ALL,
		RT_NO
	};

	/*采集卡结构体*/
	struct DETECTION_DLL_EXPORT CardConfig
	{
		/*采集卡名称*/
		QString name;

		/*采集卡通道数*/
		int channelCount;

		/*采集卡通道号*/
		int channelId;

		/*图像宽度*/
		int width;

		/*图像高度*/
		int height;

		/*图像总大小*/
		int size;
	};

	//颜色信息
	struct DETECTION_DLL_EXPORT ColorInfo 
	{
		//日志信息
		char logInfo[TINY_BUFF];

		struct DETECTION_DLL_EXPORT 
		{
			//名称
			char name[TINY_BUFF];

			//红
			uchar red;

			//绿
			uchar green;

			//蓝
			uchar blue;

			//结果
			bool result;
		}color;//颜色

		struct DETECTION_DLL_EXPORT 
		{
			//数值
			int value;

			//结果
			bool result;
		}purity;//纯度
	};
}

namespace AvmTypes
{

}

namespace DvrTypes
{
	/*播放结果*/
	enum PlayResult
	{
		PR_OK,
		PR_NG,
		PR_REPLAY
	};

	/*网络类型*/
	enum NetworkTypes
	{ 
		NT_WIFI,
		NT_ETHERNET
	};

	/*DVR系统状态*/
	enum SystemStatus
	{ 
		SS_INITIALIZING,
		SS_GENERAL_RECORD, 
		SS_PAUSE_RECORD,
		SS_HARDWARE_KEY,
		SS_CRASH_KEY, 
		SS_UPDATE_MODE, 
		SS_ERROR,
		SS_OVER_TEMPERATURE,
		SS_SOC_NOT_POWER_ON,
		SS_NO
	};

	/*WIFI状态*/
	enum WifiStatus
	{ 
		WS_CLOSE, 
		WS_INIT, 
		WS_NORMAL,
		WS_CONNECT,
		WS_ERROR,
		WS_CONNECTED,
		WS_NO
	};

	/*以太网状态*/
	enum EthernetStatus
	{ 
		ES_CONNECT,
		ES_ERROR,
		ES_NO
	};

	/*SD卡状态*/
	enum SdCardStatus
	{ 
		SCS_NORMAL,
		SCS_NO_SD,
		SCS_ERROR,
		SCS_NOT_FORMAT,
		SCS_INSUFFICIENT,
		SCS_SPEED_LOW,
		SCS_USING,
		SCS_RESERVED,
		SCS_NO
	};

	/*文件路径*/
	enum FilePath
	{ 
		FP_NOR,
		FP_EVT,
		FP_PHO,
		FP_D1, 
		FP_ALL
	};

	/*文件类型*/
	enum FileType
	{ 
		FT_PHOTO,
		FT_VIDEO
	};

	/*格式化SD卡*/
	enum FormatSdCard
	{ 
		FSC_BY_NETWORK,
		FSC_BY_CAN,
		FSC_BY_UDS
	};

	/*网络命令1*/
	enum NetCmd
	{ 
		//建立连接
		NC_CONNECTION,

		//用户界面控制
		NC_UI_CONTROL,

		//快速控制
		NC_FAST_CONTROL,

		//文件控制
		NC_FILE_CONTROL = 0x10,

		//获取系统配置
		NC_CONFIG_GET,

		//设置系统配置
		NC_CONFIG_SET,

		//升级控制
		NC_OTA_CONTROL,

		//升级响应,比如解压中
		NC_OTA_RESPOND,
	};

	/*网络命令2*/
	enum NetSub
	{ 
		//NC_CONNECTION
		NS_HANDSHAKE = 0x00,
		NS_PAIRING_REQS = 0x01,
		NS_PAIRING_CHECK = 0x02,
		NS_HEART = 0x10,
		NS_AP_CONNECT_REQS = 0x20,

		//NC_UI_CONTROL
		NS_UI_REQ_PREVIEW = 0x00,
		NS_UI_REQ_FILES = 0x01,
		NS_UI_REQ_CONFIG = 0x02,
		NS_UI_REQ_PLAYBACK = 0x03,

		//NC_FAST_CONTROL
		NS_FAST_CYCLE_RECORD = 0x00,
		NS_FAST_EMERGE = 0x10,
		NS_FAST_PHOTOGRAPHY = 0x11,

		//NC_FILE_CONTROL
		NS_GET_SDCARD_STATUS = 0x00,
		NS_GET_FILE_LIST = 0x02,
		NS_REQ_FILE_DELETE = 0x21,
		NS_REQ_FILE_SAVING = 0x22,

		//NC_CONFIG_GET
		NS_GET_RECORD_CFG = 0x00,
		NS_GET_WIFI_CFG = 0x01,
		NS_GET_VERSION = 0x20,

		//NC_CONFIG_SET
		NS_SET_RESOLUTION = 0x00,
		NS_SET_DURATION = 0x01,
		NS_SET_AUDIO_RECORD = 0x02,
		NS_SET_OSD = 0x04,
		NS_SET_WIFI_CFG = 0x05,
		NS_SDCARD_FORMATTING = 0x20,
		NS_FACTORY_RESET = 0x21,
		NS_SDCARD_UMOUNT = 0x22,

		//NC_OTA_CONTROL
		NS_FILE_DOWNLOAD = 0x00,

		//NC_OTA_RESPOND
		NS_DECOMPRESS_RES = 0x00,
	};

	/*网络错误*/
	enum NetErr
	{
		NE_OK = 0x00000000U,
		NE_PARA_CONST,
		NE_PARA_RANGE,
		NE_PARA_VALUE = 0x00000004U,
		NE_OVERTIME = 0x00000008U,
		NE_BUSY = 0x00000010U,
		NE_NOT_INIT = 0x00000020U,
		NE_NOT_SUPPORT = 0x00000040U,
		NE_BUFF_EMPTY = 0x00000080U,
		NE_BUFF_FULL = 0x00000100U,
		NE_HW_PER = 0x00000200U,
		NE_HW_IC = 0x00000400U,
		NE_ACCESS = 0x00000800U,
		NE_CHECK = 0x00001000U,
		NE_BUS_OFF = 0x00002000U,
		NE_ABORT = 0x00004000U,
		NE_OVERFLOW = 0x00008000U,
		NE_UNKNOW = 0x80000000U,
	};	
}

namespace ModuleTypes
{

}

namespace OmsTypes
{
	/*
	* @brief,算法通道
	* 0-DMS0, 1-OMS0(前), 2-OMS1(左), 3-OMS2(右)
	* 4-GES0(前), 5-GES1(左), 6-GES2(右)
	* 7-FID0, 8-FID1
	*/
	enum AlgChannel
	{
		AC_DMS0,
		AC_OMS0,
		AC_OMS1,
		AC_OMS2,
		AC_GES0,
		AC_GES1,
		AC_GES2,
		AC_FID0,
		AC_FID1
	};

	/*
	* @brief,算法状态
	* 0-STOPPED, 1-INITING, 2-FAILURE, 3-PAUSING, 4-RUNNING
	*/
	enum AlgStatus
	{
		AS_STOPPED,
		AS_INITING,
		AS_FAILURE,
		AS_PAUSING,
		AS_RUNNING
	};
}

namespace AicsTypes
{
	/*
	* @brief,转接通道
	*/
	enum AdapterChannel
	{
		AC_CAN1,
		AC_CAN2,
		AC_CAN3,
		AC_CAN4,
		AC_CAN5,
		AC_CAN6,
		AC_CAN7,
		AC_CAN8,
		AC_1000M_ETH1,
		AC_1000M_ETH2,
		AC_1000M_ETH3,
		AC_1000M_ETH4,
		AC_100M_ETH1,
		AC_100M_ETH2,
		AC_ALL
	};
}

/*
* @Misc,这个命名空间中包含一些常用的函数
*/
namespace Misc
{
	//变量命名空间
	namespace Var
	{
		static QString lastError = "未知错误";
	}

	/*
	* @brief,设置最终错误
	* @param1,最终错误
	* @return,void
	*/
	void setLastError(const QString& error);

	/*
	* @brief,获取最终错误
	* @return,QString
	*/
	QString getLastError();

	/*
	* @brief,写入运行错误
	* @param1,条形码
	* @param2,错误信息
	* @return,bool
	*/
	bool writeRunError(const QString& barCode, const QString& error);

	/*
	* @brief,cv图像转qt图像[重载1]
	* @param1,iplimage
	* @param2,qimage
	* @return,bool
	*/
	bool cvImageToQtImage(IplImage* cv, QImage* qt);

	/*
	* @brief,cv图像转qt图像[重载2]
	* @param1,matimage
	* @param2,qimage
	* @return,bool
	*/
	bool cvImageToQtImage(const cv::Mat& mat, QImage& qt);

	/*
	* @brief,创建快捷方式
	* @return,bool
	*/
	bool createShortcut();

	/*
	* @brief,是否在线
	* @param1,IP地址
	* @param2,端口
	* @param3,超时
	* @return,bool
	*/
	bool isOnline(const QString& address, ushort port, int timeout);

	/*
	* @brief,是否在线
	* @param1,指定IP地址
	* @param2,目的IP地址
	* @param3,端口
	* @param4,超时
	* @return,bool
	*/
	bool isOnline(const QString& source, const QString& destination, ushort port, int timeout);
}

/*Network transmission*/
namespace Nt
{
	/*
	* @brief,Sfr服务端
	* @notice,此类用于与Sfr客户端通讯
	*/
	class DETECTION_DLL_EXPORT SfrServer
	{
	public:
		/*
		* @brief,构造
		*/
		SfrServer();

		/*
		* @brief,析构
		*/
		~SfrServer();

		/*
		* @brief,开始监听
		* @param1,监听端口
		* @return,bool
		*/
		bool startListen(ushort port = 2000);

		/*
		* @brief,获取Sfr值
		* @param1,本地文件路径[.bmp文件格式]
		* @param2,Sfr值
		* @return,bool
		*/
		bool getSfrValue(const char* filePath, float& sfr);

		/*
		* @brief,发送数据
		* @param1,发送的数据缓存区
		* @param2,发送的数据长度
		* @return,int
		*/
		int send(const char* buffer, int len);

		/*
		* @brief,接收数据
		* @param1,接收的数据缓存区
		* @param2,接收的数据长度
		* @return,int
		*/
		int recv(char* buffer, int len);

		/*
		* @brief,关闭监听
		* @return,void
		*/
		void closeListen();

		/*
		* @brief,获取最终错误
		* @return,QString
		*/
		QString getLastError() const;

		/*
		* @brief,Sfr处理线程
		* @param1,用户数据
		* @return,void
		*/
		static void sfrProcThread(void* arg);
	protected:
		/*
		* @brief,设置最终错误
		* @param1,错误信息
		* @return,void
		*/
		void setLastError(const QString& error);
	private:

		//服务端套接字
		SOCKET m_socket = INVALID_SOCKET;

		//客户端套接字
		SOCKET m_client = INVALID_SOCKET;

		//服务端套接字地址
		sockaddr_in m_sockAddr = { 0 };

		//错误信息
		QString m_lastError = "未知错误";

		//线程是否退出
		bool m_quit = false;
	};

	/*
	* @brief,dvr客户端
	* @notice,此类只能用于与dvr设备进行通讯
	*/
	class DETECTION_DLL_EXPORT DvrClient : public TcpClient
	{
	public:
		/*
		* @brief,构造[重载1]
		*/
		DvrClient();

		/*
		* @brief,构造[重载2]
		* @param1,ip地址
		* @param2,端口
		*/
		DvrClient(const QString& address, ushort port);

		/*
		* @brief,析构
		*/
		~DvrClient();

		/*
		* @brief,发送帧数据
		* @param1,发送的数据缓存区
		* @param2,发送的数据长度
		* @param3,主命令
		* @param4,子命令
		* @return,bool
		*/
		bool sendFrameData(const char* buffer, int len, uchar cmd, uchar sub);

		/*
		* @brief,接收帧数据
		* @param1,接收的数据缓存区
		* @param2,接收的数据长度
		* @return,bool
		*/
		bool recvFrameData(char* buffer, int* const len);

		/*
		* @brief,发送帧数据拓展[重载1]
		* @param1,初始化列表
		* @param2,主命令
		* @param3,子命令
		* @return,bool
		*/
		bool sendFrameDataEx(const std::initializer_list<char>& buffer, uchar cmd, uchar sub);

		/*
		* @brief,发送帧数据拓展[重载2]
		* @param1,发送的数据缓存区
		* @param2,发送的数据长度
		* @param3,主命令
		* @param4,子命令
		* @return,bool
		*/
		bool sendFrameDataEx(const char* buffer, int len, uchar cmd, uchar sub);

		/*
		* @brief,接收帧数据拓展
		* @param1,接收的数据缓存区
		* @param2,接收的数据长度
		* @param3,主命令
		* @param4,子命令
		* @return,bool
		*/
		bool recvFrameDataEx(char* buffer, int* const len, uchar cmd, uchar sub);

	private:
		/*
		* @brief,解锁算法
		* @param1,数据地址
		* @param2,数据长度
		* @param3,旧的数据
		* @return,uint
		*/
		uint unlockAlgorithm(uchar const* memoryAddr, uint memoryLen, uint oldCrc32) const;

	private:
		/*加密算法密钥*/
		const uint m_crc32Table[256] =
		{
			0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
			0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
			0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
			0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
			0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
			0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
			0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
			0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
			0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
			0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
			0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
			0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
			0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
			0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
			0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
			0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
			0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
			0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
			0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
			0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
			0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
			0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
			0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
			0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
			0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
			0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
			0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
			0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
			0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
			0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
			0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
			0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
			0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
			0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
			0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
			0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
			0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
		};
	};


	/*
	* @brief,Oms客户端
	* @brief,数据包协议:
	* byte	0	1	2	3	4	5	6	…	n-1	n	n + 1	n + 2	n + 3
	* name	FF	A5	5A	LEN		CMD	DATA		CRC32
	*       数据头		长度		命令	数据			校验
	* 
	* @brief,数据包说明:
	* HDR	3	数据头	  固定为FF/A5/5A
	* LEN	2	数据长度	  指“类型” + “数据” + “校验”的长度
	* CMD	1	命令	      详见[3-服务层协议]
	* DATA	/	数据	      详见[3-服务层协议]，可以为空
	* CRC32	4	校验码	  数据头 + 长度 + 命令 + 数据的校验和
	*/
	class DETECTION_DLL_EXPORT OmsClient : public TcpClient
	{
	public:
		/*
		* @brief,构造
		*/
		OmsClient();

		/*
		* @brief,析构
		*/
		~OmsClient();

		/*
		* @brief,发送数据
		* @notice,内部已封装完成,只需写命令+数据即可
		* @param1,初始化列表
		* @return,int
		*/
		int sendData(const std::initializer_list<uchar>& data);

		/*
		* @brief,发送数据
		* @notice,内部已封装完成,只需写命令+数据即可
		* @param1,发送的数据地址
		* @param2,发送的数据大小
		* @return,int
		*/
		int sendData(const char* data, int size);

		/*
		* @brief,接收数据
		* @notice,接收的数据为完整数据,
		* 需要手动去判断需要的内容
		* @param1,接收的数据地址
		* @param2,接收的数据大小
		* @return,int
		*/
		int recvData(char* data, int size);

	private:
		uint getCrc(const uchar* data, uint size, uint hist);

		const int HEAD_SIZE = 5;

		const int CRC_INIT = 0;

		const int CRC_SIZE = 4;
	private:
		const uint m_crc32table[256] =
		{
			0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
			0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
			0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
			0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
			0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
			0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
			0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
			0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
			0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
			0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
			0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
			0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
			0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
			0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
			0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
			0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
			0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
			0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
			0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
			0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
			0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
			0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
			0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
			0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
			0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
			0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
			0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
			0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
			0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
			0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
			0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
			0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
			0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
			0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
			0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
			0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
			0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
		};
	};


	/*
	* @brief,Aics客户端
	* @notice,内部只验证过routineControl
	*/
	class DETECTION_DLL_EXPORT AicsClient : public TcpClient
	{
	public:
		/*
		* @brief,构造
		*/
		AicsClient();

		/*
		* @brief,析构
		*/
		~AicsClient();

		/*
		* @brief,诊断会话控制
		* @param1,标识符
		* @return,bool
		*/
		bool diagnosticSessionControl(uchar id);

		/*
		* @brief,ECU重置
		* @param1,标识符
		* @return,bool
		*/
		bool ecuReset(uchar id);

		/*
		* @brief,通过标识符读取数据
		* @param1,标识符0
		* @param2,标识符1
		* @param3,读取的数据地址
		* @param4,读取的数据大小
		* @return,bool
		*/
		bool readDataByIdentifier(uchar id0, uchar id1, uchar* data, int* size);

		/*
		* @brief,安全访问
		* @param1,标识符
		* @return,bool
		*/
		bool securityAccess(uchar id);

		/*
		* @brief,例程控制
		* @param1,标识符0
		* @param2,标识符1
		* @param3,标识符2
		* @param4,发送的数据地址
		* @param5,发送的数据大小
		* @param6,接收的数据地址
		* @param7,接收的数据大小
		* @return,bool
		*/
		bool routineControl(uchar id0, uchar id1, uchar id2, const uchar* sendData, int sendSize, uchar* recvData, int* recvSize);

		/*
		* @brief,请求下载
		* @param1,地址
		* @param2,大小
		* @return,bool
		*/
		bool requestDownload(quint64 address, quint64 size);

		/*
		* @brief,传输数据
		* @param1,标识符
		* @param2,传输的数据地址
		* @param3,传输的数据大小
		* @return,bool
		*/
		bool transferData(uchar id, const char* data, int size);

		/*
		* @brief,请求传输退出
		* @param1,接收的数据地址
		* @param2,接收的数据大小
		* @return,bool
		*/
		bool requestTransferExit(uchar* data, int* size);

		/*
		* @brief 读寄存器
		* @param[in] devices 设备类型
		* @param[in] page 页面
		* @param[in] address 寄存器地址
		* @param[out] value 读取的值
		* @return bool
		*/
		bool readRegister(uint8_t devices, uint16_t page, uint32_t address, uint32_t* value);

		/*
		* @brief 写寄存器
		* @param[in] devices 设备类型
		* @param[in] page 页面
		* @param[in] address 寄存器地址
		* @param[in] value 写入的值
		* @return bool
		*/
		bool writeRegistr(uint8_t devices, uint16_t page, uint32_t address, uint32_t value);
	private:
		bool getKey(int level, const uchar* seedData, int seedSize, uchar* keyData, int* keySize = nullptr);

		uint getCrc(const uchar* data, int size) const;

		void getCrc(const uchar* data, int size, uchar* value) const;

		int recv(char* buffer, int size);
	private:
		const uint m_crc32Table[256] =
		{
			0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
			0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
			0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
			0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
			0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
			0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
			0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
			0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
			0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
			0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
			0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
			0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
			0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
			0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
			0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
			0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
			0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
			0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
			0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
			0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
			0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
			0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
			0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
			0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
			0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
			0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
			0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
			0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
			0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
			0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
			0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
			0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
			0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
			0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
			0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
			0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
			0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
		};
	};
}
