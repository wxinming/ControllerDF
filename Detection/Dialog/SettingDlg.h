#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QMessageBox>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QItemDelegate>
#include <libcan/libcan.h>

#ifdef _WIN64
#ifdef QT_DEBUG
#include "../x64/Debug/qt/uic/ui_SettingDlg.h"
#else
#include "../x64/Release/qt/uic/ui_SettingDlg.h"
#endif//QT_DEBUG
#else
#ifdef QT_DEBUG
#include "../Win32/Debug/qt/uic/ui_SettingDlg.h"
#else
#include "../Win32/Release/qt/uic/ui_SettingDlg.h"
#endif//QT_DEBUG
#endif//_WIN64

#include "../Manage/Json.h"

namespace Dt
{
	/*
	* @brief,设备按钮
	*/
	enum DeviceButton
	{
		//电源链接按钮
		DB_POWER_CONN,

		//电源控制按钮
		DB_POWER_CTRL,

		//继电器连接按钮
		DB_RELAY_CONN,

		//电流表连接按钮
		DB_CURRE_CONN,

		//电压表连接按钮
		DB_VOLTA_CONN,
	};

	//按钮数量
#define BUTTON_COUNT 5

/*
* @brief,设置对话框
*/
	class DETECTION_DLL_EXPORT SettingDlg : public QWidget
	{
		Q_OBJECT
	public:
		/*
		* @brief,构造
		* @param1,父对象
		*/
		SettingDlg(void* base, QWidget* parent = Q_NULLPTR);

		/*
		* @brief,析构
		*/
		~SettingDlg();

		/*
		* @brief,获取最终错误
		* @return,QString
		*/
		QString getLastError() const;

		/*
		* @brief,初始化
		* @return,bool
		*/
		bool initialize();

		/*
		* @brief,设置是否已连接
		* @param1,是否已连接
		* @return,void
		*/
		void setIsConnect(const bool* isConnect);

	public slots:
		/*
		* @brief,展开配置槽
		* @return,void
		*/
		void configExpandSlot();

		/*
		* @brief,增加配置节点槽
		* @return,void
		*/
		void configAddNodeSlot();

		/*
		* @brief,删除配置节点槽
		* @return,void
		*/
		void configDelNodeSlot();

		/*
		* @brief,保存配置数据槽
		* @return,void
		*/
		void configSaveDataSlot();

		/*
		* @brief,退出配置对话框槽
		* @return,void
		*/
		void configExitDlgSlot();

		/*
		* @brief,树配置控件条目按下槽
		* @param1,当前按下的条目
		* @param2,当前按下的列
		* @return,void
		*/
		void configTreeItemPressedSlot(QTreeWidgetItem* item, int column);

		/*
		* @brief,树配置控件条目双击槽
		* @param1,当前双击的条目
		* @param2,当前双击的列
		* @return,void
		*/
		void configTreeItemDoubleClickedSlot(QTreeWidgetItem* item, int column);

		/*
		* @brief,树配置控件改变槽
		* @param1,当前改变的条目
		* @param2,当前改变的列
		* @return,void
		*/
		void configTreeItemChangedSlot(QTreeWidgetItem* item, int column);

		/*
		* @brief,电源连接槽
		* @return,void
		*/
		void powerConnectSlot();

		/*
		* @brief,电源控制槽
		* @return,void
		*/
		void powerControlSlot();

		/*
		* @brief,获取电流槽
		* @return,void
		*/
		void powerGetCurrentSlot();

		/*
		* @brief,继电器连接槽
		* @return,void
		*/
		void relayConnectSlot();

		/*
		* @brief,继电器控制槽
		* @param1,是否选中
		* @return,void
		*/
		void relayControlSlot(bool checked);

		/*
		* @brief,电流表连接槽
		* @return,void
		*/
		void currentConnectSlot();

		/*
		* @brief,电流表获取数值槽
		* @return,void
		*/
		void currentGetValueSlot();

		/*
		* @brief,电压表连接槽
		* @return,void
		*/
		void voltageConnectSlot();

		/*
		* @brief,电压表获取数值槽
		* @return,void
		*/
		void voltageGetValueSlot();

		/*
		* @brief,增加CAN表格条目槽
		* @param1,发送类型
		* @param2,报文结构体
		* @return,void
		*/
		void addCanTableItemSlot(const char* type, const can::Msg& msg);

		/*
		* @brief,CAN基本发送槽
		* @return,void
		*/
		void canBaseSendSlot();

		/*
		* @brief,CAN基本发送槽2
		* @return,void
		*/
		void canBaseSendSlot2();

		/*
		* @brief,CAN基本停止槽
		* @return,void
		*/
		void canBaseStopSlot();

		/*
		* @brief,CAN启动槽
		* @return,void
		*/
		void canStartupSlot();

		/*
		* @brief,CAN连接槽
		* @return,void
		*/
		void canConnectSlot();

		/*
		* @brief,CAN矩阵类型槽
		* @param1,矩阵类型
		* @return,void
		*/
		void canMatrixTypeSlot(const QString& text);

		/*
		* @brief,CAN基本发送定时器槽
		* @return,void
		*/
		void canBaseSendTimerSlot();

		/*
		* @brief,CAN基本发送定时2器槽
		* @return,void
		*/
		void canBaseSendTimer2Slot();

		/*
		* @brief,CAN清除日志槽
		* @return,void
		*/
		void canClearLogSlot();

		/*
		* @brief,CAN过滤启用槽
		* @return,void
		*/
		void canFilterEnableSlot();

