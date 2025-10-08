#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>

#ifdef _WIN64
#ifdef QT_DEBUG
#include "../x64/Debug/qt/uic/ui_MainDlg.h"
#else
#include "../x64/Release/qt/uic/ui_MainDlg.h"
#endif//QT_DEBUG
#else
#ifdef QT_DEBUG
#include "../Win32/Debug/qt/uic/ui_MainDlg.h"
#else
#include "../Win32/Release/qt/uic/ui_MainDlg.h"
#endif//QT_DEBUG
#endif//_WIN64

#include "../Dialog/SettingDlg.h"
//#include "../Dialog/ScanCodeDlg.h"
#include "../Dialog/DownloadDlg.h"
#include "../Detection.h"

/*
* @brief,分配一个对话框
* @notice,兼容旧框架
* @param1,继承Dt::Base的线程
*/
#define NEW_MAIN_DLG(THREAD)\
(autoReleaseNewWindow<Dt::MainDlg>(NO_THROW_NEW THREAD))->show()

/*
* @brief,运行主函数
* @notice,兼容旧框架
* @param1,继承Dt::Base的线程
*/
#define RUN_MAIN_FNC(THREAD)\
int main(int argc,char* argv[])\
{\
QApplication a(argc,argv);\
NEW_MAIN_DLG(THREAD);\
return a.exec();\
}

/*
* @brief,分配一个对话框拓展
* @param1,继承Dt::Base的线程
* @param2,机种名
*/
#define NEW_MAIN_DLG_EX(THREAD,TYPE_NAME)\
(autoReleaseNewWindow<Dt::MainDlg>(THREAD,TYPE_NAME))->show();\
connect(Dt::MainDlg::m_self, &Dt::MainDlg::applicationExitSignal, this, [&](){show();});\
hide();

namespace Dt
{
	/*
	* @brief,主界面对话框
	*/
	class DETECTION_DLL_EXPORT MainDlg : public QWidget
	{
		Q_OBJECT
	public:
		/*
		* @brief,主对话框构造
		* @param1,检测基类指针
		* @param2,控件父类
		*/
		MainDlg(Dt::Base* thread, const QString& typeName = QString(), QWidget* parent = nullptr);

		/*
		* @brief,主对话框析构
		*/
		~MainDlg();

		/*
		* @brief,指向自身的指针
		*/
		static MainDlg* m_self;

		/*
		* @brief,初始化
		* @return,bool
		*/
		bool initialize();

		/*
		* @brief,获取最终错误
		* @return,QString
		*/
		QString getLastError() const;

		/*
		* @brief,设置显示关闭提示
		* @param1,是否显示
		* @return,void
		*/
		void setShowCloseHint(bool show);
	public slots:
		/*
		* @brief,设置解锁对话框槽
		* @param1,是否显示
		* @return,void
		*/
		void setUnlockDlgSlot(bool show);

		/*
		* @brief,设置认证对话框槽
		* @param1,结果指针
		* @param2,线程调用
		* @return,void
		*/
		void setAuthDlgSlot(bool* result, bool threadCall = false);

		/*
		* @brief,设置下载对话框槽
		* @param1,下载信息结构体指针
		* @return,void
		*/
		void setDownloadDlgSlot(BaseTypes::DownloadInfo* info);

		/*
		* @brief,设置按钮槽
		* @return,void
		*/
		void settingButtonSlot();

		/*
		* @brief,连接按钮槽
		* @return,void
		*/
		void connectButtonSlot();

		/*
		* @brief,退出按钮槽
		* @return,void
		*/
		void exitButtonSlot();

		/*
		* @brief,设置消息对话框
		* @notice,子线程发射信号到主线程所使用
		* @param1,标题
		* @param2,文本
		* @return,void
		*/
		void setMessageBoxSlot(const QString& title, const QString& text);

		/*
		* @brief,设置消息对话框
		* @notice,子线程发射信号到主线程所使用
		* @param1,标题
		* @param2,文本
		* @param3,坐标
		* @return,void
		*/
		void setMessageBoxExSlot(const QString& title, const QString& text, const QPoint& point);

		/*
		* @brief,设置询问对话框
		* @notice,子线程发射信号到主线程所使用
		* @param1,标题
		* @param2,文本
		* @param3,是否为模态对话框
		* @param4,结果
		* @return,void
		*/
		void setQuestionBoxSlot(const QString& title, const QString& text, bool modal, bool* result);

		/*
		* @brief,设置询问对话框
		* @notice,子线程发射信号到主线程所使用
		* @param1,标题
		* @param2,文本
		* @param3,结果
		* @param4,在recordList中的坐标
		* @return,void
		*/
		void setQuestionBoxExSlot(const QString& title, const QString& text, bool* result, const QPoint& point);

		/*
		* @brief,设置播放询问对话框
		* @notice,子线程发射信号到主线程所使用
		* @param1,标题
		* @param2,文本
		* @param3,结果
		* @param4,在recordList中的坐标
		* @return,void
		*/
		void setPlayQuestionBoxSlot(const QString& title, const QString& text, int* result, const QPoint& point);

		/*
		* @brief,设置当前状态槽
		* @param1,当前状态内容
		* @param2,是否为系统状态
		* @return,void
		*/
		void setCurrentStatusSlot(const QString& status, bool systemStatus);

		/*
		* @brief,设置测试结果槽
		* @param1,测试结果枚举
		* @return,void
		*/
		void setTestResultSlot(BaseTypes::TestResult result);

		/*
		* @brief,添加列表项目槽
		* @param1,项目内容
		* @param2,是否为日志项目
		* @return,void
		*/
		void addListItemSlot(const QString& item, bool logItem);

		/*
		* @brief,清除列表项目槽
		* @return,void
		*/
		void clearListItemSlot();

		/*
		* @brief,更新图像槽
		* @param1,图像
		* @return,void
		*/
		void updateImageSlot(const QPixmap& pixmap);

		/*
		* @brief,使用率定时器槽
		* @return,void
		*/
		void usageRateTimerSlot();

		/*
		* @brief,重启槽
		* @param1,程序名
		* @return,void
		*/
		void restartSlot(const QString& name);

		/*
		* @断开连接定时器槽
		* @return,void
		*/
		void disconnectTimerSlot();

		/*
		* @开始测试槽
		* @param1,是否正在测试
		* @return,void
		*/
		void startTestSlot(bool testing);
	signals:
		/*
		* @brief,应用程序退出信号
		* @return,void
		*/
		void applicationExitSignal();

		/*
		* @brief 改变窗口标题信号
		* @param[in] title 标题
		* @return void
		*/
		void changeWindowTitleSignal(const QString& title);
	protected:
		/*
		* @brief,重写关闭事件
		* @param1,关闭事件
		* @return,void
		*/
		virtual void closeEvent(QCloseEvent* event);

		/*
		* @brief,设置错误
		* @param1,错误内容
		* @return,void
		*/
		void setLastError(const QString& error);
	private:
		//主对话框控件类
		Ui::MainDlgClass ui;

		//检测基类
		std::unique_ptr<Dt::Base> m_base = nullptr;

		//机种名
		QString m_typeName;

		/*保存错误*/
		QString m_lastError = "未知错误";

		/*使用率定时器*/
		QTimer m_usageRateTimer;

		/*断开连接定时器*/
		QTimer m_disconnectTimer;

		/*是否已连接*/
		bool m_connected = false;

		/*显示关闭提示*/
		bool m_showCloseHint = true;
	};
}
