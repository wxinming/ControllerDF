#include "SettingDlg.h"
#include "../Detection.h"

using namespace Dt;

#define GET_BASE() static_cast<Dt::Base*>(m_base)

SettingDlg::SettingDlg(void* base, QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	PRINT_CON_DESTRUCTION(SettingDlg);
	ui.label->setLabelMode(LabelMode::LM_PAINT);
	m_base = base;
}

SettingDlg::~SettingDlg()
{
	PRINT_CON_DESTRUCTION(SettingDlg);
	GET_BASE()->m_showAllRect = false;
	safeCloseDevice();
}

QString SettingDlg::getLastError() const
{
	return m_lastError;
}

bool SettingDlg::initialize()
{
	bool result = false;
	do
	{
		RUN_BREAK(!UTIL_JSON, "UTIL_JSON初始化失败");

		const QString title = QString("%1%2[%3]设置")
			.arg(GET_TYPE_NAME())
			.arg(GET_BASE()->getDetectionName())
			.arg(g_version == "0.0.0.0" ? utility::getVersion() : g_version);

		setWindowTitle(title);

		if (!initConfigTreeWidget() ||
			!initHardwareWidget() ||
			!initCanWidget() ||
			!initPaintWidget() ||
			!initAboutWidget())
		{
			break;
		}

		result = true;
	} while (false);
	return result;
}

void SettingDlg::setIsConnect(const bool* isConnect)
{
	m_isConnect = isConnect;
}

void SettingDlg::configExpandSlot()
{
	!m_expandAll ? ui.configTree->expandAll() : ui.configTree->collapseAll();
	ui.configExpand->setIcon(QIcon(QString(":/images/Resources/images/%1").arg(!m_expandAll ? "collapse.ico" : "expand.ico")));
	ui.configExpand->setText(!m_expandAll ? "全部收起" : "全部展开");
	m_expandAll = !m_expandAll;
}

void SettingDlg::configAddNodeSlot()
{
	do
	{
		QTreeWidgetItem* oldCurrentConfigItem = m_currentConfigItem;
		QString&& currentKey = ITEM_TO_STR(m_currentConfigItem, 0);
		const QStringList keyList = { "组件电压配置","工作电流配置","组件电阻配置","版本配置","诊断故障码配置" };
		if (!keyList.contains(currentKey))
		{
			utility::QMessageBoxEx::warning(this, "警告", currentKey + "不允许添加子节点");
			break;
		}

		const QStringList newKeyList = {
			QString("0.0V组件电压[%1]").arg(UTIL_JSON->getComponentVoltageConfigCount()),
			QString("0.0V工作电流[%1]").arg(UTIL_JSON->getWorkingCurrentConfigCount()),
			QString("0.0V组件电阻[%1]").arg(UTIL_JSON->getComponentResistorConfigCount()),
			QString("XXXX版本配置[%1]").arg(UTIL_JSON->getVersionConfigCount()),
			QString("XXXX诊断配置[%1]").arg(UTIL_JSON->getDtcConfigCount())
		};

		QStringList(Json:: * childKeyListFnc[])() const = {
			&Json::getChildComponentVoltageConfigKeyList,
			&Json::getChildWorkingCurrentConfigKeyList,
			&Json::getChildComponentResistorConfigKeyList,
			&Json::getChildVersionConfigKeyList,
			&Json::getChildDtcConfigKeyList
		};

		QStringList(Json:: * childValueListFnc[])() const = {
			&Json::getChildComponentVoltageConfigValueList,
			&Json::getChildWorkingCurrentConfigValueList,
			&Json::getChildComponentResistorConfigValueList,
			&Json::getChildVersionConfigValueList,
			&Json::getChildDtcConfigValueList
		};

		QStringList(Json:: * explainListFnc[])() const = {
			&Json::getComponentVoltageConfigExplain,
			&Json::getWorkingCurrentConfigExplain,
			&Json::getComponentResistorConfigExplain,
			&Json::getVersionConfigExplain,
			&Json::getDtcConfigExplain
		};

		QJsonObject& (Json:: * getObjectFnc[])() const = {
			&Json::getComponentVoltageConfigObj,
			&Json::getWorkingCurrentConfigObj,
			&Json::getComponentResistorConfigObj,
			&Json::getVersionConfigObj,
			&Json::getDtcConfigObj
		};

		for (int i = 0; i < keyList.count(); i++)
		{
			if (currentKey == keyList[i])
			{
				QList<QTreeWidgetItem*> childList = {};
				const QString& newKey = newKeyList[i];
				QJsonObject newObj;
				auto newItem = new QTreeWidgetItem(m_currentConfigItem, { newKey });
				newItem->setIcon(0, QIcon(":/images/Resources/images/tree.ico"));
				for (int j = 0; j < (UTIL_JSON->*childKeyListFnc[i])().count(); j++)
				{
					childList.append(new QTreeWidgetItem({
						(UTIL_JSON->*childKeyListFnc[i])()[j],
						(UTIL_JSON->*childValueListFnc[i])()[j],
						(UTIL_JSON->*explainListFnc[i])()[j]
						}));
					childList.at(j)->setIcon(0, QIcon(":/images/Resources/images/key.ico"));
					childList.at(j)->setIcon(1, QIcon(":/images/Resources/images/file.ico"));
					childList.at(j)->setIcon(2, QIcon(":/images/Resources/images/msg.ico"));
					newObj.insert((UTIL_JSON->*childKeyListFnc[i])()[j],
						(UTIL_JSON->*childValueListFnc[i])()[j]);
				}
				newItem->addChildren(childList);
				(UTIL_JSON->*getObjectFnc[i])().insert(newKey, newObj);
				break;
			}
		}
		m_currentConfigItem = oldCurrentConfigItem;
	} while (false);
	return;
}

void SettingDlg::configDelNodeSlot()
{
	do
	{
		if (!m_currentConfigItem->parent())
		{
			break;
		}

		QJsonObject& (Json:: * getObjectFnc[])() const =
		{
			&Json::getComponentVoltageConfigObj,
			&Json::getWorkingCurrentConfigObj,
			&Json::getComponentResistorConfigObj,
			&Json::getVersionConfigObj,
			&Json::getDtcConfigObj
		};

		QString&& parentKey = ITEM_TO_STR(m_currentConfigItem->parent(), 0);
		QString&& currentKey = ITEM_TO_STR(m_currentConfigItem, 0);

		const QStringList keyList = { "组件电压配置","工作电流配置","组件电阻配置","版本配置","诊断故障码配置" };
		int find = -1;
		for (int i = 0; i < keyList.count(); i++)
		{
			if (parentKey == keyList[i])
			{
				find = i;
				break;
			}
		}

		if (find == -1)
		{
			utility::QMessageBoxEx::warning(this, "警告", parentKey + "不允许删除子节点");
			break;
		}

		m_currentConfigItem->parent()->takeChild(ui.configTree->currentIndex().row());
		(UTIL_JSON->*getObjectFnc[find])().remove(currentKey);
		m_currentConfigItem = ui.configTree->currentItem();
	} while (false);
	return;
}

void SettingDlg::configSaveDataSlot()
{
	do
	{
		QStringList warn =
		{
			"配置已生效,无需重启应用程序",
			"配置已生效,需重启应用程序,如果已连接,将会断开连接,\n请耐心等待,是否重启?",
			"配置已生效,需重新连接应用程序",
			"保存成功"
		};
		const QString& title = UTIL_JSON->initialize(true) ? warn.value(m_updateWarn) : QString("保存失败,%1").arg(UTIL_JSON->getLastError());

		if (m_updateWarn == UW_NO || m_updateWarn == UW_EMPTY)
		{
			utility::QMessageBoxEx::information(this, "提示", title);
		}
		else if (m_updateWarn == UW_RESTART)
		{
			if (utility::QMessageBoxEx::question(this, "提示", title) == utility::QMessageBoxEx::Yes)
			{
				emit restartSignal(QString());
			}
		}
		else
		{
			utility::QMessageBoxEx::warning(this, "警告", title);
		}
	} while (false);
	return;
}

void SettingDlg::configExitDlgSlot()
{
	close();
}

