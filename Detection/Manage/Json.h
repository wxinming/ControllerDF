#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>

#include <Utility/Utility.h>
#pragma comment(lib,"Utility.lib")

#include "../Config.h"

//全局版本
DETECTION_DLL_EXPORT extern QString g_version;

//全局模式
DETECTION_DLL_EXPORT extern QString g_model;

//全局版本宏定义
#define G_VERSION g_version

//全局模式宏定义
#define G_MODEL g_model

//视频宽度
#define VIDEO_WIDTH 720

//视频高度
#define VIDEO_HEIGHT 480

/*
* @brief,设备配置
*/
struct DETECTION_DLL_EXPORT DeviceConfig
{
	/*机种名称*/
	QString typeName;

	/*CAN类型*/
	QString canType;

	//CAN设备号
	QString canDeviceNum;

	//CAN通道号
	QString canChannelNum;

	//CAN对端地址
	QString canPeerAddress;

	//CAN对端端口
	QString canPeerPort;

	/*CAN仲裁域波特率*/
	QString canArbiBaud;

	//CAN数据域波特率
	QString canDataBaud;

	/*CAN拓展帧*/
	QString canExpFrame;

	/*采集卡名称*/
	QString cardName;

	/*采集卡通道号*/
	QString cardChannelId;

	/*条码(判断)内容*/
	QString codeContent;

	/*条码长度*/
	QString codeLength;
};

/*
* @brief,硬件配置
*/
struct DETECTION_DLL_EXPORT HardwareConfig
{
	/*电源串口号*/
	int powerPort;

	/*电源波特率*/
	int powerBaud;

	/*电源电压*/
	float powerVoltage;

	/*电源电流*/
	float powerCurrent;

	/*继电器类型*/
	int relayType;

	/*继电器串口号,COM20为20*/
	int relayPort;

	/*继电器波特率,默认:19200*/
	int relayBaud;

	/*电压表串口号*/
	int voltmeterPort;

	/*电压表波特率,默认:9600*/
	int voltmeterBaud;

	/*电流表串口号*/
	int amperemeterPort;

	/*电流表波特率*/
	int amperemeterBaud;

	/*拓展串口1*/
	int expandCom1;

	/*拓展波特率1*/
	int expandBaud1;

	/*拓展串口2*/
	int expandCom2;

	/*拓展波特率2*/
	int expandBaud2;

	/*拓展串口3*/
	int expandCom3;

	/*拓展波特率3*/
	int expandBaud3;

	/*拓展串口4*/
	int expandCom4;

	/*拓展波特率*/
	int expandBaud4;
};

/*
* @brief,继电器配置
*/
struct DETECTION_DLL_EXPORT RelayConfig
{
	//点火线[ign]
	int ignitionLine;

	//接地线[gnd]
	int groundLine;

	//按键线[LED灯也是这根线]
	int keyLine;

	//电流表
	int amperemeter;

	//视频转接板
	int pinboard;

	//跑马灯
	int horseRaceLamp;

	//音箱
	int soundBox;

	//白灯
	int whiteLamp;

	//红灯
	int redLamp;

	//绿灯
	int greenLamp;
};

/*
* @brief,矩形配置
*/
struct DETECTION_DLL_EXPORT RectConfig
{
	/*颜色*/
	QString color;

	//纯度
	int purity;

	/*红*/
	int red;

	/*绿*/
	int green;

	/*蓝*/
	int blue;

	/*误差*/
	int deviation;

	/*起始X*/
	int startX;

	/*起始Y*/
	int startY;

	/*宽*/
	int width;

	/*高*/
	int height;

	/*启用*/
	int enable;
};

//小矩形框数量
#define SMALL_RECT_  5

//大矩形框数量
#define BIG_RECT_ 4

//图像检测数量
#define IMAGE_CHECK_COUNT  10

/*
* @brief,图像配置
*/
struct DETECTION_DLL_EXPORT ImageConfig
{
	//检测算法
	int detectAlgorithm;

	//检测纯度
	int detectPurity;

	//总是显示图像
	int alwaysShow;

	/*是否显示小图矩形框*/
	int showSmall;

	/*是否显示大图矩形框*/
	int showBig;

	//显示编号
	int showNumber;

	/*是否保存日志*/
	int saveLog;

	/*小矩形框配置*/
	RectConfig smallRect[SMALL_RECT_];

	/*大矩形框配置*/
	RectConfig bigRect[BIG_RECT_];
};

/*
* @brief 图像动态配置
*/
namespace cv { class DETECTION_DLL_EXPORT Mat; }

struct DETECTION_DLL_EXPORT ImageDynamicConfig 
{
	//判定次数,可配置
	int determineCount = 0;

	//持续时间
	int durationTime = 0;

	//动态频率
	int dynamicFrequency = 0;

	//动态占比,可配置
	double dynamicPercent = 0;

	//二值化阈值,可配置
	double binaryThreshold = 100;

	//当前频率
	qint64 currentFrequency = 0;

	//当前次数
	int currentCount = 0;

	//当前占比
	double currentPercent = 0;

	//动态图像
	cv::Mat* image = nullptr;

	//动态总和
	//如果设备卡死,图像还在继续显示,残留着上次的缓存,
	//需要这个参数的总和来判断图像是否丢失.
	uint sum = 0;
};

/*
* @brief,范围配置
*/
struct DETECTION_DLL_EXPORT RangeConfig
{
	/*DVR网速速率*/
	float minNetworkSpeed;
	float maxNetworkSpeed;

	/*DVR光轴X*/
	float minRayAxisX;
	float maxRayAxisX;

	/*DVR光轴Y*/
	float minRayAxisY;
	float maxRayAxisY;

	/*DVR角度*/
	float minRayAxisA;
	float maxRayAxisA;

	/*解像度*/
	float minSfr;
	float maxSfr;

	/*最小电流*/
	float minPowerCurrent0;
	float minPowerCurrent1;

	/*最大电流*/
	float maxPowerCurrent0;
	float maxPowerCurrent1;
};

/*
* @brief,阈值配置
*/
struct DETECTION_DLL_EXPORT ThresholdConfig
{
	/*启动延时*/
	float startDelay;

	/*唤醒电流阈值*/
	float rouseCurrent;

	/*休眠电流阈值*/
	float sleepCurrent;

	/*休眠超时阈值*/
	float sleepTimeout;

	//读取版本延时
	float readVersionDelay;

	//重读版本次数
	float rereadVersionTimes;

	//播放起始时间(ms)
	float playStartTime;
};

/*
* @brief,启用配置
*/
struct DETECTION_DLL_EXPORT EnableConfig
{
	//调试模式
	int debugMode;

	//复测模式
	int retestMode;

	/*输出运行日志*/
	int outputRunLog;

	/*保存运行日志*/
	int saveRunLog;

	//保存识别图像
	int saveRecognizeImage;

	/*保存CAN日志*/
	int saveCanLog;

	/*解锁对话框*/
	int unlockDlg;

	/*启用信号灯*/
	int signalLight;

	//判断条码
	int judgeCode;

	//查询工站
	int queryStation;

	//序列号读写
	int snReadWrite;