		/*
		* @brief,CAN高级发送槽
		* @return,void
		*/
		void canAdvanceSendSlot();

		/*
		* @brief,CAN高级停止槽
		* @return,void
		*/
		void canAdvanceStopSlot();

		/*
		* @brief,更新图像槽
		* @param1,位图
		* @return,void
		*/
		void updateImageSlot(const QPixmap& pixmap);

		/*
		* @brief,开始抓图槽
		* @return,void
		*/
		void startCaptureSlot();

		/*
		* @brief,停止抓图槽
		* @return,void
		*/
		void stopCaptureSlot();

		/*
		* @brief,保存图像槽
		* @return,void
		*/
		void saveImageSlot();

		/*
		* @brief,保存坐标槽
		* @return,void
		*/
		void saveCoordSlot();

		/*
		* @brief,连接采集卡槽
		* @return,void
		*/
		void connectCaptureSlot();

		/*
		* @brief,显示矩形框槽
		* @param1,状态
		* @return,void
		*/
		void showRectBoxSlot(int state);
	protected:
		/*
		* @brief,设置最终错误
		* @param1,错误信息
		* @return,void
		*/
		void setLastError(const QString& error);

		/*
		* @brief,初始化配置树部件
		* @return,bool
		*/
		bool initConfigTreeWidget();

		/*
		* @brief,初始化硬件部件
		* @return,bool
		*/
		bool initHardwareWidget();

		/*
		* @brief,初始化CAN部件
		* @return,bool
		*/
		bool initCanWidget();

		/*
		* @brief,初始化画图部件
		* @return,bool
		*/
		bool initPaintWidget();

		/*
		* @brief,初始化关于部件
		* @return,bool
		*/
		bool initAboutWidget();

	private:
		/*
		* @brief,获取串口编号
		* @return,int
		*/
		int getComNumber(const QString& comName) const;

		/*
		* @brief,安全关闭设备
		* @return,void
		*/
		void safeCloseDevice();

		/*
		* @brief,获取功能指针
		* @param1,指针地址
		* @return,bool
		*/
		bool getFunctionPointer(void** pointer);
	signals:
		/*
		* @brief,增加CAN表格条目信号
		* @param1,发送类型
		* @param2,报文节点
		* @return,void
		*/
		void addCanTableItemSignal(const char* type, const can::Msg& msg);

		/*
		* @brief,重启信号
		* @param1,文件名
		* @return,void
		*/
		void restartSignal(const QString& name);

		/*
		* @brief,启用CAN高级发送按钮信号
		* @return,void
		*/
		void enableCanAdvanceSendButtonSignal();
	private:
		//设置对话框控件类
		Ui::SettingDlg ui;

		//CAN矩阵算法
		can::Matrix m_matrix;

		//最终错误
		QString m_lastError = "未知错误";

		//配置条目是否打开
		bool m_configItemOpen = false;

		//当前树部件条目指针
		QTreeWidgetItem* m_currentConfigItem = nullptr;

		//当前树部件条目配置值
		QString m_currentConfigValue = "";

		//当前树部件条目列
		int m_currentConfigColumn = 0;

		//更新警告
		enum UpdateWarn {
			//无
			UW_NO,

			//重启
			UW_RESTART,

			//重新连接
			UW_RECONNECT,

			//空
			UW_EMPTY,
		}m_updateWarn = UW_NO;

		//用于CAN发送
		can::Msg m_msg, m_msg2;

		//CAN基本发送定时器
		QTimer m_canBaseSendTimer;

		//CAN基本发送定时器2
		QTimer m_canBaseSendTimer2;

		//是否开始抓图
		bool m_startCapture = false;

		//基类指针
		void* m_base = nullptr;

		//启动CAN接收
		bool m_startupCanReceive = false;

		//按钮列表
		QList<bool> m_buttonList;

		//是否已连接
		const bool* m_isConnect = nullptr;

		//继电器选中列表
		QList<QCheckBox*> m_relayBoxList;

		//CAN是否连接
		bool m_canConnect = false;

		//抓图状态
		bool m_captureStatus = false;

		//CAN日志数量
		int m_canLogCount = 0;

		//展开全部
		bool m_expandAll = false;
	};

	class DETECTION_DLL_EXPORT ItemDelegate : public QItemDelegate {
		Q_OBJECT
	public:
		ItemDelegate(QObject* parent) :QItemDelegate(parent) {}

		~ItemDelegate() {}

		QWidget* createEditor(QWidget* parent,
			const QStyleOptionViewItem& option,
			const QModelIndex& index) const Q_DECL_OVERRIDE
		{
			if (index.isValid())
			{
				QComboBox* combo = new QComboBox(parent);
				combo->addItems(index.column() == 2 ? QStringList{ "数据帧", "远程帧" } : QStringList{ "标准帧", "拓展帧" });
				combo->installEventFilter(const_cast<ItemDelegate*>(this));
				return combo;
			}
			return QItemDelegate::createEditor(parent, option, index);
		}

		void setEditorData(QWidget* editor, const QModelIndex& index) const Q_DECL_OVERRIDE
		{
			if (index.isValid())
			{
				QString value = index.model()->data(index, Qt::DisplayRole).toString();
				dynamic_cast<QComboBox*>(editor)->setCurrentText(value);
			}
			return QItemDelegate::setEditorData(editor, index);
		}

		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const Q_DECL_OVERRIDE
		{
			if (index.isValid())
			{
				model->setData(index, dynamic_cast<QComboBox*>(editor)->currentText());
			}
			return QItemDelegate::setModelData(editor, model, index);
		}
	};
}