void SettingDlg::configTreeItemPressedSlot(QTreeWidgetItem* item, int column)
{
	do
	{
		QString&& currentKey = ITEM_TO_STR(item, 0);
		const QStringList keyList =
		{
			"工作电流配置",
			"组件电压配置",
			"组件电阻配置",
			"版本配置",
			"诊断故障码配置"
		};

		ui.configAdd->setEnabled(keyList.contains(currentKey));

		if (item->parent())
		{
			QString&& parentKey = ITEM_TO_STR(item->parent(), 0);
			ui.configDel->setEnabled(keyList.contains(parentKey));
		}
		else
		{
			ui.configDel->setEnabled(false);
		}

		if (m_configItemOpen)
		{
			ui.configTree->closePersistentEditor(m_currentConfigItem, m_currentConfigColumn);
			m_configItemOpen = false;
		}
		m_currentConfigItem = item;
	} while (false);
	return;
}

void SettingDlg::configTreeItemDoubleClickedSlot(QTreeWidgetItem* item, int column)
{
	/*电压配置,电流配置,电阻配置,版本配置,诊断故障码配置可以更改*/
	/*第0列和第2列不可以编辑*/
	QString parentKey;
	QString&& currentValue = ITEM_TO_STR(item, column);
	if (item->parent())
	{
		parentKey = ITEM_TO_STR(item->parent(), 0);
	}

	if (!item->parent() && currentValue.isEmpty())
	{
		return;
	}

	if ((!column || (column == 2)) && (!item->parent() ? true :
		(parentKey != "组件电压配置" && parentKey != "工作电流配置"
			&& parentKey != "组件电阻配置" && parentKey != "版本配置"
			&& parentKey != "诊断故障码配置")))
	{
		return;
	}

	m_currentConfigValue = ITEM_TO_STR(item, column);
	ui.configTree->openPersistentEditor(item, column);
	m_currentConfigItem = item;
	m_currentConfigColumn = column;
	m_configItemOpen = true;
}

void SettingDlg::configTreeItemChangedSlot(QTreeWidgetItem* item, int column)
{
	QTreeWidgetItem* parentItem = item->parent();

	QString&& parentKey = ITEM_TO_STR(parentItem, 0);
	QString&& parentValue = ITEM_TO_STR(parentItem, 1);
	QString&& currentKey = ITEM_TO_STR(item, 0);
	QString&& currentVal = ITEM_TO_STR(item, 1);

	const QStringList oneKeyList = { "设备配置", "硬件配置", "继电器配置", "范围配置", "图像动态配置",
		"阈值配置", "静态电流配置", "按键电压配置", "启用配置", "逻辑配置", "用户配置", "其他配置1" };

	const QStringList twoKeyList = { "图像配置","工作电流配置","组件电压配置",
		"组件电阻配置","版本配置","诊断故障码配置","其他配置2" };

	bool (Json:: * setValue1Fnc[])(const QString&, const QString&) =
	{
		&Json::setDeviceConfigValue,
		&Json::setHardwareConfigValue,
		&Json::setRelayConfigValue,
		&Json::setRangeConfigValue,
		&Json::setImageDynamicConfigValue,
		&Json::setThresholdConfigValue,
		&Json::setStaticCurrentConfigValue,
		&Json::setKeyVoltageConfigValue,
		&Json::setEnableConfigValue,
		&Json::setLogicConfigValue,
		&Json::setUserConfigValue,
		&Json::setOtherConfig1Value
	};

	bool (Json:: * setValue2Fnc[])(const QString&, const QString&, const QString&) =
	{
		&Json::setImageConfigValue,
		&Json::setWorkingCurrentConfigValue,
		&Json::setComponentVoltageConfigValue,
		&Json::setComponentResistorConfigValue,
		&Json::setVersionConfigValue,
		&Json::setDtcConfigValue,
		&Json::setOtherConfig2Value
	};

	void (Json:: * setKeyFnc[])(const QString&, const QString&) =
	{
		&Json::setImageConfigKey,
		&Json::setWorkingCurrentConfigKey,
		&Json::setComponentVoltageConfigKey,
		&Json::setComponentResistorConfigKey,
		&Json::setVersionConfigKey,
		&Json::setDtcConfigKey,
		&Json::setOtherConfig2Key
	};

	bool success = true;
	if (!parentItem->parent())
	{
		bool setKey = false;
		for (int i = 0; i < twoKeyList.size(); i++)
		{
			if (parentKey == twoKeyList[i])
			{
				setKey = true;
				(UTIL_JSON->*setKeyFnc[i])(m_currentConfigValue, currentKey);
				break;
			}
		}

		if (!setKey)
		{
			for (int i = 0; i < oneKeyList.size(); i++)
			{
				if (parentKey == oneKeyList[i])
				{
					/*更新提示,RESTART优先级2,RECONNECT优先级1,NO优先级0*/
					if (parentKey == "设备配置" || parentKey == "用户配置")
					{
						m_updateWarn = UpdateWarn::UW_RESTART;
					}
					else if (parentKey == "硬件配置")
					{
						if (m_updateWarn != UW_RESTART)
							m_updateWarn = UpdateWarn::UW_RECONNECT;
					}
					else
					{
						if (m_updateWarn != UW_RESTART && m_updateWarn != UW_RECONNECT)
							m_updateWarn = UpdateWarn::UW_NO;
					}

					if (!(UTIL_JSON->*setValue1Fnc[i])(currentKey, currentVal))
					{
						success = false;
					}

					if (parentKey == "设备配置" && currentKey == "条码长度")
					{
						bool convert = false;
						currentVal.toInt(&convert);
						if (!convert)
							item->setData(column, Qt::EditRole, currentVal.length());
					}
					break;;
				}
			}
		}
	}
	else
	{
		QString&& grandpaKey = ITEM_TO_STR(parentItem->parent(), 0);
		for (int i = 0; i < twoKeyList.size(); i++)
		{
			if (grandpaKey == twoKeyList[i])
			{
				if (!(UTIL_JSON->*setValue2Fnc[i])(parentKey, currentKey, currentVal))
				{
					success = false;
				}
				if (m_updateWarn != UW_RESTART && m_updateWarn != UW_RECONNECT)
					m_updateWarn = UpdateWarn::UW_NO;
				break;
			}
		}
	}

	if (!success)
	{
		utility::QMessageBoxEx::warning(this, "错误", UTIL_JSON->getLastError());
		item->setData(column, Qt::EditRole, m_currentConfigValue);
	}
	ui.configTree->closePersistentEditor(item, column);
	m_configItemOpen = false;
	m_currentConfigItem = nullptr;
}

void SettingDlg::powerConnectSlot()
{
	do
	{
		auto&& hardware = UTIL_JSON->getParsedDefConfig()->hardware;
		auto device = GET_BASE()->m_power;

		bool convert = false;
		int port = getComNumber(ui.powerCombo->currentText());
		RUN_BREAK(port == -1, "电源串口转换编号失败");
		float voltage = ui.powerVoltage->text().toFloat(&convert);
		RUN_BREAK(!convert, "电源电压不为数字");
		float current = ui.powerMaxCurrent->text().toFloat(&convert);
		RUN_BREAK(!convert, "电源电流不为数字");

		if (!m_buttonList[DB_POWER_CONN])
		{
			if (!device->isOpen())
			{
				if (!device->open(std::to_string(port), hardware.powerBaud))
				{
					utility::QMessageBoxEx::warning(this, "错误", "打开电源失败");
					break;
				}
				device->setVoltage(voltage);
				device->setCurrent(current);
			}
		}
		else
		{
			device->close();
		}
		ui.powerConnect->setText(!m_buttonList[DB_POWER_CONN] ? "断开" : "连接");
		ui.powerCombo->setEnabled(m_buttonList[DB_POWER_CONN]);
		m_buttonList[DB_POWER_CONN] = !m_buttonList[DB_POWER_CONN];
	} while (false);
	return;
}

void SettingDlg::powerControlSlot()
{
	do
	{
		if (!GET_BASE()->m_power->isOpen()) {
			utility::QMessageBoxEx::information(this, "提示", "请先连接电源");
			break;
		}

		if (!GET_BASE()->m_power->output(!m_buttonList[DB_POWER_CTRL])) {
			utility::QMessageBoxEx::warning(this, "错误", "电源上电失败");
			break;
		}
		ui.powerOn->setText(!m_buttonList[DB_POWER_CTRL] ? "关闭" : "开启");
		m_buttonList[DB_POWER_CTRL] = !m_buttonList[DB_POWER_CTRL];
	} while (false);
	return;
}