	//日期读写
	int dateReadWrite;

	//自动断开连接
	int autoDisconnect;

	//自动配置网卡
	int autoConfigNetworkCard;

	//检测组件电压
	int checkComponentVoltage;

	//检测静态电流
	int checkStaticCurrent;

	//检测CAN唤醒休眠
	int checkCanRouseSleep;

	//检测版本号
	int checkVersion;

	//检测DTC
	int checkDtc;

	//检测图像
	int checkImage;

	//检测工作电流
	int checkWorkingCurrent;

	//清除DTC
	int clearDtc;

	//外网通讯
	int outerNetCommunicate;

	//重读版本号
	int rereadVersion;

	//显示逻辑编号
	int showLogicNumber;
};

/*
* @brief,逻辑配置
*/
struct DETECTION_DLL_EXPORT LogicConfig {
	int execute;
};

/*
* @brief,默认配置
*/
struct DETECTION_DLL_EXPORT DefConfig
{
	DeviceConfig device;

	HardwareConfig hardware;

	RelayConfig relay;

	ImageConfig image;

	ImageDynamicConfig imageDynamic;

	RangeConfig range;

	ThresholdConfig threshold;

	EnableConfig enable;

	LogicConfig logic;
};

/*
* @组件电压配置[比如pcb板子上的各个部件]
*/
struct DETECTION_DLL_EXPORT ComponentVoltageConfig
{
	/*检测结果*/
	bool result;

	/*检测值*/
	float read;

	/*上限*/
	float high;

	/*下限*/
	float low;

	/*继电器IO*/
	int relay;

	/*延时*/
	int delay;

	/*名称*/
	char name[TINY_BUFF];
};

/*
* @brief,工作电流配置
*/
struct DETECTION_DLL_EXPORT WorkingCurrentConfig
{
	/*检测结果*/
	bool result;

	/*检测值*/
	float read;

	/*上限*/
	float high;

	/*下限*/
	float low;

	/*电源电压*/
	float voltage;

	/*名称*/
	char name[TINY_BUFF];
};

/*
* @brief,静态电流配置
*/
struct DETECTION_DLL_EXPORT StaticCurrentConfig
{
	/*检测结果*/
	bool result;

	/*检测值*/
	float read;

	/*上限*/
	float high;

	/*下限*/
	float low;
};

/*
* @brief,按键电压配置
*/
struct DETECTION_DLL_EXPORT KeyVoltageConfig
{
	/*高电平检测值*/
	float hRead;

	/*低电平检测值*/
	float lRead;

	/*高电平上限*/
	float hULimit;

	/*高电平下限*/
	float hLLimit;

	/*低电平上限*/
	float lULimit;

	/*低电平下限*/
	float lLLimit;

	/*高电平结果*/
	bool hResult;

	/*低电平结果*/
	bool lResult;
};

/*
* @brief,组件电阻配置
*/
struct DETECTION_DLL_EXPORT ComponentResistorConfig
{
	/*检测结果*/
	bool result;

	/*检测值*/
	float read;

	/*上限*/
	float high;

	/*下限*/
	float low;

	/*继电器IO*/
	int relay;

	/*名称*/
	char name[TINY_BUFF];
};

/*
* @brief,值配置
*/
typedef struct DETECTION_DLL_EXPORT ValConfig
{
	ComponentVoltageConfig* componentVoltage;

	ComponentResistorConfig* componentResistor;

	WorkingCurrentConfig* workingCurrent;

	StaticCurrentConfig staticCurrent;

	KeyVoltageConfig keyVoltage;
} HwdConfig;

/*
* @brief,版本配置
*/
struct DETECTION_DLL_EXPORT VersonConfig
{
	/*结果*/
	bool result;

	//启用
	bool enable;

	//错误
	bool error;

	/*大小*/
	int size;

	//重读次数
	int reread;

	/*DID*/
	uchar did[4];

	/*编码*/
	char encode[TINY_BUFF];

	/*配置值*/
	char setup[TINY_BUFF];

	/*名称*/
	char name[TINY_BUFF];

	/*读取值*/
	char read[TINY_BUFF];

	//错误信息
	char errstr[TINY_BUFF];
};

/*
* @brief,诊断故障码配置
*/
struct DETECTION_DLL_EXPORT DtcConfig
{
	/*是否启用*/
	bool enable;

	/*是否存在*/
	bool exist;

	/*DTC*/
	uchar dtc[4];

	/*名称*/
	char name[TINY_BUFF];
};

/*
* @唯一诊断服务配置
*/
struct DETECTION_DLL_EXPORT UdsConfig
{
	VersonConfig* version;

	DtcConfig* dtc;
};

/*
* @brief,配置文件类
*/
class DETECTION_DLL_EXPORT Json : public QObject
{
	Q_OBJECT
public:

	/*
	* @brief,获取实例
	* @return,Json*
	*/
	static Json* getInstance();

	/*
	* @brief,设置使用用户路径
	* @param1,是否使用
	* @return,void
	*/
	static void setUsingUserPath(bool on);

	/*
	* @brief,是否使用用户路径
	* @return,void
	*/
	static bool usingUserPath();

	/*
	* @brief,获取使用路径
	* @return,QString
	*/
	static QString getUsingPath();

	/*
	* @brief,获取最终错误
	* @return,QString
	*/
	QString getLastError() const;

	/*
	* @brief,获取错误列表
	* @return,QStringList
	*/
	QStringList getErrorList() const;

	/*
	* @brief,设置类型名称
	* @param1,类型名称
	* @return,void
	*/
	void setTypeName(const QString& typeName);

	/*
	* @brief,获取类型名称
	* @return,QString
	*/
	QString getTypeName() const;

	/*
	* @brief,初始化
	* @param1,是否更新文件
	* @return,bool
	*/
	bool initialize(bool update = false);

	/*
	* @brief,获取所有主键
	* @notice1,此列表顺序不可修改,否则将会导致SettingDlg加载崩溃,
	* @notice2,如果增加新键也需要增加SettingDlg::configTreeItemChangedSlot中的键
	* @return,QStringList
	*/
	QStringList getAllMainKey() const;

	/*
	* @brief,获取库版本
	* @return,const char*
	*/
	static const char* getLibVersion();

	/*
	* @brief,获取json路径
	* @return,QString
	*/
	QString getJsonPath() const;

	/*
	* @brief,获取共享路径
	* @return,QString
	*/
	QString getSharePath() const;

	/*
	* @brief,获取设备配置数量
	* @return,int
	*/
	int getDeviceConfigCount() const;

	/*
	* @brief,获取设备配置值
	* @param1,键
	* @return,QString
	*/
	QString getDeviceConfigValue(const QString& key) const;

	/*
	* @brief,获取设备配置键列表
	* @return,const QStringList&
	*/
	QStringList getDeviceConfigKeyList() const;

	/*
	* @brief,获取已解析的设备配置
	* @return,const DeviceConfig&
	*/
	const DeviceConfig& getParsedDeviceConfig() const;

	/*
	* @brief,获取设备配置对象
	* @return,const QJsonObject&
	*/
	const QJsonObject& getDeviceConfigObj() const;

