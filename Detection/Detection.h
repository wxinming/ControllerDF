#pragma once
#pragma execution_character_set("utf-8")

#include "Manage/Types.h"

/*
* @brief,检测命名空间
*/
namespace Dt
{
	/*
	* @brief,检测框架基类
	*/
	class DETECTION_DLL_EXPORT Base : public QThread
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		* @param1,父对象
		*/
		explicit Base(QObject* parent = nullptr);

		/*
		* @brief,虚析构
		*/
		virtual ~Base();

		/*
		* @brief,获取最终错误
		* @return,QString
		*/
		QString getLastError() const;

		/*
		* @brief,设置测试顺序
		* @param1,测试顺序
		* @return,void
		*/
		void setTestSequence(int testSequence);

		/*
		* @brief,设置连接状态
		* @param1,状态
		* @return,void
		*/
		void setConnectStatus(bool status);

		/*
		* @brief,设置检测类型
		* @param1,类型
		* @return,void
		*/
		void setDetectionType(BaseTypes::DetectionType type);

		/*
		* @brief,获取检测类型
		* @return,BaseTypes::DetectionType
		*/
		static BaseTypes::DetectionType getDetectionType();

		/*
		* @brief,获取检测名称
		* @return,QString
		*/
		static QString getDetectionName();

		/*
		* @brief,设置soc延时
		* @param1,延时时间ms
		* @return,void
		*/
		void setSocDelay(ulong delay);

		/*
		* @brief,初始化
		* @return,void
		*/
		virtual bool initialize();

		/*
		* @brief,初始化控制台窗口
		* @notice,调用此函数前,不可进行任何输出,
		* 否则重定向流将失败,你无法看到打印内容.
		*/
		bool initConsoleWindow();

		/*
		* @brief,退出控制台窗口
		* @return,bool
		*/
		bool exitConsoleWindow() const;

		/*
		* @brief,打开设备
		* @return,bool
		*/
		virtual bool openDevice();

		/*
		* @brief,关闭设备
		* @return,bool
		*/
		virtual bool closeDevice();

		/*
		* @brief,准备测试[重载1]
		* @return,bool
		*/
		virtual bool prepareTest();

		/*
		* @brief,准备测试[重载2]
		* @param1,总线状态报文ID
		* @param2,完全启动的值
		* @param3,完全启动处理
		* @param4,启动超时
		* @return,bool
		*/
		virtual bool prepareTest(int id, int value = 0, CanProc proc = nullptr, ulong timeout = START_DELAY);

		/*
		* @brief,结束测试
		* @param1,测试结果
		* @return,bool
		*/
		virtual bool finishTest(bool success);

		/*
		* @brief,保存日志
		* @param1,测试结果
		* @return,bool
		*/
		virtual bool saveLog(bool success);

		/*
		* @brief,设置准备测试无等待
		* @param1,是否无等待
		* @return,bool
		*/
		void setPrepareTestNoWait(bool on);

		/*
		* @brief,设置安全启动
		* @notice,每次启动时,会自动判断是否有CAN报文
		* @param1,是否开启
		* @return,bool
		*/
		void setSafeStart(bool on);

		/*
		* @brief,检测工作电流
		* @return,bool
		*/
		virtual bool checkWorkingCurrent();

		/*
		* @brief,检测静态电流
		* @param1,是否打开IGN
		* @notice,如果接下来检测休眠和唤醒setIgn则为false,
		* @param2,是否设置为16V电压
		* @return,bool
		*/
		virtual bool checkStaticCurrent(bool setIgn = true, bool set16Vol = true);

		/*
		* @brief,检测组件电压
		* @return,bool
		*/
		virtual bool checkComponentVoltage();

		/*
		* @brief,清除诊断故障码
		* @return,bool
		*/
		virtual bool clearDtc();

		/*
		* @brief,检测版本号
		* @return,bool
		*/
		virtual bool checkVersion();

		/*
		* @brief,检测诊断故障码
		* @return,bool
		*/
		virtual bool checkDtc();

		/*
		* @brief,写入序列号
		* @param1,匿名函数
		* @return,bool
		*/
		bool writeSn(const std::function<bool()>& lambda);

		/*
		* @brief,检测序列号
		* @param1,匿名函数
		* @return,bool
		*/
		bool checkSn(const std::function<bool()>& lambda);

		/*
		* @brief,写入数据
		* @param1,匿名函数
		* @return,bool
		*/
		bool writeDate(const std::function<bool()>& lambda);

		/*
		* @brief,检测日期
		* @param1,匿名函数
		* @return,bool
		*/
		bool checkDate(const std::function<bool()>& lambda);

		/*
		* @brief,写入出场设置
		* @param1,匿名函数
		* @return,bool
		*/
		bool writeSet(const std::function<bool()>& lambda);

		/*
		* @brief,设置函数
		* @notice,此函数用于,处理一些没有在框架中的函数,
		* 将其放入到lambda中执行
		*/
		bool setFnc(const std::function<bool()>& lambda);

		/*
		* @brief,设置输出CAN日志
		* @param1,是否输出
		* @return,bool
		*/
		void setOutputCanLog(bool output = true);

		/*
		* @brief,设置保存CAN日志
		* @param1,是否保存
		* @return,void
		*/
		void setSaveCanLog(bool save = true);

		/*
		* @brief,设置CAN日志名称
		* @param1,机种名称
		* @param2,条码名称
		* @return,void
		*/
		void setCanLogName(const QString& modelName, const QString& code);

		/*
		* @brief,刷新CAN日志缓存区
		* @return,void
		*/
		void flushCanLogBuffer();

		/*
		* @brief,清空CAN报文缓存区
		* @return,void
		*/
		void clearCanMsgBuffer();

		/*
		* @brief,复位CAN
		* @return,bool
		*/
		bool resetCan();

		/*
		* @brief,开始CAN报文
		* @return,void
		*/
		void startCanMsg();

		/*
		* @brief,停止CAN报文
		* @return,void
		*/
		void stopCanMsg();

		/*
		* @brief,发送Can报文
		* @param1,发送的报文ID
		* @param2,发送的报文周期
		* @param3,发送的起始位置
		* @param4,发送的报文长度
		* @param5,发送的报文数据
		* @param6,发送的报文类型
		* @param7,发送的报文次数
		* @param8,第N帧与第N+1帧的间隔
		* @return,bool
		*/
		bool sendCanMsg(int id, int period, int start, int length, quint64 data, can::SendType type = can::SendType::CYCLE, int count = 0, int interval = 0);

		/*
		* @brief,发送Can报文
		* @param1,发送的报文ID
		* @param2,发送的报文周期
		* @param3,数据初始化列表
		* @param4,发送的报文类型
		* @param5,发送的报文次数
		* @param6,第N帧与第N+1帧的间隔
		* @return,bool
		*/
		bool sendCanMsg(int id, int period, DataList data, can::SendType type = can::SendType::CYCLE, int count = 0, int interval = 0);

		/*
		* @brief,发送Can报文
		* @param1,报文
		* @return,void
		*/
		void sendCanMsg(const can::Msg& msg);

		/*
		* @brief,发送Can报文
		* @param1,发送的报文ID
		* @param2,发送处理[lambda]
		* @param3,发送的报文周期
		* @param4,发送的报文类型
		* @param5,发送的报文次数
		* @param6,第N帧与第N+1帧的间隔
		* @return,void
		*/
		void sendCanMsg(int id, can::Base::SendProc proc, int period, can::SendType type = can::SendType::CYCLE, int count = 0, int interval = 0);

		/*
		* @brief,发送Can报文
		* @param1,报文初始化列表
		* @return,void
		*/
		void sendCanMsg(CanList msg);

		/*
		* @brief,发送Can报文
		* @param1,报文
		* @param2,起始位置
		* @param3,数据长度
		* @param4,数据
		* @return,void
		*/
		void sendCanMsg(const can::Msg& msg, int start, int length, quint64 data);

		/*
		* @brief,接收Can报文
		* @param1,报文数组
		* @param2,最大接收大小
		* @return,int
		*/
		int receiveCanMsg(can::Msg* msg, int maxSize);

		/*
		* @brief,删除Can报文
		* @param1,报文初始化列表
		* @return,void
		*/
		void deleteCanMsgs(CanList list);

		/*
		* @brief,删除Can报文
		* @param1,报文id列表
		* @return,void
		*/
		void deleteCanMsgs(IdList list);

		/*
		* @brief,删除Can报文
		* @param1,报文
		* @return,void
		*/
		void deleteCanMsg(const can::Msg& msg);

		/*
		* @brief,删除Can报文
		* @notice,此函数将会删除所有在发送列表中的报文
		* @param1,报文id
		* @return,void
		*/
		void deleteCanMsg(int id);

		/*
		* @brief,删除所有Can报文
		* @return,void
		*/
		void deleteAllCanMsgs();

		/*
		* @brief,生成CAN报文
		* @param1,发送的报文ID
		* @param2,发送的报文周期(ms)
		* @param3,发送的起始位置
		* @param4,发送的长度
		* @param5,发送的数据
		* @param6,发送的类型
		* @param7,发送的次数
		* @return,can::Msg
		*/
		template <class T> can::Msg generateCanMsg(int id, int period, int start, int length, T data, can::SendType type = can::SendType::ST_CYCLE, int count = 0);