void SettingDlg::powerGetCurrentSlot()
{
	do
	{
		if (!GET_BASE()->m_power->isOpen()) {
			utility::QMessageBoxEx::information(this, "提示", "请先连接电源");
			break;
		}

		float current = 0.0f;
		if (!GET_BASE()->m_power->getCurrent(current)) {
			utility::QMessageBoxEx::warning(this, "错误", "获取电源当前电流失败");
			break;
		}
		ui.powerCurrentValue->setText(Q_SPRINTF("%.3f", current));
	} while (false);
}

void SettingDlg::relayConnectSlot()
{
	do
	{
		const int port = getComNumber(ui.relayCombo->currentText());
		auto device = GET_BASE()->m_relay;
		if (!device->isOpen())
		{
			auto def = UTIL_JSON->getParsedDefConfig();
			if (def->hardware.relayType == relay::CW_MR_DO16_KN || def->hardware.relayType == relay::CW_EMR_DO16) {
				if (!device->open(std::to_string(port), def->hardware.relayBaud)) {
					utility::QMessageBoxEx::warning(this, "错误", "打开继电器失败");
					break;
				}
			}
			else {
				in_addr addr = { 0 };
				addr.S_un.S_addr = def->hardware.relayPort;
				auto ipAddr = inet_ntoa(addr);
				if (!ipAddr) {
					utility::QMessageBoxEx::warning(this, "错误", "打开继电器失败,IP地址无效");
					break;
				}

				if (!device->open(ipAddr, def->hardware.relayBaud)) {
					utility::QMessageBoxEx::warning(this, "错误", "打开继电器失败");
					break;
				}
			}
		}
		else
		{
			for (const auto& x : m_relayBoxList)
			{
				x->setChecked(false);
			}
			device->close();
		}
		ui.relayCombo->setEnabled(m_buttonList[DB_RELAY_CONN]);
		ui.relayConnect->setText(!m_buttonList[DB_RELAY_CONN] ? "断开" : "连接");
		m_buttonList[DB_RELAY_CONN] = !m_buttonList[DB_RELAY_CONN];
	} while (false);
	return;
}

void SettingDlg::relayControlSlot(bool checked)
{
	bool convert = false;
	do
	{
		QCheckBox* box = dynamic_cast<QCheckBox*>(QObject::sender());
		RUN_BREAK(!box, "dynamic_cast<QCheckBox*>(QObject::sender())失败");
		if (!GET_BASE()->m_relay->isOpen())
		{
			box->setChecked(false);
			utility::QMessageBoxEx::information(this, "提示", "请先连接继电器");
			break;
		}

		const int io = box->objectName().mid(7).toInt(&convert);
		RUN_BREAK(!convert, "无法控制IO,objectName转换失败");
		if (!GET_BASE()->m_relay->setOne(io, checked))
		{
			utility::QMessageBoxEx::warning(this, "错误", Q_SPRINTF("%s继电器IO%d失败", checked ? "打开" : "关闭", io));
			break;
		}
	} while (false);
	return;
}

void SettingDlg::currentConnectSlot()
{
	do
	{
		const int port = getComNumber(ui.currentCombo->currentText());
		auto device = GET_BASE()->m_amperemeter;
		if (!m_buttonList[DB_CURRE_CONN]) {
			if (!device->isOpen()) {
				if (!device->open(std::to_string(port), UTIL_JSON->getParsedDefConfig()->hardware.amperemeterBaud)) {
					utility::QMessageBoxEx::warning(this, "错误", "打开电流表失败");
					break;
				}
			}
		}
		else {
			device->close();
		}
		ui.currentCombo->setEnabled(m_buttonList[DB_CURRE_CONN]);
		ui.currentConnect->setText(!m_buttonList[DB_CURRE_CONN] ? "断开" : "连接");
		m_buttonList[DB_CURRE_CONN] = !m_buttonList[DB_CURRE_CONN];
	} while (false);
	return;
}

void SettingDlg::currentGetValueSlot()
{
	do
	{
		auto device = GET_BASE()->m_amperemeter;
		if (!device->isOpen()) {
			utility::QMessageBoxEx::information(this, "提示", "请先连接电流表");
			break;
		}

		float current = 0.0f;
		if (!device->getCurrent(current)) {
			utility::QMessageBoxEx::warning(this, "错误", "获取电流失败");
			break;
		}
		ui.currentValue->setText(N_TO_Q_STR(current));
	} while (false);
	return;
}

void SettingDlg::voltageConnectSlot()
{
	do
	{
		int port = getComNumber(ui.voltageCombo->currentText());
		auto device = GET_BASE()->m_amperemeter;
		if (!m_buttonList[DB_VOLTA_CONN]) {
			if (!device->isOpen()) {
				if (!device->open(std::to_string(port), UTIL_JSON->getParsedDefConfig()->hardware.voltmeterBaud)) {
					utility::QMessageBoxEx::warning(this, "错误", "打开电压表失败");
					break;
				}
			}
		}
		else {
			device->close();
		}
		ui.voltageConnect->setText(!m_buttonList[DB_VOLTA_CONN] ? "断开" : "连接");
		ui.voltageCombo->setEnabled(m_buttonList[DB_VOLTA_CONN]);
		m_buttonList[DB_VOLTA_CONN] = !m_buttonList[DB_VOLTA_CONN];
	} while (false);
	return;
}

void SettingDlg::voltageGetValueSlot()
{
	do
	{
		auto device = GET_BASE()->m_voltmeter;
		if (!device->isOpen()) {
			utility::QMessageBoxEx::information(this, "提示", "请先连接电压表");
			break;
		}

		float voltage = 0.0f;
		if (!device->getVoltage(voltage)) {
			utility::QMessageBoxEx::warning(this, "错误", "读取电压失败");
			break;
		}
		ui.voltageValue->setText(N_TO_Q_STR(voltage));
	} while (false);
	return;
}

void SettingDlg::addCanTableItemSlot(const char* type, const can::Msg& msg)
{
	if (ui.canFilterEnable->isChecked())
	{
		bool findHave = false, findNot = true, _not = false;
		QStringList filterList = ui.canFilter->text().split("|", QString::SkipEmptyParts);
		for (int i = 0; i < filterList.size(); i++)
		{
			if (!i && filterList[i].contains("^"))
			{
				_not = true;
				filterList[i] = filterList[i].mid(1);
			}

			const int value = filterList[i].toInt(nullptr, 16);
			if (value == msg.id)
			{
				if (_not)
					findNot = false;
				else
					findHave = true;
				break;
			}

		}

		if (_not && !findNot)
		{
			return;
		}

		if (!_not && !findHave)
		{
			return;
		}
	}

	const int rowCount = ui.canTable0->rowCount();
	ui.canTable0->insertRow(rowCount);
	ui.canTable0->setItem(rowCount, 0, new QTableWidgetItem(QString::number(++m_canLogCount)));
	ui.canTable0->setItem(rowCount, 1, new QTableWidgetItem(type));
	ui.canTable0->setItem(rowCount, 2, new QTableWidgetItem(utility::getCurrentTimeEx()));
	ui.canTable0->setItem(rowCount, 3, new QTableWidgetItem(QString::number(msg.id, 16)));
	ui.canTable0->setItem(rowCount, 4, new QTableWidgetItem("数据帧"));
	ui.canTable0->setItem(rowCount, 5, new QTableWidgetItem(msg.expFrame ? "拓展帧" : "标准帧"));
	ui.canTable0->setItem(rowCount, 6, new QTableWidgetItem(QString::number(msg.dlc)));

	QString data;
	char temp[32] = { 0 };
	for (int i = 0; i < msg.dlc; ++i) {
		if (i != msg.dlc - 1) {
			sprintf_s(temp, "%02x ", msg.data[i]);
		}
		else {
			sprintf_s(temp, "%02x", msg.data[i]);
		}
		data.append(temp);
	}
	ui.canTable0->setItem(rowCount, 7, new QTableWidgetItem(data));
	//ui.canTable0->setItem(rowCount, 7, new QTableWidgetItem(
	//	Q_SPRINTF("%02x %02x %02x %02x %02x %02x %02x %02x",
	//	msg.data[0], msg.data[1], msg.data[2], msg.data[3], msg.data[4], msg.data[5], msg.data[6], msg.data[7])));
	ui.canTable0->scrollToBottom();
}