	/*
	* @brief,设置设备配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setDeviceConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取设备配置说明
	* @return,QStringList
	*/
	QStringList getDeviceConfigExplain() const;

	/*
	* @brief,获取硬件配置值
	* @param1,键
	* @return,QString
	*/
	QString getHardwareConfigValue(const QString& key) const;

	/*
	* @brief,获取硬件配置数量
	* @return,int
	*/
	int getHardwareConfigCount() const;

	/*
	* @brief,获取硬件配置键列表
	* @return,QStringList
	*/
	QStringList getHardwareConfigKeyList() const;

	/*
	* @brief,获取已解析的硬件配置
	* @return,const HardwareConfig&
	*/
	const HardwareConfig& getParseHardwareConfig() const;

	/*
	* @brief,设置硬件配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setHardwareConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取硬件配置说明
	* @return,QStringList
	*/
	QStringList getHardwareConfigExplain() const;

	/*
	* @brief,获取继电器配置值
	* @param1,键
	* @return,QString
	*/
	QString getRelayConfigValue(const QString& key) const;

	/*
	* @brief,获取继电器配置数量
	* @return,int
	*/
	int getRelayConfigCount() const;

	/*
	* @brief,获取已解析的继电器配置
	* @return,const RelayConfig&
	*/
	const RelayConfig& getParsedRelayConfig() const;

	/*
	* @brief,获取继电器配置键列表
	* @return,QStringList
	*/
	QStringList getRelayConfigKeyList() const;

	/*
	* @brief,设置继电器配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setRelayConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取继电器配置说明
	* @return,QStringList
	*/
	QStringList getRelayConfigExplain() const;

	/*
	* @brief,获取用户配置键列表
	* @return,QStringList
	*/
	QStringList getUserConfigKeyList() const;

	/*
	* @brief,获取用户配置说明
	* @return,QStringList
	*/
	QStringList getUserConfigExplain() const;

	/*
	* @brief,获取用户配置值
	* @param1,键
	* @return,QString
	*/
	QString getUserConfigValue(const QString& key) const;

	/*
	* @brief,获取用户配置数量
	* @return,int
	*/
	int getUserConfigCount() const;

	/*
	* @brief,设置用户配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setUserConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取用户权限
	* @return,bool
	*/
	bool getUserPrivileges() const;

	/*
	* @brief,获取范围配置值
	* @param1,键
	* @return,QString
	*/
	QString getRangeConfigValue(const QString& key) const;

	/*
	* @brief,获取范围配置数量
	* @return,int
	*/
	int getRangeConfigCount() const;

	/*
	* @brief,获取范围配置键列表
	* @return,QStringList
	*/
	QStringList getRangeConfigKeyList() const;

	/*
	* @brief,获取已解析的范围配置
	* @return,const RangeConfig&
	*/
	const RangeConfig& getParsedRangeConfig() const;

	/*
	* @brief,设置范围配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setRangeConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取范围配置说明
	* @return,QStringList
	*/
	QStringList getRangeConfigExplain() const;

	/*
	* @brief,获取阈值配置值
	* @param1,键
	* @return,QString
	*/
	QString getThresholdConfigValue(const QString& key) const;

	/*
	* @brief,获取阈值配置数量
	* @return,int
	*/
	int getThresholdConfigCount() const;

	/*
	* @brief,获取阈值配置键列表
	* @return,QStringList
	*/
	QStringList getThresholdConfigKeyList() const;

	/*
	* @brief,获取已解析的配置
	* @return,const ThresholdConfig&
	*/
	const ThresholdConfig& getParsedThresholdConfig() const;

	/*
	* @brief,设置阈值配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setThresholdConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取阈值配置说明
	* @return,QStringList
	*/
	QStringList getThresholdConfigExplain() const;

	/*
	* @brief,获取已解析的图像配置
	* @return,const ImageConfig&
	*/
	const ImageConfig& getParsedImageConfig() const;

	/*
	* @brief,获取图像配置数量
	* @return,int
	*/
	int getImageConfigCount() const;

	/*
	* @brief,获取父图像配置键列表
	* @return,QStringList
	*/
	QStringList getParentImageConfigKeyList() const;

	/*
	* @brief,设置子图像配置键列表索引
	* @param1,索引
	* @return,void
	*/
	void setChildImageConfigKeyListIndex(int index);

	/*
	* @brief,获取子图像配置键列表[重载1]
	* @param1,索引
	* @return,QStringList
	*/
	QStringList getChildImageConfigKeyList(int index) const;

	/*
	* @brief,获取子图像配置键列表[重载2]
	* @return,QStringList
	*/
	QStringList getChildImageConfigKeyList() const;

	/*
	* @brief,获取图像配置值
	* @param1,父键
	* @param2,子键
	* @return,QString
	*/
	QString getImageConfigValue(const QString& parentKey, const QString& childKey) const;

	/*
	* @brief,设置图像配置键
	* @notice,不允许设置图像配置建
	* @param1,旧父键
	* @param2,新父键
	* @return,void
	*/
	void setImageConfigKey(const QString& oldParentKey, const QString& newParentKey);