		/*
		* @brief,设置Can矩阵类型
		* @param1,矩阵类型
		* @return,void
		*/
		void setCanMatrixType(can::Matrix::Type type);

		/*
		* @brief,获取Can矩阵类型
		* @return,MatrixType
		*/
		can::Matrix::Type getCanMatrixType() const;

		/*
		* @brief,获取Can矩阵错误
		* @return,QString
		*/
		QString getCanMatrixError() const;

		/*
		* @brief,打包Can报文
		* @param1,打包的缓存区
		* @param2,数据起始位
		* @param3,数据长度
		* @param4,打包的数据
		* @return,bool
		*/
		template <class T> bool packCanMsg(uchar* buffer, int start, int length, T data);

		/*
		* @brief,解包Can报文
		* @param1,解包的缓存区
		* @param2,数据起始位
		* @param3,数据长度
		* @param4,解包的数据
		* @return,bool
		*/
		template <class T> bool unpackCanMsg(const uchar* buffer, int start, int length, T& data);

		/*
		* @brief,打包Can报文
		* @param1,打包的报文
		* @param2,数据起始位
		* @param3,数据长度
		* @param4,打包的数据
		* @return,void
		*/
		template <class T> void packCanMsg(can::Msg& msg, int start, int length, T data);

		/*
		* @brief,解包Can报文
		* @param1,解包的报文
		* @param2,数据起始位
		* @param3,数据长度
		* @return,unsigned long long
		*/
		quint64 unpackCanMsg(const can::Msg& msg, int start, int length);

		/*
		* @brief,自动处理Can报文
		* @param1,接收的报文ID
		* @param2,成功的值
		* @param3,报文处理函数
		* @param4,超时时间
		* @return,bool
		*/
		bool autoProcessCanMsg(int id, quint64 value, CanProc proc, ulong timeout = 10000U);

		/*
		* @brief,自动处理Can报文
		* @param1,接收的报文ID
		* @param2,成功的值列表
		* @param3,报文处理函数
		* @param4,超时时间
		* @return,bool
		*/
		bool autoProcessCanMsg(int id, ValList valList, CanProc proc, ulong timeout = 10000U);

		/*
		* @brief,自动处理Can报文
		* @param1,接收的报文ID
		* @param2,成功的值
		* @param3,报文起始位置
		* @param4,报文长度
		* @param5,超时时间
		* @notice,使用前需要设定CAN矩阵类型
		* @return,bool
		*/
		bool autoProcessCanMsg(int id, int value, int start, int length, ulong timeout = 10000U);

		/*
		* @brief,自动处理Can报文拓展
		* @param1,接收的报文ID列表
		* @param2,成功的值
		* @param3,报文处理函数
		* @param4,超时时间
		* @return,bool
		*/
		bool autoProcessCanMsgEx(IdList idList, int value, CanProc proc, ulong timeout = 10000U);

		/*
		* @brief,自动处理Can报文拓展
		* @param1,接收的报文ID列表
		* @param2,成功的值列表
		* @param3,报文处理函数
		* @param4,超时时间
		* @return,bool
		*/
		bool autoProcessCanMsgEx(IdList idList, ValList valList, CanProc proc, ulong timeout = 10000U);

		/*
		* @brief,设置Can处理函数[重载1]
		* @param1,实现的功能名称
		* @param2,报文
		* @param3,报文处理信息
		* @return,bool
		*/
		bool setCanProcessFnc(const char* name, const can::Msg& msg, const CanProcInfo& procInfo);

		/*
		* @brief,设置Can处理函数[重载2]
		* @param1,实现的功能名称
		* @param2,报文
		* @param3,接收的报文ID
		* @param4,成功的值
		* @param5,报文处理
		* @return,bool
		*/
		bool setCanProcessFnc(const char* name, const can::Msg& msg, int id, int value, CanProc proc);

		/*
		* @brief,设置Can处理函数拓展[重载1]
		* @param1,实现的功能名称
		* @param2,接收的报文列表
		* @param3,报文处理信息
		* @return,bool
		*/
		bool setCanProcessFncEx(const char* name, CanList list, const CanProcInfo& procInfo);

		/*
		* @brief,设置Can处理函数拓展[重载2]
		* @param1,实现的功能名称
		* @param2,报文列表
		* @param3,接收的报文ID
		* @param4,成功的值
		* @param5,报文处理
		* @return,bool
		*/
		bool setCanProcessFncEx(const char* name, CanList list, int id, int value, CanProc proc);

		/*
		* @brief,设置Uds处理函数
		* @param1,实现的功能名称
		* @param2,DID列表
		* @param3,成功的值
		* @param4,读取大小
		* @param5,UDS处理函数
		* @param6,超时时间
		* @return,bool
		*/
		bool setUdsProcessFnc(const char* name, DidList list, int value, int size, UdsProc proc, ulong timeout = 10000U);

		/*
		* @brief,设置Uds处理函数拓展
		* @param1,实现的功能名称
		* @param2,DID列表
		* @param3,值列表
		* @param5,读取的大小
		* @param6,UDS处理函数拓展
		* @param7,超时时间
		* @return,bool
		*/
		bool setUdsProcessFncEx(const char* name, DidList list, ValList valList, int size, UdsProcEx procEx, ulong timeout = 10000U);

		/*
		 * @brief,自动回收,用于处理缓存,导致占用空间的问题
		 * @param1,路径列表
		 * @param2,后缀名列表
		 * @param3,几个月回收一次,-1代表立即回收
		 * @return,void
		*/
		void autoRecycle(const QStringList& path,
			const QStringList& suffixName = { ".mp4",".jpg",".png",".bmp",".net",".run",".can",".tmp",".dmp" },
			int interval = 1);

		/*
		* @brief,设置自动回收
		* @param1,是否自动回收
		* @return,void
		*/
		void setAutoRecycle(bool autoRecycle);

		/*
		* @brief,设置回收后缀名
		* @param1,后缀名列表
		* @return,void
		*/
		void setRecycleSuffixName(const QStringList& suffixName);

		/*
		* @brief,设置回收间隔月
		* @param1,间隔
		* @return,void
		*/
		void setRecycleIntervalMonth(int interval);

		/*
		* @brief,设置Uds访问级别
		* @param1,级别
		* @return,void
		*/
		void setUdsAccessLevel(int level);

		/*
		* @brief,设置Uds会话级别
		* @param1,级别
		* @return,void
		*/
		void setUdsSessionLevel(int level);

		/*
		* @brief,还原Uds访问级别
		* @return,void
		*/
		void restoreUdsAccessLevel();

		/*
		* @brief,还原Uds会话级别
		* @return,void
		*/
		void restoreUdsSessionLevel();

		/*
		* @brief,进入安全访问
		* @param1,会话级别
		* @param2,访问级别
		* @param3,重试次数
		* @return,bool
		*/
		bool enterSecurityAccess(int session = uds::DiagnosticSessionType::EXTEND,
			int access = uds::SecurityAccessType::DEFAULT_LEVEL2, int repeat = 3);

		/*
		* @brief,通过Did读取数据
		* @param1,did0
		* @param2,did1
		* @param3,读取的数据缓存区
		* @param4,读取的数据大小
		* @return,bool
		*/
		bool readDataByDid(uchar did0, uchar did1, uchar* data, int* size = nullptr);

		/*
		* @brief,通过Did写入数据[重载1]
		* @param[in] did0 标识0
		* @param[in] did1 标识1
		* @param[in] data 写入的数据缓存区
		* @param[in] size 写入的数据大小
		* @param[in] wait 等待多久去校验
		* @return bool
		*/
		bool writeDataByDid(uchar did0, uchar did1, const uchar* data, int size, int wait = 0);

		/*
		* @brief,通过Did写入数据[重载2]
		* @param1,did0
		* @param2,did1
		* @param3,写入的数据初始化列表
		* @return,bool
		*/
		bool writeDataByDid(uchar did0, uchar did1, const std::initializer_list<uchar>& data);

		/*
		* @brief,通过Did写入数据[重载1]
		* @notice,这个函数不安全,可能会出现访问越界
		* @param1,例程控制数据
		* @param2,did0
		* @param3,did1
		* @param4,写入的数据缓存区
		* @param5,写入的数据大小
		* @return,bool
		*/
		bool writeDataByDidEx(const uchar* routine, uchar did0, uchar did1, const uchar* data, int size);

		/*
		* @brief,通过Did写入数据[重载2]
		* @param1,例程控制数据列表
		* @param2,did0
		* @param3,did1
		* @param4,写入的数据缓存区
		* @param5,写入的数据大小
		* @return,bool
		*/
		bool writeDataByDidEx(const std::initializer_list<uchar>& routine, uchar did0, uchar did1, const uchar* data, int size);

		/*
		* @brief,通过Did确认数据
		* @param1,did0
		* @param2,did1
		* @param3,确认的数据缓存区
		* @param4,确认的数据大小
		* @return,bool
		*/
		bool confirmDataByDid(uchar did0, uchar did1, const uchar* data, int size);

		/*
		* @brief,通过Did安全写入数据
		* @param1,did0
		* @param2,did1
		* @param3,写入的数据缓存区
		* @param4,写入的数据长度
		* @return,bool
		*/
		bool safeWriteDataByDid(uchar did0, uchar did1, const uchar* data, int size);