void SettingDlg::canBaseSendSlot()
{
	do
	{
		bool convert = false;
		m_msg.dlc = 8;
		m_msg.expFrame = ui.canFrameType->currentText() == "拓展帧";
		m_msg.remFrame = ui.canFrameFormat->currentText() == "远程帧";
		m_msg.id = ui.canFrameId->text().toInt(&convert, 16);
		m_msg.sendType = can::SendType::EVENT;

		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "帧ID不为16进制");
			break;
		}

		QStringList datas = ui.canData->text().split(" ", QString::SkipEmptyParts);
		if (datas.size() != 8)
		{
			utility::QMessageBoxEx::warning(this, "错误", "数据格式必须为8位,中间以空格区分");
			break;
		}

		int i = 0;
		bool overflow = false;
		for (; i < datas.size(); i++)
		{
			int value = datas[i].toInt(&convert, 16);
			m_msg.data[i] = value;
			if (!convert)
			{
				break;
			}

			if (value < 0 || value > 0xff)
			{
				overflow = true;
				break;
			}
		}

		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", QString("数据%1不为16进制").arg(i + 1));
			break;
		}

		if (overflow)
		{
			utility::QMessageBoxEx::warning(this, "错误", "单个数据不可超过0~255范围");
			break;
		}

		const int sendCount = ui.canSendCount->text().toInt(&convert);
		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "发送次数不为整数");
			break;
		}

		const int sendPeriod = ui.canSendPeriod->text().toInt(&convert);
		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "时间间隔不为整数");
			break;
		}

		m_msg.sendCycle = sendPeriod;
		m_msg.sendCount = sendCount;

		ui.canBaseSend->setEnabled(false);
		GET_BASE()->m_can->addMsg(m_msg);
		GET_BASE()->m_can->startAsyncSendMsg();

		m_canBaseSendTimer.start(sendPeriod * sendCount);
	} while (false);
}

void SettingDlg::canBaseSendSlot2()
{
	do
	{
		memset(&m_msg2, 0x00, sizeof(can::Msg));
		bool convert = false;
		m_msg2.dlc = 8;
		m_msg2.expFrame = ui.canFrameType2->currentText() == "拓展帧";
		m_msg2.remFrame = ui.canFrameFormat->currentText() == "远程帧";
		m_msg2.id = ui.canFrameId2->text().toInt(&convert, 16);
		m_msg2.sendType = can::SendType::EVENT;

		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "帧ID不为16进制");
			break;
		}

		const int startPos = ui.canStartPos->text().toInt(&convert);
		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "起始不为整数");
			break;
		}

		const int dataLength = ui.canDataLength->text().toInt(&convert);
		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "数据长度不为整数");
			break;
		}

		quint64 data = ui.canData2->text().toULongLong(&convert);
		if (!convert) {
			utility::QMessageBoxEx::warning(this, "错误", "数据不为整数");
			break;
		}

		const int sendCount = ui.canSendCount->text().toInt(&convert);
		if (!convert) {
			utility::QMessageBoxEx::warning(this, "错误", "发送次数不为整数");
			break;
		}

		const int sendPeriod = ui.canSendPeriod->text().toInt(&convert);
		if (!convert) {
			utility::QMessageBoxEx::warning(this, "错误", "时间间隔不为整数");
			break;
		}

		if (!m_matrix.pack(m_msg2.data, startPos, dataLength, data)) {
			QMessageBox::warning(this, "错误", LS_TO_Q_STR(m_matrix.getLastError()));
			break;
		}

		m_msg.sendCycle = sendPeriod;
		m_msg.sendCount = sendCount;
		ui.canBaseSend2->setEnabled(false);

		GET_BASE()->m_can->addMsg(m_msg2);
		GET_BASE()->m_can->startAsyncSendMsg();

		m_canBaseSendTimer2.start(sendPeriod * sendCount);
	} while (false);
	return;
}

void SettingDlg::canBaseStopSlot()
{
	if (m_canBaseSendTimer.isActive()) {
		GET_BASE()->m_can->deleteMsg(m_msg.id);
		m_canBaseSendTimer.stop();
		ui.canBaseSend->setEnabled(true);
	}

	if (m_canBaseSendTimer2.isActive()) {
		GET_BASE()->m_can->deleteMsg(m_msg2.id);
		m_canBaseSendTimer2.stop();
		ui.canBaseSend2->setEnabled(true);
	}
}

void SettingDlg::canStartupSlot()
{
	if (!m_startupCanReceive) {
		ui.canStartup->setText("停止");
		connect(this, &SettingDlg::addCanTableItemSignal, this, &SettingDlg::addCanTableItemSlot);
	}
	else {
		ui.canStartup->setText("开始");
		disconnect(this, &SettingDlg::addCanTableItemSignal, this, &SettingDlg::addCanTableItemSlot);
	}
	m_startupCanReceive = !m_startupCanReceive;
}

void SettingDlg::canConnectSlot()
{
	const DeviceConfig& config = UTIL_JSON->getParsedDeviceConfig();
	auto transmit = GET_BASE()->m_can;
	if (m_canConnect) {
		transmit->close();
	}
	else {
		can::Device device;
		device.deviceIndex = config.canDeviceNum.toInt();
		memset(device.enableChannel, 0, sizeof(device.enableChannel));
		device.enableChannel[config.canChannelNum.toInt()] = true;
		device.arbiBaud[config.canChannelNum.toInt()] = config.canArbiBaud.toInt();
		device.dataBaud[config.canChannelNum.toInt()] = config.canDataBaud.toInt();
		device.isExpandFrame = config.canExpFrame.toInt();
		auto peerAddress = config.canPeerAddress.toStdString();
		device.peerAddress = peerAddress.c_str();
		device.peerPort = config.canPeerPort.toInt();
		if (!transmit->open(device)) {
			utility::QMessageBoxEx::warning(this, "错误", LS_TO_Q_STR(transmit->getLastError()));
			return;
		}
	}
	ui.canConnect->setText(m_canConnect ? "连接" : "断开");
	m_canConnect = !m_canConnect;
}

void SettingDlg::canMatrixTypeSlot(const QString& text)
{
	if (text == "MOTO_LSB") {
		m_matrix.setType(can::Matrix::Type::MOTOROLA_LSB);
	}
	else if (text == "MOTO_MSB") {
		m_matrix.setType(can::Matrix::Type::MOTOROLA_MSB);
	}
	else {
		m_matrix.setType(can::Matrix::Type::INTEL);
	}
}

void SettingDlg::canBaseSendTimerSlot()
{
	ui.canBaseSend->setEnabled(true);
	m_canBaseSendTimer.stop();
}

void SettingDlg::canBaseSendTimer2Slot()
{
	ui.canBaseSend2->setEnabled(true);
	m_canBaseSendTimer2.stop();
}

void SettingDlg::canClearLogSlot()
{
	int rowCount = ui.canTable0->rowCount();
	for (int i = 0; i < rowCount; ++i)
		ui.canTable0->removeRow(0);
	m_canLogCount = 0;
}

void SettingDlg::canFilterEnableSlot()
{
	if (ui.canFilter->text().isEmpty())
	{
		utility::QMessageBoxEx::warning(this, "错误", "过滤条件为空");
		ui.canFilterEnable->setChecked(false);
		return;
	}

	bool success = true, convert = false;
	QStringList filterList = ui.canFilter->text().split("|", QString::SkipEmptyParts);
	for (int i = 0; i < filterList.size(); i++)
	{
		if (!i && filterList[i].contains("^"))
		{
			filterList[i] = filterList[i].mid(1);
		}

		filterList[i].toInt(&convert, 16);

		if (!convert)
		{
			success = false;
			break;
		}
	}

	if (!success)
	{
		utility::QMessageBoxEx::warning(this, "错误", "过滤条件只能包含^|和数字");
		ui.canFilterEnable->setChecked(false);
		return;
	}
	ui.canFilter->setEnabled(!ui.canFilterEnable->isChecked());
}

struct DETECTION_DLL_EXPORT CanAdvance
{
	QList<can::Msg> msgList;
	int sendCount;
	int sendDelay;
	int frameDelay;
	int exitThread;
	SettingDlg* dlg;
};

static CanAdvance g_advance = {};