	/*
	* @brief,设置图像配置值
	* @param1,父键
	* @param2,子键
	* @param3,值
	* @return,bool
	*/
	bool setImageConfigValue(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief,获取图像配置说明[重载1]
	* @param1,索引
	* @return,QStringList
	*/
	QStringList getImageConfigExplain(int index) const;

	/*
	* @brief,获取图像配置说明[重载2]
	* @return,QStringList
	*/
	QStringList getImageConfigExplain() const;

	/*
	* @brief 获取图像动态配置数量
	* @return int
	*/
	int getImageDynamicConfigCount() const;

	/*
	* @brief 设置图像动态配置值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setImageDynamicConfigValue(const QString& key, const QString& value);

	/*
	* @brief 获取图像动态配置值
	* @param[in] key 键
	* @return QString
	*/
	QString getImageDynamicConfigValue(const QString& key) const;

	/*
	* @brief 获取图像动态配置键列表
	* @return QStringList
	*/
	QStringList getImageDynamicConfigKeyList() const;

	/*
	* @brief 获取图像动态配置说明
	* @return QStringList
	*/
	QStringList getImageDynamicConfigExplain() const;

	/*
	* @brief 获取已解析的图像动态配置
	* @return ImageDynamicConfig*
	*/
	ImageDynamicConfig* getParsedImageDynamicConfig() const;

	/*
	* @brief,获取启用配置数量
	* @return,int
	*/
	int getEnableConfigCount() const;

	/*
	* @brief,获取启用配置键列表
	* @return,QStringList
	*/
	QStringList getEnableConfigKeyList() const;

	/*
	* @brief,获取启用配置值列表
	* @return,QStringList
	*/
	QStringList getEnableConfigValueList() const;

	/*
	* @brief,获取启用配置值
	* @param1,键
	* @return,QString
	*/
	QString getEnableConfigValue(const QString& key) const;

	/*
	* @brief,设置启用配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setEnableConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取启用配置说明
	* @return,QStringList
	*/
	QStringList getEnableConfigExplain() const;

	/*
	* @brief,获取逻辑配置数量
	* @return,int
	*/
	int getLogicConfigCount() const;

	/*
	* @brief,获取逻辑配置键列表
	* @return,QStringList
	*/
	QStringList getLogicConfigKeyList() const;

	/*
	* @brief,获取逻辑配置值列表
	* @return,QStringList
	*/
	QStringList getLogicConfigValueList() const;

	/*
	* @brief,获取逻辑配置值
	* @param1,键
	* @return,QString
	*/
	QString getLogicConfigValue(const QString& key) const;

	/*
	* @brief,设置逻辑配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setLogicConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取逻辑配置说明
	* @return,QStringList
	*/
	QStringList getLogicConfigExplain() const;

	/*
	* @brief,获取已解析的默认配置
	* @return,DefConfig*
	*/
	DefConfig* getParsedDefConfig() const;

	/*
	* @brief,获取组件电压配置数量
	* @return,int
	*/
	int getComponentVoltageConfigCount() const;

	/*
	* @brief,获取子组件电压配置键列表
	* @return,QStringList
	*/
	QStringList getChildComponentVoltageConfigKeyList() const;

	/*
	* @brief,获取子组件电压配置值列表
	* @return,QStringList
	*/
	QStringList getChildComponentVoltageConfigValueList() const;

	/*
	* @brief,获取父组件电压配置键列表
	* @return,QStringList
	*/
	QStringList getParentComponentVoltageConfigKeyList() const;

	/*
	* @brief,获取组件电压配置值
	* @param1,父键
	* @param2,子键
	* @return,QString
	*/
	QString getComponentVoltageConfigValue(const QString& parentKey, const QString& childKey) const;

	/*
	* @brief,设置组件电压配置键
	* @param1,旧父键
	* @param2,新父键
	* @return,void
	*/
	void setComponentVoltageConfigKey(const QString& oldParentKey, const QString& newParentKey);

	/*
	* @brief,设置组件电压配置值
	* @param1,父键
	* @param2,子键
	* @param3,值
	* @return,bool
	*/
	bool setComponentVoltageConfigValue(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief,获取组件电压配置对象
	* @return,QJsonObject&
	*/
	QJsonObject& getComponentVoltageConfigObj() const;

	/*
	* @brief,获取组件电压配置说明
	* @return,QStringList
	*/
	QStringList getComponentVoltageConfigExplain() const;

	/*
	* @brief,添加组件电压配置键值
	* @notice,[下限~上限,端口]
	* @param1,QMap
	* @return,void
	*/
	void addComponentVoltageConfigKeyValue(const QMap<QString, QString>& keyValue);

	/*
	* @brief,获取按键电压配置数量
	* @return,int
	*/
	int getKeyVoltageConfigCount() const;

	/*
	* @brief,获取按键电压配置键列表
	* @return,QStringList
	*/
	QStringList getKeyVoltageConfigKeyList() const;

	/*
	* @brief,获取按键电压配置值列表
	* @return,QStringList
	*/
	QStringList getKeyVoltageConfigValueList() const;

	/*
	* @brief,获取按键电压配置值
	* @param1,键
	* @return,QString
	*/
	QString getKeyVoltageConfigValue(const QString& key) const;

	/*
	* @brief,设置按键电压配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setKeyVoltageConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取按键电压配置说明
	* @return,QStringList
	*/
	QStringList getKeyVoltageConfigExplain() const;

	/*
	* @brief,获取父工作电流配置键列表
	* @return,QStringList
	*/
	QStringList getParentWorkingCurrentConfigKeyList() const;

	/*
	* @brief,获取子工作电流配置键列表
	* @return,QStringList
	*/
	QStringList getChildWorkingCurrentConfigKeyList() const;

	/*
	* @brief,获取子工作电流配置值列表
	* @return,QStringList
	*/
	QStringList getChildWorkingCurrentConfigValueList() const;

	/*
	* @brief,获取子工作电流配置值列表
	* @return,QStringList
	*/
	QStringList getChildWorkingCurrentConfigValueList(int index) const;

	/*
	* @brief,获取工作电流配置数量
	* @return,int
	*/
	int getWorkingCurrentConfigCount() const;

	/*
	* @brief,获取工作电流配置值
	* @param1,父键
	* @param2,子键
	* @return,QString
	*/
	QString getWorkingCurrentConfigValue(const QString& parentKey, const QString& childKey) const;

	/*
	* @brief,设置工作电流配置键
	* @param1,旧父键
	* @param2,新父键
	* @return,void
	*/
	void setWorkingCurrentConfigKey(const QString& oldParentKey, const QString& newParentKey);

	/*
	* @brief,设置工作电流配置值
	* @param1,父键
	* @param2,子键
	* @param3,值
	* @return,bool
	*/
	bool setWorkingCurrentConfigValue(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief,获取工作电流配置对象
	* @return,QjsonObject&
	*/
	QJsonObject& getWorkingCurrentConfigObj() const;

	/*
	* @brief,获取工作电流配置说明
	* @return,QStringList
	*/
	QStringList getWorkingCurrentConfigExplain() const;

	/*
	* @brief,获取静态电流配置数量
	* @return,int
	*/
	int getStaticCurrentConfigCount() const;

	/*
	* @brief,获取静态电流配置键列表
	* @return,QStringList
	*/
	QStringList getStaticCurrentConfigKeyList() const;

	/*
	* @brief,获取静态电流配置值列表
	* @return,QStringList
	*/
	QStringList getStaticCurrentConfigValueList() const;

	/*
	* @brief,获取静态电流配置值
	* @param1,键
	* @return,QString
	*/
	QString getStaticCurrentConfigValue(const QString& key) const;

	/*
	* @brief,设置静态电流配置值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setStaticCurrentConfigValue(const QString& key, const QString& value);

	/*
	* @brief,获取静态电流配置说明
	* @return,QStringList
	*/
	QStringList getStaticCurrentConfigExplain() const;

	/*
	* @brief,获取父组件电阻配置键列表
	* @return,QStringList
	*/
	QStringList getParentComponentResistorConfigKeyList() const;

	/*
	* @brief,获取子组件电阻配置键列表
	* @return,QStringList
	*/
	QStringList getChildComponentResistorConfigKeyList() const;

	/*
	* @brief,获取子组件电阻配置值列表
	* @return,QStringList
	*/
	QStringList getChildComponentResistorConfigValueList() const;

	/*
	* @brief,获取组件电阻配置数量
	* @return,int
	*/
	int getComponentResistorConfigCount() const;

	/*
	* @brief,获取组件电阻配置值
	* @param1,父键
	* @param2,子键
	* @return,QString
	*/
	QString getComponentResistorConfigValue(const QString& parentKey, const QString& childKey) const;

	/*
	* @brief,设置组件电阻配置键
	* @param1,旧父键
	* @param2,新父键
	* @return,void
	*/
	void setComponentResistorConfigKey(const QString& oldParentKey, const QString& newParentKey);

	/*
	* @brief,设置组件电阻配置值
	* @param1,父键
	* @param2,子键
	* @param3,值
	* @return,bool
	*/
	bool setComponentResistorConfigValue(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief,获取组件电阻配置对象
	* @return,QJsonObject&
	*/
	QJsonObject& getComponentResistorConfigObj() const;

	/*
	* @brief,获取组件电阻配置说明
	* @return,QStringList
	*/
	QStringList getComponentResistorConfigExplain() const;

	/*
	* @brief,获取已解析的硬件配置
	* @return,HwdConfig*
	*/
	HwdConfig* getParsedHwdConfig() const;

	/*
	* @brief,获取版本配置数量
	* @return,int
	*/
	int getVersionConfigCount() const;

	/*
	* @brief,获取父版本配置键列表
	* @return,QStringList
	*/
	QStringList getParentVersionConfigKeyList() const;

	/*
	* @brief,获取子版本配置键列表
	* @return,QStringList
	*/
	QStringList getChildVersionConfigKeyList() const;

	/*
	* @brief,获取子版本配置值列表
	* @return,QStringList
	*/
	QStringList getChildVersionConfigValueList() const;

	/*
	* @brief,获取版本配置值
	* @param1,父键
	* @parma2,子键
	* @return,QString
	*/
	QString getVersionConfigValue(const QString& parentKey, const QString& childKey) const;

	/*
	* @brief,设置版本配置键
	* @param1,旧父键
	* @parma2,新父键
	* @return,void
	*/
	void setVersionConfigKey(const QString& oldParentKey, const QString& newParentKey);

	/*
	* @brief,设置版本配置值
	* @param1,父键
	* @param2,子键
	* @param3,值
	* @return,bool
	*/
	bool setVersionConfigValue(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief,获取版本配置对象
	* @return,QJsonObject&
	*/
	QJsonObject& getVersionConfigObj() const;

	/*
	* @brief,获取版本配置说明
	* @return,QStringList
	*/
	QStringList getVersionConfigExplain() const;

	/*
	* @brief,添加版本配置键值
	* @notice,[DID,值,编码]
	* @param1,QMap
	* @return,void
	*/
	void addVersionConfigKeyValue(const QMap<QString, QString>& keyValue);

	/*
	* @brief,添加版本配置键值
	* @notice,[DID,值,编码]
	* @param1,键
	* @param2,值列表
	* @return,void
	*/
	void addVersionConfigKeyValue(const QString& key, const QString& value);

	/*
	* @brief,获取诊断故障码配置数量
	* @return,int
	*/
	int getDtcConfigCount() const;

	/*
	* @brief,获取父诊断故障码配置键列表
	* @return,QStringList
	*/
	QStringList getParentDtcConfigKeyList() const;

	/*
	* @brief,获取子诊断故障码配置键列表
	* @return,QStringList
	*/
	QStringList getChildDtcConfigKeyList() const;

	/*
	* @brief,获取子诊断故障码配置值列表
	* @return,QStringList
	*/
	QStringList getChildDtcConfigValueList() const;

	/*
	* @brief,获取诊断故障码值
	* @parma1,父键
	* @param2,子键
	* @return,QString
	*/
	QString getDtcConfigValue(const QString& parentKey, const QString& childKey) const;

	/*
	* @brief,设置诊断故障码配置键
	* @param1,旧父键
	* @param2,新父键
	* @return,void
	*/
	void setDtcConfigKey(const QString& oldParentKey, const QString& newParentKey);

	/*
	* @brief,设置诊断故障码配置值
	* @param1,父键
	* @param2,子键
	* @parma3,值
	* @return,bool
	*/
	bool setDtcConfigValue(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief,获取诊断故障码对象
	* @return,QJsonObject&
	*/
	QJsonObject& getDtcConfigObj() const;

	/*
	* @brief,获取诊断故障码说明
	* @return,QStringList
	*/
	QStringList getDtcConfigExplain() const;

	/*
	* @brief,添加诊断故障码配置键值列表
	* @notice,[DTC,忽略]
	* @param1,键列表
	* @param2,值列表
	* @return,void
	*/
	void addDtcConfigKeyValue(const QMap<QString, QString>& keyValue);

	/*
	* @brief,添加诊断故障码配置键值
	* @notice,[DTC,忽略]
	* @param1,键
	* @param2,值列表
	* @return,void
	*/
	void addDtcConfigKeyValue(const QString& key, const QString& value);

	/*
	* @brief,获取已解析的唯一诊断服务配置
	* @return,UdsConfig*
	*/
	UdsConfig* getParsedUdsConfig() const;

	/*
	* @brief,设置其他配置1值
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	bool setOtherConfig1Value(const QString& key, const QString& value);

	/*
	* @brief,获取其他配置1值
	* @param1,键
	* @return,QString
	*/
	QString getOtherConfig1Value(const QString& key) const;

	/*
	* @brief,获取其他配置1数量
	* @return,int
	*/
	int getOtherConfig1Count() const;

	/*
	* @brief,获取其他配置1键列表
	* @return,QStringList
	*/
	QStringList getOtherConfig1KeyList() const;

	/*
	* @brief,获取其他1配置说明
	* @return,QStringList
	*/
	QStringList getOtherConfig1Explain() const;

	/*
	* @brief,添加其他配置1键值[重载1]
	* @param1,QString映射
	* @return,void
	*/
	void addOtherConfig1KeyValue(const QMap<QString, QString>& keyValue);

	/*
	* @brief,添加其他配置1键值[重载2]
	* @param1,键
	* @param2,值
	* @return,bool
	*/
	void addOtherConfig1KeyValue(const QString& key, const QString& value);

	/*
	* @brief,获取其他配置1对象
	* @return,QJsonObject&
	*/
	QJsonObject& getOtherConfig1Obj() const;

	/*
	* @brief,设置其他配置2键
	* @param1,父键
	* @param2,值
	* @return,bool
	*/
	void setOtherConfig2Key(const QString& parentKey, const QString& value);

	/*
	* @brief,设置其他配置2值
	* @param1,父键
	* @param2,子键
	* @param3,值
	* @return,bool
	*/
	bool setOtherConfig2Value(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief,获取其他配置2值
	* @param1,父键
	* @param2,子键
	* @return,QString
	*/
	QString getOtherConfig2Value(const QString& parentKey, const QString& childKey) const;

	/*
	* @brief,获取其他配置2数量
	* @return,int
	*/
	int getOtherConfig2Count() const;

	/*
	* @brief,获取父其他配置2键列表
	* @return,QStringList
	*/
	QStringList getParentOtherConfig2KeyList() const;

	/*
	* @brief,获取子其他配置2建列表
	* @return,QString
	*/
	QStringList getChildOtherConfig2KeyList() const;

	/*
	* @brief,设置子其他配置2键列表索引
	* @param1,索引
	*/
	void setChildOtherConfig2KeyListIndex(int index);

	/*
	* @brief,获取其他配置2说明
	* @return,QStringList
	*/
	QStringList getOtherConfig2Explain() const;

	/*
	* @brief,添加其他配置2键值[重载1]
	* @param1,父键
	* @param2,QString映射
	* @return,void
	*/
	void addOtherConfig2KeyValue(const QString& parentKey, const QMap<QString, QString>& keyValue);

	/*
	* @brief,添加其他配置2键值[重载2]
	* @param1,父键列表
	* @param2,QString映射列表
	* @return,void
	*/
	void addOtherConfig2KeyValue(const QStringList& parentKeyList, const QList<QMap<QString, QString>>& keyValueList);

	/*
	* @brief,获取其他配置2对象
	* @return,QJsonObject&
	*/
	QJsonObject& getOtherConfig2Obj() const;

	/*
	* @brief,获取启用项目
	* @param1,要获取的项目
	* @return,bool
	*/
	bool getEnableItem(int item) const;

	/*
	* @brief,UDS编码转换
	* @param1,版本信息配置
	* @return,bool
	*/
	bool udsEncodeConvert(VersonConfig* config);

	/*
	* @brief,UDS编码表达式
	* @param1,表达式
	* @notice,必须以$开头
	* 样例1.
	* uchar data[] = {'A','B','C','D',0x14,0x16,0x01,0x01,0xAA,0XBB};
	* 协议,0~3字节为ASCII编码,4~7字节为BCD编码,8~9字节为HEX编码
	* 所需表达式为:$4A4B2H
	* A代表:ASCII B代表:BCD H代表:HEX
	* 解析出来的字符串为:ABCD20220101AABB
	* 样例2.
	* uchar data[] = {0xFF,0x0C,'K','S','E','L','P','W','G','H','I','Q'};
	* 所需表达式为:$2H*A
	* *A代表从第二个字节往后都是ASCII编码,*只能用在最后面来计算余下的字节编码.
	* @param2,数据
	* @param3,数据大小
	* @param4,附加字符,如果解析出来的字符串每个编码不一样,将会在每个编码连接处添加上指定的字符
	* 如上述->解析出来的字符串为:ABCD20220101AABB,添加附加字符 '-' 之后将会是
	* ABCD-20220101-AABB
	* @return,bool
	*/
	bool udsEncodeExpression(const QString& expression, char* data = nullptr, int size = 0, const char* append = nullptr);
protected:
	/*
	* @brief,设置最终错误
	* @param1,错误
	* @return,void
	*/
	void setLastError(const QString& error);

	/*
	* @brief,读取共享json文件
	* @param1,文件名
	* @return,bool
	*/
	bool readShareJsonFile(const QString& name);

	/*
	* @brief,写入共享json文件
	* @param1,文件名
	* @return,bool
	*/
	bool writeShareJsonFile(const QString& name);

	/*
	* @brief,更新共享json文件
	* @param1,文件名
	* @return,bool
	*/
	bool updateShareJsonFile(const QString& name);

	/*
	* @brief,读取默认json文件
	* @param1,文件名
	* @return,bool
	*/
	bool readDefJsonFile(const QString& name);

	/*
	* @brief,写入默认json文件
	* @param1,文件名
	* @return,bool
	*/
	bool writeDefJsonFile(const QString& name);

	/*
	* @brief,更新默认json文件
	* @param1,文件名
	* @return,bool
	*/
	bool updateDefJsonFile(const QString& name);

	/*
	* @brief,读取硬件json文件
	* @parma1,文件名
	* @return,bool
	*/
	bool readHwdJsonFile(const QString& name);

	/*
	* @brief,写入硬件jsjon文件
	* @param1,文件名
	* @return,bool
	*/
	bool writeHwdJsonFile(const QString& name);

	/*
	* @brief,更新硬件json文件
	* @param1,文件名
	* @return,bool
	*/
	bool updateHwdJsonFile(const QString& name);

	/*
	* @brief,读取图像json文件
	* @param1,文件名
	* @return,bool
	*/
	bool readImgJsonFile(const QString& name);

	/*
	* @brief,写入图像json文件
	* @param1,文件名
	* @return,bool
	*/
	bool writeImgJsonFile(const QString& name);

	/*
	* @brief,更新图像json文件按
	* @param1,文件名
	* @return,bool
	*/
	bool updateImgJsonFile(const QString& name);

	/*
	* @brief,读取其他json文件
	* @param1,文件名
	* @return,bool
	*/
	bool readOtherJsonFile(const QString& name);

	/*
	* @brief,写入其他json文件
	* @param1,文件名
	* @return,bool
	*/
	bool writeOtherJsonFile(const QString& name);

	/*
	* @brief,更新其他json文件
	* @param1,文件名
	* @return,bool
	*/
	bool updateOtherJsonFile(const QString& name);

	/*
	* @brief,读取唯一诊断服务json文件
	* @param1,文件名
	* @return,bool
	*/
	bool readUdsJsonFile(const QString& name);

	/*
	* @brief,写入唯一诊断服务json文件
	* @param1,文件名
	* @return,bool
	*/
	bool writeUdsJsonFile(const QString& name);

	/*
	* @brief,更新唯一诊断服务json文件
	* @param1,文件名
	* @return,bool
	*/
	bool updateUdsJsonFile(const QString& name);

	/*
	* @brief,解析范围值
	* @param1,配置值
	* @param2,最小值
	* @param3,最大值
	* @return,bool
	*/
	bool parseRangeValue(const QString& value, float& min, float& max);

	/*
	* @brief,解析设备配置数据
	* @return,bool
	*/
	bool parseDeviceConfigData();

	/*
	* @brief,解析硬件配置数据
	* @return,bool
	*/
	bool parseHardwareConfigData();

	/*
	* @brief,解析继电器配置数据
	* @return,bool
	*/
	bool parseRelayConfigData();

	/*
	* @brief,解析用户配置数据
	* @return,bool
	*/
	bool parseUserConfigData();

	/*
	* @brief,解析图像配置数据
	* @return,bool
	*/
	bool parseImageConfigData();

	/*
	* @brief 解析图像动态配置数据
	* @return bool
	*/
	bool parseImageDynamicConfigData();

	/*
	* @brief,解析范围配置数据
	* @return,bool
	*/
	bool parseRangeConfigData();

	/*
	* @brief,解析阈值配置数据
	* @return,bool
	*/
	bool parseThresholdConfigData();

	/*
	* @brief,解析启用配置数据
	* @return,bool
	*/
	bool parseEnableConfigData();

	/*
	* @brief,解析逻辑配置数据
	* @return,bool
	*/
	bool parseLogicConfigData();

	/*
	* @brief,解析组件电压配置数据
	* @return,bool
	*/
	bool parseComponentVoltageConfigData();

	/*
	* @brief,解析按键电压配置数据
	* @return,bool
	*/
	bool parseKeyVoltageConfigData();

	/*
	* @brief,解析工作电流配置数据
	* @return,bool
	*/
	bool parseWorkingCurrentConfigData();

	/*
	* @brief,解析静态电流配置数据
	* @return,bool
	*/
	bool parseStaticCurrentConfigData();

	/*
	* @brief,解析组件电阻配置数据
	* @return,bool
	*/
	bool parseComponentResistorConfigData();

	/*
	* @brief,解析版本配置数据
	* @return,bool
	*/
	bool parseVersionConfigData();

	/*
	* @brief,解析诊断故障码配置数据
	* @return,bool
	*/
	bool parseDtcConfigData();

	/*
	* @brief,诊断故障码种类转换
	* @parma1,诊断故障码
	* @return,QString
	*/
	QString dtcCategoryConvert(const QString& dtc);

private:
	/*
	* @brief,构造
	*/
	Json(QObject* parent = nullptr);

	/*
	* @brief,析构
	*/
	~Json();

	/*
	* @brief,拷贝构造删除
	*/
	Json(const Json&) = delete;

	/*
	* @brief,赋值构造删除
	*/
	Json& operator=(const Json&) = delete;

	/*
	* @notice,此处如果添加键/值,还需进入结构体中添加变量,
	* 添加变量需注意,变量类型是否一致,否则将会导致程序崩溃.
	*/

	/*设备配置键列表*/
	const QStringList m_deviceConfigKeyList =
	{
		"机种名称",//2
		"CAN卡类型",//1
		"CAN卡设备号",
		"CAN卡通道号",
		"CAN卡对端地址",
		"CAN卡对端端口",
		"CAN卡仲裁域",//3
		"CAN卡数据域",
		"CAN卡拓展帧",//4
		"采集卡名称",//5
		"采集卡通道号",//6
		"条码判断",//7
		"条码长度"//8
	};

	/*设备配置值列表*/
	const QStringList m_deviceConfigValueList =
	{
		"未知",//2
		"2",//1
		"0",
		"0",
		"127.0.0.1",
		"8000",
		"500",//3
		"2000",
		"0",//4
		"TCHD",//5
		"0",//6
		"NULL",//7
		"0"//8
	};

	/*硬件配置键列表*/
	const QStringList m_hardwareConfigKeyList =
	{
		"电源串口",//1
		"电源波特率",//2
		"电源电压",//3
		"电源电流",//4
		"继电器类型",//5
		"继电器串口",//6
		"继电器波特率",//7
		"电压表串口",//8
		"电压表波特率",//9
		"电流表串口",//10
		"电流表波特率",//11
		"拓展串口1",//12
		"拓展波特率1",//13
		"拓展串口2",//14
		"拓展波特率2",//15
		"拓展串口3",//16
		"拓展波特率3",//17
		"拓展串口4",
		"拓展波特率4"
	};

	/*硬件配置值列表*/
	const QStringList m_hardwareConfigValueList =
	{
		"1",//1
		"19200",//2
		"12.0",//3
		"1.0",//4
		"0",//5
		"2",//6
		"19200",//7
		"3",//8
		"9600",//9	
		"4",//10
		"9600",//11
		"5",//12
		"9600",//13
		"6",//14
		"9600",//15
		"7",//16
		"9600",//17
		"8",
		"9600"
	};

	/*继电器配置键列表*/
	const QStringList m_relayConfigKeyList =
	{
		"点火线",//2
		"接地线",//1
		"按键线",//4
		"电流表",//3
		"转接板",//5
		"跑马灯",//6
		"音箱",//7
		"白灯",//8
		"红灯",//9
		"绿灯"//10
	};

	/*继电器配置值列表*/
	const QStringList m_relayConfigValueList =
	{
		"0",//1
		"1",//2
		"2",//3
		"3",//4
		"4",//5
		"5",//6
		"6",//7
		"13",//8
		"14",//9
		"15"//10
	};

	/*范围配置键列表*/
	const QStringList m_rangeConfigKeyList =
	{
		"网速",//1
		"光轴X坐标",//2
		"光轴Y坐标",//3
		"光轴角度",//4
		"解像度",//5
		"最小电流",//6
		"最大电流"//7
	};

	/*范围配置值列表*/
	const QStringList m_rangeConfigValueList =
	{
		"0.2~9999.0",//1
		"0.0~1920.0",//2
		"0.0~1080.0",//3
		"0.0~120.0",//4
		"40.0~150.0",//5
		"0.0~1000.0",//6
		"0.0~1000.0"//7
	};

	/*用户配置键列表*/
	/*ROOT用户权限0,用于此程序开发者*/
	/*INVO用户权限1,用于现场调试者*/
	/*TEST用户权限2,用于作业员*/
	const QStringList m_userConfigKeyList =
	{
		"用户名",
		"密码"
	};

	/*用户配置值列表*/
	const QStringList m_userConfigValueList =
	{
		"INVO",
		"1."
	};

	/*父图像键列表*/
	const QStringList m_parentImageKeyList =
	{
		"前小图矩形框",//1
		"后小图矩形框",//2
		"左小图矩形框",//3
		"右小图矩形框",//4
		"汽车图矩形框",//5
		"前大图矩形框",//6
		"后大图矩形框",//7
		"左大图矩形框",//8
		"右大图矩形框",//9
		"检测启用状态"//10
	};

	/*子图像键列表*/
	const QStringList m_childImageKeyList[IMAGE_CHECK_COUNT] =
	{
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//1
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//2
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//3
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//4
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//5
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//6
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//7
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//8
		{"颜色","纯度","R","G","B","误差","X坐标","Y坐标","宽","高","启用"},//9
		{"检测算法","检测纯度","总是显示","显示小图","显示大图","显示编号","保存日志"}//10
	};

	/*子图像值列表*/
	const QStringList m_childImageValueList[IMAGE_CHECK_COUNT] =
	{
		{"!=黑色","99","201","212","85","100","80","48","64","76","1"},//1
		{"!=黑色","99","255","255","255","100","80","346","64","76","1"},//2
		{"!=黑色","99","176","58","177","100","4","147","53","159","1"},//3
		{"!=黑色","99","164","78","7","100","155","147","60","159","1"},//4
		{"!=黑色","50","153","212","81","100","80","147","64","159","1"},//5
		{"!=黑色","99","153","212","81","100","313","45","156","139","1"},//6
		{"!=黑色","99","113","50","34","100","502","45","156","139","1"},//7
		{"!=黑色","99","100","108","30","18","313","283","156","139","1"},//8
		{"!=黑色","99","168","55","66","77","502","283","156","139","1"},//9
		{"1","0","0","1","1","1","0"}//10
	};

	/*子图像配置索引*/
	int m_childImageConfigIndex = 0;

	//图像动态配置键列表
	QJsonObject m_imageDynamicConfigObj;

	const QStringList m_imageDynamicConfigKeyList = 
	{
		"判定次数",
		"持续时间",
		"动态频率",
		"动态占比",
		"二值化阈值"
	};

	const QStringList m_imageDynamicConfigValueList = 
	{
		"10",
		"10000",
		"300",
		"10",
		"5"
	};

	const QStringList m_imageDynamicConfigExplainList = 
	{
		"超过多少次数NG",
		"动态检测持续多久(ms)",
		"图像多久变化一次(ms)",
		"两幅图像中的不同占比",
		"二值化阈值"
	};

	/*阈值键列表*/
	const QStringList m_thresholdConfigKeyList =
	{
		"启动延时",//1
		"唤醒电流",//2
		"休眠电流",//3
		"休眠超时",//4
		"读取版本延时",//5
		"重读版本次数",//6
		"播放起始时间"//7
	};

	/*阈值值列表*/
	const QStringList m_thresholdConfigValueList =
	{
		"15000",//1
		"0.3",//2
		"0.001",//3
		"15000",//4
		"200",//5
		"3",//6
		"0"//7
	};

	/*启用配置键列表*/
	const QStringList m_enableConfigKeyList =
	{
		"调试模式",//0
		"复测模式",//1
		"输出运行日志",//2
		"保存运行日志",//3
		"保存识别图像",//4
		"保存CAN日志",//5
		"解锁对话框",//6
		"信号灯提示",//7
		"判断条码",//8
		"查询工站",//9
		"序列号读写",//10
		"日期读写",//11
		"自动断开连接",//12
		"自动配置网卡",//13
		"检测组件电压",//14
		"检测静态电流",//15
		"检测唤醒休眠",
		"检测版本号",//16
		"检测DTC",//17
		"检测图像",//18
		"检测工作电流",//19
		"清除DTC",//20
		"外网通讯",//21
		"重读版本号",//22
		"显示逻辑编号"//23
	};

	/*启用配置值列表*/
	const QStringList m_enableConfigValueList =
	{
		"0",//0
		"0",//1
		"0",//2
		"1",//3
		"0",//4
		"0",//5
		"0",//6
		"0",//7
		"1",//8
		"0",//9
		"1",//10
		"1",//11
		"1",//12
		"1",//13
		"1",//14
		"1",//15
		"1",
		"1",//16
		"1",//17
		"1",//18
		"1",//19
		"1",//20
		"1",//21
		"0",//22
		"0"//23
	};

	//按键电压配置键列表
	const QStringList m_keyVoltageConfigKeyList =
	{
		"高电平上限",
		"高电平下限",
		"低电平上限",
		"低电平下限"
	};

	//按键电压配置值列表
	const QStringList m_keyVoltageConfigValueList =
	{
		"16.0",
		"8.0",
		"1.0",
		"0.0"
	};

	//静态电流配置键列表
	const QStringList m_staticCurrentConfigKeyList =
	{
		"上限",
		"下限"
	};

	//静态电流配置值列表
	const QStringList m_staticCurrentConfigValueList =
	{
		"50",
		"0"
	};

	//组件电压配置键列表
	const QStringList m_componentVoltageConfigKeyList =
	{
		"上限",
		"下限",
		"继电器IO",
		"延时"
	};

	//组件电压配置值列表
	const QStringList m_componentVoltageConfigValueList =
	{
		"1.8",
		"1.0",
		"1",
		"1200"
	};

	//工作电流配置键列表
	const QStringList m_workingCurrentConfigKeyList =
	{
		"上限",
		"下限",
		"电压"
	};

	//工作电流配置值列表
	const QStringList m_workingCurrentConfigValueList =
	{
		"0.12",
		"0.10",
		"12.0"
	};

	//电阻配置键列表
	const QStringList m_componentResistorConfigKeyList =
	{
		"上限",
		"下限",
		"继电器IO"
	};

	//电阻配置值列表
	const QStringList m_componentResistorConfigValueList =
	{
		"8000",
		"5000",
		"5"
	};

	//版本配置键列表
	const QStringList m_versionConfigKeyList =
	{
		"DID",
		"编码",
		"值",
		"启用"
	};

	//版本配置值列表
	const QStringList m_versionConfigValueList =
	{
		"0xFFFF",
		"ASCII",
		"XXXXXX",
		"1"
	};

	//诊断故障码键列表
	const QStringList m_dtcConfigKeyList =
	{
		"DTC",
		"启用"
	};

	//诊断故障码值列表
	const QStringList m_dtcConfigValueList =
	{
		"B1FC016",
		"1"
	};

	//逻辑配置键列表
	const QStringList m_logicConfigKeyList = 
	{
		"测试1执行",//1
		"测试2执行",//2
		"测试3执行",//3
		"测试4执行",//4
		"测试5执行",//5
		"测试6执行",//6
		"测试7执行",//7
		"测试8执行",//8
		"测试9执行",//9
		"测试10执行",//10
		"测试11执行",//11
		"测试12执行",//12
		"测试13执行",//13
		"测试14执行",//14
		"测试15执行",//15
		"测试16执行",//16
		"测试17执行",//17
		"测试18执行",//18
		"测试19执行",//19
		"测试20执行",//20
		"测试21执行",//21
		"测试22执行",//22
		"测试23执行",//23
		"测试24执行",//24
		"测试25执行",//25
		"测试26执行",//26
		"测试27执行",//27
		"测试28执行",//28
		"测试29执行",//29
		"测试30执行",//30
		"测试31执行",//30
		"测试32执行"//30
	};

	//逻辑配置值列表
	const QStringList m_logicConfigValueList = 
	{
		"1",//1
		"1",//2
		"1",//3
		"1",//4
		"1",//5
		"1",//6
		"1",//7
		"1",//8
		"1",//9
		"1",//10
		"1",//11
		"1",//12
		"1",//13
		"1",//14
		"1",//15
		"1",//16
		"1",//17
		"1",//18
		"1",//19
		"1",//20
		"1",//21
		"1",//22
		"1",//23
		"1",//24
		"1",//25
		"1",//26
		"1",//27
		"1",//28
		"1",//29
		"1",//30
		"1",//31
		"1"//32
	};

	/*保存错误信息*/
	QString m_lastError = "未知错误";

	/*保存错误列表*/
	QStringList m_errorList;

	/*设备配置对象*/
	QJsonObject m_deviceConfigObj;

	/*硬件配置对象*/
	QJsonObject m_hardwareConfigObj;

	/*继电器IO配置*/
	QJsonObject m_relayConfigObj;

	/*范围配置*/
	QJsonObject m_rangeConfigObj;

	/*用户配置对象*/
	QJsonObject m_userConfigObj;

	/*图像父配置对象*/
	QJsonObject m_imageConfigObj;

	/*阈值配置对象*/
	QJsonObject m_thresholdConfigObj;

	/*启用配置对象*/
	QJsonObject m_enableConfigObj;

	//逻辑配置对象
	QJsonObject m_logicConfigObj;

	/*默认配置*/
	DefConfig m_defConfig;

	/*硬件检测结构体*/
	HwdConfig m_hwdConfig = { 0 };

	/*组件电压配置*/
	QJsonObject m_componentVoltageConfigObj;

	/*按键配置*/
	QJsonObject m_keyVoltageConfigObj;

	/*工作电流配置*/
	QJsonObject m_workingCurrentConfigObj;

	/*静态电流配置*/
	QJsonObject m_staticCurrentConfigObj;

	/*电阻配置*/
	QJsonObject m_componentResistorConfigObj;

	/*UDS结构体*/
	UdsConfig m_udsConfig = { 0 };

	/*版本配置*/
	QJsonObject m_versionConfigObj;

	/*DTC配置*/
	QJsonObject m_dtcConfigObj;

	/*其他配置1*/
	QJsonObject m_otherConfig1Obj;

	//其他2配置
	QJsonObject m_otherConfig2Obj;

	//其他2索引
	int m_other2ConfigIndex = 0;

	//机种名
	QString m_typeName;

	//json路径
	QString m_jsonPath;

	//共享路径
	QString m_sharePath;

	//用户路径
	static bool m_userPath;
};