		/*
		* @brief,获取Uds最终错误
		* @return,void
		*/
		QString getUdsLastError() const;

		/*
		* @brief,初始化检测日志
		* @return,void
		*/
		void initDetectionLog();

		/*
		* @brief,设置检测日志
		* @param1,检测日志类型
		* @param2,日志处理函数
		* @return,void
		*/
		void setDetectionLog(BaseTypes::DetectionLog log = BaseTypes::DL_ALL, const std::function<void(int)>& fnc = nullptr);

		/*
		* @brief,创建日志文件
		* @param1,测试结果
		* @return,QString
		*/
		QString createLogFile(bool success);

		/*
		* @brief,写入日志
		* @param1,测试结果
		* @return,bool
		*/
		bool writeLog(bool success);

		/*
		* @brief,线程暂停
		* @return,void
		*/
		void threadPause();

		/*
		* @brief,线程是否暂停
		* @return,bool
		*/
		bool threadIsPause() const;

		/*
		* @brief,线程继续
		* @return,void
		*/
		void threadContinue();

		/*
		* @brief,线程退出
		* @return,void
		*/
		void threadQuit();

		/*
		* @brief,设置扫描对话框窗口
		* @param1,是否显示
		* @return,void
		*/
		bool setScanDlgWindow(bool show = true);

		/*
		* @brief,设置认证对话框
		* @return,bool
		*/
		bool setAuthDlg();

		/*
		* @brief,设置解锁对话框
		* @param1,是否显示
		* @void
		*/
		void setUnlockDlg(bool show = true);

		/*
		* @brief,设置消息对话框
		* @notice,此类中的所有对话框,只能在线程中运行
		* @param1,标题文本
		* @param2,内容文本
		* @return,void
		*/
		void setMessageBox(const QString& title, const QString& text);

		/*
		* @brief,设置消息对话框拓展
		* @param1,标题文本
		* @param2,内容文本
		* @param3,日志列表中的位置
		* @return,void
		*/
		void setMessageBoxEx(const QString& title, const QString& text, const QPoint& point = QPoint(0, 0));

		/*
		* @brief,设置询问对话框
		* @param1,标题文本
		* @param2,内容文本
		* @param3,是否为模态对话框
		* @return,bool
		*/
		bool setQuestionBox(const QString& title, const QString& text, bool modal = true);

		/*
		* @brief,设置询问对话框拓展
		* @param1,标题文本
		* @param2,内容文本
		* @param3,日志列表中的位置
		* @return,bool
		*/
		bool setQuestionBoxEx(const QString& title, const QString& text, const QPoint& point = QPoint(0, 0));

		/*
		* @brief,设置测试结果
		* @param1,结果
		* @return,void
		*/
		void setTestResult(BaseTypes::TestResult testResult);

		/*
		* @brief,设置当前状态
		* @param1,状态
		* @param2,是否为系统状态
		* @return,void
		*/
		void setCurrentStatus(const QString& status, bool systemStatus = false);

		/*
		* @brief,添加列表项
		* @param1,列表项数据
		* @param2,是否为日志项
		* @return,void
		*/
		void addListItem(const QString& item, bool logItem = true);

		/*
		* @brief,添加列表项拓展
		* @param1,列表项数据
		* @return,void
		*/
		void addListItemEx(const QString& item);

		/*
		* @brief,清空列表项
		* @return,void
		*/
		void clearListItem();

		/*
		* @brief,设置下载对话框
		* @param1,下载信息结构体
		* @return,bool
		*/
		bool setDownloadDlg(BaseTypes::DownloadInfo* info);

		/*
		* @brief,等待启动
		* @param1,启动延时
		* @return,bool
		*/
		virtual bool waitStartup(ulong delay);

		/*
		* @brief,检测是否PING通
		* @param1,IP地址
		* @param2,次数
		* @return,bool
		*/
		bool checkPing(const QString& address, int times);

		/*
		* @brief,检测是否PING通
		* @param1,IP地址
		* @param2,端口
		* @param3,超时时间
		* @return,bool
		*/
		bool checkPing(const QString& address, int port, int timeout);

		/*
		* @brief,检测是否PING通
		* @param1,UTIL_JSON节点中的IP地址
		* @param2,UTIL_JSON节点中的端口
		* @param3,超时时间
		* @return,bool
		*/
		bool checkPing(const QString& jsonAddress, const QString& jsonPort, int timeout);

		/*
		* @brief,检测是否PING通
		* @param1,指定IP地址
		* @param2,目的IP地址
		* @param3,目的端口
		* @param4,超时时间
		* @return,bool
		*/
		bool checkPing(const QString& source, const QString& destination, int port, int timeout);

		/*
		* @brief,检测是否PING通
		* @param1,UTIL_JSON节点中的指定IP地址
		* @param2,UTIL_JSON节点中的目的IP地址
		* @param3,UTIL_JSON节点中的目的端口
		* @param4,超时时间
		* @return,bool
		*/
		bool checkPing(const QString& jsonSource, const QString& jsonDestination, const QString& jsonPort, int timeout);

		/*
		* @brief,设置唯一诊断服务车辆类型
		* @param1,名称
		* @return,void
		*/
		void setUdsVehicleType(uds::VehicleType vehicleType);

		/*
		* @brief,获取唯一诊断服务车辆类型
		* @return,QString
		*/
		uds::VehicleType getUdsVehicleType() const;

		/*
		* @setClassName,设置类名
		* @param1,类名
		* @return,void
		*/
		static void setClassName(const QString& name);

		/*
		* @brief,获取类名
		* @return,QString
		*/
		static QString getClassName();

		/*
		* @brief,获取日志路径
		* @return,QString
		*/
		static QString getLogPath(bool absolutePath = false);

		/*
		* @brief,获取组件电压配置键值
		* @return,QMap<QString,QString>
		*/
		QMap<QString, QString> getComponentVoltageKeyValue() const;

		/*
		* @brief,获取版本配置键值
		* @return,QMap<QString,QString>
		*/
		QMap<QString, QString> getVersionConfigKeyValue() const;

		/*
		* @brief,获取诊断故障码配置键值
		* @return,QMap<QString,QString>
		*/
		QMap<QString, QString> getDtcConfigKeyValue() const;

		/*
		* @brief,获取其他配置1键值
		* @return,QMap<QString,QString>
		*/
		QMap<QString, QString> getOtherConfig1KeyValue() const;

		/*
		* @brief,获取其他配置2父键列表
		* @return,QList<QString>
		*/
		QList<QString> getOtherConfig2ParentKeyList() const;

		/*
		* @brief,获取其他配置2子键值列表
		* @return,QList<QMap<QString, QString>>
		*/
		QList<QMap<QString, QString>> getOtherConfig2ChildKeyValueList() const;

		/*
		* @brief,添加周期事件报文[重载1]
		* @param1,报文
		* @return,void
		*/
		void addCycleEventMsg(const can::Msg& msg);

		/*
		* @brief,清空周期事件报文
		* @return,void
		*/
		void clearCycleEventMsg();

		/*
		* @brief,添加周期报文
		* @param1,报文结构体
		* @return,void
		*/
		void addCycleMsg(const can::Msg& msg);

		/*
		* @brief,清除周期报文
		* @return,void
		*/
		void clearCycleMsg();

		/*
		* @brief,是否正在测试中
		* @return,bool
		*/
		bool isTesting() const;

		/*
		* @brief,设置设备故障码
		* @param1,故障码
		* @return,void
		*/
		void setDeviceFaultCode(DeviceFaultCode code);

		/*
		* @brief,获取设备故障码
		* @return,DeviceFaultCode
		*/
		DeviceFaultCode getDeviceFaultCode() const;

		/*
		* @brief,设置设备故障提示
		* @param1,是否提示
		* @return,void
		*/
		void setDeviceFaultHint(bool hint);
	public:
		//显示所有矩形框
		bool m_showAllRect = false;

		/*电源类*/
		std::shared_ptr<power::Base> m_power = nullptr;

		/*16路继电器类*/
		std::shared_ptr<relay::Base> m_relay = nullptr;

		/*电压表类*/
		std::shared_ptr<Voltmeter> m_voltmeter = nullptr;

		/*电流表*/
		std::shared_ptr<Amperemeter> m_amperemeter = nullptr;

		/*CAN连接管理*/
		std::shared_ptr<can::Base> m_can = nullptr;

		/*UDS传输*/
		std::shared_ptr<uds::Base> m_uds = nullptr;

	protected:
		/*线程是否退出*/
		bool m_quit = false;

		/*是否连接*/
		bool m_connect = false;

		/*测试是否成功*/
		bool m_success = false;

		/*等待SOC启动延时*/
		ulong m_socDelay = 3000U;

		/*测试顺序*/
		int m_testSequence = TS_NO;

		/*所用时间*/
		uint m_elapsedTime = 0;

		/*统计产品*/
		uint m_total = 1;

		/*检测类型*/
		static BaseTypes::DetectionType m_detectionType;

		/*默认配置*/
		DefConfig* m_defConfig = nullptr;

		/*硬件检测配置 */
		HwdConfig* m_hwdConfig = nullptr;

		/*UDS检测配置*/
		UdsConfig* m_udsConfig = nullptr;