void SettingDlg::canAdvanceSendSlot()
{
	static bool press = false;
	if (press)
		return;
	press = true;
	bool success = true, convert = false;
	do
	{
		g_advance.msgList.clear();
		g_advance.dlg = this;
		g_advance.exitThread = 0;
		g_advance.sendCount = ui.canAdvanceSendCount->text().toInt(&convert);
		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "发送次数不为整数");
			break;
		}

		g_advance.sendDelay = ui.canAdvanceSendDelay->text().toInt(&convert);
		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "发送间隔不为整数");
			break;
		}

		g_advance.frameDelay = ui.canAdvanceFrameDelay->text().toInt(&convert);
		if (!convert)
		{
			utility::QMessageBoxEx::warning(this, "错误", "每帧间隔不为整数");
			break;
		}

		for (int i = 0; i < ui.canTable1->rowCount(); ++i)
		{
			auto item = ui.canTable1->item(i, 0);
			if (!item->checkState())
			{
				continue;
			}

			can::Msg msg = { 0 };
			msg.dlc = 8;

			item = ui.canTable1->item(i, 1);
			msg.expFrame = item->text() == "拓展帧";

			item = ui.canTable1->item(i, 2);
			msg.remFrame = item->text() == "远程帧";

			item = ui.canTable1->item(i, 3);
			msg.id = item->text().toInt(&convert, 16);

			if (!convert)
			{
				success = false;
				utility::QMessageBoxEx::warning(this, "错误",
					Q_SPRINTF("编号%d,帧ID不为16进制整数", i));
				break;
			}

			item = ui.canTable1->item(i, 4);
			QStringList datas = item->text().split(" ", QString::SkipEmptyParts);
			if (datas.size() != 8)
			{
				success = false;
				utility::QMessageBoxEx::warning(this, "错误",
					Q_SPRINTF("编号%d,数据格式必须为8位,中间以空格区分", i));
				break;
			}

			int j = 0;
			bool overflow = false;
			for (; j < datas.size(); j++)
			{
				int value = datas[j].toInt(&convert, 16);
				msg.data[j] = value;
				if (!convert)
				{
					break;
				}

				if (value < 0 || value > 0xff)
				{
					overflow = true;
					break;
				}
			}

			if (!convert)
			{
				success = false;
				utility::QMessageBoxEx::warning(this, "错误",
					Q_SPRINTF("编号%d,数据%d不为16进制", i, j + 1));
				break;
			}

			if (overflow)
			{
				success = false;
				utility::QMessageBoxEx::warning(this, "错误",
					Q_SPRINTF("编号%d,数据字节%d不可超过0~255范围", i, j + 1));
				break;
			}
			g_advance.msgList.push_back(msg);
		}

		if (!success)
		{
			break;
		}
		ui.canAdvanceSend->setEnabled(false);

		_beginthread([](void* args)->void
			{
				auto send = static_cast<Dt::Base*>(args)->m_can;
				CanAdvance* advance = &g_advance;
				for (int i = 0; i < advance->sendCount; ++i)
				{
					for (int j = 0; j < advance->msgList.size(); ++j)
					{
						advance->msgList[j].sendCycle = 0;
						advance->msgList[j].sendType = can::SendType::EVENT;
						advance->msgList[j].sendCount = 1;
						send->addMsg(advance->msgList[j]);
						send->startAsyncSendMsg();

						if (j != advance->msgList.size() - 1)
						{
							Sleep(advance->frameDelay);
						}

						if (advance->exitThread)
						{
							break;
						}
					}

					if (advance->exitThread)
					{
						break;
					}
					Sleep(advance->sendDelay);
				}
				emit advance->dlg->enableCanAdvanceSendButtonSignal();
			}, 0, m_base);
	} while (false);
	press = false;
	return;
}

void SettingDlg::canAdvanceStopSlot()
{
	g_advance.exitThread = 1;
}

void SettingDlg::updateImageSlot(const QPixmap& pixmap)
{
	ui.label->setPixmap(pixmap);
}

void SettingDlg::startCaptureSlot()
{
	if (m_startCapture) return;
	m_startCapture = true;
	connect(static_cast<Dt::Base*>(m_base), &Dt::Base::updateImageSignal, this, &SettingDlg::updateImageSlot);
}

void SettingDlg::stopCaptureSlot()
{
	if (!m_startCapture) return;
	m_startCapture = false;
	disconnect(static_cast<Dt::Base*>(m_base), &Dt::Base::updateImageSignal, this, &SettingDlg::updateImageSlot);
}

void SettingDlg::saveImageSlot()
{
	Dt::Function* function = nullptr;
	if (!getFunctionPointer(reinterpret_cast<void**>(&function)))
		return;

	auto capture = function->getCaptureCard();
	if (!capture)
	{
		utility::QMessageBoxEx::warning(this, "错误", "采集卡类库为空指针");
		return;
	}

	if (capture->getCardName() == "NULL")
	{
		utility::QMessageBoxEx::warning(this, "错误", "不支持采集卡名称NULL抓图");
		return;
	}

	const QString path = "Log\\Capture";
	utility::makePath(path);

	auto tick = GetTickCount64();

	QString file = Q_SPRINTF("%s\\%llu_Source.bmp", Q_TO_C_STR(path), tick);

	if (!cv::imwrite(Q_TO_C_STR(file), capture->getSourceImage()))
	{
		utility::QMessageBoxEx::warning(this, "错误", QString("保存原始图像%1失败").arg(file));
	}

	file = Q_SPRINTF("%s\\%llu_Scaled.bmp", Q_TO_C_STR(path), tick);
	if (!cv::imwrite(Q_TO_C_STR(file), capture->getScaledImage()))
	{
		utility::QMessageBoxEx::warning(this, "错误", QString("保存缩放图像%1失败").arg(file));
	}
}

void SettingDlg::saveCoordSlot()
{
	QVector<QPoint> start, end;
	ui.label->getCoordinate(&start, &end);

	QStringList rectList = { "前小图矩形框","后小图矩形框","左小图矩形框","右小图矩形框","汽车图矩形框" };
	bool paintBigImage = ui.paintBigImageBox->isChecked();

	if (paintBigImage)
	{
		rectList = QStringList{ "前大图矩形框","后大图矩形框","左大图矩形框","右大图矩形框" };
		if (start.size() < 1 || end.size() < 1)
		{
			utility::QMessageBoxEx::warning(this, "提示", "画大图坐标数量最少为1个坐标,包含[前后(可选)左(可选)右(可选)].");
			return;
		}

		if (start.size() > rectList.size() || end.size() > rectList.size())
		{
			utility::QMessageBoxEx::warning(this, "提示", "画大图坐标数量最多为4个坐标,包含[前后(可选)左(可选)右(可选)].");
			return;
		}

		if (start.size() == 1 || end.size() == 1)
		{
			QPoint s = start.front(), e = end.front();
			start.clear();
			end.clear();

			for (int i = 0; i < rectList.size(); i++)
			{
				start.push_back(s);
				end.push_back(e);
			}
		}
	}
	else
	{
		if (start.size() != 5 || end.size() != 5)
		{
			utility::QMessageBoxEx::warning(this, "提示", "画小图坐标数量必须为5个坐标,包含[前后左右小汽车图].");
			return;
		}
	}

	for (int i = 0; i < start.size(); i++)
	{
		UTIL_JSON->setImageConfigValue(rectList[i], "X坐标", QString::number(start[i].x()));
		UTIL_JSON->setImageConfigValue(rectList[i], "Y坐标", QString::number(start[i].y()));
		UTIL_JSON->setImageConfigValue(rectList[i], "宽", QString::number(end[i].x() - start[i].x()));
		UTIL_JSON->setImageConfigValue(rectList[i], "高", QString::number(end[i].y() - start[i].y()));
	}

	utility::QMessageBoxEx::information(this, "提示", QString("保存%1").arg(UTIL_JSON->initialize(true) ?
		"成功" : "失败," + UTIL_JSON->getLastError()));
}

void SettingDlg::connectCaptureSlot()
{
	Dt::Function* function = nullptr;
	if (!getFunctionPointer(reinterpret_cast<void**>(&function)))
	{
		return;
	}

	CapBase* capBase = function->getCaptureCard();
	int channelId = function->getCaptureCardConfig().channelId;
	m_captureStatus ? capBase->close() : capBase->open(channelId);
	m_captureStatus ? capBase->stopCapture() : capBase->startCapture();
	ui.connectCapture->setText(m_captureStatus ? "连接设备" : "断开设备");
	m_captureStatus = !m_captureStatus;
}

void Dt::SettingDlg::showRectBoxSlot(int state)
{
	GET_BASE()->m_showAllRect = state;
}

void SettingDlg::setLastError(const QString& error)
{
	DEBUG_INFO() << error;
	m_lastError = error;
}