		/*保存错误*/
		QString m_lastError = "未知错误";

		//条形码
		QString m_barcode;

		/*日志链表*/
		QList<QString> m_logList;

		/*唤醒电流*/
		float m_rouseCurrent = 0.0f;

		/*矩阵算法*/
		can::Matrix m_matrix;

		//类名
		static QString m_className;

		//组件电压配置键值
		QMap<QString, QString> m_componentVoltageKeyValue;

		//版本配置键值
		QMap<QString, QString> m_versionConfigKeyValue;

		//诊断故障码配置键值
		QMap<QString, QString> m_dtcConfigKeyValue;

		//其他配置1键值
		QMap<QString, QString> m_otherConfig1KeyValue;

		//其他配置2父键列表
		QList<QString> m_otherConfig2ParentKeyList;

		//其他配置2子键值列表
		QList<QMap<QString, QString>> m_otherConfig2ChildKeyValueList;

		//报文缓存大小
		static const int MSG_BUFFER_SIZE = 0xFF;

		//设备故障代码
		DeviceFaultCode m_faultCode = DFC_NO;

		//设备故障提示
		bool m_faultHint = true;

		//逻辑索引
		int m_logicIndex = 0;

		//周期事件报文
		QList<can::Msg> m_cycleEventMsg;

		//周期报文
		QList<can::Msg> m_cycleMsg;

		//准备测试无等待
		bool m_prepareTestNoWait = false;

		//安全启动
		bool m_safeStart = true;

	protected:
		/*
		* @brief,设置最终错误[重载1]
		* @param1,错误信息
		* @return,void
		*/
		void setLastError(const QString& error);

		/*
		* @brief,设置最终错误[重载2]
		* @param1,错误信息
		* @param2,是否在添加到日志列表中
		* @param3,是否显示消息对话框
		* @return,void
		*/
		void setLastError(const QString& error, bool addItem, bool msgBox = false);

		/*
		* @brief,开始测试
		* @param1,是否正在测试
		* @return,void
		*/
		void startTest(bool testing);

		/*
		* @brief,设置其他动作,[比如时间同步,激活控制器报文...]
		* @return,bool
		*/
		virtual bool setOtherAction();

	protected:
		//Uds访问等级
		int m_udsAccessLevel = static_cast<int>(uds::SecurityAccessType::DEFAULT_LEVEL1);

		//Uds会话等级
		int m_udsSessionLevel = static_cast<int>(uds::DiagnosticSessionType::EXTEND);

		/*启用自动回收*/
		bool m_autoRecycle = true;

		/*回收后缀名*/
		QStringList m_recycleSuffixName = {};

		/*回收间隔月*/
		int m_recycleIntervalMonth = -1;

		//车辆类型
		uds::VehicleType m_udsVehicleType = uds::SGMW_201S_AVM;

		//线程等待
		bool m_threadWait = false;

		//测试中
		bool m_testing = false;

	signals:
		/*更新图像信号*/
		void updateImageSignal(const QPixmap& pixmap);

		/*设置消息对话框信号*/
		void setMessageBoxSignal(const QString& title, const QString& text);

		/*设置消息对话框拓展版信号*/
		void setMessageBoxExSignal(const QString& title, const QString& text, const QPoint& point);

		/*设置询问对话框信号*/
		void setQuestionBoxSignal(const QString& title, const QString& text, bool modal, bool* result);

		/*设置询问对话框拓展版信号*/
		void setQuestionBoxExSignal(const QString& title, const QString& text, bool* result, const QPoint& point);

		/*设置测试结果信号*/
		void setTestResultSignal(BaseTypes::TestResult result);

		/*设置当前状态信号*/
		void setCurrentStatusSignal(const QString& status, bool append);

		/*设置解锁对话框信号*/
		void setUnlockDlgSignal(bool show);

		/*设置认证对话框信号*/
		void setAuthDlgSignal(bool* result, bool threadCall);

		/*增加一个列表项目信号*/
		void addListItemSignal(const QString& item, bool logItem);

		/*清空列表控件信号*/
		void clearListItemSignal();

		/*设置下载对话框信号*/
		void setDownloadDlgSignal(BaseTypes::DownloadInfo* info);