bool SettingDlg::initConfigTreeWidget()
{
	bool result = false, success = true;
	do
	{
		ui.configAdd->setEnabled(false);
		ui.configDel->setEnabled(false);

		connect(ui.configExpand, &QPushButton::clicked, this, &SettingDlg::configExpandSlot);
		connect(ui.configAdd, &QPushButton::clicked, this, &SettingDlg::configAddNodeSlot);
		connect(ui.configDel, &QPushButton::clicked, this, &SettingDlg::configDelNodeSlot);
		connect(ui.configSave, &QPushButton::clicked, this, &SettingDlg::configSaveDataSlot);
		connect(ui.configExit, &QPushButton::clicked, this, &SettingDlg::configExitDlgSlot);

		ui.configTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
		ui.configTree->setHeaderLabels(QStringList{ "键","值" ,"说明" });
		ui.configTree->setColumnCount(3);

		connect(ui.configTree, &QTreeWidget::itemPressed, this, &SettingDlg::configTreeItemPressedSlot);
		connect(ui.configTree, &QTreeWidget::itemDoubleClicked, this, &SettingDlg::configTreeItemDoubleClickedSlot);
		connect(ui.configTree, &QTreeWidget::itemChanged, this, &SettingDlg::configTreeItemChangedSlot);

		QList<QTreeWidgetItem**> treeRootList;

		QTreeWidgetItem* parentDeviceConfig(nullptr),
			* parentHardwareConfig(nullptr),
			* parentRelayConfig(nullptr),
			* parentRangeConfig(nullptr),
			* parentThresholdConfig(nullptr),
			* parentImageConfig(nullptr),
			* parentImageDynamicConfig(nullptr),
			* parentPowerCurrentConfig(nullptr),
			* parentStaticCurrentConfig(nullptr),
			* parentKeyVoltageConfig(nullptr),
			* parentComponetVoltageConfig(nullptr),
			* parentComponetResistorConfig(nullptr),
			* parentVersionConfig(nullptr),
			* parentDtcConfig(nullptr),
			* parentEnableConfig(nullptr),
			* parentLogicConfig(nullptr),
			* parentUserConfig(nullptr),
			* parentOther1Config(nullptr),
			* parentOther2Config(nullptr);

		treeRootList.append(&parentDeviceConfig);
		treeRootList.append(&parentHardwareConfig);
		treeRootList.append(&parentRelayConfig);
		treeRootList.append(&parentRangeConfig);
		treeRootList.append(&parentThresholdConfig);
		treeRootList.append(&parentImageConfig);
		treeRootList.append(&parentImageDynamicConfig);
		treeRootList.append(&parentPowerCurrentConfig);
		treeRootList.append(&parentStaticCurrentConfig);
		treeRootList.append(&parentKeyVoltageConfig);
		treeRootList.append(&parentComponetVoltageConfig);
		treeRootList.append(&parentComponetResistorConfig);
		treeRootList.append(&parentVersionConfig);
		treeRootList.append(&parentDtcConfig);
		treeRootList.append(&parentEnableConfig);
		treeRootList.append(&parentLogicConfig);
		treeRootList.append(&parentUserConfig);
		treeRootList.append(&parentOther1Config);
		treeRootList.append(&parentOther2Config);

		QList<QTreeWidgetItem*> rootList;
		for (int i = 0; i < treeRootList.size(); i++)
		{
			*treeRootList[i] = new(std::nothrow) QTreeWidgetItem({ UTIL_JSON->getAllMainKey()[i] });
			if (!*treeRootList[i])
			{
				success = false;
				setLastError(QString("%1,分配内存失败").arg(UTIL_JSON->getAllMainKey()[i]));
				break;
			}
			(*treeRootList[i])->setIcon(0, QIcon(":/images/Resources/images/store.ico"));
			rootList.append(*treeRootList[i]);
		}

		if (!success)
		{
			break;
		}

		/*父级目录*/
		ui.configTree->addTopLevelItems(rootList);

		/************************************************************************/
		/* Function1                                                            */
		/************************************************************************/
		/*返回临时变量*/
		int(Json:: * getCount1Fnc[])() const = {
			&Json::getDeviceConfigCount,
			&Json::getHardwareConfigCount,
			&Json::getRelayConfigCount,
			&Json::getRangeConfigCount,
			&Json::getImageDynamicConfigCount,
			&Json::getThresholdConfigCount,
			&Json::getStaticCurrentConfigCount,
			&Json::getKeyVoltageConfigCount,
			&Json::getEnableConfigCount,
			&Json::getLogicConfigCount,
			&Json::getUserConfigCount,
			&Json::getOtherConfig1Count
		};

		QStringList(Json:: * getKey1Fnc[])() const = {
			&Json::getDeviceConfigKeyList,
			&Json::getHardwareConfigKeyList,
			&Json::getRelayConfigKeyList,
			&Json::getRangeConfigKeyList,
			&Json::getImageDynamicConfigKeyList,
			&Json::getThresholdConfigKeyList,
			&Json::getStaticCurrentConfigKeyList,
			&Json::getKeyVoltageConfigKeyList,
			&Json::getEnableConfigKeyList,
			&Json::getLogicConfigKeyList,
			&Json::getUserConfigKeyList,
			&Json::getOtherConfig1KeyList
		};

		/*返回临时变量*/
		QString(Json:: * getValue1Fnc[])(const QString & key)const = {
			&Json::getDeviceConfigValue,
			&Json::getHardwareConfigValue,
			&Json::getRelayConfigValue,
			&Json::getRangeConfigValue,
			&Json::getImageDynamicConfigValue,
			&Json::getThresholdConfigValue,
			&Json::getStaticCurrentConfigValue,
			&Json::getKeyVoltageConfigValue,
			&Json::getEnableConfigValue,
			&Json::getLogicConfigValue,
			&Json::getUserConfigValue,
			&Json::getOtherConfig1Value
		};

		QStringList(Json:: * getExplain1Fnc[])() const = {
			&Json::getDeviceConfigExplain,
			&Json::getHardwareConfigExplain,
			&Json::getRelayConfigExplain,
			&Json::getRangeConfigExplain,
			&Json::getImageDynamicConfigExplain,
			&Json::getThresholdConfigExplain,
			&Json::getStaticCurrentConfigExplain,
			&Json::getKeyVoltageConfigExplain,
			&Json::getEnableConfigExplain,
			&Json::getLogicConfigExplain,
			&Json::getUserConfigExplain,
			&Json::getOtherConfig1Explain
		};

		QList<QTreeWidgetItem*> getParent1List = {
			parentDeviceConfig,
			parentHardwareConfig,
			parentRelayConfig,
			parentRangeConfig,
			parentImageDynamicConfig,
			parentThresholdConfig,
			parentStaticCurrentConfig,
			parentKeyVoltageConfig,
			parentEnableConfig,
			parentLogicConfig,
			parentUserConfig,
			parentOther1Config
		};

		for (int i = 0; i < getParent1List.size(); i++)
		{
			QList<QTreeWidgetItem*> childList;
			for (int j = 0; j < (UTIL_JSON->*getCount1Fnc[i])(); j++)
			{
				childList.append(new QTreeWidgetItem({ (UTIL_JSON->*getKey1Fnc[i])().value(j) }));
				childList[j]->setIcon(0, QIcon(":/images/Resources/images/key.ico"));
				childList[j]->setIcon(1, QIcon(":/images/Resources/images/file.ico"));
				childList[j]->setIcon(2, QIcon(":/images/Resources/images/msg.ico"));
				childList[j]->setText(1, (UTIL_JSON->*getValue1Fnc[i])((UTIL_JSON->*getKey1Fnc[i])().value(j)));
				childList[j]->setText(2, (UTIL_JSON->*getExplain1Fnc[i])().value(j));
				if ((UTIL_JSON->*getKey1Fnc[i])().value(j) == "CAN卡类型") {
					QStringList canNameList = { "支持以下CAN卡类型" };
					//canNameList.append("0:NULL-CAN");
					//canNameList.append("1:ZLG-USBCAN");
					//canNameList.append("2:ZLG-USBCANFD");
					//canNameList.append("3:ZLG-NETCANFD");
					//canNameList.append("4:GC-USBCANFD");
					auto types = can::getSupportDeviceNames();
					auto index = 0;
					for (auto& x : types) {
						canNameList.append(Q_SPRINTF("%d:%s", index++, x.c_str()));
					}
					childList[j]->setToolTip(2, canNameList.join('\n'));
				}
				else if ((UTIL_JSON->*getKey1Fnc[i])().value(j) == "采集卡通道数") {
					childList[j]->setToolTip(2, "如果采集卡名称为ANY,此处为2,将不使用采集卡");
				}
			}
			getParent1List[i]->addChildren(childList);
		}

		if (!UTIL_JSON->getOtherConfig1Count()) {
			int index = ui.configTree->indexOfTopLevelItem(parentOther1Config);
			ui.configTree->takeTopLevelItem(index);
		}

		/************************************************************************/
		/* Function2                                                            */
		/************************************************************************/
		QList<QTreeWidgetItem*>getParent2List = {
			parentImageConfig,
			parentPowerCurrentConfig,
			parentComponetVoltageConfig,
			parentComponetResistorConfig,
			parentVersionConfig,
			parentDtcConfig,
			parentOther2Config
		};

		int (Json:: * getCount2Fnc[])()const = {
			&Json::getImageConfigCount,
			&Json::getWorkingCurrentConfigCount,
			&Json::getComponentVoltageConfigCount,
			&Json::getComponentResistorConfigCount,
			&Json::getVersionConfigCount,
			&Json::getDtcConfigCount,
			&Json::getOtherConfig2Count
		};

		/*返回临时遍历主键*/
		QStringList(Json:: * getParentKey2Fnc[])()const = {
			&Json::getParentImageConfigKeyList,
			&Json::getParentWorkingCurrentConfigKeyList,
			&Json::getParentComponentVoltageConfigKeyList,
			&Json::getParentComponentResistorConfigKeyList,
			&Json::getParentVersionConfigKeyList,
			&Json::getParentDtcConfigKeyList,
			&Json::getParentOtherConfig2KeyList
		};

		QStringList(Json:: * getChildKey2Fnc[])() const = {
			&Json::getChildImageConfigKeyList,
			&Json::getChildWorkingCurrentConfigKeyList,
			&Json::getChildComponentVoltageConfigKeyList,
			&Json::getChildComponentResistorConfigKeyList,
			&Json::getChildVersionConfigKeyList,
			&Json::getChildDtcConfigKeyList,
			&Json::getChildOtherConfig2KeyList
		};

		/*返回临时变量*/
		QString(Json:: * getValue2Fnc[])(const QString&, const QString&)const = {
			&Json::getImageConfigValue,
			&Json::getWorkingCurrentConfigValue,
			&Json::getComponentVoltageConfigValue,
			&Json::getComponentResistorConfigValue,
			&Json::getVersionConfigValue,
			&Json::getDtcConfigValue,
			&Json::getOtherConfig2Value
		};

		QStringList(Json:: * getExplain2Fnc[])() const = {
			&Json::getImageConfigExplain,
			&Json::getWorkingCurrentConfigExplain,
			&Json::getComponentVoltageConfigExplain,
			&Json::getComponentResistorConfigExplain,
			&Json::getVersionConfigExplain,
			&Json::getDtcConfigExplain,
			&Json::getOtherConfig2Explain
		};

		for (int i = 0; i < getParent2List.size(); i++)
		{
			QList<QTreeWidgetItem*> parentList;
			for (int j = 0; j < (UTIL_JSON->*getCount2Fnc[i])(); j++)
			{
				UTIL_JSON->setChildImageConfigKeyListIndex(j);
				UTIL_JSON->setChildOtherConfig2KeyListIndex(j);
				parentList.append(new QTreeWidgetItem({ (UTIL_JSON->*getParentKey2Fnc[i])()[j] }));
				QList<QTreeWidgetItem*> childList;
				for (int k = 0; k < (UTIL_JSON->*getChildKey2Fnc[i])().count(); k++)
				{
					childList.append(new QTreeWidgetItem({ (UTIL_JSON->*getChildKey2Fnc[i])()[k] }));
					childList.at(k)->setText(1, (UTIL_JSON->*getValue2Fnc[i])(
						(UTIL_JSON->*getParentKey2Fnc[i])()[j],
						(UTIL_JSON->*getChildKey2Fnc[i])()[k]));
					childList.at(k)->setText(2, (UTIL_JSON->*getExplain2Fnc[i])()[k]);
					childList.at(k)->setIcon(0, QIcon(":/images/Resources/images/key.ico"));
					childList.at(k)->setIcon(1, QIcon(":/images/Resources/images/file.ico"));
					childList.at(k)->setIcon(2, QIcon(":/images/Resources/images/msg.ico"));
				}
				parentList.at(j)->addChildren(childList);
				parentList.at(j)->setIcon(0, QIcon(":/images/Resources/images/tree.ico"));
			}
			getParent2List[i]->addChildren(parentList);
		}

		if (!UTIL_JSON->getOtherConfig2Count())
		{
			int index = ui.configTree->indexOfTopLevelItem(parentOther2Config);
			ui.configTree->takeTopLevelItem(index);
		}
		result = true;
	} while (false);
	return result;
}