		/*开始测试信号*/
		void startTestSignal(bool testing);
	};


	/*
	* @brief,检测框架硬件类
	*/
	class DETECTION_DLL_EXPORT Hardware : public Base
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		* @param1,父对象
		*/
		explicit Hardware(QObject* parent = nullptr);

		/*
		* @brief,析构
		*/
		~Hardware();

	protected:
		/*
		* @brief,线程运行函数
		* @notice,继承Hardware的子类必须重写此函数
		* @return,void
		*/
		virtual void run() override = 0;
	};


	/*
	* @brief,检测框架功能类
	*/
	class DETECTION_DLL_EXPORT Function : public Base
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		*/
		explicit Function(QObject* parent = nullptr);

		/*
		* @brief,析构
		*/
		~Function();

		/*
		* @brief,初始化
		* @return,bool
		*/
		virtual bool initialize();

		/*
		* @brief,打开设备
		* @return,bool
		*/
		virtual bool openDevice();

		/*
		* @brief,关闭设备
		* @return,bool
		*/
		virtual bool closeDevice();

		/*
		* @brief,检测CAN唤醒休眠,[重载1]
		* @param1,唤醒报文
		* @param2,总线状态报文ID
		* @param3,唤醒成功的值
		* @param4,报文处理函数
		* @return,bool
		*/
		bool checkCanRouseSleep(const can::Msg& msg, int id, int value = 0, CanProc proc = nullptr);

		/*
		* @brief 检测CAN唤醒
		* @param[in] timeout 唤醒超时时间
		* @param[in] msg 唤醒报文
		* @param[in] id 响应id
		* @return bool
		*/
		bool checkCanRouse(int timeout = 30000, const can::Msg& msg = can::Msg(), int id = 0, int value = 0, CanProc proc = nullptr);

		/*
		* @brief,检测CAN休眠
		* @param1,超时时间
		* @param3,异步调用
		* @return,bool
		*/
		bool checkCanSleep(int timeout = 30000, bool asyncCall = false);

		/*
		* @brief,检测CAN休眠异步
		* @notice,调用此函数不可进行发送CAN报文操作,接下来必须调用
		* waitCanSleepAsync,否则将会出错.
		* @param1,超时时间
		* @return,bool
		*/
		bool checkCanSleepAsync(int timeout = 30000);

		/*
		* @brief,等待CAN休眠异步
		* @return,bool
		*/
		bool waitCanSleepAsync();

		/*
		* @brief,检测CAN唤醒休眠,[重载2]
		* @param1,总线状态报文ID
		* @param2,唤醒成功的值
		* @param3,报文处理函数
		* @return,bool
		*/
		bool checkCanRouseSleep(int id, int value = 0, CanProc proc = nullptr);

		/*
		* @brief,保存分析图像
		* @param1,图像名
		* @param2,图像源
		* @return,void
		*/
		bool saveAnalyzeImage(const QString& name, const cv::Mat& image);

		/*
		* @brief,在图像上画矩形
		* @param1,图像源
		* @return,void
		*/
		inline void drawRectOnImage(cv::Mat& mat);

		/*
		* @brief,检测图像颜色
		* @param1,矩形配置
		* @param2,颜色信息
		* @return,bool
		*/
		bool checkColor(const RectConfig& rectConfig, FncTypes::ColorInfo* colorInfo);

		/*
		* @brief 检测图像动态
		* @return bool
		*/
		bool checkImageDynamic();

		/*
		* @brief,设置矩形类型
		* @param1,矩形类型
		* @return,void
		*/
		void setRectType(FncTypes::RectType rectType = FncTypes::RT_SMALL_ALL);

		/*
		* @brief,获取矩形类型
		* @return,FncTypes::RectType
		*/
		FncTypes::RectType getRectType() const;

		/*
		* @brief,还原矩形类型
		* @return,void
		*/
		void restoreRectType();

		/*
		* @brief,显示图像
		* @param1,图像源
		* @param2,图像名称
		* @return,void
		*/
		void showImage(const cv::Mat& mat, const QString& name = "image") const;

		/*
		* @brief,设置显示图像
		* @param1,是否显示
		* @return,void
		*/
		void setShowImage(bool show);

		/*
		* @brief,获取采集卡配置
		* @return,FncTypes::CardConfig
		*/
		FncTypes::CardConfig getCaptureCardConfig() const;

		/*
		* @brief,获取采集卡
		* @return,CapBase*
		*/
		CapBase* getCaptureCard() const;

		/*
		* @brief,识别图像文字[重载1]
		* @param1,roi区域
		* @param2,图像放大倍数
		* @param3,启用灰度图
		* @param4,二值化阈值
		* @param5,二值化类型
		* @parma6,识别的文本内容
		* @return,bool
		*/
		bool recognizeImageText(const cv::Rect& rect, float scale, int gray, float threshold, int type, QString& text);

		/*
		* @brief,识别图像文字[重载2]
		* @param1,json节点
		* @param2,识别的文本内容
		* @return,bool
		*/
		bool recognizeImageText(const QString& jsonNode, QString& text);

		/*
		* @brief,识别图像处理
		* @param1,json节点
		* @param2,识别的文本内容进行判断的逻辑函数,
		* lambda参数,参数1识别的文本,参数2配置的值
		* @parma3,超时
		* @return,bool
		*/
		bool recognizeImageProc(const QString& jsonNode,
			const std::function<bool(const QString& text,
				const QString& value)>& proc = nullptr,
			ulong timeout = 10000);

		/*
		* @brief,获取触屏信息
		* @return,TouchScreenInfo
		*/
		TouchScreenInfo getTouchScreenInfo() const;

		/*
		* @brief,检测四分格图像
		* @return,bool
		*/
		bool checkQuadImage();

		/*
		* @brief,设置自定义画图
		* @param1,函数
		* @return,void
		*/
		void setCustomDraw(std::function<void(cv::Mat&)> draw);
	protected:
		/*
		* @brief,线程运行函数
		* @notice,继承Function的子类必须重写此函数
		* @return,void
		*/
		virtual void run() override = 0;

		/*
		* @brief,采集卡基类指针
		*/
		CapBase* m_capBase = nullptr;

		/*
		* @brief,光学字符识别
		*/
		Tesseract m_ocr;

		/*
		* @brief,触屏信息
		*/
		TouchScreenInfo m_touchScreen = { 0 };

	private:
		//矩形类型
		FncTypes::RectType m_rectType = FncTypes::RT_NO;

		//采集卡配置结构体
		FncTypes::CardConfig m_cardConfig;

		//是否显示图像
		bool m_showImage = false;

		//其他矩形框
		cv::Rect m_otherRect;

		//自定义画图
		std::function<void(cv::Mat&)> m_customDraw = nullptr;

		//自定义锁
		QMutex m_customLock;

		//CAN休眠电流
		float m_sleepCurrent = 0;

		//CAN休眠future
		std::future<bool> m_sleepFuture;

		//CAN休眠标记
		bool m_sleepFlag = false;
	};


	/*
	* @brief,检测框架全景类
	*/
	class DETECTION_DLL_EXPORT Avm : public Function
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		* @param1,父对象
		*/
		explicit Avm(QObject* parent = nullptr);

		/*
		* @brief,析构
		*/
		~Avm();

		/*
		* @brief,初始化
		* @return,bool
		*/
		virtual bool initialize();

		/*
		* @brief,设置跑马灯
		* @param1,开关
		* @return,void
		*/
		void setHorseRaceLamp(bool _switch);

		/*
		* @brief,通过按键触发AVM
		* @param1,高电平延时
		* @param2,AVM反馈报文ID
		* @param3,AVM反馈成功的值
		* @param4,处理AVM反馈函数
		* @param5,是否显示日志
		* @return,bool
		*/
		bool triggerAvmByKey(ulong delay = 300, int id = 0, int value = 0, CanProc proc = nullptr, bool showLog = false);

		/*
		* @brief,通过按键退出AVM
		* @param1,高电平延时
		* @param2,AVM反馈报文ID
		* @param3,AVM反馈成功的值
		* @param4,处理AVM反馈函数
		* @param5,是否显示日志
		* @return,bool
		*/
		bool exitAvmByKey(ulong delay = 300, int id = 0, int value = 0, CanProc proc = nullptr, bool showLog = false);

		/*
		* @brief,通过报文触发AVM
		* @param1,触发AVM的报文
		* @param2,AVM反馈报文ID
		* @param3,AVM反馈成功的值
		* @param4,处理AVM反馈函数
		* @param5,是否显示日志
		* @return,bool
		*/
		bool triggerAvmByMsg(const can::Msg& msg, int id = 0, int value = 0, CanProc proc = nullptr, bool showLog = false);

		/*
		* @brief,通过报文退出AVM
		* @param1,触发AVM的报文
		* @param2,AVM反馈报文ID
		* @param3,AVM反馈成功的值
		* @param4,处理AVM反馈函数
		* @param5,是否显示日志
		* @return,bool
		*/
		bool exitAvmByMsg(const can::Msg& msg, int id = 0, int value = 0, CanProc proc = nullptr, bool showLog = false);

		/*
		* @brief,检测视频出画不使用任何
		* @return,bool
		*/
		bool checkVideoUseNot();

		/*
		* @brief,检测视频出画使用报文[拓展版]
		* @param1,触发全景报文
		* @param2,触发全景成功报文
		* @param3,触发全景成功的值
		* @param4,触发全景成功函数处理
		* @param5,是否检测按键电压
		* @return,bool
		*/
		bool checkVideoUseMsg(const can::Msg& msg, int id, int value, CanProc proc, bool keyVol = false);

		/*
		* @brief,检测视频出画使用按键
		* @param1,总线状态报文ID
		* @param2,进入全景成功的值
		* @param3,处理全景报文函数
		* @param4,高电平延时
		* @param5,是否检测按键电压
		* @return,bool
		*/
		bool checkVideoUseKey(int id, int value, CanProc proc, ulong hDelay = 300, bool keyVol = true);

		/*
		* @brief,检测视频出画使用人工
		* @return,bool
		*/
		bool checkVideoUsePerson();

		/*
		* @brief,检查单个图像使用报文
		* @param1,矩形类型
		* @param2,触发报文
		* @param3,成功报文
		* @param4,成功的值
		* @param5,处理函数
		* @param6,超时
		* @return,bool
		*/
		bool checkSingleImageUseMsg(FncTypes::RectType type, const can::Msg& msg,
			int id = 0, int value = 0, CanProc proc = 0, ulong timeout = 10000U);

		/*
		* @brief,检测AVM前后视图使用报文
		* @notice,[F]代表前,[R]代表后,
		* 默认全景加前,把切后报文写第一个,反之一样.
		* @param1,前后景报文列表
		* @param2,报文延时
		* @param3,接收ID
		* @param4,请求列表
		* @param5,lambda
		* @return,bool
		*/
		bool checkFRViewUseMsg(CanList msgList, int id, ValList valList, CanProc proc);

		/*
		* @brief,检测按键电压
		* @param1,进入全景成功报文ID
		* @param2,进入全景成功的值
		* @param3,处理进入全景的报文
		* @param4,高电平延时
		* @notice,延时取决于高电平电压是否准确
		* @return,bool
		*/
		bool checkFRViewUseKey(int id, int value, CanProc proc, ulong hDelay = 300U);

		/*
		* @brief,检测按键电压
		* @param1,高电平延时
		* @notice,延时取决于高电平电压是否准确
		* @param2,进入全景成功报文ID
		* @param3,进入全景成功的值
		* @param4,处理进入全景的报文
		* @param5,是否显示日志
		* @return,bool
		*/
		bool checkKeyVoltage(ulong hDelay = 300U, int id = 0, int value = 0, CanProc proc = nullptr, bool showLog = false);

		/*
		* @brief,检测按键电压
		* @param1,Can报文
		* @param2,进入全景成功报文ID
		* @param3,进入全景成功的值
		* @param4,处理进入全景的报文
		* @param5,是否显示日志
		* @return,bool
		*/
		bool checkKeyVoltage(const can::Msg& msg, int id = 0, int value = 0, CanProc proc = nullptr, bool showLog = false);

		/*
		* @brief,检测快速启动图像
		* @param1,矩形类型
		* @param2,持续时间
		* @return,bool
		*/
		bool checkFastStartupImage(FncTypes::RectType rect, ulong duration = 15000U);
	protected:
		/*
		* @brief,线程运行函数
		* @notice,继承Avm的子类必须重写此函数
		* @return,void
		*/
		virtual void run() override = 0;
	private:
		/*
		* @brief,检测小图颜色
		* @return,bool
		*/
		bool checkSmallImageColor();
	};


	/*
	* @brief,视频录制类
	*/
	class DETECTION_DLL_EXPORT Dvr : public Function
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		* @param1,父对象
		*/
		explicit Dvr(QObject* parent = nullptr);

		/*
		* @brief,析构
		*/
		~Dvr();

		/*
		* @brief,初始化
		* @return,bool
		*/
		virtual bool initialize();

		/*
		* @brief,准备测试
		* @return,bool
		*/
		virtual bool prepareTest();

		/*
		* @brief,结束测试
		* @param1,测试是否成功
		* @return,bool
		*/
		virtual bool finishTest(bool success);

		/*
		* @brief,设置系统状态报文ID
		* @notice,继承dvr的子类必须设置状态报文ID
		* @param1,系统状态报文ID
		* @return,void
		*/
		template<class... Args> void setSystemStatusMsgId(Args&&...args);

		/*
		* @brief,设置SD卡状态
		* @param1,SD卡状态
		* @return,void
		*/
		void setSdCardStatus(DvrTypes::SdCardStatus status);

		/*
		* @brief,设置系统状态
		* @notice,此函数用于处理部分dvr上电之后不在正常录制状态
		* @param1,系统状态
		* @return,void
		*/
		void setSystemStatus(DvrTypes::SystemStatus status);

		/*
		* @brief,获取所有状态
		* @param1,系统状态报文
		* @param2,所获取的状态值
		* @return,bool
		*/
		template<class T>bool getAllStatus(T& status);

		/*
		* @brief,自动处理状态[重载1]
		* 自动处理状态,[重载1]
		* @param1,系统状态报文
		* @param2,处理超时
		* @return,bool
		*/
		template<class T> bool autoProcessStatus(ulong timeout = 30000U);

		/*
		* @brief,自动处理状态[重载2]
		* @param1,系统状态报文
		* @param2,系统精确状态值
		* @param3,处理超时
		* @return,bool
		*/
		template<class T> bool autoProcessStatus(T value, ulong timeout = 30000U);

		/*
		* @brief,检测DVR[重载1]
		* @param1,RTSP协议地址
		* @param2,是否使用WIFI
		* @param3,是否使用采集卡
		* @param4,是否下载视频
		* @return,bool
		*/
		bool checkDvr(const QString& rtspUrl, bool useWifi = true, bool useCard = false, bool downloadVideo = true);

		/*
		* @brief,检测DVR[重载2]
		* @param1,是否使用WIFI
		* @param2,是否使用采集卡
		* @param3,是否下载视频
		* @return,bool
		*/
		bool checkDvr(bool useWifi = true, bool useCard = false, bool downloadVideo = true);

		/*
		* @brief,设置声音灯光
		* @param1,是否开启
		* @return,bool
		*/
		bool setSoundLight(bool enable);

		/*
		* @brief,获取声音灯光
		* @return,bool true打开,false关闭
		*/
		bool getSoundLigth() const;

		/*
		* @brief,设置声音
		* @param1,是否开启
		* @return,bool
		*/
		bool setSound(bool enable);

		/*
		* @brief,设置灯光
		* @param1,是否开启
		* @return,bool
		*/
		bool setLight(bool enable);

		/*
		* @brief,设置VLC媒体句柄
		* @param1,句柄
		* @return,void
		*/
		void setVlcMediaHwnd(HWND vlcHwnd);

		/*
		* @brief,获取VLC媒体句柄
		* @return,HWND
		*/
		HWND getVlcMediaHwnd() const;

		/*
		* @brief,vlc开始播放rtsp文件
		* @param1,网络地址
		* @return,bool
		*/
		bool vlcRtspStart(const QString& url);

		/*
		* @brief,vlc停止播放rtsp文件
		* @return,bool
		*/
		bool vlcRtspStop();

		/*
		* @brief,vlc开始播放本地文件
		* @param1,文件名称
		* @return,bool
		*/
		bool vlcLocalStart(const QString& fileName);

		/*
		* @brief,vlc停止播放本地文件
		* @return,bool
		*/
		bool vlcLocalStop();

		/*
		* @brief,获取文件URL
		* @param1,获取的URL地址
		* @param2,文件路径类型
		* @return,bool
		*/
		bool getFileUrl(QString& url, DvrTypes::FilePath filePath);

		/*
		* @brief,下载文件[重载1]
		* @param1,下载的URL地址
		* @param2,是否下载视频
		* @return,bool
		*/
		bool downloadFile(const QString& url, bool isVideo = true);

		/*
		* @brief,下载文件[重载2]
		* @param1,下载的URL地址
		* @param2,下载的文件类型
		* @return,bool
		*/
		bool downloadFile(const QString& url, DvrTypes::FileType types);

		/*
		* @brief,检测光轴
		* @param1,照片URL地址
		* @return,bool
		*/
		bool checkRayAxis(const QString& url);

		/*
		* @brief,检测解像度
		* @param1,照片URL地址
		* @return,bool
		*/
		bool checkSfr(const QString& url);

		/*
		* @brief,检测录制使用报文
		* @param1,报文
		* @param2,接收的报文ID
		* @param3,成功的值
		* @param4,报文处理函数
		* @param5,超时时间
		* @return,bool
		*/
		bool checkRecordUseMsg(const can::Msg& msg, int id, int value, CanProc proc, ulong timeout = 25000U);

		/*
		* @brief,检测录制使用报文
		* @param1,发送的报文ID
		* @param2,报文发送周期
		* @param3,起始位置
		* @param4,长度
		* @param5,数据
		* @param6,发送类型
		* @param7,发送次数
		* @param8,接收的报文ID
		* @param9,成功的值
		* @param10,报文处理函数
		* @return,bool
		*/
		bool checkRecordUseMsg(int id0, int period, int start, int length, int data,
			can::SendType type, int count, int id1, int value, CanProc proc);

		/*
		* @brief,检测录制使用报文
		* @param1,报文
		* @param2,报文发送周期
		* @param3,起始位置
		* @param4,长度
		* @param5,数据
		* @param6,发送类型
		* @param7,发送次数
		* @param8,接收的报文ID
		* @param9,成功的值
		* @param10,接收的报文起始位置
		* @param11,接收的报文长度
		* @return,bool
		*/
		bool checkRecordUseMsg(int id0, int period, int start0, int length0, int data,
			can::SendType type, int count, int id1, int value, int start1, int length1);

		/*
		* @brief,检测录制使用按键
		* @param1,接收的报文ID
		* @param2,拍照成功数值
		* @param3,报文处理函数
		* @param4,高电平持续时间
		* @return,bool
		*/
		bool checkRecordUseKey(int id, int value, CanProc proc, ulong time = 3000L);

		/*
		* @brief,检测录制使用按键
		* @param1,接收的报文ID
		* @param2,报文起始位置
		* @param3,报文长度
		* @param4,成功的值
		* @param5,高电平持续时间
		* @return,bool
		*/
		bool checkRecordUseKey(int id, int value, int start, int length, ulong time = 3000L);

		/*
		* @brief,检测光轴解像度使用报文拓展
		* @param1,CAN报文列表
		* @param2,接收的报文ID
		* @param3,拍照成功数值
		* @param4,报文处理函数
		* @return,bool
		*/
		bool checkRayAxisSfrUseMsgEx(CanList list, int id, int value, CanProc proc);

		/*
		* @brief,检测光轴解像度使用报文
		* @param1,CAN报文
		* @param2,接收的报文ID
		* @param3,拍照成功数值
		* @param4,报文处理函数
		* @return,bool
		*/
		bool checkRayAxisSfrUseMsg(can::Msg msg, int id, int value, CanProc proc);

		/*
		* @brief,检测光轴解像度使用报文
		* @param1,报文
		* @param2,报文发送周期
		* @param3,起始位置
		* @param4,长度
		* @param5,数据
		* @param6,发送类型
		* @param7,发送次数
		* @param8,接收的报文ID
		* @param9,拍照成功数值
		* @param10,报文处理函数
		* @return,bool
		*/
		bool checkRayAxisSfrUseMsg(int id0, int period, int start, int length, int data,
			can::SendType type, int count, int id1, int value, CanProc proc);

		/*
		* @brief,检测光轴解像度使用报文
		* @param1,发送的报文ID
		* @param2,发送的报文周期
		* @param3,发送的起始位置
		* @param4,发送的报文长度
		* @param5,发送的数据
		* @param6,发送的类型
		* @param7,发送的次数
		* @param8,接收的报文ID
		* @param9,接收成功的值
		* @param10,接收的起始位置
		* @param11,接收的长度
		* @return,bool
		*/
		bool checkRayAxisSfrUseMsg(int id0, int period, int start0, int length0, int data,
			can::SendType type, int count, int id1, int value, int start1, int length1);

		/*
		* @brief,检测光轴解像度使用网络
		* @return,bool
		*/
		bool checkRayAxisSfrUseNet();

		/*
		* @brief,检测光轴解像度使用按键
		* @param1,接收的报文ID
		* @param2,拍照成功数值
		* @param3,报文处理函数
		* @param4,高电平持续时间
		* @return,bool
		*/
		bool checkRayAxisSfrUseKey(int id, int value, CanProc proc, ulong time = 300L);

		/*
		* @brief,检测光轴解像度使用按键
		* @param1,接收的报文ID
		* @param2,成功的值
		* @param3,报文起始位置
		* @param4,报文长度
		* @param5,高电平持续时间
		* @return,bool
		*/
		bool checkRayAxisSfrUseKey(int id, int value, int start, int length, ulong time = 300L);

		/*
		* @brief,检测光轴解像度
		* @return,bool
		*/
		bool checkRayAxisSfr(bool local = false);

		/*
		* @brief,搜索磁盘文件
		* @param1,文件路径
		* @param2,文件
		* @param3,超时
		* @return,bool
		*/
		bool searchDiskFile(DvrTypes::FilePath path, QString& file, int timeout = 30000);

		/*
		* @brief,格式化SD卡
		* @param1,是否网络格式化,否则本地格式化(需要配合checkPlaybackByFile使用)
		* @param2,是否忽略结果[只有netFormat为true时才生效]
		* @param3,如果即不是本地格式化或网络格式化,需要填写此参数
		* @notice,忽略结果就是只格式化,不判断是否成功
		* @return,bool
		*/
		bool formatSdCard(bool netFormat = true, bool ignoreResult = false, const QString& volume = QString());

		/*
		* @brief,检测网络版本
		* @param1,SOC版本,UTIL_JSON节点
		* @param2,MCU版本,UTIL_JSON节点
		* @param3,硬件版本,UTIL_JSON节点
		* @return,bool
		*/
		bool checkNetworkVersion(const QString& socJsonNode = "SOC版本", const QString& mcuJsonNode = "MCU版本", const QString& hwdJsonNode = "硬件版本");

		/*
		* @brief,取消挂载SD卡
		* @return,bool
		*/
		bool umountSdCard();

		/*
		* @brief,更改WIFI密码
		* @return,bool
		*/
		bool changeWifiPassword();

		/*
		* @brief,设置地址端口
		* @notice,这个函数是设置DVR服务端的IP地址和端口
		* @param1,IP地址
		* @param2,端口
		* @return,void
		*/
		void setAddressPort(const QString& address, const ushort& port);

		/*
		* @brief,网络拍照
		* @return,bool
		*/
		bool networkPhotoGraph();

		/*
		* @brief,获取Sfr服务端
		* @return,Nt::SfrServer*
		*/
		Nt::SfrServer* getSfrServer() const;

		/*
		* @brief,设置播放询问对话框
		* @param1,标题
		* @param2,文本
		* @parma3,主窗口日志列表窗口中的坐标
		* @return,int
		*/
		int setPlayQuestionBox(const QString& title, const QString& text, const QPoint& point = QPoint(0, 0));

		/*
		* @brief,通过文件检测回放
		* @param1,是否检测SD卡状态
		* @return,bool
		*/
		bool checkPlaybackByFile(bool checkState = false);

		/*
		* @brief,检测视频出画使用人工
		* @return,bool
		*/
		bool checkVideoUsePerson();
	signals:
		/*
		* @brief,设置播放询问对话框信号
		* @param1,标题
		* @param2,文本
		* @parma3,主窗口日志列表窗口中的坐标
		* @return,int
		*/
		void setPlayQuestionBoxSignal(const QString&, const QString&, int* result, const QPoint& point);
	protected:
		/*
		* @brief,线程运行函数
		* @notice,继承Dvr的子类必须重写此函数
		* @return,void
		*/
		virtual void run() override = 0;

		/*
		* @brief,获取WIFI信息
		* @notice,使用WIFI的必须重写此函数,为getAllStatus,
		* changeWifiPassword多态使用
		* @param1,是否为原始数据
		* @param2,是否显示日志
		* @return,bool
		*/
		virtual bool getWifiInfo(bool rawData = false, bool showLog = true);

		/*
		* @brief,写入网络日志
		* @param1,文件名
		* @param2,文件数据
		* @param3,文件大小
		* @return,bool
		*/
		bool writeNetLog(const char* name, const char* data, uint size);

		/*
		* @brief,获取系统状态
		* @notice,继承dvr的类如果产品框架
		* 没有改变,必须重写此函数
		* @param1,报文
		* @return,int -1失败
		*/
		virtual int getSystemStatus(const can::Msg& msg);

		/*
		* @brief,获取WIFI状态
		* @notice,继承dvr的类如果产品框架
		* 没有改变,必须重写此函数
		* @param1,报文
		* @return,int -1失败
		*/
		virtual int getWifiStatus(const can::Msg& msg);

		/*
		* @brief,获取SD卡状态
		* @notice,继承dvr的类如果产品框架
		* 没有改变,必须重写此函数
		* @param1,报文
		* @return,int -1失败
		*/
		virtual int getSdCardStatus(const can::Msg& msg);
	protected:
		//WIFI管理
		WifiMgr m_wifiMgr;

		//WIFI信息结构体
		WifiInfo m_wifiInfo = { 0 };

		//为checkPlaybackByFile和formatSdCard使用
		QString m_volume;

		//VLC句柄
		HWND m_vlcHwnd = nullptr;

		//VLC实例
		libvlc_instance_t* m_vlcInstance = nullptr;

		//VLC媒体
		libvlc_media_t* m_vlcMedia = nullptr;

		//VLC媒体播放者
		libvlc_media_player_t* m_vlcMediaPlayer = nullptr;

	private:
		//DVR客户端
		Nt::DvrClient m_dvrClient;

		//SFR服务端
		Nt::SfrServer m_sfrServer;

		//声音和灯光是否打开
		bool m_soundLight = false;

		//IP地址
		QString m_address = "10.0.0.10";

		//端口
		ushort m_port = 2000;

		//SD卡状态
		DvrTypes::SdCardStatus m_sdCardStatus = DvrTypes::SCS_NORMAL;

		//系统状态
		DvrTypes::SystemStatus m_systemStatus = DvrTypes::SS_GENERAL_RECORD;

		/*
		* @HashCode,哈希码结构体用于模板判断
		*/
		struct DETECTION_DLL_EXPORT HashCode
		{
			uint systemStatus;

			uint wifiStatus;

			uint ethernetStatus;

			uint sdCardStatus;
		}m_hashCode;

		/*
		* @brief,Dvr通讯文件列表结构体
		* @notice,详情请查阅通讯协议文档
		*/
		struct DETECTION_DLL_EXPORT FileList
		{
			uint listCount;
			struct DETECTION_DLL_EXPORT FileInfo
			{
				ushort index;

				uchar path;

				uchar type;

				uchar suffix;

				uchar reserved[3];

				uint size;

				uint date;
			}fileInfo[100];
		};

		//系统状态报文ID
		QList<int> m_systemStatusMsgId;
	};


	/*
	* @brief,模块信息检查
	*/
	class DETECTION_DLL_EXPORT Module : public Base
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		*/
		explicit Module(QObject* parent = nullptr);

		/*
		* @brief,析构
		*/
		~Module();

		/*
		* @brief,初始化
		* @return,bool
		*/
		virtual bool initialize();

		/*
		* @brief,打印标签
		* @param1,打印标签处理函数
		* @return,bool
		*/
		bool printLabel(const std::function<bool(void)>& fnc);
	protected:
		/*
		* @brief,线程运行函数
		* @notice,继承Module的子类必须重写此函数
		* @return,void
		*/
		virtual void run() override = 0;

		/*
		* @brief,获取打印机错误
		* @return,QString
		*/
		QString getPrinterError() const;

		//打印机类
		TSCPrinterMgr m_printer;
	};

	/*
	* @brief,检测框架乘客监测系统类
	*/
	class DETECTION_DLL_EXPORT Oms : public Avm
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		*/
		explicit Oms(QObject* parent = nullptr);

		/*
		* @brief,析构
		*/
		~Oms();

		/*
		* @brief,检测J2版本
		* @param1,IP地址[或UTIL_JSON其他配置1节点名]
		* @param2,版本号[或UTIL_JSON其他配置1节点名]
		* @return,bool
		*/
		bool checkJ2Version(const QString& address, const QString& version);

		/*
		* @brief,触发四分格图像
		* @param1,地址
		* @param2,端口
		* @param3,触发次数
		* @return,bool
		*/
		bool triggerQuadImage(const QString& address, ushort port, int count = 3);

		/*
		* @brief,触发四分格图像
		* @param1,UTIL_JSON节点中的地址
		* @param2,UTIL_JSON节点中的端口
		* @param3,触发次数
		* @return,bool
		*/
		bool triggerQuadImage(const QString& jsonAddress, const QString& jsonPort, int count = 3);

		/*
		* @brief,检测算法状态
		* @param1,地址
		* @param2,端口
		* @param3,超时时间
		* @return,bool
		*/
		bool checkAlgorithmStatus(const QString& address, ushort port, int timeout = 5000);

		/*
		* @brief,检测算法状态
		* @param1,UTIL_JSON节点中的地址
		* @param2,UTIL_JSON节点中的端口
		* @param3,超时时间
		* @return,bool
		*/
		bool checkAlgorithmStatus(const QString& jsonAddress, const QString& jsonPort, int timeout = 5000);
	protected:
		/*
		* @brief,线程运行函数
		* @notice,继承Oms的子类必须重写此函数
		* @return,void
		*/
		virtual void run() = 0;

	protected:
		Nt::OmsClient m_omsClient;
	};


	/*
	* @brief,检测框架人工智能网连空间类
	*/
	class DETECTION_DLL_EXPORT Aics : public Base
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		*/
		explicit Aics(QObject* parent = nullptr);

		/*
		* @brief,析构
		*/
		~Aics();

		/*
		* @brief,初始化
		* @return,bool
		*/
		bool initialize();

		/*
		* @brief,打开设备
		* @return,void
		*/
		bool openDevice();

		/*
		* @brief,关闭设备
		* @return,void
		*/
		bool closeDevice();

		/*
		* @brief,设置转接盒
		* @param1,通道
		* @param2,是否开启
		* @return,void
		*/
		bool setAdapterBox(AicsTypes::AdapterChannel channel, bool on);

		/*
		* @brief,自动处理Lin报文
		* @param1,接收的报文pid
		* @param2,成功的值
		* @param3,报文处理函数
		* @param4,超时时间
		* @return,bool
		*/
		bool autoProcessLinMsg(int pid, int value, LinProc linProc, ulong timeout = 10000U);
	protected:

		/*
		* @brief,线程运行函数
		* @notice,继承Aics的子类必须重写此函数
		* @return,void
		*/
		virtual void run() = 0;

	protected:
		//Lin传输
		LinTransmit* m_linTransmit = nullptr;

		//Aics客户端
		Nt::AicsClient m_aicsClient;
	private:
		/*
		* @brief,关闭父类设备
		* @return,void
		*/
		void closeParentClassDevice();
	private:
		//转接盒串口
		comm::Serial m_boxPort;

		//转接盒数据
		uchar m_boxData[TINY_BUFF] = { 0xff,0xa5,0x5a,0x0e };
	};

	template<class T> inline can::Msg Base::generateCanMsg(int id, int period, int start, int length, T data, can::SendType type, int count)
	{
		can::Msg msg = { id, 8, { 0 }, period, type, count };
		packCanMsg(msg, start, length, data);
		return msg;
	}

	template<class... Args> inline void Dvr::setSystemStatusMsgId(Args&&... args)
	{
		static_cast<void>(std::initializer_list<int>{(m_systemStatusMsgId.append(args), 0)...});
	}

	template<class T> inline bool Dvr::getAllStatus(T& status)
	{
		bool result = false, success = false;
		do
		{
			const uint statusCode = typeid(status).hash_code();
			const uint startTime = GetTickCount64();

			std::unique_ptr<can::Msg[]> msg(new can::Msg[MSG_BUFFER_SIZE]);
			while (true)
			{
				memset(msg.get(), 0, MSG_BUFFER_SIZE * sizeof(can::Msg));
				const int size = receiveCanMsg(msg.get(), MSG_BUFFER_SIZE);
				for (int i = 0; i < size; ++i)
				{
					if (m_systemStatusMsgId.contains(msg[i].id))
					{
						if (statusCode == m_hashCode.systemStatus)
						{
							status = static_cast<T>(getSystemStatus(msg[i]));

							switch (status)
							{
							case DvrTypes::SS_INITIALIZING:setCurrentStatus("系统初始化中", true); break;
							case DvrTypes::SS_GENERAL_RECORD:setCurrentStatus("正常录制", true); break;
							case DvrTypes::SS_PAUSE_RECORD:setCurrentStatus("暂停录制", true); break;
							case DvrTypes::SS_HARDWARE_KEY:setCurrentStatus("紧急录制 按键", true); break;
							case DvrTypes::SS_CRASH_KEY:setCurrentStatus("紧急录制 碰撞", true); break;
							case DvrTypes::SS_UPDATE_MODE:setCurrentStatus("更新模式", true); break;
							case DvrTypes::SS_ERROR:setCurrentStatus("系统故障", true); break;
							case DvrTypes::SS_OVER_TEMPERATURE:
								setCurrentStatus("正常录制", true);
								status = static_cast<T>(DvrTypes::SS_GENERAL_RECORD);
								break;
							case DvrTypes::SS_SOC_NOT_POWER_ON:setCurrentStatus("SOC未上电", true); break;
							case DvrTypes::SS_NO:break;
							default:setCurrentStatus("未知系统状态", true); break;
							}
						}
						else if (statusCode == m_hashCode.wifiStatus)
						{
							status = static_cast<T>(getWifiStatus(msg[i]));

							switch (status)
							{
							case DvrTypes::WS_CLOSE:setCurrentStatus("WIFI已关闭", true); break;
							case DvrTypes::WS_INIT:setCurrentStatus("WIFI正在初始化", true); break;
							case DvrTypes::WS_NORMAL:setCurrentStatus("WIFI正常", true); break;
							case DvrTypes::WS_CONNECT:setCurrentStatus("WIFI正在连接", true); break;
							case DvrTypes::WS_ERROR:setCurrentStatus("WIFI错误", true); break;
							case DvrTypes::WS_NO:break;
							default:setCurrentStatus("未知WIFI状态", true); break;
							}

							if (status == DvrTypes::WS_NORMAL)
							{
								if (!getWifiInfo())
								{
									addListItem(getLastError());
									break;
								}
								success = m_wifiMgr.connect(m_wifiInfo);
								addListItem(Q_SPRINTF("连接WIFI %s", OK_NG(success)));
								if (!success)
								{
									addListItem(LC_TO_Q_STR(m_wifiMgr.getLastError()));
									break;
								}
								addListItem("正在连接服务端,该过程大约需要1~20秒,请耐心等待...");
								success = m_dvrClient.connect(20);
								status = static_cast<T>(success ? DvrTypes::WS_CONNECTED : DvrTypes::WS_ERROR);
								m_dvrClient.disconnect();
								addListItem(Q_SPRINTF("连接服务端 %s", OK_NG(success)));
								if (success)
								{
									addListItem("等待系统稳定中,大约需要5秒,请耐心等待...");
									msleep(5000);
								}
							}
						}
						else if (statusCode == m_hashCode.ethernetStatus)
						{
							addListItem("正在连接服务端,该过程大约需要1~20秒,请耐心等待...");
							success = m_dvrClient.connect(20);
							status = static_cast<T>(success ? DvrTypes::ES_CONNECT : DvrTypes::ES_ERROR);
							m_dvrClient.disconnect();
							setCurrentStatus(Q_SPRINTF("以太网%s连接", success ? "已" : "未"), true);
							addListItem(Q_SPRINTF("连接服务端 %s", OK_NG(success)));
						}
						else if (statusCode == m_hashCode.sdCardStatus)
						{
							status = static_cast<T>(getSdCardStatus(msg[i]));

							switch (status)
							{
							case DvrTypes::SCS_NORMAL:setCurrentStatus("SD卡正常", true); break;
							case DvrTypes::SCS_NO_SD:setCurrentStatus("请插入SD卡", true); break;
							case DvrTypes::SCS_ERROR:setCurrentStatus("SD卡错误", true); break;
							case DvrTypes::SCS_NOT_FORMAT:setCurrentStatus("SD卡未格式化", true); break;
							case DvrTypes::SCS_INSUFFICIENT:setCurrentStatus("SD卡空间不足", true); break;
							case DvrTypes::SCS_SPEED_LOW:setCurrentStatus("SD卡速度低", true); break;
							case DvrTypes::SCS_USING:setCurrentStatus("SD卡正在使用中", true); break;
							case DvrTypes::SCS_NO:break;
							default:setCurrentStatus("未知SD卡状态", true); break;
							}

							if (status == DvrTypes::SCS_USING)
							{
								status = static_cast<T>(DvrTypes::SCS_NORMAL);
							}
						}
						success = true;
						break;
					}
				}

				if (success || GetTickCount64() - startTime > 3000)
				{
					break;
				}
			}

			if (!success)
			{
				break;
			}
			result = true;
		} while (false);
		return result;
	}

	template<class T> inline bool Dvr::autoProcessStatus(ulong timeout)
	{
		T status;
		const uint statusCode = typeid(status).hash_code();
		bool result = false, success = false;
		do
		{
			const uint startTime = GetTickCount64();
			while (true)
			{

				if (!getAllStatus<T>(status))
				{
					setLastError("未收到报文或其他错误,请查阅运行日志");
					break;
				}

				DEBUG_INFO_EX("状态 %d", (int)status);
				if (statusCode == m_hashCode.systemStatus)
				{
					if (status == static_cast<T>(DvrTypes::SS_GENERAL_RECORD))
					{
						success = true;
						break;
					}
				}
				else if (statusCode == m_hashCode.wifiStatus)
				{
					if (status == static_cast<T>(DvrTypes::/*WS_NORMAL*/WS_CONNECTED))
					{
						success = true;
						break;
					}
				}
				else if (statusCode == m_hashCode.ethernetStatus)
				{
					if (status == static_cast<T>(DvrTypes::ES_CONNECT))
					{
						success = true;
						break;
					}
				}
				else if (statusCode == m_hashCode.sdCardStatus)
				{
					if (status == static_cast<T>(DvrTypes::SCS_NORMAL))
					{
						success = true;
						break;
					}
				}

				if (success || GetTickCount64() - startTime > timeout)
				{
					setLastError("获取状态不满足条件,报错请查阅运行日志");
					break;
				}
			}

			if (!success)
			{
				break;
			}
			result = true;
		} while (false);
		return result;
	}

	template<class T> inline bool Dvr::autoProcessStatus(T value, ulong timeout)
	{
		T status;
		const uint statusCode = typeid(status).hash_code();
		bool result = false, success = false;
		do
		{
			const uint startTime = GetTickCount64();
			while (true)
			{

				if (!getAllStatus<T>(status))
				{
					setLastError("未收到报文或其他错误,请查阅运行日志");
					break;
				}

				DEBUG_INFO_EX("状态 %d,数值 %d", (int)status, (int)value);
				if (statusCode == m_hashCode.systemStatus)
				{
					if (status == value)
					{
						success = true;
						break;
					}
				}
				else if (statusCode == m_hashCode.wifiStatus)
				{
					if (status == value)
					{
						success = true;
						break;
					}
				}
				else if (statusCode == m_hashCode.ethernetStatus)
				{
					if (status == value)
					{
						success = true;
						break;
					}
				}
				else if (statusCode == m_hashCode.sdCardStatus)
				{
					if (status == value)
					{
						success = true;
						break;
					}
				}

				if (success || GetTickCount64() - startTime > timeout)
				{
					setLastError("获取状态不满足条件,报错请查阅运行日志");
					break;
				}
			}

			if (!success)
			{
				break;
			}
			result = true;
		} while (false);
		return result;
	}

	template<class T> inline bool Base::packCanMsg(uchar* buffer, int start, int length, T data)
	{
		return m_matrix.pack(buffer, start, length, data);
	}

	template<class T> inline bool Base::unpackCanMsg(const uchar* buffer, int start, int length, T& data)
	{
		return m_matrix.unpack(buffer, start, length, data);
	}

	template<class T> inline void Base::packCanMsg(can::Msg& msg, int start, int length, T data)
	{
		m_matrix.pack(msg.data, start, length, data);
	}
}

#include "Dialog/MainDlg.h"