bool SettingDlg::initHardwareWidget()
{
	bool result = false;
	do
	{
		for (int i = 0; i < BUTTON_COUNT; i++)
		{
			m_buttonList.append(false);
		}

		//组合框配置
		auto portList = comm::Serial::getPortList();
		for (int i = 0; i < portList.size(); i++) {
			ui.powerCombo->addItem(QIcon(":/images/Resources/images/firefox.ico"), portList.at(i).name.c_str());
			ui.relayCombo->addItem(QIcon(":/images/Resources/images/firefox.ico"), portList.at(i).name.c_str());
			ui.voltageCombo->addItem(QIcon(":/images/Resources/images/firefox.ico"), portList.at(i).name.c_str());
			ui.currentCombo->addItem(QIcon(":/images/Resources/images/firefox.ico"), portList.at(i).name.c_str());
		}

		//设备串口配置
		auto&& hardware = UTIL_JSON->getParsedDefConfig()->hardware;
		ui.powerCombo->setCurrentText(Q_SPRINTF("COM%d", hardware.powerPort));
		ui.relayCombo->setCurrentText(Q_SPRINTF("COM%d", hardware.relayPort));
		ui.voltageCombo->setCurrentText(Q_SPRINTF("COM%d", hardware.voltmeterPort));
		ui.currentCombo->setCurrentText(Q_SPRINTF("COM%d", hardware.amperemeterPort));

		//电源配置
		ui.powerVoltage->setText(N_TO_Q_STR(hardware.powerVoltage));
		ui.powerMaxCurrent->setText(N_TO_Q_STR(hardware.powerCurrent));
		ui.powerCurrentValue->setText("0");
		connect(ui.powerConnect, &QPushButton::clicked, this, &SettingDlg::powerConnectSlot);
		connect(ui.powerOn, &QPushButton::clicked, this, &SettingDlg::powerControlSlot);
		connect(ui.powerGetCurrent, &QPushButton::clicked, this, &SettingDlg::powerGetCurrentSlot);

		//继电器配置
		connect(ui.relayConnect, &QPushButton::clicked, this, &SettingDlg::relayConnectSlot);
		m_relayBoxList.append(ui.relayIo0);
		m_relayBoxList.append(ui.relayIo1);
		m_relayBoxList.append(ui.relayIo2);
		m_relayBoxList.append(ui.relayIo3);
		m_relayBoxList.append(ui.relayIo4);
		m_relayBoxList.append(ui.relayIo5);
		m_relayBoxList.append(ui.relayIo6);
		m_relayBoxList.append(ui.relayIo7);
		m_relayBoxList.append(ui.relayIo8);
		m_relayBoxList.append(ui.relayIo9);
		m_relayBoxList.append(ui.relayIo10);
		m_relayBoxList.append(ui.relayIo11);
		m_relayBoxList.append(ui.relayIo12);
		m_relayBoxList.append(ui.relayIo13);
		m_relayBoxList.append(ui.relayIo14);
		m_relayBoxList.append(ui.relayIo15);
		for (int i = 0; i < m_relayBoxList.size(); i++)
		{
			connect(m_relayBoxList[i], &QCheckBox::clicked, this, &SettingDlg::relayControlSlot);
			for (int j = 0; j < UTIL_JSON->getRelayConfigCount(); j++)
			{
				int value = UTIL_JSON->getRelayConfigValue(UTIL_JSON->getRelayConfigKeyList()[j]).toInt();
				if (i == value)
				{
					m_relayBoxList[i]->setText(QString("%1[%2]").arg(i).arg(UTIL_JSON->getRelayConfigKeyList()[j]));
					break;
				}
			}
		}

		//电流表
		ui.currentValue->setText("0.0");
		connect(ui.currentConnect, &QPushButton::clicked, this, &SettingDlg::currentConnectSlot);
		connect(ui.currentGetValue, &QPushButton::clicked, this, &SettingDlg::currentGetValueSlot);

		//电压表
		ui.voltageValue->setText("0.0");
		connect(ui.voltageConnect, &QPushButton::clicked, this, &SettingDlg::voltageConnectSlot);
		connect(ui.voltageGetValue, &QPushButton::clicked, this, &SettingDlg::voltageGetValueSlot);

		if (*m_isConnect) {
			ui.powerCombo->setEnabled(false);
			ui.powerConnect->setEnabled(false);

			ui.relayCombo->setEnabled(false);
			ui.relayConnect->setEnabled(false);

			ui.voltageCombo->setEnabled(false);
			ui.voltageConnect->setEnabled(false);

			ui.currentCombo->setEnabled(false);
			ui.currentConnect->setEnabled(false);
		}
		else {
			if (GET_BASE()->m_power->isOpen()) {
				ui.powerConnect->setText("断开");
				ui.powerConnect->setEnabled(false);
				m_buttonList[DB_POWER_CONN] = true;
			}

			if (GET_BASE()->m_relay->isOpen()) {
				ui.relayConnect->setText("断开");
				ui.relayCombo->setEnabled(false);
				m_buttonList[DB_RELAY_CONN] = true;
			}

			if (GET_BASE()->m_voltmeter->isOpen()) {
				ui.voltageConnect->setText("断开");
				ui.voltageCombo->setEnabled(false);
				m_buttonList[DB_VOLTA_CONN] = true;
			}

			if (GET_BASE()->m_amperemeter->isOpen()) {
				ui.currentConnect->setText("断开");
				ui.currentCombo->setEnabled(false);
				m_buttonList[DB_CURRE_CONN] = true;
			}
		}
		result = true;
	} while (false);
	return result;
}

bool SettingDlg::initCanWidget()
{
	REGISTER_META_TYPE(can::Msg);

	auto transmit = GET_BASE()->m_can;
	if (!transmit) {
		ui.canBaseSend->setEnabled(false);
		ui.canBaseSend2->setEnabled(false);
		ui.canBaseStop->setEnabled(false);
		ui.canStartup->setEnabled(false);
	}
	else {
		transmit->setMsgProc([&](const char* type, const can::Msg& msg)->void
			{ emit addCanTableItemSignal(type, msg); });
	}
	ui.canConnect->setEnabled(transmit ? !transmit->isOpen() : false);

	connect(ui.canStartup, &QPushButton::clicked, this, &SettingDlg::canStartupSlot);
	connect(ui.canConnect, &QPushButton::clicked, this, &SettingDlg::canConnectSlot);

	/*CAN表格初始化*/
	ui.canTable0->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.canTable0->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
	ui.canTable0->setColumnWidth(7, 155);
	ui.canTable0->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.canTable0->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.canTable0->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.canTable0->verticalHeader()->setHidden(true);

	ui.canTable1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.canTable1->verticalHeader()->setHidden(true);
	ui.canTable1->setRowCount(100);
	ui.canTable1->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui.canTable1->setColumnWidth(0, 50);
	ui.canTable1->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
	ui.canTable1->setColumnWidth(4, 160);

	for (int i = 0; i < ui.canTable1->rowCount(); ++i)
	{
		QTableWidgetItem* check = new QTableWidgetItem(N_TO_Q_STR(i));
		check->setCheckState(Qt::Unchecked);
		check->setFlags(check->flags() & (~Qt::ItemIsEditable));
		ui.canTable1->setItem(i, 0, check);

		ui.canTable1->setItem(i, 1, new QTableWidgetItem("标准帧"));
		ui.canTable1->setItemDelegateForColumn(1, new ItemDelegate(this));

		ui.canTable1->setItem(i, 2, new QTableWidgetItem("数据帧"));
		ui.canTable1->setItemDelegateForColumn(2, new ItemDelegate(this));

		ui.canTable1->setItem(i, 3, new QTableWidgetItem("000"));
		ui.canTable1->setItem(i, 4, new QTableWidgetItem("00 00 00 00 00 00 00 00"));
	}

	connect(ui.canBaseSend, &QPushButton::clicked, this, &SettingDlg::canBaseSendSlot);

	connect(ui.canBaseSend2, &QPushButton::clicked, this, &SettingDlg::canBaseSendSlot2);

	connect(ui.canBaseStop, &QPushButton::clicked, this, &SettingDlg::canBaseStopSlot);

	connect(ui.canMatrixType, &QComboBox::currentTextChanged, this, &SettingDlg::canMatrixTypeSlot);

	connect(&m_canBaseSendTimer, &QTimer::timeout, this, &SettingDlg::canBaseSendTimerSlot);

	connect(&m_canBaseSendTimer2, &QTimer::timeout, this, &SettingDlg::canBaseSendTimer2Slot);

	connect(ui.canClearLog, &QPushButton::clicked, this, &SettingDlg::canClearLogSlot);

	connect(ui.canFilterEnable, &QCheckBox::clicked, this, &SettingDlg::canFilterEnableSlot);

	connect(ui.canAdvanceSend, &QPushButton::clicked, this, &SettingDlg::canAdvanceSendSlot);

	connect(ui.canAdvanceStop, &QPushButton::clicked, this, &SettingDlg::canAdvanceStopSlot);

	connect(this, &SettingDlg::enableCanAdvanceSendButtonSignal, this,
		[&] {ui.canAdvanceSend->setEnabled(true); });
	return true;
}

bool SettingDlg::initPaintWidget()
{
	Dt::Function* function = dynamic_cast<Dt::Function*>(static_cast<Dt::Base*>(m_base));
	CapBase* cap = function ? function->getCaptureCard() : nullptr;
	ui.connectCapture->setEnabled(cap ? !cap->isOpen() : false);

	connect(ui.startCapture, &QPushButton::clicked, this, &SettingDlg::startCaptureSlot);
	connect(ui.stopCapture, &QPushButton::clicked, this, &SettingDlg::stopCaptureSlot);
	connect(ui.saveCoord, &QPushButton::clicked, this, &SettingDlg::saveCoordSlot);
	connect(ui.saveImage, &QPushButton::clicked, this, &SettingDlg::saveImageSlot);
	connect(ui.connectCapture, &QPushButton::clicked, this, &SettingDlg::connectCaptureSlot);
	connect(ui.showRectBox, &QCheckBox::stateChanged, this, &SettingDlg::showRectBoxSlot);

	ui.label->setShowCoordSizeLabel(ui.coordSizeLabel);
	return true;
}

bool SettingDlg::initAboutWidget()
{
	ui.frameVersion->setText(Json::getLibVersion());
	ui.appVersion->setText(utility::getVersion());
	ui.matrixVersion->setText("1.0.0.3");
	return true;
}

int SettingDlg::getComNumber(const QString& comName) const
{
	bool convert = false;
	int number = comName.mid(3).toInt(&convert);
	return convert ? number : -1;
}

void SettingDlg::safeCloseDevice()
{
	auto power = GET_BASE()->m_power;
	if (power) {
		if (!*m_isConnect && power->isOpen()) {
			power->close();
		}
	}

	auto relay = GET_BASE()->m_relay;
	if (relay) {
		if (!*m_isConnect && relay->isOpen()) {
			relay->close();
		}
	}

	auto voltage = GET_BASE()->m_voltmeter;
	if (voltage) {
		if (!*m_isConnect && voltage->isOpen()) {
			voltage->close();
		}
	}

	auto current = GET_BASE()->m_amperemeter;
	if (current) {
		if (!*m_isConnect && current->isOpen()) {
			current->close();
		}
	}

	auto transmit = GET_BASE()->m_can;
	if (transmit) {
		transmit->setMsgProc();
		if (!*m_isConnect && transmit->isOpen()) {
			transmit->close();
		}
	}
}

bool SettingDlg::getFunctionPointer(void** pointer)
{
	*pointer = dynamic_cast<Dt::Function*>(static_cast<Dt::Base*>(m_base));
	if (!*pointer)
	{
		utility::QMessageBoxEx::information(this, "提示", "非功能检测基类及子类不支持视频出画");
		return false;
	}
	return true;
}

