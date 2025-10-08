#include "Detection.h"

BaseTypes::DetectionType Dt::Base::m_detectionType = BaseTypes::DT_AVM;

QString Dt::Base::m_className = QString();

Dt::Base::Base(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Base);
	REGISTER_META_TYPE(BaseTypes::TestResult);
	REGISTER_META_TYPE(BaseTypes::DownloadInfo*);
	REGISTER_META_TYPE(bool*);
}

Dt::Base::~Base()
{
	PRINT_CON_DESTRUCTION(Dt::Base);
	autoRecycle({ getLogPath() });
	exitConsoleWindow();
	threadQuit();
}

QString Dt::Base::getLastError() const
{
	return m_lastError;
}

void Dt::Base::setTestSequence(int testSequence)
{
	m_testSequence = testSequence;
}

void Dt::Base::setConnectStatus(bool status)
{
	m_connect = status;
	if (!status)
	{
		setScanDlgWindow(false);
	}
}

void Dt::Base::setDetectionType(BaseTypes::DetectionType type)
{
	m_detectionType = type;
}

BaseTypes::DetectionType Dt::Base::getDetectionType()
{
	return m_detectionType;
}

QString Dt::Base::getDetectionName()
{
	QString name;
	switch (m_detectionType)
	{
	case BaseTypes::DT_HARDWARE:
		name = "硬件";
		break;
	case BaseTypes::DT_FUNCTION:
	case BaseTypes::DT_AVM:
	case BaseTypes::DT_DVR:
	case BaseTypes::DT_OMS:
	case BaseTypes::DT_AICS:
		name = "功能";
		break;
	case BaseTypes::DT_TEMPERATURE:
		name = "高温";
		break;
	case BaseTypes::DT_MODULE:
		name = "模块";
		break;
	default:name = "未知"; break;
	}
	return name.append("检测");
}

void Dt::Base::setSocDelay(ulong delay)
{
	m_socDelay = delay;
}

bool Dt::Base::initialize()
{
	bool result = false;
	do
	{
		utility::initialize();

		utility::setDumpFileSavePath(getLogPath() + "\\Dump");

		utility::setDumpFileRemark(m_className);

		m_defConfig = UTIL_JSON->getParsedDefConfig();

		m_hwdConfig = UTIL_JSON->getParsedHwdConfig();

		m_udsConfig = UTIL_JSON->getParsedUdsConfig();

		utility::setDebug(&m_defConfig->enable.outputRunLog);

		m_power = power::autoReleaseNew(power::DeviceType::ITECH_6832A);
		m_relay = relay::autoReleaseNew(static_cast<relay::DeviceType>(m_defConfig->hardware.relayType));
		m_voltmeter = std::make_shared<Voltmeter>();
		m_amperemeter = std::make_shared<Amperemeter>();

		can::DeviceType canType = static_cast<can::DeviceType>(m_defConfig->device.canType.toInt());
		m_can = can::autoReleaseNew(canType);
		m_can->setMatrix(&m_matrix);

#ifdef QT_DEBUG
		setOutputCanLog(true);
#endif

		m_uds = uds::autoReleaseNew(m_udsVehicleType, m_can);

		if (!initConsoleWindow()) {
			setLastError(getLastError(), false, true);
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::initConsoleWindow()
{
	bool result = false;
	do
	{
		if (g_debug && !*g_debug)
		{
			result = true;
			break;
		}

		RUN_BREAK(!AllocConsole(), "分配控制台失败");
		SetConsoleTitleW(Q_TO_WC_STR(Q_SPRINTF("检测框架[%s]调试控制台", Json::getLibVersion())));
		HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
		if (output != INVALID_HANDLE_VALUE) {
			SetConsoleTextAttribute(output, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		}
		RUN_BREAK(!freopen("CONOUT$", "w", stderr), "重定向输出流stderr失败");

		DEBUG_INFO() << "初始化控制台成功";
		DEBUG_INFO() << "重定向stderr流成功";
		RUN_BREAK(!freopen("CONOUT$", "w", stdout), "重定向输出流stdout失败");

		DEBUG_INFO() << "重定向stdout流成功";
		DEBUG_INFO() << "调试机种:" << m_defConfig->device.typeName;
		DEBUG_INFO() << "采集卡:" << m_defConfig->device.cardName;
		DEBUG_INFO() << "采集卡通道:" << m_defConfig->device.cardChannelId << "[没有画面请仔细确认此处]";

		for (int i = 0; i < UTIL_JSON->getErrorList().size(); i++) {
			if (!i) {
				DEBUG_INFO() << "UTIL_JSON配置文件出现严重性错误:";
			}
			DEBUG_INFO() << UTIL_JSON->getErrorList()[i];
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::exitConsoleWindow() const
{
	return (g_debug && !*g_debug) ? true : FreeConsole() == TRUE;
}

bool Dt::Base::openDevice()
{
	bool result = false;
	do
	{
		setDeviceFaultCode(DFC_NO);

		can::Device device;
		device.deviceIndex = m_defConfig->device.canDeviceNum.toInt();
		memset(device.enableChannel, 0, sizeof(device.enableChannel));
		device.enableChannel[m_defConfig->device.canChannelNum.toInt()] = true;
		device.arbiBaud[m_defConfig->device.canChannelNum.toInt()] = m_defConfig->device.canArbiBaud.toInt();
		device.dataBaud[m_defConfig->device.canChannelNum.toInt()] = m_defConfig->device.canDataBaud.toInt();
		device.isExpandFrame = m_defConfig->device.canExpFrame.toInt();
		auto peerAddress = m_defConfig->device.canPeerAddress.toStdString();
		device.peerAddress = peerAddress.c_str();
		device.peerPort = m_defConfig->device.canPeerPort.toInt();

		if (!m_can->open(device)) {
			setDeviceFaultCode(DFC_CAN_CARD);
			setLastError(QString("连接CAN卡失败,%1").arg(LS_TO_Q_STR(m_can->getLastError())), false, m_faultHint);
			break;
		}

		auto& hardware = m_defConfig->hardware;
		if (!m_power->open(std::to_string(hardware.powerPort), hardware.powerBaud)) {
			setDeviceFaultCode(DFC_POWER);
			m_can->close();
			setLastError("打开电源失败", false, m_faultHint);
			break;
		}

		m_power->setVoltage(hardware.powerVoltage);
		m_power->setCurrent(hardware.powerCurrent);

		if (hardware.relayType == relay::CW_MR_DO16_KN || hardware.relayType == relay::CW_EMR_DO16) {
			if (!m_relay->open(std::to_string(hardware.relayPort), hardware.relayBaud)) {
				setDeviceFaultCode(DFC_REALY);
				m_can->close();
				m_power->close();
				setLastError("打开继电器失败", false, m_faultHint);
				break;
			}
		}
		else {
			in_addr addr = { 0 };
			addr.S_un.S_addr = hardware.relayPort;
			auto ipAddr = inet_ntoa(addr);
			if (!ipAddr) {
				setDeviceFaultCode(DFC_REALY);
				m_can->close();
				m_power->close();
				setLastError("打开继电器失败,IP地址无效", false, m_faultHint);
				break;
			}

			if (!m_relay->open(ipAddr, hardware.relayBaud)) {
				setDeviceFaultCode(DFC_REALY);
				m_can->close();
				m_power->close();
				setLastError("打开继电器失败", false, m_faultHint);
				break;
			}
		}

		if (!m_voltmeter->open(std::to_string(hardware.voltmeterPort), hardware.voltmeterBaud)) {
			setDeviceFaultCode(DFC_VOLTMETER);
			m_can->close();
			m_power->close();
			m_relay->close();
			setLastError("打开电压表失败", false, m_faultHint);
			break;
		}

		if (!m_amperemeter->open(std::to_string(hardware.amperemeterPort), hardware.amperemeterBaud)) {
			setDeviceFaultCode(DFC_AMPEREMETER);
			m_can->close();
			m_power->close();
			m_relay->close();
			m_voltmeter->close();
			setLastError("打开电流表失败", false, m_faultHint);
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool Dt::Base::closeDevice()
{
	bool result = false;
	do
	{
		if (!m_can->close()) {
			setLastError("CAN断开连接失败", false, true);
		}

		if (!m_power->output(false)) {
			setLastError("关闭电源失败", false, true);
		}

		if (!m_power->close()) {
			setLastError("关闭电源失败", false, true);
		}

		if (!m_relay->close()) {
			setLastError("关闭继电器失败", false, true);
		}

		if (!m_voltmeter->close()) {
			setLastError("关闭电压表失败", false, true);
		}

		if (!m_amperemeter->close()) {
			setLastError("关闭电流表失败", false, true);
		}

		result = true;
	} while (false);
	return result;
}

bool Dt::Base::prepareTest()
{
	setCurrentStatus("准备测试");
	setTestResult(BaseTypes::TestResult::TR_TS);
	bool result = false, success = true;
	do
	{
		startTest(true);

		m_lastError = "未知错误";

		m_logicIndex = 0;

		setSaveCanLog(m_defConfig->enable.saveCanLog);

		setCanLogName(m_defConfig->device.typeName, m_barcode);

		clearListItem();

		m_elapsedTime = GetTickCount64();

		addListItem(QString("扫描条码:%1").arg(m_barcode));

		addListItem(Q_SPRINTF("第%u块产品开始测试", m_total), false);

		initDetectionLog();

		addListItem("等待系统启动,请耐心等待...");

		RUN_BREAK(!m_power->output(true), "电源上电失败,请检测连接");

		msleep(300);

		clearCanMsgBuffer();

		//如果在调试模式下,则删除所有报文
		if (g_debug && *g_debug) {
			m_can->deleteAllMsgs();
		}

		for (auto& x : m_cycleEventMsg) {
			x.sendType = can::SendType::CE;
			sendCanMsg(x);
		}

		for (auto& x : m_cycleMsg) {
			x.sendType = can::SendType::CYCLE;
			sendCanMsg(x);
		}

		m_can->startAsyncSendMsg();

		if (m_detectionType == BaseTypes::DT_AVM ||
			m_detectionType == BaseTypes::DT_OMS ||
			m_detectionType == BaseTypes::DT_AICS)
		{
			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.ignitionLine, true), "继电器打开点火线端口失败");
			msleep(300);

			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.groundLine, true), "继电器打开接地线端口失败");
			msleep(300);

			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.pinboard, true), "继电器打开转接板失败");
			msleep(300);

			if (m_defConfig->enable.signalLight)
			{
				RUN_BREAK(!m_relay->setOne(m_defConfig->relay.whiteLamp, true), "继电器打开白色灯端口失败");
				msleep(300);

				RUN_BREAK(!m_relay->setOne(m_defConfig->relay.redLamp, false), "继电器关闭红色灯端口失败");
				msleep(300);

				RUN_BREAK(!m_relay->setOne(m_defConfig->relay.greenLamp, false), "继电器关闭绿色灯端口失败");
				msleep(300);
			}
		}

		if (m_detectionType != BaseTypes::DT_DVR)
		{
			if (!m_prepareTestNoWait)
			{
				const uint startTime = GetTickCount64();
				if (m_safeStart)
					success = autoProcessCanMsg(0, 0, nullptr, 5000);

				if (success)
					msleep(START_DELAY);
				setCurrentStatus(success ? "状态正常" : "状态异常", true);
				addListItem(Q_SPRINTF("系统启动%s用时 %.2f秒", success ? "成功" : "失败", static_cast<float>(GetTickCount64() - startTime) / 1000));
				addListItem(Q_SPRINTF("系统启动 %s", OK_NG(success)), false);
				RUN_BREAK(!success, "初始化系统异常,未收到CAN报文");
			}
		}
		else
		{
			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.ignitionLine, true), "继电器打开点火线端口失败");
			msleep(300);
		}

		RUN_BREAK(!setOtherAction(), "设置其他动作失败," + getLastError());

		result = true;
	} while (false);
	return result;
}

bool Dt::Base::prepareTest(int id, int value, CanProc proc, ulong timeout)
{
	setCurrentStatus("准备测试");
	setTestResult(BaseTypes::TestResult::TR_TS);
	bool result = false, success = true;
	do
	{
		startTest(true);

		m_lastError = "未知错误";

		m_logicIndex = 0;

		setSaveCanLog(m_defConfig->enable.saveCanLog);

		setCanLogName(m_defConfig->device.typeName, m_barcode);

		clearListItem();

		m_elapsedTime = GetTickCount64();

		addListItem(QString("扫描条码:%1").arg(m_barcode));
		addListItem(Q_SPRINTF("第%u块产品开始测试", m_total), false);

		initDetectionLog();

		addListItem("等待系统启动,请耐心等待...");

		RUN_BREAK(!m_power->output(true), "电源上电失败,请检查连接");

		msleep(300);

		clearCanMsgBuffer();

		//如果在调试模式下,则删除所有报文
		if (g_debug && *g_debug) {
			m_can->deleteAllMsgs();
		}

		for (auto& x : m_cycleEventMsg) {
			x.sendType = can::SendType::CE;
			sendCanMsg(x);
		}

		for (auto& x : m_cycleMsg) {
			x.sendType = can::SendType::CYCLE;
			sendCanMsg(x);
		}
		m_can->startAsyncSendMsg();

		if (m_detectionType == BaseTypes::DT_AVM ||
			m_detectionType == BaseTypes::DT_OMS ||
			m_detectionType == BaseTypes::DT_AICS)
		{
			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.ignitionLine, true), "继电器打开点火线端口失败");
			msleep(300);

			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.groundLine, true), "继电器打开接地线端口失败");
			msleep(300);

			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.pinboard, true), "继电器打开转接板端口失败");
			msleep(300);

			if (m_defConfig->enable.signalLight)
			{
				RUN_BREAK(!m_relay->setOne(m_defConfig->relay.whiteLamp, true), "继电器打开白色灯端口失败");
				msleep(300);

				RUN_BREAK(!m_relay->setOne(m_defConfig->relay.redLamp, false), "继电器关闭红色灯端口失败");
				msleep(300);

				RUN_BREAK(!m_relay->setOne(m_defConfig->relay.greenLamp, false), "继电器关闭绿色灯端口失败");
				msleep(300);
			}
		}

		if (m_detectionType != BaseTypes::DT_DVR)
		{
			if (!m_prepareTestNoWait)
			{
				const uint startTime = GetTickCount64();
				if (m_safeStart)
					success = autoProcessCanMsg(id, value, proc, timeout);
				setCurrentStatus(success ? "状态正常" : "状态异常", true);
				addListItem(Q_SPRINTF("系统启动%s用时 %.2f秒", success ? "成功" : "失败", static_cast<float>(GetTickCount64() - startTime) / 1000));
				addListItem(Q_SPRINTF("系统启动 %s", OK_NG(success)), false);
				RUN_BREAK(!success, "初始化系统异常," + getLastError());
			}
		}
		else
		{
			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.ignitionLine, true), "继电器打开点火线端口失败");
			msleep(300);
		}

		RUN_BREAK(!setOtherAction(), "设置其他动作失败," + getLastError());

		result = true;
	} while (false);
	return result;
}

bool Dt::Base::finishTest(bool success)
{
	bool result = false;
	do
	{
		startTest(false);

		if (success)
		{
			++m_total;
		}

		flushCanLogBuffer();

		addListItem(Q_SPRINTF("测试用时 %.2f秒", (float)(GetTickCount64() - m_elapsedTime) / 1000), false);

		setTestResult(success ? BaseTypes::TestResult::TR_OK : BaseTypes::TestResult::TR_NG);

		//如果在调试模式下,则不停止发送报文和删除报文.
		if (!(g_debug && *g_debug))
		{
			m_can->stopAsyncSendMsg();
			m_can->deleteAllMsgs();
		}

		RUN_BREAK(!m_power->output(false), "电源掉电失败,请检查连接");

		msleep(300);

		if (m_detectionType == BaseTypes::DT_AVM ||
			m_detectionType == BaseTypes::DT_OMS ||
			m_detectionType == BaseTypes::DT_AICS)
		{
			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.ignitionLine, false), "继电器关闭点火线端口失败");
			msleep(300);

			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.groundLine, false), "继电器关闭接地线端口失败");
			msleep(300);

			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.pinboard, false), "继电器关闭转接板端口失败");
			msleep(300);

			if (m_defConfig->enable.signalLight)
			{
				RUN_BREAK(!m_relay->setOne(m_defConfig->relay.whiteLamp, false), "继电器关闭白色端口灯失败");
				msleep(300);


				RUN_BREAK(!(success ? m_relay->setOne(m_defConfig->relay.greenLamp, true) :
					m_relay->simulateKeystroke(m_defConfig->relay.redLamp, 3000)),
					Q_SPRINTF("关闭%s信号灯失败", success ? "绿色" : "红色"));
			}
		}
		else if (m_detectionType == BaseTypes::DT_DVR)
		{
			RUN_BREAK(!m_relay->setOne(m_defConfig->relay.ignitionLine, false), "继电器关闭点火线端口失败");
			msleep(300);
		}
		else if (m_detectionType == BaseTypes::DT_HARDWARE)
		{
			RUN_BREAK(!m_relay->setAll(false), "继电器断开失败,请检查连接");
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::saveLog(bool success)
{
	bool result = false;
	do
	{
		m_success = success;

		m_logicIndex = 0;

		if (success ? true : setQuestionBox("提示", getLastError() + "\n\n检测NG是否要保存日志?"))
		{
			if (!writeLog(success))
			{
				break;
			}
		}

		if (!finishTest(success))
		{
			break;
		}

		if (!success && m_defConfig->enable.unlockDlg)
		{
			setUnlockDlg();
		}
		result = true;
	} while (false);
	return result;
}

void Dt::Base::setPrepareTestNoWait(bool on)
{
	m_prepareTestNoWait = on;
}

void Dt::Base::setSafeStart(bool on)
{
	m_safeStart = on;
}

bool Dt::Base::checkWorkingCurrent()
{
	setCurrentStatus("检测工作电流");
	bool result = false, success = true, deviceFail = false;
	do
	{
		WorkingCurrentConfig* info = m_hwdConfig->workingCurrent;
		for (int i = 0; i < UTIL_JSON->getWorkingCurrentConfigCount(); i++)
		{
			float voltage = 0.0f;
			RUN_BREAK(deviceFail = !m_power->getVoltage(voltage), "获取电源电压失败");

			if (fabs(voltage - info[i].voltage) > 0.1)
			{
				RUN_BREAK(deviceFail = !m_power->setVoltage(info[i].voltage), "设置电源电压失败");
				msleep(3000);
			}

			RUN_BREAK(deviceFail = !m_power->getCurrent(info[i].read), "获取电源电流失败");

			(info[i].read >= info[i].low) && (info[i].read <= info[i].high) ? info[i].result = true : info[i].result = success = false;

			addListItem(Q_SPRINTF("%s  %.3f  %s", info[i].name, info[i].read, OK_NG(info[i].result)));

			WRITE_LOG("%s,%.3f", info[i].name, info[i].read);
		}

		RUN_BREAK(deviceFail, getLastError());

		RUN_BREAK(!success, "检测工作电流失败");

		result = true;
	} while (false);
	m_power->setVoltage(m_defConfig->hardware.powerVoltage);
	addListItem(Q_SPRINTF("检测工作电流 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Base::checkStaticCurrent(bool setIgn, bool set16Vol)
{
	setCurrentStatus("检测静态电流");
	m_can->stopAsyncSendMsg();
	bool result = false, success = false;
	do
	{

		addListItem("检测静态电流需要一定时间大约30秒,请耐心等待...");
		float current = 0.0f;
		RUN_BREAK(!m_power->getCurrent(current), "获取工作电流失败");

		RUN_BREAK(current < 0.1, "电源电流小于0.1毫安,请检查电源是否上电或硬件接触是否正常");

		auto& relay = m_defConfig->relay;
		RUN_BREAK(!m_relay->setOne(relay.ignitionLine, false), "继电器关闭点火线端口失败");
		msleep(300);

		ulong startTime = GetTickCount64();
		while (true)
		{
			if (m_power->getCurrent(current))
			{
				if (current <= m_defConfig->threshold.sleepCurrent)
				{
					success = true;
					break;
				}
			}

			if (GetTickCount64() - startTime >= m_defConfig->threshold.sleepTimeout)
			{
				break;
			}
			msleep(300);
		}

		RUN_BREAK(!success, "系统休眠超时");
		msleep(500);

		RUN_BREAK(!m_relay->setOne(relay.amperemeter, true), "继电器静态电流表端口打开失败");
		msleep(300);

		RUN_BREAK(!m_relay->setOne(relay.groundLine, false), "继电器接电线端口关闭失败");
		msleep(300);

		StaticCurrentConfig& info = m_hwdConfig->staticCurrent;

		HardwareConfig& hardware = m_defConfig->hardware;

		startTime = GetTickCount64();
		success = false;
		QVector<float> currentV;
		int rigth = 0;
		while (true)
		{
			RUN_BREAK(!m_amperemeter->getCurrent(info.read), LC_TO_Q_STR(m_amperemeter->getLastError()));

			currentV.push_back(info.read);

			DEBUG_INFO_EX("电流 %.2f %.2f %d", info.read, abs(currentV.back() - currentV.at(currentV.size() - 2)), rigth);

			if (currentV.size() > 2 && abs(currentV.back() - currentV.at(currentV.size() - 2)) <= 2.000f)
			{
				rigth++;
				if (rigth >= 10)
				{
					success = true;
					break;
				}
			}

			RUN_BREAK(GetTickCount64() - startTime > 30000, "检测静态电流超时,电流未稳定");

			msleep(200);
		}

		if (!success)
		{
			break;
		}

		info.result = ((info.read >= info.low) && (info.read < info.high));

		addListItem(Q_SPRINTF("静态电流  %.3fuA  %s", info.read, OK_NG(info.result)));

		WRITE_LOG("静态电流,%.3fuA", info.read);

		RUN_BREAK(!info.result, "检测静态电流失败");

		if (set16Vol)
		{
			RUN_BREAK(!m_power->setVoltage(16.0f), "电源设置16V电压失败");
		}

		RUN_BREAK(!m_relay->setOne(relay.groundLine, true), "继电器接电线端口打开失败");
		msleep(300);

		RUN_BREAK(!m_relay->setOne(relay.amperemeter, false), "继电器静态电流表端口关闭失败");
		msleep(300);

		if (setIgn)
		{
			RUN_BREAK(!m_relay->setOne(relay.ignitionLine, true), "继电器点火线端口打开失败");
		}
		result = true;
	} while (false);
	m_can->startAsyncSendMsg();
	addListItem(Q_SPRINTF("检测静态电流 %s", OK_NG(result)), false);
	if (result && setIgn) waitStartup(START_DELAY);
	return result;
}

bool Dt::Base::checkComponentVoltage()
{
	setCurrentStatus("检测组件电压");
	bool result = false, success = true, deviceFail = false;
	do
	{
		ComponentVoltageConfig* info = m_hwdConfig->componentVoltage;
		for (int i = 0; i < UTIL_JSON->getComponentVoltageConfigCount(); i++)
		{
			RUN_BREAK(deviceFail = !m_relay->setOne(info[i].relay, true), "打开继电器失败");
			msleep(info[i].delay);

			RUN_BREAK(deviceFail = !m_voltmeter->getVoltage(info[i].read),
				"电压表读取失败," + LC_TO_Q_STR(m_voltmeter->getLastError()));

			(info[i].read >= info[i].low) && (info[i].read <= info[i].high) ? info[i].result = true : info[i].result = success = false;

			addListItem(Q_SPRINTF("%s  %.3f  %s", info[i].name, info[i].read, OK_NG(info[i].result)));

			WRITE_LOG("%s,%.3f", info[i].name, info[i].read);

			RUN_BREAK(deviceFail = !m_relay->setOne(info[i].relay, false), "关闭继电器失败");
			msleep(300);
		}

		RUN_BREAK(deviceFail, getLastError());

		RUN_BREAK(!success, "检测组件电压失败");

		result = true;
	} while (false);
	addListItem(Q_SPRINTF("检测组件电压 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Base::clearDtc()
{
	setCurrentStatus("清除DTC");
	bool result = false;
	do
	{
		RUN_BREAK(!m_uds->clearDiagnosticInformation(), "清除DTC失败," + getUdsLastError());
		result = true;
	} while (false);
	addListItemEx(Q_SPRINTF("清除DTC %s", OK_NG(result)));
	WRITE_LOG("清除DTC,%s", OK_NG(result));
	return result;
}

bool Dt::Base::checkVersion()
{
	setCurrentStatus("检测版本号");
tryAngin:
	bool result = false, success = false, convert = true;
	do
	{
		QList<int> modify;
		auto info = m_udsConfig->version;
		int total = 0, right = 0;
		for (int i = 0; i < UTIL_JSON->getVersionConfigCount(); i++) {
			if (!info[i].enable) {
				continue;
			}

			++total;
		reread:
			info[i].size = sizeof(info[i].read);
			if (!m_uds->readDataByIdentifier(info[i].did[0], info[i].did[1], (uchar*)info[i].read, reinterpret_cast<size_t*>(&info[i].size))) {
				strcpy_s(info[i].read, "读取失败");
				info[i].result = false;
				addListItem(Q_SPRINTF("0x%02X%02X  %s  %s  %s",
					info[i].did[0], info[i].did[1],
					info[i].name, info[i].read, OK_NG(info[i].result)));

				if (m_defConfig->enable.rereadVersion &&
					++info[i].reread <= int(m_defConfig->threshold.rereadVersionTimes)) {
					addListItem(Q_SPRINTF("重新读取[%s],第%d次", info[i].name, info[i].reread));
					msleep(m_defConfig->threshold.readVersionDelay);
					goto reread;
				}
				info[i].error = true;
				strcpy_s(info[i].errstr, Q_TO_C_STR(getUdsLastError()));
				continue;
			}

			DEBUG_INFO_EX("0x%02X%02X  %d  %s", info[i].did[0], info[i].did[1], info[i].size, info[i].read);

			if (!UTIL_JSON->udsEncodeConvert(&info[i])) {
				setLastError(UTIL_JSON->getLastError());
				convert = false;
				continue;
			}

			if (memcmp(info[i].setup, info[i].read, strlen(info[i].setup))) {
				info[i].result = false;
				modify.push_back(i);
			}
			else {
				info[i].result = true;
				++right;
			}

			addListItem(Q_SPRINTF("0x%02X%02X  %s  %s  %s",
				info[i].did[0], info[i].did[1],
				info[i].name, info[i].read, OK_NG(info[i].result)));
			msleep(m_defConfig->threshold.readVersionDelay);
		}

		success = (total == right);

		/*如果出错,则进行自动修正*/
		if (convert && !success && UTIL_JSON->getUserPrivileges())
		{
			if (setQuestionBox("友情提示", "检测版本数据不匹配,\n是否自动修改为正确数据?") && setAuthDlg())
			{
				bool complete = true;
				for (int i = 0; i < modify.size(); i++)
				{
					if (!UTIL_JSON->setVersionConfigValue(info[modify[i]].name, "值", info[modify[i]].read))
					{
						complete = false;
					}
				}

				if (complete && UTIL_JSON->initialize(true))
				{
					setDetectionLog(BaseTypes::DL_VERSION);
					addListItem("已自动修正,重新检测版本号");
					goto tryAngin;
				}
				else
				{
					setMessageBox("错误", QString("自动修改为正确数据失败,\n%1,请手动修改").arg(UTIL_JSON->getLastError()));
				}
			}
		}

		QString errstr;
		if (!success)
		{
			for (int i = 0; i < UTIL_JSON->getVersionConfigCount(); ++i)
			{
				if (info[i].enable)
				{
					if (info[i].error)
						errstr.append(Q_SPRINTF("0x%02X%02X %s %s\n", info[i].did[0], info[i].did[1],
							info[i].name, info[i].errstr));
					else if (!info[i].result)
						errstr.append(Q_SPRINTF("0x%02X%02X %s 对比失败\n", info[i].did[0], info[i].did[1],
							info[i].name));
				}
			}

			int index = errstr.lastIndexOf("\n");
			if (index != -1)
			{
				errstr.remove(index, 1);
			}
		}

		/*写入最终日志*/
		setDetectionLog(BaseTypes::DL_VERSION, [&](int i)->void {
			WRITE_LOG("%s,%s", info[i].name, info[i].read);
			});

		RUN_BREAK(!convert, "检测版本号失败," + getLastError());
		RUN_BREAK(!success, "检测版本号失败,\n" + errstr);
		result = true;
	} while (false);
	addListItem(Q_SPRINTF("检测版本号 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Base::checkDtc()
{
	setCurrentStatus("检测DTC");
	bool again = false;
tryAgain:
	bool result = false, success = true;
	QList<int> modify;
	do
	{
		uchar dtcInfo[4096] = { 0 };
		size_t count = sizeof(dtcInfo);
		RUN_BREAK(!m_uds->readDtcInformation(02, 0xff, dtcInfo, &count), "读取DTC失败");

		auto config = m_udsConfig->dtc;
		for (int i = 0; i < count / 4; i++)
		{
			for (int j = 0; j < UTIL_JSON->getDtcConfigCount(); j++)
			{
				if (config[j].enable)
				{
					continue;
				}

				if ((dtcInfo[i * 4 + 0] == config[j].dtc[0]) &&
					(dtcInfo[i * 4 + 1] == config[j].dtc[1]) &&
					(dtcInfo[i * 4 + 2] == config[j].dtc[2]))
				{
					modify.push_back(j);
					config[j].dtc[3] = dtcInfo[i * 4 + 3];
					config[j].exist = true;
					success = false;
					addListItem(Q_SPRINTF("%s  存在  %d", config[j].name, config[j].dtc[3]));
				}
			}
		}

		/*如果存在DTC*/
		if (!success && !again)
		{
			RUN_BREAK(!m_uds->clearDiagnosticInformation(), "清除DTC失败");
			setDetectionLog(BaseTypes::DL_DTC);
			addListItem("清除DTC成功,重新检测DTC");
			again = true;
			goto tryAgain;
		}

		/*如果清除了DTC,第二次重新测试还没有通过,则进行自动修正*/
		if (!success && again)
		{
			/*进行提示该产品存在故障*/
			addListItem("请注意,该产品进行清除DTC,重新测试NG,可能存在故障");

			/*进行自动修正,需要认证*/
			if (UTIL_JSON->getUserPrivileges())
			{
				if (!setQuestionBox("友情提示", "检测DTC存在异常,\n是否自动忽略存在项目?"))
				{
					break;
				}

				bool complete = true;
				for (int i = 0; i < modify.size(); i++)
				{
					if (!UTIL_JSON->setDtcConfigValue(config[modify[i]].name, "忽略", "1"))
						complete = false;
				}

				if (complete && UTIL_JSON->initialize(true))
				{
					setDetectionLog(BaseTypes::DL_DTC);
					addListItem("已自动修正,重新检测DTC");
					goto tryAgain;
				}
				else
				{
					setMessageBox("错误", QString("自动修改为正确数据失败,\n%1,请手动修改").arg(UTIL_JSON->getLastError()));
				}
			}
		}
		RUN_BREAK(!success, "检测DTC失败,该产品可能存在故障,\n请联系管理员.");
		result = true;
	} while (false);

	/*写入最终DTC日志*/
	WRITE_LOG("检测DTC,%s", OK_NG(result));
	addListItem(Q_SPRINTF("检测DTC %s", OK_NG(result)), false);
	return result;
}

bool Dt::Base::writeSn(const std::function<bool()>& lambda)
{
	setCurrentStatus("写入序列号");
	bool result = false;
	do
	{
		if (!lambda())
			break;

		result = true;
	} while (false);
	WRITE_LOG("写入序列号,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("写入序列号 %s", OK_NG(result)));
	return result;
}

bool Dt::Base::checkSn(const std::function<bool()>& lambda)
{
	setCurrentStatus("检测序列号");
	bool result = false;
	do
	{
		if (!lambda())
			break;

		result = true;
	} while (false);
	WRITE_LOG("检测序列号,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测序列号 %s", OK_NG(result)));
	return result;
}

bool Dt::Base::writeDate(const std::function<bool()>& lambda)
{
	setCurrentStatus("写入日期");
	bool result = false;
	do
	{
		if (!lambda())
			break;

		result = true;
	} while (false);
	WRITE_LOG("写入日期,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("写入日期 %s", OK_NG(result)));
	return result;
}

bool Dt::Base::checkDate(const std::function<bool()>& lambda)
{
	setCurrentStatus("检测日期");
	bool result = false;
	do
	{
		if (!lambda())
			break;

		result = true;
	} while (false);
	WRITE_LOG("检测日期,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测日期 %s", OK_NG(result)));
	return result;
}

bool Dt::Base::writeSet(const std::function<bool()>& lambda)
{
	setCurrentStatus("写入设置");
	bool result = false;
	do
	{
		if (!lambda())
			break;

		result = true;
	} while (false);
	WRITE_LOG("写入设置,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("写入设置 %s", OK_NG(result)));
	return result;
}

bool Dt::Base::setFnc(const std::function<bool()>& lambda)
{
	return lambda();
}

void Dt::Base::setOutputCanLog(bool output)
{
	m_can->setOutputLog(output);
}

void Dt::Base::setSaveCanLog(bool save)
{
	//
}

void Dt::Base::setCanLogName(const QString& modelName, const QString& code)
{
}

void Dt::Base::flushCanLogBuffer()
{

}

void Dt::Base::clearCanMsgBuffer()
{
	m_can->clearBuffer();
}

bool Dt::Base::resetCan()
{
	return m_can->reset();
}

void Dt::Base::startCanMsg()
{
	m_can->startAsyncSendMsg();
}

void Dt::Base::stopCanMsg()
{
	m_can->stopAsyncSendMsg();
}

bool Dt::Base::sendCanMsg(int id, int period, int start, int length, quint64 data, can::SendType type, int count, int interval)
{
	bool result = false;
	do
	{
		can::Msg msg = { 0 };
		msg.id = id;
		msg.dlc = 8;
		msg.sendCycle = period;
		msg.sendType = type;
		msg.sendCount = count;
		msg.addInterval = interval;

		RUN_BREAK(!m_matrix.pack(msg.data, start, length, data), getCanMatrixError());

		m_can->addMsg(msg);
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::sendCanMsg(int id, int period, DataList data, can::SendType type, int count, int interval)
{
	bool result = false;
	do
	{
		can::Msg msg = { 0 };
		msg.id = id;
		msg.dlc = 8;
		msg.sendCycle = period;
		msg.sendType = type;
		msg.sendCount = count;
		msg.addInterval = interval;
		if (data.size()) {
			memcpy(msg.data, data.begin(), data.size());
		}
		m_can->addMsg(msg);
		result = true;
	} while (false);
	return result;
}

void Dt::Base::sendCanMsg(const can::Msg& msg)
{
	m_can->addMsg(msg);
}

void Dt::Base::sendCanMsg(int id, can::Base::SendProc proc, int period, can::SendType type, int count, int interval)
{
	m_can->addMsg(id, proc, period, type, count, interval);
}

void Dt::Base::sendCanMsg(CanList list)
{
	m_can->addMsg(list);
}

void Dt::Base::sendCanMsg(const can::Msg& msg, int start, int length, quint64 data)
{
	m_can->addMsg(msg, start, length, data);
}

int Dt::Base::receiveCanMsg(can::Msg* msg, int maxSize)
{
	return m_can->recvMsg(msg, maxSize);
}

void Dt::Base::deleteCanMsgs(CanList list)
{
	for (auto& x : list) {
		m_can->deleteMsg(x);
	}
}

void Dt::Base::deleteCanMsgs(IdList list)
{
	for (auto& x : list) {
		m_can->deleteMsg(x);
	}
}

void Dt::Base::deleteCanMsg(const can::Msg& msg)
{
	m_can->deleteMsg(msg);
}

void Dt::Base::deleteCanMsg(int id)
{
	m_can->deleteMsg(id);
}

void Dt::Base::deleteAllCanMsgs()
{
	m_can->deleteAllMsgs();
}

void Dt::Base::setCanMatrixType(can::Matrix::Type type)
{
	m_matrix.setType(type);
}

can::Matrix::Type Dt::Base::getCanMatrixType() const
{
	return m_matrix.getType();
}

QString Dt::Base::getCanMatrixError() const
{
	return LS_TO_Q_STR(m_matrix.getLastError());
}

quint64 Dt::Base::unpackCanMsg(const can::Msg& msg, int start, int length)
{
	quint64 data;
	m_matrix.unpack(msg.data, start, length, data);
	return data;
}

bool Dt::Base::autoProcessCanMsg(int id, quint64 value, CanProc proc, ulong timeout)
{
	bool result = false, success = false, deviceFail = false;
	do
	{
		std::unique_ptr<can::Msg[]> msg(new can::Msg[MSG_BUFFER_SIZE]);;
		const uint startTime = GetTickCount64();
		while (true)
		{
			memset(msg.get(), 0, MSG_BUFFER_SIZE * sizeof(can::Msg));
			const int size = receiveCanMsg(msg.get(), MSG_BUFFER_SIZE);

			//判断是否有CAN报文
			if (!id && !value && !proc && size)
			{
				success = true;
				break;
			}

			for (int i = 0; i < size; i++)
			{
				if (msg[i].id == id)
				{
					if (!proc)
					{
						msleep(m_socDelay);
						m_power->getCurrent(m_rouseCurrent);

						if (m_rouseCurrent < 0.1f)
						{
							deviceFail = true;
							break;
						}

						if (m_rouseCurrent >= m_defConfig->threshold.rouseCurrent)
						{
							success = true;
							break;
						}
					}
					else
					{
						if (proc(value, msg[i]))
						{
							success = true;
							break;
						}
					}
				}
			}

			RUN_BREAK(deviceFail, "电源未上电");

			if (success) break;

			RUN_BREAK(GetTickCount64() - startTime > timeout, "CAN报文处理失败");
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::autoProcessCanMsg(int id, ValList valList, CanProc proc, ulong timeout)
{
	bool result = false, success = false;
	do
	{
		std::unique_ptr<can::Msg[]> msg(new can::Msg[MSG_BUFFER_SIZE]);
		QList<int> cmpList;
		const uint startTime = GetTickCount64();
		while (true)
		{
			memset(msg.get(), 0, MSG_BUFFER_SIZE * sizeof(can::Msg));
			const int size = receiveCanMsg(msg.get(), MSG_BUFFER_SIZE);
			for (int i = 0; i < size; ++i)
			{
				if (msg[i].id == id)
				{
					for (int j = 0; j < valList.size(); ++j)
					{
						if (proc(valList.begin()[j], msg[i]))
						{
							if (cmpList.size())
							{
								for (const auto& cmp : cmpList)
									if (cmp != valList.begin()[j])
										cmpList.append(valList.begin()[j]);
							}
							else
							{
								cmpList.append(valList.begin()[j]);
							}
						}
					}
				}

				if (cmpList.size() == valList.size())
				{
					success = true;
					break;
				}
			}

			if (success) break;

			RUN_BREAK(GetTickCount64() - startTime > timeout, "CAN报文处理失败");
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::autoProcessCanMsg(int id, int value, int start, int length, ulong timeout)
{
	return autoProcessCanMsg(id, value, CAN_PROC_FNC(&){ return FVAL == unpackCanMsg(FMSG, start, length); }, timeout);
}

bool Dt::Base::autoProcessCanMsgEx(IdList idList, int value, CanProc proc, ulong timeout)
{
	bool result = false, success = false;
	do
	{
		std::unique_ptr<can::Msg[]> msg(new can::Msg[MSG_BUFFER_SIZE]);
		const uint startTime = GetTickCount64();
		while (true)
		{
			memset(msg.get(), 0, MSG_BUFFER_SIZE * sizeof(can::Msg));
			const int size = receiveCanMsg(msg.get(), MSG_BUFFER_SIZE);
			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < idList.size(); ++j)
				{
					if (msg[i].id == idList.begin()[j])
					{
						if (proc(value, msg[i]))
						{
							success = true;
							break;
						}
					}
				}

				if (success)
				{
					break;
				}
			}

			if (success) break;

			RUN_BREAK(GetTickCount64() - startTime > timeout, "CAN报文处理失败");
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::autoProcessCanMsgEx(IdList idList, ValList valList, CanProc proc, ulong timeout)
{
	bool result = false, success = false;
	do
	{
		RUN_BREAK(idList.size() != valList.size(), "ID列表与值列表大小不一致");

		std::unique_ptr<can::Msg[]> msg(new can::Msg[MSG_BUFFER_SIZE]);
		QList<int> cmpList;
		const uint startTime = GetTickCount64();
		while (true)
		{
			memset(msg.get(), 0, MSG_BUFFER_SIZE * sizeof(can::Msg));
			const int size = receiveCanMsg(msg.get(), MSG_BUFFER_SIZE);
			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < idList.size(); ++j)
				{
					if (msg[i].id == idList.begin()[j])
					{
						if (proc(valList.begin()[j], msg[i]))
						{
							if (cmpList.size())
							{
								for (const auto& cmp : cmpList)
									if (cmp != idList.begin()[j])
										cmpList.append(idList.begin()[j]);
							}
							else
							{
								cmpList.append(idList.begin()[j]);
							}
							break;
						}
					}
				}

				if (cmpList.size() == idList.size())
				{
					success = true;
					break;
				}
			}

			if (success) break;

			RUN_BREAK(GetTickCount64() - startTime > timeout, "CAN报文处理失败");
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::setCanProcessFnc(const char* name, const can::Msg& msg, const CanProcInfo& procInfo)
{
	return setCanProcessFncEx(name, { msg }, procInfo);
}

bool Dt::Base::setCanProcessFnc(const char* name, const can::Msg& msg, int id, int value, CanProc proc)
{
	return setCanProcessFnc(name, msg, { id, value, proc });
}

bool Dt::Base::setCanProcessFncEx(const char* name, CanList list, const CanProcInfo& procInfo)
{
	if (name)
		setCurrentStatus(name);

	bool result = false;
	do
	{
		if (name)
			addListItem(Q_SPRINTF("正在进行%s,请耐心等待...", name));

		for (const auto& x : list)
		{
			sendCanMsg(x);
			if (memcmp(&x, list.end(), sizeof(x)))
			{
				msleep(x.addInterval ? x.addInterval : 100);
			}
		}

		if (!autoProcessCanMsg(procInfo.id, procInfo.val, procInfo.proc))
		{
			break;
		}
		result = true;
	} while (false);

	for (const auto& x : list)
	{
		if (x.sendType == can::SendType::CYCLE)
		{
			deleteCanMsg(x);
		}
	}

	if (name)
	{
		WRITE_LOG("%s,%s", name, OK_NG(result));
		addListItemEx(Q_SPRINTF("%s %s", name, OK_NG(result)));
	}
	return result;
}

bool Dt::Base::setCanProcessFncEx(const char* name, CanList list, int id, int value, CanProc proc)
{
	return setCanProcessFncEx(name, list, { id,value,proc });
}

bool Dt::Base::setUdsProcessFnc(const char* name, DidList list, int value, int size, UdsProc proc, ulong timeout)
{
	if (name)
		setCurrentStatus(name);
	bool result = false, success = false;
	do
	{
		RUN_BREAK(list.size() != 2, "DID列表大小必须为2");

		const ulong startTime = GetTickCount64();
		int _size = 0;
		uchar _data[BUFF_SIZE] = { 0 };
		while (true)
		{
			memset(_data, 0, BUFF_SIZE);
			RUN_BREAK(!readDataByDid(list.begin()[0], list.begin()[1], _data, &_size), getUdsLastError());

			if (g_debug && *g_debug)
			{
				if (name)
				{
					DEBUG_INFO_EX("%s 读取大小 %d", name, size);
				}
				for (int i = 0; i < size; i++)
					printf("0x%02X ", _data[i]);
				printf("\n");
			}

			if (size)
			{
				RUN_BREAK(_size != size, "数据长度不匹配");
			}

			if (proc(value, _size, _data))
			{
				success = true;
				break;
			}

			RUN_BREAK(GetTickCount64() - startTime > timeout, "UDS处理数据超时,条件不满足");
			msleep(100);
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	if (name)
	{
		addListItemEx(Q_SPRINTF("%s %s", name, OK_NG(result)));
		WRITE_LOG("%s,%s", name, OK_NG(result));
	}
	return result;
}

bool Dt::Base::setUdsProcessFncEx(const char* name, DidList list, ValList val, int size, UdsProcEx procEx, ulong timeout)
{
	setCurrentStatus(name);
	bool result = false, success = false;
	do
	{
		RUN_BREAK(list.size() != 2, "DID列表大小必须为2");

		const ulong startTime = GetTickCount64();
		int _size = 0;
		uchar _data[BUFF_SIZE] = { 0 };
		while (true)
		{
			memset(_data, 0, BUFF_SIZE);
			RUN_BREAK(!readDataByDid(list.begin()[0], list.begin()[1], _data, &_size), getUdsLastError());

			if (g_debug && *g_debug)
			{
				DEBUG_INFO_EX("%s 读取大小 %d", name, size);
				for (int i = 0; i < size; i++)
					printf("0x%02X ", _data[i]);
				printf("\n");
			}

			if (size)
			{
				RUN_BREAK(_size != size, "数据长度不匹配");
			}

			if (procEx(val, _size, _data))
			{
				success = true;
				break;
			}

			RUN_BREAK(GetTickCount64() - startTime > timeout, "UDS处理数据超时,条件不满足");
			msleep(100);
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	addListItemEx(Q_SPRINTF("%s %s", name, OK_NG(result)));
	WRITE_LOG("%s,%s", name, OK_NG(result));
	return result;
}

void Dt::Base::autoRecycle(const QStringList& path, const QStringList& suffixName, int interval)
{
	do
	{
		if (!m_autoRecycle)
		{
			break;
		}

		auto&& currentDate = QDate::currentDate();
		for (int i = 0; i < path.size(); i++)
		{
			auto&& fileList = utility::getFileListBySuffixName(path[i],
				m_recycleSuffixName.isEmpty() ? suffixName : m_recycleSuffixName);
			for (auto& x : fileList)
			{
				QFileInfo fi(x);
				auto date = fi.created().date();
				int newYear = currentDate.year() - date.year();
				int current = (currentDate.month() + newYear * 12) - date.month();
				bool recycle = current >= ((m_recycleIntervalMonth == -1) ? interval : m_recycleIntervalMonth);
				DEBUG_INFO_EX("自动回收%s,%s间隔:%d个月,当前:%d个月,文件名:%s", SU_FA(recycle),
					(recycle || (interval == -1)) ? "" : "条件不满足,", interval, current, Q_TO_C_STR(x));
				if (recycle || (interval == -1))
				{
					QFile::remove(x);
				}
			}
		}
	} while (false);
	return;
}

void Dt::Base::setAutoRecycle(bool autoRecycle)
{
	m_autoRecycle = autoRecycle;
}

void Dt::Base::setRecycleSuffixName(const QStringList& suffixName)
{
	m_recycleSuffixName = suffixName;
}

void Dt::Base::setRecycleIntervalMonth(int interval)
{
	m_recycleIntervalMonth = interval;
}

void Dt::Base::setUdsAccessLevel(int level)
{
	m_udsAccessLevel = level;
}

void Dt::Base::setUdsSessionLevel(int level)
{
	m_udsSessionLevel = level;
}

void Dt::Base::restoreUdsAccessLevel()
{
	m_udsAccessLevel = uds::SecurityAccessType::DEFAULT_LEVEL1;
}

void Dt::Base::restoreUdsSessionLevel()
{
	m_udsSessionLevel = uds::DiagnosticSessionType::EXTEND;
}

bool Dt::Base::enterSecurityAccess(int session, int access, int repeat)
{
	bool result = false, success = false;
	do
	{
		for (int i = 0; i < repeat; i++) {
			if (!m_uds->diagnosticSessionControl(session)) {
				setLastError("诊断会话控制失败," + getUdsLastError());
				continue;
			}

			if (!m_uds->securityAccess(access)) {
				setLastError("安全访问失败," + getUdsLastError());
				continue;
			}
			m_lastError = "未知错误";
			success = true;
			break;
		}

		if (!success) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::readDataByDid(uchar did0, uchar did1, uchar* data, int* size)
{
	return m_uds->readDataByIdentifier(did0, did1, data, reinterpret_cast<size_t*>(size));
}

bool Dt::Base::writeDataByDid(uchar did0, uchar did1, const uchar* data, int size, int wait)
{
	bool result = false;
	do
	{
		if (!enterSecurityAccess(m_udsSessionLevel, m_udsAccessLevel)) {
			break;
		}

		if (!m_uds->writeDataByIdentifier(did0, did1, data, size)) {
			setLastError(getUdsLastError());
			break;
		}

		if (wait) {
			msleep(wait);
		}

		if (!confirmDataByDid(did0, did1, data, size)) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::writeDataByDid(uchar did0, uchar did1, const std::initializer_list<uchar>& data)
{
	return writeDataByDid(did0, did1, data.begin(), data.size());
}

bool Dt::Base::writeDataByDidEx(const uchar* routine, uchar did0, uchar did1, const uchar* data, int size)
{
	bool result = false;
	do
	{
		if (!enterSecurityAccess(m_udsSessionLevel, m_udsAccessLevel)) {
			break;
		}

		if (!m_uds->routineControl(routine[0], routine[1], routine[2], (uchar*)&routine[4], routine[3])) {
			setLastError(getUdsLastError());
			break;
		}

		if (!m_uds->writeDataByIdentifier(did0, did1, data, size)) {
			setLastError(getUdsLastError());
			break;
		}

		if (!confirmDataByDid(did0, did1, data, size)) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::writeDataByDidEx(const std::initializer_list<uchar>& routine, uchar did0, uchar did1, const uchar* data, int size)
{
	bool result = false;
	do
	{
		RUN_BREAK(routine.size() < 5, "例程控制必须>=5个字节");
		if (!writeDataByDidEx(routine.begin(), did0, did1, data, size)) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::confirmDataByDid(uchar did0, uchar did1, const uchar* data, int size)
{
	bool result = false, success = false;
	do
	{
		int recvSize = 0;
		uchar buffer[256] = { 0 };

		RUN_BREAK(!readDataByDid(did0, did1, buffer, &recvSize), getUdsLastError());

		RUN_BREAK(size != recvSize, "长度对比失败");

		success = !memcmp(data, buffer, size);

		QString src, dst;
		for (size_t i = 0; i < size; i++)
		{
			src.append(Q_SPRINTF("%02X", data[i]));
			dst.append(Q_SPRINTF("%02X", buffer[i]));
		}

		addListItem(QString("上位机写入数据[%1]").arg(src));
		addListItem(QString("控制器内部数据[%1]").arg(dst));
		addListItem(QString("序列号对比 %1").arg(OK_NG(success)));

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Base::safeWriteDataByDid(uchar did0, uchar did1, const uchar* data, int size)
{
	bool result = false, success = true;
	do
	{
		if (!writeDataByDid(did0, did1, data, size))
		{
			break;
		}

		addListItem("系统将重启进行校验");
		msleep(300);
		m_power->output(false);
		addListItem("正在等待控制器放电");
		msleep(2000);
		addListItem("控制器放电完成");
		m_power->output(true);
		addListItem(Q_SPRINTF("系统正在启动,请耐心等待,大约需要%ld秒", START_DELAY / 1000));
		msleep(START_DELAY);
		addListItem(Q_SPRINTF("正在进行重新校验 0x%02x%02x", did0, did1));
		success = confirmDataByDid(did0, did1, data, size);
		addListItem(Q_SPRINTF("重新校验 0x%02x%02x %s", did0, did1, OK_NG(success)));

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

QString Dt::Base::getUdsLastError() const
{
	return LS_TO_Q_STR(m_uds->getLastError());
}

void Dt::Base::initDetectionLog()
{
	m_logList.clear();

	for (uint i = 0; i < UTIL_JSON->getWorkingCurrentConfigCount(); i++)
	{
		m_hwdConfig->workingCurrent[i].read = 0.0f;
		m_hwdConfig->workingCurrent[i].result = false;
	}

	for (uint i = 0; i < UTIL_JSON->getComponentResistorConfigCount(); i++)
	{
		m_hwdConfig->componentResistor[i].read = 0.0f;
		m_hwdConfig->componentResistor[i].result = false;
	}

	for (uint i = 0; i < UTIL_JSON->getComponentVoltageConfigCount(); i++)
	{
		m_hwdConfig->componentVoltage[i].read = 0.0f;
		m_hwdConfig->componentVoltage[i].result = false;
	}

	for (uint i = 0; i < UTIL_JSON->getVersionConfigCount(); i++)
	{
		m_udsConfig->version[i].result = false;
		m_udsConfig->version[i].error = false;
		m_udsConfig->version[i].reread = 0;
		memset(m_udsConfig->version[i].read, 0x00, sizeof(m_udsConfig->version[i].read));
		memset(m_udsConfig->version[i].errstr, 0x00, sizeof(m_udsConfig->version[i].errstr));
	}

	for (uint i = 0; i < UTIL_JSON->getDtcConfigCount(); i++)
	{
		m_udsConfig->dtc[i].exist = false;
	}
}

void Dt::Base::setDetectionLog(BaseTypes::DetectionLog log, const std::function<void(int)>& fnc)
{
	if (log == BaseTypes::DL_ALL)
	{
		m_logList.clear();
	}

	if (log == BaseTypes::DL_POWER_CURRENT || log == BaseTypes::DL_ALL)
	{
		for (uint i = 0; i < UTIL_JSON->getWorkingCurrentConfigCount(); i++)
		{
			if (!fnc)
			{
				m_hwdConfig->workingCurrent[i].read = 0.0f;
				m_hwdConfig->workingCurrent[i].result = false;
			}
			else
			{
				fnc(i);
			}
		}
	}

	if (log == BaseTypes::DL_COMPONET_RESISTOR || log == BaseTypes::DL_ALL)
	{
		for (uint i = 0; i < UTIL_JSON->getComponentResistorConfigCount(); i++)
		{
			if (!fnc)
			{
				m_hwdConfig->componentResistor[i].read = 0.0f;
				m_hwdConfig->componentResistor[i].result = false;
			}
			else
			{
				fnc(i);
			}
		}
	}

	if (log == BaseTypes::DL_COMPONET_VOLTAGE || log == BaseTypes::DL_ALL)
	{
		for (uint i = 0; i < UTIL_JSON->getComponentVoltageConfigCount(); i++)
		{
			if (!fnc)
			{
				m_hwdConfig->componentVoltage[i].read = 0.0f;
				m_hwdConfig->componentVoltage[i].result = false;
			}
			else
			{
				fnc(i);
			}
		}
	}

	if (log == BaseTypes::DL_VERSION || log == BaseTypes::DL_ALL)
	{
		for (uint i = 0; i < UTIL_JSON->getVersionConfigCount(); i++)
		{
			if (!fnc)
			{
				m_udsConfig->version[i].result = false;
				m_udsConfig->version[i].error = false;
				m_udsConfig->version[i].reread = 0;
				memset(m_udsConfig->version[i].read, 0x00, sizeof(m_udsConfig->version[i].read));
				memset(m_udsConfig->version[i].errstr, 0x00, sizeof(m_udsConfig->version[i].errstr));
			}
			else
			{
				fnc(i);
			}
		}
	}

	if (log == BaseTypes::DL_DTC || log == BaseTypes::DL_ALL)
	{
		for (uint i = 0; i < UTIL_JSON->getDtcConfigCount(); i++)
		{
			if (!fnc)
			{
				m_udsConfig->dtc[i].exist = false;
			}
			else
			{
				fnc(i);
			}
		}
	}
}

QString Dt::Base::createLogFile(bool success)
{
	QString result = "";
	do
	{
		QDir dir;
		const auto& device = m_defConfig->device;
		/*log/error/20200228/机种_条码_时分秒.csv*/
		const QString filePath = QString("%1\\%2\\%3\\").arg(getLogPath(),
			success ? "Normal" : "Error", utility::getCurrentDate(true));
		RUN_BREAK(!utility::makePath(filePath), utility::getLastError());

		QString fileName = QString("%1_%2_%3.csv").arg(device.typeName,
			m_barcode.isEmpty() ? "未知条码" : m_barcode, utility::getCurrentTimeEx(true));
		QStringList character = { "\\","/",":","*","?","<",">","|" };
		for (const auto& x : character)
		{
			if (fileName.contains(x))
				fileName.replace(x, "-");
		}
		result = filePath + fileName;
	} while (false);
	return result;
}

bool Dt::Base::writeLog(bool success)
{
	bool result = false;
	do
	{
		const QString fileName = createLogFile(success);
		if (fileName.isEmpty()) {
			break;
		}

		QFile file(fileName);
		RUN_BREAK(!file.open(QFile::WriteOnly), "创建日志文件失败," + file.errorString());

		QTextStream stream(&file);
		stream.setCodec("gb2312");

		stream << Q_SPRINTF("条形码,%s\n检测结果,%s\n", Q_TO_C_STR(m_barcode), OK_NG(success));
		//删除CSV日志中相同的键
		static auto removeLogSame = [](QList<QString>& list) {
			for (int i = 0; i < list.count(); i++)
			{
				for (int k = i + 1; k < list.count(); k++)
				{
					auto split0 = list.at(i).split(",");
					auto split1 = list.at(k).split(",");
					if (split0.size() >= 2 && split1.size() >= 2)
					{
						/*
						* 如果日志的多个key相同,则保留最新的key
						* a,ng
						* a,ng
						* a,ok
						* 则保留a,ok
						*/
						if (split0[0] == split1[0])
						{
							list.removeAt(i);
							k--;
						}
					}
				}
			}
		};

		removeLogSame(m_logList);

		for (int i = 0; i < m_logList.size(); i++)
		{
			stream << m_logList[i] << endl;
		}

		file.close();
		result = true;
	} while (false);
	return result;
}

void Dt::Base::threadPause()
{
	m_threadWait = true;
	while (m_threadWait) { msleep(100); }
}

bool Dt::Base::threadIsPause() const
{
	return m_threadWait;
}

void Dt::Base::threadContinue()
{
	if (m_threadWait)
		m_threadWait = false;
}

void Dt::Base::threadQuit()
{
	m_connect = false;
	m_quit = true;

	if (threadIsPause())
	{
		closeDevice();
		threadContinue();
	}

	if (isRunning())
	{
		wait(5000);
	}
}

bool Dt::Base::setScanDlgWindow(bool show)
{
	return utility::ScanWidget::setWindow(show, &m_barcode);
}

bool Dt::Base::setAuthDlg()
{
	bool result = false;
	emit setAuthDlgSignal(&result, true);
	threadPause();
	return result;
}

void Dt::Base::setUnlockDlg(bool show)
{
	emit setUnlockDlgSignal(show);
	threadPause();
}

void Dt::Base::setMessageBox(const QString& title, const QString& text)
{
	emit setMessageBoxSignal(title, text);
	threadPause();
}

void Dt::Base::setMessageBoxEx(const QString& title, const QString& text, const QPoint& point)
{
	emit setMessageBoxExSignal(title, text, point);
	threadPause();
}

bool Dt::Base::setQuestionBox(const QString& title, const QString& text, bool modal)
{
	bool result = false;
	emit setQuestionBoxSignal(title, text, modal, &result);
	threadPause();
	return result;
}

bool Dt::Base::setQuestionBoxEx(const QString& title, const QString& text, const QPoint& point)
{
	bool result = false;
	emit setQuestionBoxExSignal(title, text, &result, point);
	threadPause();
	return result;
}

void Dt::Base::setTestResult(BaseTypes::TestResult testResult)
{
	emit setTestResultSignal(testResult);
}

void Dt::Base::setCurrentStatus(const QString& status, bool systemStatus)
{
	emit setCurrentStatusSignal(status, systemStatus);
}

void Dt::Base::addListItem(const QString& item, bool logItem)
{
	emit addListItemSignal(QString("%1  %2").arg(utility::getCurrentTimeEx(),
		(logItem ? item : (m_defConfig->enable.showLogicNumber ?
			Q_SPRINTF("No.%02d %s", LOGIC_INDEX + 1, Q_TO_C_STR(item)) : item))), logItem);
}

void Dt::Base::addListItemEx(const QString& item)
{
	addListItem(item, false); addListItem(item, true);
}

void Dt::Base::clearListItem()
{
	emit clearListItemSignal();
}

bool Dt::Base::setDownloadDlg(BaseTypes::DownloadInfo* info)
{
	emit setDownloadDlgSignal(info);
	threadPause();
	return info->result;
}

bool Dt::Base::waitStartup(ulong delay)
{
	addListItem(Q_SPRINTF("等待系统稳定中,大约需要%lu秒,请耐心等待...", delay / 1000));
	msleep(delay);
	return true;
}

bool Dt::Base::checkPing(const QString& address, int times)
{
	setCurrentStatus("检测Ping");
	bool result = false, success = false;
	do
	{
		RUN_BREAK(address.isEmpty(), "IP地址未设置,请设置IP地址");
		addListItem(Q_SPRINTF("正在Ping %s,请耐心等待...", address));
		success = utility::ping(address, times);
		addListItem(Q_SPRINTF("Ping %s %s", address, OK_NG(success)));
		RUN_BREAK(!success, Q_SPRINTF("Ping %s失败", address));
		result = true;
	} while (false);
	addListItemEx(Q_SPRINTF("检测Ping %s", OK_NG(result)));
	WRITE_LOG("检测Ping,%s", OK_NG(result));
	return result;
}

bool Dt::Base::checkPing(const QString& address, int port, int timeout)
{
	setCurrentStatus("检测Ping");
	bool result = false, success = false;
	do
	{
		RUN_BREAK(address.isEmpty(), "目的IP地址未设置,请设置IP地址");
		addListItem(QString("正在Ping %1,请耐心等待...").arg(address));
		success = Misc::isOnline(address, port, timeout);
		addListItem(Q_SPRINTF("Ping %s %s", Q_TO_C_STR(address), OK_NG(success)));
		RUN_BREAK(!success, Q_SPRINTF("Ping %s失败", Q_TO_C_STR(address)));
		result = true;
	} while (false);
	addListItemEx(Q_SPRINTF("检测Ping %s", OK_NG(result)));
	WRITE_LOG("检测Ping,%s", OK_NG(result));
	return result;
}

bool Dt::Base::checkPing(const QString& jsonAddress, const QString& jsonPort, int timeout)
{
	const QString address = UTIL_JSON->getOtherConfig1Value(jsonAddress);
	const int port = UTIL_JSON->getOtherConfig1Value(jsonPort).toInt();
	return checkPing(address, port, timeout);
}

bool Dt::Base::checkPing(const QString& source, const QString& destination, int port, int timeout)
{
	setCurrentStatus("检测Ping");
	bool result = false, success = false;
	do
	{
		RUN_BREAK(source.isEmpty(), "指定IP未设置,请设置IP地址");
		RUN_BREAK(destination.isEmpty(), "目的IP未设置,请设置IP地址");
		addListItem(QString("[%1]正在Ping[%2],请耐心等待...").arg(source, destination));
		success = Misc::isOnline(source, destination, port, timeout);
		addListItem(Q_SPRINTF("Ping %s %s", Q_TO_C_STR(destination), OK_NG(success)));
		RUN_BREAK(!success, Q_SPRINTF("Ping %s失败", Q_TO_C_STR(destination)));
		result = true;
	} while (false);
	addListItemEx(Q_SPRINTF("检测Ping %s", OK_NG(result)));
	WRITE_LOG("检测Ping,%s", OK_NG(result));
	return result;
}

bool Dt::Base::checkPing(const QString& jsonSource, const QString& jsonDestination, const QString& jsonPort, int timeout)
{
	const QString source = UTIL_JSON->getOtherConfig1Value(jsonSource);
	const QString destination = UTIL_JSON->getOtherConfig1Value(jsonDestination);
	const int port = UTIL_JSON->getOtherConfig1Value(jsonPort).toInt();
	return checkPing(source, destination, port);
}

void Dt::Base::setUdsVehicleType(uds::VehicleType vehicleType)
{
	m_udsVehicleType = vehicleType;
}

uds::VehicleType Dt::Base::getUdsVehicleType() const
{
	return m_udsVehicleType;
}

void Dt::Base::setClassName(const QString& name)
{
	m_className = name;
}

QString Dt::Base::getClassName()
{
	return m_className;
}

QString Dt::Base::getLogPath(bool absolutePath)
{
	const QString logPath = QString("%1\\Log").arg(Json::getUsingPath());
	if (Json::usingUserPath())
		return logPath;
	return absolutePath ? QString("%1\\%2").arg(utility::getCurrentDirectory(), logPath) : logPath;
}

QMap<QString, QString> Dt::Base::getComponentVoltageKeyValue() const
{
	return m_componentVoltageKeyValue;
}

QMap<QString, QString> Dt::Base::getVersionConfigKeyValue() const
{
	return m_versionConfigKeyValue;
}

QMap<QString, QString> Dt::Base::getDtcConfigKeyValue() const
{
	return m_dtcConfigKeyValue;
}

QMap<QString, QString> Dt::Base::getOtherConfig1KeyValue() const
{
	return m_otherConfig1KeyValue;
}

QList<QString> Dt::Base::getOtherConfig2ParentKeyList() const
{
	return m_otherConfig2ParentKeyList;
}

QList<QMap<QString, QString>> Dt::Base::getOtherConfig2ChildKeyValueList() const
{
	return m_otherConfig2ChildKeyValueList;
}

void Dt::Base::addCycleEventMsg(const can::Msg& msg)
{
	m_cycleEventMsg.append(msg);
}

void Dt::Base::clearCycleEventMsg()
{
	m_cycleEventMsg.clear();
}

void Dt::Base::addCycleMsg(const can::Msg& msg)
{
	m_cycleMsg.append(msg);
}

void Dt::Base::clearCycleMsg()
{
	m_cycleMsg.clear();
}

bool Dt::Base::isTesting() const
{
	return m_testing;
}

void Dt::Base::setDeviceFaultCode(DeviceFaultCode code)
{
	m_faultCode = code;
}

DeviceFaultCode Dt::Base::getDeviceFaultCode() const
{
	return m_faultCode;
}

void Dt::Base::setDeviceFaultHint(bool hint)
{
	m_faultHint = hint;
}

void Dt::Base::setLastError(const QString& error)
{
	DEBUG_INFO() << error;
	Misc::writeRunError(m_barcode, error);
	m_lastError = error;
}

void Dt::Base::setLastError(const QString& error, bool addItem, bool msgBox)
{
	DEBUG_INFO() << error;
	Misc::writeRunError(m_barcode, error);
	m_lastError = error;
	if (addItem)
	{
		addListItem(m_lastError);
	}

	if (msgBox)
	{
		utility::QMessageBoxEx::warning(static_cast<QWidget*>(nullptr), "错误", error);
	}
}

void Dt::Base::startTest(bool testing)
{
	emit startTestSignal(testing);
	m_testing = testing;
	return;
}

bool Dt::Base::setOtherAction()
{
	return true;
}


Dt::Hardware::Hardware(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Hardware);
	m_detectionType = BaseTypes::DetectionType::DT_HARDWARE;
}

Dt::Hardware::~Hardware()
{
	PRINT_CON_DESTRUCTION(Dt::Hardware);
}


Dt::Function::Function(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Function);
	m_detectionType = BaseTypes::DetectionType::DT_FUNCTION;
}

Dt::Function::~Function()
{
	PRINT_CON_DESTRUCTION(Dt::Function);
	CaptureCard::freeCap(m_capBase);
	autoRecycle({ "Log" }, { ".mp4",".jpg",".bmp",".png" }, -1);
}

bool Dt::Function::initialize()
{
	bool result = false;
	do
	{
		if (!Dt::Base::initialize())
		{
			break;
		}

		m_cardConfig.name = m_defConfig->device.cardName;
		m_cardConfig.channelId = m_defConfig->device.cardChannelId.toInt();

		m_capBase = CaptureCard::allocCap(m_cardConfig.name);
		RUN_BREAK(!m_capBase, "采集卡分配内存失败");
		m_capBase->setConfigFilePath("Config\\Share");

#ifdef QT_DEBUG
		if (m_cardConfig.name == "VIDEO")
			m_capBase->setImageOriginalSize(640, 480);
#endif
		connect(m_capBase, QPIX_SIGNAL, this, &Base::updateImageSignal);
		m_capBase->setImageProc([&](cv::Mat& mat) {drawRectOnImage(mat); });

		/*
		* @notice,setImageScaleSize必须设置在初始化中,如果未设置,
		* 直接在设置界面捕捉图像进行画图,图像将不会被缩放.
		*/
		m_capBase->setImageScaleSize(VIDEO_WIDTH, VIDEO_HEIGHT);

		RUN_BREAK(strcmp(Tesseract::getVersion(), "1.0.0.3"),
			Q_SPRINTF("光学字符识别动态库版本不正确,\n请更新%s\\ocr\\Tesseract.dll", MY_SDK_PATH));

		RUN_BREAK(!m_ocr.initialize(utility::existMySdkPath() ?
			Q_TO_C_STR(Q_SPRINTF("%s\\ocr", MY_SDK_PATH)) : "ocr",
			"chi_sim"), LC_TO_Q_STR(m_ocr.getLastError()));

		result = true;
	} while (false);
	return result;
}

bool Dt::Function::openDevice()
{
	bool result = false;
	do
	{
		if (!Dt::Base::openDevice())
		{
			break;
		}

		if (!m_capBase->open(m_cardConfig.channelId))
		{
			setDeviceFaultCode(DFC_CAPTURE_CARD);
			Dt::Base::closeDevice();
			setLastError("打开采集卡失败," + m_capBase->getLastError(), false, m_faultHint);
			break;
		}
		m_capBase->startCapture();

		result = true;
	} while (false);
	return result;
}

bool Dt::Function::closeDevice()
{
	bool result = false;
	do
	{
		if (!Dt::Base::closeDevice())
		{
			break;
		}

		m_capBase->stopCapture();
		m_capBase->close();

		result = true;
	} while (false);
	return result;
}

bool Dt::Function::checkCanRouseSleep(const can::Msg& msg, int id, int value, CanProc proc)
{
	bool result = false, success = false;
	do
	{
		setCurrentStatus("检测CAN唤醒");
		sendCanMsg(msg);
		success = autoProcessCanMsg(id, value, proc);
		deleteCanMsg(msg);
		WRITE_LOG("CAN唤醒,%.3fA", m_rouseCurrent);
		addListItem(Q_SPRINTF("CAN唤醒 %s %.3fA", OK_NG(success), m_rouseCurrent));
		addListItem(Q_SPRINTF("CAN唤醒 %s", OK_NG(success)), false);
		RUN_BREAK(!success, "CAN唤醒失败");

		/*m_canSend->stop();
		success = false;
		setCurrentStatus("检测CAN休眠");
		const ulong startTime = GetTickCount64();
		float current = 0.0f;
		while (true)
		{
			m_power.GetCurrent(&current);

			if (current <= m_defConfig->threshold.canSleep)
			{
				success = true;
				break;
			}

			if (success || GetTickCount64() - startTime >= 20000)
			{
				break;
			}
			msleep(300);
		}
		m_canSend->start();

		WRITE_LOG("%s CAN休眠,%.3fA", OK_NG(success), current);
		addListItem(Q_SPRINTF("CAN休眠 %s %.3fA", OK_NG(success), current));
		addListItem(Q_SPRINTF("CAN休眠 %s", OK_NG(success)), false);
		RUN_BREAK(!success, "CAN休眠失败");*/
		if (!checkCanSleep())
		{
			break;
		}
		m_relay->setOne(m_defConfig->relay.ignitionLine, true);
		msleep(300);
		result = true;
	} while (false);
	return result;
}

bool Dt::Function::checkCanRouse(int timeout, const can::Msg& msg, int id, int value, CanProc proc)
{
	setCurrentStatus("检测CAN唤醒");
	bool result = false, success = false;
	do
	{
		if (msg.empty()) {
			can::Msg msg = { 0x100,8,{0},100 };
			sendCanMsg(msg);
		}
		else {
			sendCanMsg(msg);
		}
		success = autoProcessCanMsg(id, value, proc, timeout);
		deleteCanMsg(msg);
		WRITE_LOG("CAN唤醒,%.3fA", m_rouseCurrent);
		addListItem(Q_SPRINTF("CAN唤醒 %s %.3fA", OK_NG(success), m_rouseCurrent));
		addListItem(Q_SPRINTF("CAN唤醒 %s", OK_NG(success)), false);
		RUN_BREAK(!success, "CAN唤醒失败");
		result = true;
	} while (false);
	return result;
}

bool Dt::Function::checkCanSleep(int timeout, bool asyncCall)
{
	setCurrentStatus("检测CAN休眠");
	bool result = false, success = false, device = true;
	do
	{
		if (!asyncCall) {
			addListItem("正在检测CAN休眠,请耐心等待...");
		}
		else {
			if (m_defConfig->enable.showLogicNumber) {
				addListItem("休眠线程启动 OK", false);
			}
			m_sleepFlag = false;
		}
		m_can->stopAsyncSendMsg();
		auto startTime = GetTickCount64();
		float current = 0.0f;
		while (true)
		{
			if (!m_power->getCurrent(current))
			{
				device = false;
				break;
			}

			if (current <= m_defConfig->threshold.sleepCurrent)
			{
				success = true;
				break;
			}

			if (GetTickCount64() - startTime > timeout)
			{
				break;
			}
			msleep(300);
		}
		m_can->startAsyncSendMsg();

		m_sleepCurrent = current;

		if (!asyncCall) {
			WRITE_LOG("CAN休眠,%.3fA", current);
			addListItem(Q_SPRINTF("CAN休眠 %s %.3fA", OK_NG(success), current));
			addListItem(Q_SPRINTF("CAN休眠 %s", OK_NG(success)), false);
		}
		RUN_BREAK(!success || !device, Q_SPRINTF("CAN休眠失败%s", device ? "" : ",电源设备异常,无法获取电流"));
		result = true;
	} while (false);
	return result;
}

bool Dt::Function::checkCanSleepAsync(int timeout)
{
	addListItem("检测CAN休眠异步");
	if (m_success && m_sleepFuture.valid())
	{
		//如果测试成功,并没有调用wait&get则提示代码错误
		QString error("CAN休眠异步线程代码错误,未调用waitCanSleepAsync获取状态");
		addListItem(error);
		setLastError(error);
		return false;
	}
	else if (m_sleepFuture.valid())
	{
		//等待上次的线程完全退出,避免多线程抢占设备资源导致崩溃
		m_sleepFuture.wait();
		m_sleepFuture.get();
	}

	m_sleepFlag = m_defConfig->enable.showLogicNumber;
	m_sleepFuture = std::async([this, timeout]()->bool {return checkCanSleep(timeout, true); });
	auto start = GetTickCount64();
	while (m_defConfig->enable.showLogicNumber && m_sleepFlag) 
	{ 
		if (GetTickCount64() - start > 5000)
		{
			setLastError("代码逻辑错误,checkCanSleep中asyncCall为真,但m_sleepFlag未设置为假");
			return false;
		}
		msleep(100);
	};
	return true;
}

bool Dt::Function::waitCanSleepAsync()
{
	addListItem("等待CAN休眠异步线程执行完毕,请耐心等待...");
	if (!m_sleepFuture.valid())
	{
		setLastError("代码逻辑异常,未调用checkCanSleepAsync");
		return false;
	}
	m_sleepFuture.wait();
	bool success = m_sleepFuture.get();
	WRITE_LOG("CAN休眠异步,%.3fA", m_sleepCurrent);
	addListItem(Q_SPRINTF("CAN休眠异步 %s %.3fA", OK_NG(success), m_sleepCurrent));
	addListItem(Q_SPRINTF("CAN休眠异步 %s", OK_NG(success)), false);
	if (!success)
	{
		setLastError("CAN休眠异步失败");
		return false;
	}
	return true;
}

bool Dt::Function::checkCanRouseSleep(int id, int value, CanProc proc)
{
	bool result = false;
	do
	{
		can::Msg msg = { 0x100,8,{0},100 };
		if (!checkCanRouseSleep(msg, id, value, proc))
		{
			break;
		}

		waitStartup(START_DELAY);
		result = true;
	} while (false);
	return result;
}

bool Dt::Function::saveAnalyzeImage(const QString& name, const cv::Mat& image)
{
	bool result = false;
	do
	{
		if (!m_defConfig->image.saveLog)
		{
			result = true;
			break;
		}

		const QString path = QString("%1\\Image").arg(getLogPath());
		RUN_BREAK(!utility::makePath(path), utility::getLastError());

		const QString fileName = QString("%1\\%2.bmp").arg(path, name);
		showImage(image, name);
		RUN_BREAK(!cv::imwrite(fileName.toStdString(), image), "保存分析图像失败");
		result = true;
	} while (false);
	return result;
}

inline void Dt::Function::drawRectOnImage(cv::Mat& mat)
{
	m_customLock.lock();
	if (m_customDraw)
	{
		m_customDraw(mat);
	}
	m_customLock.unlock();

	const int i = static_cast<int>(m_rectType);

	static auto putText = [](cv::Mat& mat, int index, const cv::Rect& rect)->void {
		static int fontFace = CV_FONT_HERSHEY_SIMPLEX;
		static double fontScale = 0.5;
		static int thickness = 1;
		static int baseLine = 0;

		std::string text(std::to_string(index + 1));
		auto&& size = cv::getTextSize(text, fontFace, fontScale, thickness, &baseLine);
		cv::putText(mat, text, cv::Point(rect.x, rect.y + size.height), fontFace, fontScale, CV_RGB(255, 255, 0), thickness);
	};

	const auto& image = m_defConfig->image;

	if (m_defConfig->image.alwaysShow && i != FncTypes::RT_OTHER || m_showAllRect)
	{
		if (m_defConfig->image.showSmall)
		{
			const auto value = m_defConfig->image.smallRect;
			for (int i = 0; i < SMALL_RECT_; ++i)
			{
				if (value[i].enable)
				{
					cv::Rect rect(value[i].startX, value[i].startY, value[i].width, value[i].height);
					cv::rectangle(mat, rect, (i == (SMALL_RECT_ - 1)) ? CV_RGB(255, 0, 0) : CV_RGB(0, 255, 0), 1);
					if (image.showNumber)
					{
						putText(mat, i, rect);
					}
				}
			}
		}

		if (m_defConfig->image.showBig)
		{
			const auto value = m_defConfig->image.bigRect;
			for (int i = 0; i < BIG_RECT_; ++i)
			{
				if (value[i].enable)
				{
					cv::Rect rect(value[i].startX, value[i].startY, value[i].width, value[i].height);
					cv::rectangle(mat, rect, CV_RGB(255, 0, 0));
					if (image.showNumber)
					{
						putText(mat, i, rect);
					}
				}
			}
		}
		return;
	}

	if (m_rectType == FncTypes::RT_NO)
	{
		return;
	}

	if (m_defConfig->image.showSmall && i == FncTypes::RT_SMALL_ALL)
	{
		const auto value = m_defConfig->image.smallRect;
		for (int i = 0; i < SMALL_RECT_; ++i)
		{
			if (value[i].enable)
			{
				cv::Rect rect(value[i].startX, value[i].startY, value[i].width, value[i].height);
				cv::rectangle(mat, rect, (i == (SMALL_RECT_ - 1)) ? CV_RGB(255, 0, 0) : CV_RGB(0, 255, 0), 1);
				if (image.showNumber)
				{
					putText(mat, i, rect);
				}
			}
		}
	}
	else if (i == FncTypes::RT_OTHER && !m_otherRect.empty())
	{
		cv::rectangle(mat, m_otherRect, CV_RGB(255, 255, 0), 1);
		restoreRectType();//避免速度太快导致不显示矩形框,在此处进行还原
	}
	else if (m_defConfig->image.showBig && i == FncTypes::RT_BIG_ALL)
	{
		const auto value = m_defConfig->image.bigRect;
		for (int i = 0; i < BIG_RECT_; ++i)
		{
			if (value[i].enable)
			{
				cv::Rect rect(value[i].startX, value[i].startY, value[i].width, value[i].height);
				cv::rectangle(mat, rect, CV_RGB(255, 0, 0));
				if (image.showNumber)
				{
					putText(mat, i, rect);
				}
			}
		}
	}
	else if (m_defConfig->image.showBig)
	{
		const auto value = m_defConfig->image.bigRect;
		cv::Rect rect(value[i].startX, value[i].startY, value[i].width, value[i].height);
		cv::rectangle(mat, rect, CV_RGB(255, 255, 255), 1);
		if (image.showNumber)
		{
			putText(mat, i, rect);
		}
	}
	return;
}

bool Dt::Function::checkColor(const RectConfig& rectConfig, FncTypes::ColorInfo* colorInfo)
{
	bool result = false, success = false, syntaxError = false;
	do
	{
		QString dataInfo;
		cv::Mat image = m_capBase->getScaledImage().clone();
		if (image.empty())
		{
			dataInfo = "分析图像为空";
			break;
		}

		if (m_defConfig->image.saveLog)
		{
			QString fileName = "NoName";
			switch (getRectType())
			{
			case FncTypes::RT_FRONT_BIG:fileName = "All&Front"; break;
			case FncTypes::RT_REAR_BIG:fileName = "All&Rear"; break;
			case FncTypes::RT_LEFT_BIG:fileName = "All&Left"; break;
			case FncTypes::RT_RIGHT_BIG:fileName = "All&Right"; break;
			case FncTypes::RT_SMALL_ALL:fileName = "All&Small"; break;
			case FncTypes::RT_BIG_ALL:fileName = "All&Big"; break;
			default:break;
			}
			saveAnalyzeImage(fileName, image);
		}


		cv::Mat roi = image(cv::Rect(rectConfig.startX, rectConfig.startY, rectConfig.width, rectConfig.height));
		showImage(roi, "ROI");

		std::vector<int> vec;

		if (m_defConfig->image.detectAlgorithm)
		{
			cv::Mat matHsv;
			cv::cvtColor(roi, matHsv, cv::COLOR_BGR2HSV);

			uint size = matHsv.cols * matHsv.rows * matHsv.elemSize();
			uint red = 0, green = 0, blue = 0;
			uint count = 0;

			/*求ROI平均值*/
			for (uint i = 0; i < size; i += matHsv.elemSize())
			{
				blue += matHsv.data[i];
				green += matHsv.data[i + 1];
				red += matHsv.data[i + 2];
				++count;
			}

			red /= count;
			green /= count;
			blue /= count;

			vec.push_back(blue);
			vec.push_back(green);
			vec.push_back(red);

			char color[32] = { 0 };

			if ((vec[0] >= 0 && vec[0] <= 180) && (vec[1] >= 0 && vec[1] <= 255) && (vec[2] >= 0 && vec[2] <= 46))
			{
				strcpy(color, "黑色");
			}
			else if ((vec[0] >= 0 && vec[0] <= 180) && (vec[1] >= 0 && vec[1] <= 43) && (vec[2] >= 46 && vec[2] <= 220))
			{
				strcpy(color, "灰色");
			}
			else if ((vec[0] >= 0 && vec[0] <= 180) && (vec[1] >= 0 && vec[1] <= 35) && (vec[2] >= 221 && vec[2] <= 255))
			{
				strcpy(color, "白色");
			}
			else if (((vec[0] >= 0 && vec[0] <= 10) || (vec[0] >= 156 && vec[0] <= 180)) && (vec[1] >= 43 && vec[1] <= 255) && (vec[2] >= 46 && vec[2] <= 255))
			{
				strcpy(color, "红色");
			}
			else if ((vec[0] >= 11 && vec[0] <= 25) && (vec[1] >= 43 && vec[1] <= 255) && (vec[2] >= 46 && vec[2] <= 255))
			{
				strcpy(color, "橙色");
			}
			else if ((vec[0] >= 26 && vec[0] <= 34) && (vec[1] >= 43 && vec[1] <= 255) && (vec[2] >= 46 && vec[2] <= 255))
			{
				strcpy(color, "黄色");
			}
			else if ((vec[0] >= 35 && vec[0] <= 77) && (vec[1] >= 43 && vec[1] <= 255) && (vec[2] >= 46 && vec[2] <= 255))
			{
				strcpy(color, "绿色");
			}
			else if ((vec[0] >= 78 && vec[0] <= 99) && (vec[1] >= 43 && vec[1] <= 255) && (vec[2] >= 46 && vec[2] <= 255))
			{
				strcpy(color, "青色");
			}
			else if ((vec[0] >= 100 && vec[0] <= 124) && (vec[1] >= 43 && vec[1] <= 255) && (vec[2] >= 46 && vec[2] <= 255))
			{
				strcpy(color, "蓝色");
			}
			else if ((vec[0] >= 125 && vec[0] <= 155) && (vec[1] >= 43 && vec[1] <= 255) && (vec[2] >= 46 && vec[2] <= 255))
			{
				strcpy(color, "紫色");
			}
			else
			{
				strcpy(color, "未知颜色");
			}

			strcpy_s(colorInfo->color.name, color);

			QString imageColor = rectConfig.color;

			/*去除imageColor语法中所有空格*/

			imageColor.remove(QRegExp("\\s"));

			QStringList configColorList = imageColor.mid(2).split(",", QString::SkipEmptyParts);
			if (imageColor.contains("!="))
			{
				for (auto& x : configColorList)
				{
					if (x != color)
					{
						success = true;
					}
					else
					{
						success = false;
						break;
					}
				}
			}
			else if (imageColor.contains("=="))
			{
				for (auto& x : configColorList)
				{
					if (x == color)
					{
						success = true;
					}
					else
					{
						success = false;
						break;
					}
				}
			}
			else
			{
				syntaxError = true;
			}
			//dataInfo.sprintf("色彩模型RGB[%03d,%03d,%03d]  配置语法[%s%s]  分析所得颜色[%s]  %s", 
			//	vec[2], vec[1], vec[0], Q_TO_C_STR(rectConfig.color), syntaxError ? ",语法错误" : "", color, OK_NG(success));
			dataInfo.sprintf("色彩模型RGB[%03d,%03d,%03d]  配置语法[%s%s]  分析所得颜色[%s]  ",
				vec[2], vec[1], vec[0], Q_TO_C_STR(rectConfig.color), syntaxError ? ",语法错误" : "", color);
		}
		else
		{
			/*RGB使用均值算法*/
			cv::Scalar mean, stddev;
			cv::meanStdDev(roi, mean, stddev);

			for (int i = 0; i < 3; i++)
			{
				vec.push_back(mean.val[i]);
			}

			int limit = 0, upper = 0;
			const int rgb[3] = { rectConfig.red,rectConfig.green,rectConfig.blue };
			for (int i = 0; i < 3; i++)
			{
				limit = rgb[i] - rectConfig.deviation;
				upper = rgb[i] + rectConfig.deviation;
				if (!((vec[abs(i - 2)] >= limit) && (vec[abs(i - 2)] <= upper)))
				{
					success = false;
				}
			}

			//dataInfo.sprintf("配置RGB[%03d,%03d,%03d]  实测RGB[%03d,%03d,%03d]  允许误差[%03d]  %s",
			//	rectConfig.red, rectConfig.green, rectConfig.blue,
			//	vec[2], vec[1], vec[0], rectConfig.deviation, OK_NG(success));

			dataInfo.sprintf("配置RGB[%03d,%03d,%03d]  实测RGB[%03d,%03d,%03d]  允许误差[%03d]  ",
				rectConfig.red, rectConfig.green, rectConfig.blue,
				vec[2], vec[1], vec[0], rectConfig.deviation);
		}

		colorInfo->color.red = vec[2];
		colorInfo->color.green = vec[1];
		colorInfo->color.blue = vec[0];
		colorInfo->color.result = success;

		if (m_defConfig->image.detectPurity)
		{
			cv::Mat purity;
			cv::cvtColor(image, purity, CV_BGR2GRAY);
			cv::threshold(purity, purity, 100, 0xff, CV_THRESH_BINARY);
			cv::Canny(purity, purity, 10, 100);

			int sum = 0;
			for (int i = 0; i < purity.rows; ++i)
			{
				for (int j = 0; j < purity.cols; ++j)
				{
					if (purity.at<uchar>(i, j))
					{
						++sum;
					}
				}
			}

			colorInfo->purity.value = static_cast<int>(100.0f - (static_cast<float>(sum) / purity.total()) * 100);
			if (!(colorInfo->purity.result = (colorInfo->purity.value >= rectConfig.purity)))
				success = false;
			dataInfo.append(Q_SPRINTF("色彩纯度[%03d,%03d]  ", rectConfig.purity, colorInfo->purity.value));
		}

		dataInfo.append(OK_NG(success));

		strcpy_s(colorInfo->logInfo, Q_TO_C_STR(dataInfo));

		if (m_defConfig->image.saveLog)
		{
			const auto& image = m_defConfig->image;
			if (m_rectType == FncTypes::RT_SMALL_ALL)
			{
				const QStringList nameList = { "FrontSmall", "RearSmall", "LeftSmall", "RightSmall", "MiddelSmall" };
				RUN_BREAK(nameList.size() != SMALL_RECT_, "保存日志失败,nameList.size() != SMALL_RECT_");

				for (int i = 0; i < SMALL_RECT_; i++)
				{
					if (!memcmp(&image.smallRect[i], &rectConfig, sizeof(RectConfig)))
					{
						saveAnalyzeImage(nameList.at(i), roi);
						break;
					}
				}
			}
			else
			{
				const QStringList nameList = { "FrontBig", "RearBig", "LeftBig", "RightBig" };
				RUN_BREAK(nameList.size() != BIG_RECT_, "保存日志失败,nameList.size() != BIG_RECT_");

				for (int i = 0; i < BIG_RECT_; i++)
				{
					if (!memcmp(&image.bigRect[i], &rectConfig, sizeof(RectConfig)))
					{
						saveAnalyzeImage(nameList.at(i), roi);
						break;
					}
				}
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

bool Dt::Function::checkImageDynamic()
{
	setCurrentStatus("检测图像动态");
	bool result = false, success = true;
	auto** mat = &m_defConfig->imageDynamic.image;
	if (!*mat)
	{
		*mat = new cv::Mat;
	}

	do
	{
		addListItem("检测图像动态中,请耐心等待...");
		m_defConfig->imageDynamic.currentFrequency = 0;
		m_defConfig->imageDynamic.currentCount = 0;

		auto startTime = GetTickCount64();
		while (true)
		{
			cv::Mat diff;
			cv::Mat ele = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(8, 8));
			cv::Mat image = m_capBase->getScaledImage().clone();
			if (image.empty())
			{
				success = false;
				setLastError("分析图像为空");
				break;
			}

			cv::cvtColor(image, image, CV_BGR2GRAY);

			if ((*mat)->empty())
			{
				image.copyTo(**mat);
				continue;
			}

			//非线程安全,需要判断,否则会抛出异常
			if ((*mat)->size() != image.size())
			{
				image.copyTo(**mat);
				continue;
			}

			cv::absdiff(image, **mat, diff);
			cv::dilate(diff, diff, ele);
			cv::erode(diff, diff, ele);
			cv::threshold(diff, diff, m_defConfig->imageDynamic.binaryThreshold, 255, 0);

			double pixel = diff.total() * 255;

			uint sum = 0;
			for (int i = 0; i < diff.rows; ++i)
			{
				for (int j = 0; j < diff.cols; ++j)
				{
					sum += diff.at<uchar>(i, j);
				}
			}

			m_defConfig->imageDynamic.currentPercent = (static_cast<double>(sum) / pixel) * 100.0f;

			if (GetTickCount64() - m_defConfig->imageDynamic.currentFrequency >=
				m_defConfig->imageDynamic.dynamicFrequency)
			{
				if (m_defConfig->imageDynamic.currentPercent <= m_defConfig->imageDynamic.dynamicPercent)
				{
					if (++m_defConfig->imageDynamic.currentCount >= m_defConfig->imageDynamic.determineCount)
					{
						success = false;
						setLastError("检测图像动态失败,图像存在卡死状态");
						break;
					}
				}

				m_defConfig->imageDynamic.currentFrequency = GetTickCount64();
			}

			image.copyTo(**mat);

			if (GetTickCount64() - startTime > m_defConfig->imageDynamic.durationTime)
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
	delete *mat;
	*mat = nullptr;

	addListItem(Q_SPRINTF("检测图像动态%s,动态占比[%.2lf%%/%.2lf%%]", OK_NG(result),
		m_defConfig->imageDynamic.dynamicPercent,
		m_defConfig->imageDynamic.currentPercent));

	WRITE_LOG("检测图像动态,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测图像动态 %s", OK_NG(result)));
	return result;
}

void Dt::Function::setRectType(FncTypes::RectType rectType)
{
	m_rectType = rectType;
}

FncTypes::RectType Dt::Function::getRectType() const
{
	return m_rectType;
}

void Dt::Function::restoreRectType()
{
	m_rectType = FncTypes::RT_NO;
}

void Dt::Function::showImage(const cv::Mat& mat, const QString& name) const
{
	if (m_showImage)
	{
		cv::namedWindow(Q_TO_C_STR(name), 0);
		cv::imshow(Q_TO_C_STR(name), mat);
		cv::waitKey();
	}
}

void Dt::Function::setShowImage(bool show)
{
	m_showImage = show;
}

FncTypes::CardConfig Dt::Function::getCaptureCardConfig() const
{
	return m_cardConfig;
}

CapBase* Dt::Function::getCaptureCard() const
{
	return m_capBase;
}

bool Dt::Function::recognizeImageText(const cv::Rect& rect, float scale, int gray, float threshold, int type, QString& text)
{
	bool result = false;
	do
	{
		cv::Mat mat = m_capBase->getScaledImage().clone();
		RUN_BREAK(mat.empty(), "识别图像文本失败,获取的图像为空");
		cv::Mat roi = mat(rect);
		cv::resize(roi, roi, cv::Size((float)roi.cols * scale,
			(float)roi.rows * scale), 0, 0, cv::INTER_CUBIC);

		if (gray)
			cv::cvtColor(roi, roi, CV_RGB2GRAY);

		if (threshold > 0.00000001f)
			cv::threshold(roi, roi, threshold, 255, type ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY);

		m_otherRect = rect;
		setRectType(FncTypes::RT_OTHER);

		std::vector<uchar> bmpData;
		RUN_BREAK(!cv::imencode(".bmp", roi, bmpData), "cv::Mat数据流转换为bmp数据流失败");

		if (m_defConfig->enable.saveRecognizeImage)
		{
			const QString filePath = QString("%1\\Image").arg(getLogPath());
			RUN_BREAK(!utility::makePath(filePath), utility::getLastError());

			const QString fileName = QString("%1\\%2.bmp").arg(filePath,
				utility::getCurrentTimeEx().remove(":"));

			RUN_BREAK(!cv::imwrite(fileName.toStdString(), roi), "保存识别图像失败");
		}

		wchar_t buffer[4096] = { 0 };
		if (!m_ocr.recognizeText(bmpData.data(), bmpData.size(), buffer, sizeof(buffer)))
		{
			setLastError(LC_TO_Q_STR(m_ocr.getLastError()));
			break;
		}
		text = QString::fromWCharArray(buffer);
		result = true;
	} while (false);
	return result;
}

bool Dt::Function::recognizeImageText(const QString& jsonNode, QString& text)
{
	bool result = false, convert = false;
	do
	{
		RUN_BREAK(!UTIL_JSON->getOtherConfig2Obj().contains(jsonNode),
			QString("其他配置2中,未包含节点对象%1").arg(jsonNode));

		const int x = UTIL_JSON->getOtherConfig2Value(jsonNode, "X").toInt(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[X]");

		const int y = UTIL_JSON->getOtherConfig2Value(jsonNode, "Y").toInt(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[Y]");

		const int width = UTIL_JSON->getOtherConfig2Value(jsonNode, "宽度").toInt(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[宽度]");

		const int height = UTIL_JSON->getOtherConfig2Value(jsonNode, "高度").toInt(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[高度]");

		const float scale = UTIL_JSON->getOtherConfig2Value(jsonNode, "放大倍数").toFloat(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[放大倍数]");

		const int gray = UTIL_JSON->getOtherConfig2Value(jsonNode, "转为灰度图").toFloat(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[转为灰度图]");

		const float threshold = UTIL_JSON->getOtherConfig2Value(jsonNode, "二值化阈值").toFloat(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[二值化阈值]");

		const int type = UTIL_JSON->getOtherConfig2Value(jsonNode, "二值化类型").toInt(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[二值化类型]");

		const int enable = UTIL_JSON->getOtherConfig2Value(jsonNode, "启用").toInt(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[启用]");

		if (!recognizeImageText(cv::Rect(x, y, width, height), scale, gray, threshold, type, text))
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Function::recognizeImageProc(const QString& jsonNode, const std::function<bool(const QString& text, const QString& value)>& proc, ulong timeout)
{
	bool result = false, success = false, convert = false;
	do
	{
		const int enable = UTIL_JSON->getOtherConfig2Value(jsonNode, "启用").toInt(&convert);
		RUN_BREAK(!convert, jsonNode + ",丢失键[启用]");

		if (!enable)
		{
			result = true;
			break;
		}

		QString name = jsonNode;
		if (name.contains("识别"))
		{
			name.remove("识别");
		}

		addListItem(QString("正在进行获取%1,请耐心等待...").arg(name));
		addListItem(QString("正在进行图像识别,获取%1结果,请耐心等待...").arg(name));

		const ulong start = GetTickCount64();
		const QString value = UTIL_JSON->getOtherConfig2Value(jsonNode, "识别内容");
		while (true)
		{
			QString text;
			if (!recognizeImageText(jsonNode, text))
			{
				break;
			}

			text = text.replace(" ", "").simplified();
			DEBUG_INFO() << QString("获取%1结果:").arg(name) << text;
			if (proc)
			{
				if (proc(text, value))
				{
					success = true;
					addListItem("识别图像成功,识别内容:" + text);
					break;
				}
			}
			else
			{
				if (text.contains(value))
				{
					success = true;
					addListItem("识别图像成功,识别内容:" + text);
					break;
				}
			}

			if (GetTickCount64() - start > timeout)
			{
				setLastError("识别图像内容超时");
				addListItem("识别图像失败,识别内容:" + text);
				break;
			}
			msleep(100);
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

TouchScreenInfo Dt::Function::getTouchScreenInfo() const
{
	return m_touchScreen;
}

bool Dt::Function::checkQuadImage()
{
	setCurrentStatus("检测四分格图像");
	bool result = false, success = true;
	setRectType(FncTypes::RT_BIG_ALL);
	msleep(2000);
	do
	{
		const QStringList viewName = { "前","后","左","右" };
		for (int i = 0; i < BIG_RECT_; i++)
		{
			const auto& rect = m_defConfig->image.bigRect[i];
			if (!rect.enable)
			{
				continue;
			}

			FncTypes::ColorInfo color = { 0 };
			if (!checkColor(rect, &color))
			{
				success = false;
			}
			addListItem(QString("%1大图矩形框,%2").arg(viewName[i], color.logInfo));
		}
		RUN_BREAK(!success, "检测四分格图像失败");
		result = true;
	} while (false);
	restoreRectType();
	WRITE_LOG("检测四分格图像,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测四分格图像 %s", OK_NG(result)));
	return result;
}

void Dt::Function::setCustomDraw(std::function<void(cv::Mat&)> draw)
{
	m_customLock.lock();
	m_customDraw = draw;
	m_customLock.unlock();
}


Dt::Avm::Avm(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Avm);
	m_detectionType = BaseTypes::DetectionType::DT_AVM;
}

Dt::Avm::~Avm()
{
	PRINT_CON_DESTRUCTION(Dt::Avm);
}

bool Dt::Avm::initialize()
{
	bool result = false;
	do
	{
		if (!Dt::Function::initialize())
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

void Dt::Avm::setHorseRaceLamp(bool _switch)
{
	m_relay->setOne(m_defConfig->relay.horseRaceLamp, _switch);
	msleep(300);
}

bool Dt::Avm::triggerAvmByKey(ulong delay, int id, int value, CanProc proc, bool showLog)
{
	bool result = false;
	do
	{
		if (showLog)
		{
			addListItem("正在进行按键触发全景,请耐心等待...");
		}

		RUN_BREAK(!m_relay->simulateKeystroke(m_defConfig->relay.keyLine, delay), "按键闭合失败");
		if (id && proc)
		{
			RUN_BREAK(!autoProcessCanMsg(id, value, proc), "按键触发全景失败");
		}
		result = true;
	} while (false);

	if (showLog)
	{
		addListItem(Q_SPRINTF("按键触发全景 %s", OK_NG(result)));
		WRITE_LOG("按键触发全景,%s", OK_NG(result));
	}
	return result;
}

bool Dt::Avm::exitAvmByKey(ulong delay, int id, int value, CanProc proc, bool showLog)
{
	bool result = false;
	if (showLog)
	{
		addListItem("正在进行按键退出全景,请耐心等待...");
	}

	result = triggerAvmByKey(delay, id, value, proc);

	if (showLog)
	{
		addListItem(Q_SPRINTF("按键退出全景 %s", OK_NG(result)));
		WRITE_LOG("按键退出全景,%s", OK_NG(result));
	}
	return result;
}

bool Dt::Avm::triggerAvmByMsg(const can::Msg& msg, int id, int value, CanProc proc, bool showLog)
{
	bool result = false;
	do
	{
		if (showLog)
		{
			addListItem("正在报文触发全景,请耐心等待...");
		}

		sendCanMsg(msg);
		if (id && proc)
		{
			RUN_BREAK(!autoProcessCanMsg(id, value, proc), "报文触发全景失败");
		}
		result = true;
	} while (false);

	if (msg.sendType == can::SendType::CYCLE)
	{
		deleteCanMsg(msg);
	}

	if (showLog)
	{
		addListItem(Q_SPRINTF("报文触发全景 %s", OK_NG(result)));
		WRITE_LOG("报文触发全景,%s", OK_NG(result));
	}
	return result;
}

bool Dt::Avm::exitAvmByMsg(const can::Msg& msg, int id, int value, CanProc proc, bool showLog)
{
	bool result = false;
	if (showLog)
	{
		addListItem("正在进行报文退出全景,请耐心等待...");
	}
	result = triggerAvmByMsg(msg, id, value, proc);

	if (showLog)
	{
		addListItem(Q_SPRINTF("报文退出全景 %s", OK_NG(result)));
		WRITE_LOG("报文退出全景,%s", OK_NG(result));
	}
	return result;
}

bool Dt::Avm::checkVideoUseNot()
{
	setCurrentStatus("检测视频出画");
	bool result = false, success = true;
	do
	{
		RUN_BREAK(!checkSmallImageColor(), "检测视频出画失败");
		result = true;
	} while (false);
	WRITE_LOG("检测视频出画,%s", OK_NG(result));
	addListItem(Q_SPRINTF("检测视频出画 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Avm::checkVideoUseMsg(const can::Msg& msg, int id, int value, CanProc proc, bool keyVol)
{
	setCurrentStatus("检测视频出画");
	bool result = false, success = true;
	do
	{
		if (keyVol)
		{
			if (!checkKeyVoltage(msg, id, value, proc))
			{
				break;
			}
		}
		else
		{
			if (!triggerAvmByMsg(msg, id, value, proc))
			{
				break;
			}
		}
		RUN_BREAK(!checkSmallImageColor(), "检测视频出画失败");
		result = true;
	} while (false);
	WRITE_LOG("检测视频出画,%s", OK_NG(result));
	addListItem(Q_SPRINTF("检测视频出画 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Avm::checkVideoUseKey(int id, int value, CanProc proc, ulong hDelay, bool keyVol)
{
	setCurrentStatus("检测视频出画");
	bool result = false, success = true;
	do
	{
		if (keyVol)
		{
			if (!checkKeyVoltage(hDelay, id, value, proc))
			{
				break;
			}
		}
		else
		{
			if (!triggerAvmByKey(hDelay, id, value, proc))
			{
				break;
			}
		}
		RUN_BREAK(!checkSmallImageColor(), "检测视频出画失败");
		result = true;
	} while (false);
	WRITE_LOG("检测视频出画,%s", OK_NG(result));
	addListItem(Q_SPRINTF("检测视频出画 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Avm::checkVideoUsePerson()
{
	setCurrentStatus("检测视频出画");
	bool result = false, success = false;
	do
	{
		success = setQuestionBoxEx("提示", "视频是否出画?");
		RUN_BREAK(!success, "检测视频出画失败");
		result = true;
	} while (false);
	WRITE_LOG("检测视频出画,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测视频出画 %s", OK_NG(result)));
	return result;
}

bool Dt::Avm::checkSingleImageUseMsg(FncTypes::RectType type, const can::Msg& msg,
	int id, int value, CanProc proc, ulong timeout)
{
	setCurrentStatus("检测单个图像");
	bool result = false, success = true;
	do
	{
		RUN_BREAK(type == FncTypes::RT_SMALL_ALL ||
			type == FncTypes::RT_NO, "仅支持大矩形框检测");

		setRectType(type);

		sendCanMsg(msg);
		if (proc && !autoProcessCanMsg(id, value, proc, timeout))
		{
			break;
		}

		msleep(2000);
		const QStringList viewName = { "前","后","左","右","中" };
		FncTypes::ColorInfo colorInfo = { 0 };
		const int index = static_cast<int>(type);
		if (!checkColor(m_defConfig->image.smallRect[index], &colorInfo))
		{
			success = false;
		}
		addListItem(QString("%1摄像头大图,%2").arg(viewName.at(index), colorInfo.logInfo));
		RUN_BREAK(!success, "检测单个图像失败");
		result = true;
	} while (false);
	restoreRectType();
	deleteCanMsg(msg);
	WRITE_LOG("检测单个图像,%s", OK_NG(result));
	addListItem(Q_SPRINTF("检测单个图像 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Avm::checkFRViewUseMsg(CanList msgList, int id, ValList valList, CanProc proc)
{
	setCurrentStatus("检测前后视图");
	bool result = false, success = true;
	do
	{
		RUN_BREAK(msgList.size() != 2, "检测前后视图msgList.size()!=2");

		const QStringList viewName = { "前","后","左","右","中" };
		FncTypes::ColorInfo colorInfo = { 0 };
		int subscript = -1;
		for (int i = 0; i < msgList.size(); i++)
		{
			subscript = abs(i - 1);

			setRectType(static_cast<FncTypes::RectType>(subscript));

			sendCanMsg(msgList.begin()[subscript]);

			m_can->startAsyncSendMsg();

			if (!autoProcessCanMsg(id, valList.begin()[subscript], proc)) {
				success = false;
				setLastError(QString("进入%1大视图失败").arg(viewName.at(subscript)));
				break;
			}

			msleep(1000);

			memset(&colorInfo, 0, sizeof(colorInfo));
			if (!checkColor(m_defConfig->image.bigRect[subscript], &colorInfo))
			{
				success = false;
				setLastError("检测前后视图失败");
			}
			deleteCanMsg(msgList.begin()[subscript]);
			addListItem(QString("%1摄像头大图,%2").arg(viewName[subscript], colorInfo.logInfo));
		}

		if (!success)
		{
			break;
		}
		msleep(1000);
		result = true;
	} while (false);
	restoreRectType();
	WRITE_LOG("检测前后视图,%s", OK_NG(result));
	addListItem(Q_SPRINTF("检测前后视图 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Avm::checkFRViewUseKey(int id, int value, CanProc proc, ulong hDelay)
{
	setCurrentStatus("检测前后视图");
	bool result = false, success = true;
	do
	{
		const QStringList viewName = { "前","后","左","右","中" };
		FncTypes::ColorInfo colorInfo = { 0 };
		//先检测后视图在检测前视图
		for (int i = 1; i >= 0; i--)
		{
			if (i == 0)
			{
				if (!checkKeyVoltage(hDelay, id, value, proc))
				{
					success = false;
					break;
				}
			}

			setRectType(static_cast<FncTypes::RectType>(i));
			msleep(500);

			memset(&colorInfo, 0, sizeof(colorInfo));
			if (!checkColor(m_defConfig->image.bigRect[i], &colorInfo))
			{
				success = false;
				setLastError("检测前后视图失败");
			}
			addListItem(QString("%1摄像头大图,%2").arg(viewName[i], colorInfo.logInfo));
		}

		if (!success)
		{
			break;
		}
		msleep(1000);
		result = true;
	} while (false);
	restoreRectType();
	WRITE_LOG("检测前后视图,%s", OK_NG(result));
	addListItem(Q_SPRINTF("检测前后视图 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Avm::checkKeyVoltage(ulong hDelay, int id, int value, CanProc proc, bool showLog)
{
	setCurrentStatus("检测按键电压");
	bool result = false, success = true;
	do
	{
		if (showLog)
		{
			addListItem("正在检测按键电压请耐心等待...");
		}
		KeyVoltageConfig& keyVol = m_hwdConfig->keyVoltage;
		RUN_BREAK(!m_voltmeter->getVoltage(keyVol.lRead), "读取电压表失败," + LC_TO_Q_STR(m_voltmeter->getLastError()));

		(keyVol.lRead >= keyVol.lLLimit) && (keyVol.lRead <= keyVol.lULimit) ? keyVol.lResult = true : keyVol.lResult = success = false;

		addListItem(Q_SPRINTF("按键低电平  %.3f  %s", keyVol.lRead, OK_NG(keyVol.lResult)));

		WRITE_LOG("按键低电平,%.3f", keyVol.lRead);

		if (!triggerAvmByKey(hDelay, id, value, proc, showLog))
		{
			break;
		}

		msleep(1500);

		RUN_BREAK(!m_voltmeter->getVoltage(keyVol.hRead), "读取电压表失败," + LC_TO_Q_STR(m_voltmeter->getLastError()));

		(keyVol.hRead >= keyVol.hLLimit) && (keyVol.hRead <= keyVol.hULimit) ? keyVol.hResult = true : keyVol.hResult = success = false;

		addListItem(Q_SPRINTF("按键高电平  %.3f  %s", keyVol.hRead, OK_NG(keyVol.hResult)));

		WRITE_LOG("按键高电平,%.3f", keyVol.hRead);

		RUN_BREAK(!success, "检测按键电压失败");
		result = true;
	} while (false);
	WRITE_LOG("检测按键电压,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测按键电压 %s", OK_NG(result)));
	return result;
}

bool Dt::Avm::checkKeyVoltage(const can::Msg& msg, int id, int value, CanProc proc, bool showLog)
{
	setCurrentStatus("检测按键电压");
	bool result = false, success = true;
	do
	{
		if (showLog)
		{
			addListItem("正在检测按键电压请耐心等待...");
		}
		KeyVoltageConfig& keyVol = m_hwdConfig->keyVoltage;
		RUN_BREAK(!m_voltmeter->getVoltage(keyVol.lRead), "读取电压表失败," + LC_TO_Q_STR(m_voltmeter->getLastError()));

		(keyVol.lRead >= keyVol.lLLimit) && (keyVol.lRead <= keyVol.lULimit) ? keyVol.lResult = true : keyVol.lResult = success = false;

		addListItem(Q_SPRINTF("按键低电平  %.3f  %s", keyVol.lRead, OK_NG(keyVol.lResult)));

		WRITE_LOG("按键低电平,%.3f", keyVol.lRead);

		if (!triggerAvmByMsg(msg, id, value, proc, showLog))
		{
			break;
		}

		msleep(1500);

		RUN_BREAK(!m_voltmeter->getVoltage(keyVol.hRead), "读取电压表失败," + LC_TO_Q_STR(m_voltmeter->getLastError()));

		(keyVol.hRead >= keyVol.hLLimit) && (keyVol.hRead <= keyVol.hULimit) ? keyVol.hResult = true : keyVol.hResult = success = false;

		addListItem(Q_SPRINTF("按键高电平  %.3f  %s", keyVol.hRead, OK_NG(keyVol.hResult)));

		WRITE_LOG("按键高电平,%.3f", keyVol.hRead);

		RUN_BREAK(!success, "检测按键电压失败");
		result = true;
	} while (false);

	WRITE_LOG("检测按键电压,%s", OK_NG(result));

	addListItemEx(Q_SPRINTF("检测按键电压 %s", OK_NG(result)));
	return result;
}

bool Dt::Avm::checkFastStartupImage(FncTypes::RectType rect, ulong duration)
{
	setCurrentStatus("检测快速启动");
	bool result = false, purity = true, color = false;
	do
	{
		addListItem("正在检测快速启动,请耐心等待...");
		int index = static_cast<int>(rect);
		if (index > static_cast<int>(FncTypes::RT_RIGHT_BIG))
		{
			setLastError("不支持的矩形类型");
			break;
		}

		int detect = m_defConfig->image.detectPurity;

		if (!detect)
			m_defConfig->image.detectPurity = 1;

		char colorName[TINY_BUFF] = { 0 };
		int purityValue = 0;
		setRectType(rect);
		auto startTime = GetTickCount64();
		FncTypes::ColorInfo colorInfo = { 0 };
		while (true)
		{
			memset(&colorInfo, 0, sizeof(colorInfo));

			checkColor(m_defConfig->image.bigRect[index], &colorInfo);

			if (colorInfo.color.result && !color)
			{
				strcpy_s(colorName, colorInfo.color.name);
				color = true;
			}

			if (!colorInfo.color.result && !color)
			{
				strcpy_s(colorName, colorInfo.color.name);
			}

			purityValue = colorInfo.purity.value;

			if (!colorInfo.purity.result)
			{
				purity = false;
				break;
			}

			if (GetTickCount64() - startTime > duration)
			{
				break;
			}
			msleep(30);
		}

		if (!detect)
			m_defConfig->image.detectPurity = 0;

		const char* view = "未知";
		switch (rect)
		{
		case FncTypes::RT_FRONT_BIG:
			view = "前视";
			break;
		case FncTypes::RT_REAR_BIG:
			view = "后视";
			break;
		case FncTypes::RT_LEFT_BIG:
			view = "左视";
			break;
		case FncTypes::RT_RIGHT_BIG:
			view = "右视";
			break;
		default:
			break;
		}
		restoreRectType();
		addListItem(Q_SPRINTF("快速启动%s,检测持续时间%02d秒,颜色 %s %s,色彩纯度 %03d %s",
			view, duration / 1000, colorName, OK_NG(color), purityValue, OK_NG(purity)));
		RUN_BREAK(!(color && purity), "检测快速启动失败");
		result = true;
	} while (false);
	addListItemEx(Q_SPRINTF("检测快速启动 %s", OK_NG(result)));
	WRITE_LOG("检测快速启动,%s", OK_NG(result));
	return result;
}

bool Dt::Avm::checkSmallImageColor()
{
	bool success = true;
	setRectType(FncTypes::RT_SMALL_ALL);
	msleep(2000);
	const QStringList viewName =
	{
		"前摄像头小图",
		"后摄像头小图",
		"左摄像头小图",
		"右摄像头小图",
		"中汽车模型图"
	};

	for (int i = 0; i < SMALL_RECT_; i++)
	{
		if (!m_defConfig->image.smallRect[i].enable)
		{
			continue;
		}

		FncTypes::ColorInfo colorInfo = { 0 };
		if (!checkColor(m_defConfig->image.smallRect[i], &colorInfo))
		{
			success = false;
		}
		addListItem(QString("%1,%2").arg(viewName.at(i), colorInfo.logInfo));
	}
	restoreRectType();
	return success;
}


Dt::Dvr::Dvr(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Dvr);
	m_detectionType = BaseTypes::DetectionType::DT_DVR;
}

Dt::Dvr::~Dvr()
{
	PRINT_CON_DESTRUCTION(Dt::Dvr);
	utility::killProcess("win32_demo.exe");
}

bool Dt::Dvr::initialize()
{
	bool result = false;
	do
	{
		if (!Dt::Function::initialize())
		{
			break;
		}

		m_hashCode.systemStatus = typeid(DvrTypes::SystemStatus).hash_code();

		m_hashCode.wifiStatus = typeid(DvrTypes::WifiStatus).hash_code();

		m_hashCode.ethernetStatus = typeid(DvrTypes::EthernetStatus).hash_code();

		m_hashCode.sdCardStatus = typeid(DvrTypes::SdCardStatus).hash_code();

		RUN_BREAK(!m_sfrServer.startListen(), m_sfrServer.getLastError());

		m_dvrClient.setAddressPort(Q_TO_C_STR(m_address), m_port);

		utility::killProcess("win32_demo.exe");

		const QString appName = "App\\sfr_client\\bin\\win32_demo.exe";
		if (utility::existMySdkPath())
		{
			RUN_BREAK(!utility::startProcess(QString("%1\\%2").arg(MY_SDK_PATH).arg(appName), QString(),
				SW_NORMAL, true), QString("启动%1应用程序失败").arg(appName));
		}
		else
		{
			RUN_BREAK(!utility::startProcess(appName, QString(), SW_NORMAL), QString("启动%1应用程序失败").arg(appName));
		}

		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::prepareTest()
{
	bool result = false, success = false;
	do
	{
		if (!Dt::Base::prepareTest())
		{
			break;
		}

		addListItem("正在检测系统状态,请耐心等待...");
		const uint startTime = GetTickCount64();
		success = autoProcessStatus(m_systemStatus, START_DELAY);
		addListItem(Q_SPRINTF("检测系统状态%s,用时:%.3f秒", OK_NG(success), float(GetTickCount64() - startTime) / 1000.000f));
		RUN_BREAK(!success, "系统初始化失败," + getLastError());

		success = autoProcessStatus(m_sdCardStatus, START_DELAY);
		addListItem(Q_SPRINTF("SD卡状态 %s", OK_NG(success)));
		RUN_BREAK(!success, "SD卡初始化失败," + getLastError());

		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::finishTest(bool success)
{
	bool result = false;
	do
	{
		if (m_wifiMgr.connected() && !m_wifiMgr.disconnect())
		{
			addListItem(LC_TO_Q_STR(m_wifiMgr.getLastError()));
		}

		if (!Dt::Base::finishTest(success))
		{
			break;
		}

		if (getSoundLigth())
		{
			RUN_BREAK(!setSoundLight(false), "继电器断开失败,请检查连接");
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::getWifiInfo(bool rawData, bool showLog)
{
	setLastError("子类未重写虚函数bool Dt::Dvr::getWifiInfo(bool,bool)");
	return false;
}

void Dt::Dvr::setSdCardStatus(DvrTypes::SdCardStatus status)
{
	m_sdCardStatus = status;
}

void Dt::Dvr::setSystemStatus(DvrTypes::SystemStatus status)
{
	m_systemStatus = status;
}

bool Dt::Dvr::checkDvr(const QString& rtspUrl, bool useWifi, bool useCard, bool downloadVideo)
{
	setCurrentStatus("检测DVR");
	bool result = false, record = false, success = false;
	do
	{
		if (useCard)
		{
			addListItem("检测采集卡出画");
			success = setQuestionBoxEx("提示", "采集卡出画是否成功?", QPoint(80, 0));
			addListItem(Q_SPRINTF("检测采集卡出画 %s", OK_NG(success)));
			WRITE_LOG("采集卡出画,%s", OK_NG(success));
			RUN_BREAK(!success, "检测采集卡出画失败");
		}

		addListItem("正在检测网络状态,请耐心等待...");
		uint startTime = GetTickCount64();
		success = useWifi ? autoProcessStatus<DvrTypes::WifiStatus>() :
			autoProcessStatus<DvrTypes::EthernetStatus>();
		addListItem(Q_SPRINTF("检测网络状态%s,用时:%.3f秒", success ? "正常" : "异常",
			float(GetTickCount64() - startTime) / 1000.00f));
		RUN_BREAK(!success, "网络状态异常");

		addListItem("检测网络出画");
		int repalyCount = 1;
	replay1:
		success = vlcRtspStart(rtspUrl);
		RUN_BREAK(!success, "RTSP协议出画失败");

		int play = setPlayQuestionBox("提示", "网络出画是否成功?", QPoint(130, 0));
		if (play == DvrTypes::PR_OK) success = true;
		else if (play == DvrTypes::PR_NG) success = false;
		else
		{
			vlcRtspStop();
			msleep(1000);
			addListItem(Q_SPRINTF("网络出画重播第%d次", repalyCount));
			if (repalyCount++ >= 5) success = false; else goto replay1;
		}
		vlcRtspStop();
		addListItem(Q_SPRINTF("检测网络出画 %s", OK_NG(success)));
		WRITE_LOG("网络出画,%s", OK_NG(success));
		RUN_BREAK(!success, "网络出画失败");

		if (getSoundLigth())
		{
			RUN_BREAK(!setSoundLight(false), "关闭音响和灯光失败");
		}

		addListItem("获取紧急录制文件路径");
		QString url;
		success = getFileUrl(url, DvrTypes::FP_EVT);
		addListItem(QString("获取紧急录制文件路径:%1").arg(success ? url : "无效路径"));
		RUN_BREAK(!success, "获取紧急录制文件路径失败,\n" + getLastError());

		if (downloadVideo)
		{
			addListItem("下载紧急录制文件,大约需要10~30秒,请等待...");
			success = downloadFile(url, DvrTypes::FileType::FT_VIDEO);
			addListItem(Q_SPRINTF("下载紧急录制文件 %s", OK_NG(success)));
			RUN_BREAK(!success, "下载紧急录制文件失败," + getLastError());
		}

		msleep(1000);

		addListItem("播放紧急录制视频中...");
		repalyCount = 1;
	replay2:
		success = vlcRtspStart(url);
		RUN_BREAK(!success, "播放紧急录制视频失败");
		//success = setQuestionBoxEx("提示", "紧急录制是否回放?", QPoint(80, 0));
		play = setPlayQuestionBox("提示", "紧急录制视频是否回放?", QPoint(130, 0));
		if (play == DvrTypes::PR_OK) success = true;
		else if (play == DvrTypes::PR_NG) success = false;
		else
		{
			vlcRtspStop();
			msleep(1000);
			addListItem(Q_SPRINTF("紧急录制视频重播第%d次", repalyCount));
			if (repalyCount++ >= 5) success = false; else goto replay2;
		}
		addListItem(Q_SPRINTF("紧急录制视频回放 %s", OK_NG(success)));
		vlcRtspStop();
		WRITE_LOG("紧急录制,%s", OK_NG(success));
		RUN_BREAK(!success, "紧急录制视频回放失败");
		result = true;
	} while (false);
	addListItem(Q_SPRINTF("检测DVR %s", OK_NG(result)), false);
	return result;
}

bool Dt::Dvr::checkDvr(bool useWifi, bool useCard, bool downloadVideo)
{
	return checkDvr(QString("rtsp://%1/stream2").arg(m_address), useWifi, useCard, downloadVideo);
}

bool Dt::Dvr::setSoundLight(bool enable)
{
	bool result = false;
	do
	{
		if (!m_relay->setOne(m_defConfig->relay.horseRaceLamp, enable))
		{
			setLastError(Q_SPRINTF("%s继电器端口跑马灯失败", enable ? "打开" : "关闭"));
			break;
		}
		msleep(300);

		if (!m_relay->setOne(m_defConfig->relay.soundBox, true))
		{
			setLastError("打开继电器端口音箱失败");
			break;
		}
		msleep(150);

		if (!m_relay->setOne(m_defConfig->relay.soundBox, false))
		{
			setLastError("关闭继电器端口音箱失败");
			break;
		}
		msleep(300);
		m_soundLight = enable;
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::getSoundLigth() const
{
	return m_soundLight;
}

bool Dt::Dvr::setSound(bool enable)
{
	bool result = false;
	do
	{
		if (!m_relay->setOne(m_defConfig->relay.soundBox, true))
		{
			setLastError("打开继电器端口音箱失败");
			break;
		}
		msleep(150);

		if (!m_relay->setOne(m_defConfig->relay.soundBox, false))
		{
			setLastError("关闭继电器端口音箱失败");
			break;
		}
		msleep(300);

		m_soundLight = enable;
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::setLight(bool enable)
{
	bool result = false;
	do
	{
		if (!m_relay->setOne(m_defConfig->relay.horseRaceLamp, enable))
		{
			setLastError(Q_SPRINTF("%s继电器端口跑马灯失败", enable ? "打开" : "关闭"));
			break;
		}
		msleep(300);
		result = true;
	} while (false);
	return result;
}

void Dt::Dvr::setVlcMediaHwnd(HWND vlcHwnd)
{
	m_vlcHwnd = vlcHwnd;
}

HWND Dt::Dvr::getVlcMediaHwnd() const
{
	return m_vlcHwnd;
}

bool Dt::Dvr::vlcRtspStart(const QString& url)
{
	bool result = false;
	do
	{
		RUN_BREAK(url.isEmpty(), "RTSP协议地址为空");

		RUN_BREAK(!m_vlcHwnd, "请调用setVlcMediaHwnd设置播放控件句柄");

		const char* const vlcArgs[] =
		{
			//"--rtsp-frame-buffer-size=1000000",
			"--ipv4",
			"--no-prefer-system-codecs",
			"--rtsp-caching=300",
			"--network-caching=500",
			"--rtsp-tcp"
		};

		if (!m_vlcInstance)
		{
			m_vlcInstance = libvlc_new(sizeof(vlcArgs) / sizeof(*vlcArgs), vlcArgs);
			RUN_BREAK(!m_vlcInstance, "创建vlc实例失败,请确认文件内是否包含\n[lua ,plugins ,libvlc.dll ,libvlccore.dll]");
		}

		if (!m_vlcMedia)
		{
			m_vlcMedia = libvlc_media_new_location(m_vlcInstance, url.toStdString().c_str());
			RUN_BREAK(!m_vlcMedia, "创建vlc媒体失败");
		}

		if (!m_vlcMediaPlayer)
		{
			m_vlcMediaPlayer = libvlc_media_player_new_from_media(m_vlcMedia);
			RUN_BREAK(!m_vlcMediaPlayer, "创建vlc媒体播放器失败");
		}

		libvlc_media_player_set_hwnd(m_vlcMediaPlayer, m_vlcHwnd);

		RUN_BREAK(libvlc_media_player_play(m_vlcMediaPlayer) == -1, "VLC媒体播放器播放视频失败");

		libvlc_media_player_set_time(m_vlcMediaPlayer, (libvlc_time_t)m_defConfig->threshold.playStartTime);
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::vlcRtspStop()
{
	bool result = false;
	do
	{
		if (m_vlcInstance)
		{
			libvlc_release(m_vlcInstance);
			m_vlcInstance = nullptr;
		}

		if (m_vlcMedia)
		{
			libvlc_media_release(m_vlcMedia);
			m_vlcMedia = nullptr;
		}

		if (m_vlcMediaPlayer)
		{
			libvlc_media_player_stop(m_vlcMediaPlayer);
			libvlc_media_player_release(m_vlcMediaPlayer);
			m_vlcMediaPlayer = nullptr;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::vlcLocalStart(const QString& fileName)
{
	bool result = false;
	do
	{
		RUN_BREAK(fileName.isEmpty(), "本地路径为空");

		RUN_BREAK(!m_vlcHwnd, "请调用setVlcMediaHwnd设置播放控件句柄");

		const char* const vlcArgs[] =
		{
			"-I",
			"dummy",
			"--ignore-config",
			//"--extraintf=logger",
			"--verbose=2"
		};

		if (!m_vlcInstance)
		{
			m_vlcInstance = libvlc_new(sizeof(vlcArgs) / sizeof(*vlcArgs), vlcArgs);
			RUN_BREAK(!m_vlcInstance, "创建vlc实例失败,请确认文件内是否包含\n[lua ,plugins ,libvlc.dll ,libvlccore.dll]");
		}

		if (!m_vlcMedia)
		{
			m_vlcMedia = libvlc_media_new_path(m_vlcInstance, Q_TO_LC_STR(fileName));
			RUN_BREAK(!m_vlcMedia, "创建vlc媒体失败");
			libvlc_media_parse(m_vlcMedia);
		}

		if (!m_vlcMediaPlayer)
		{
			m_vlcMediaPlayer = libvlc_media_player_new(m_vlcInstance);
			RUN_BREAK(!m_vlcMediaPlayer, "创建vlc媒体播放器失败");
			libvlc_media_player_set_media(m_vlcMediaPlayer, m_vlcMedia);
		}

		libvlc_media_player_set_hwnd(m_vlcMediaPlayer, m_vlcHwnd);

		RUN_BREAK(libvlc_media_player_play(m_vlcMediaPlayer) == -1, "VLC媒体播放器播放视频失败");

		libvlc_media_player_set_time(m_vlcMediaPlayer, (libvlc_time_t)m_defConfig->threshold.playStartTime);
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::vlcLocalStop()
{
	return vlcRtspStop();
}

bool Dt::Dvr::getFileUrl(QString& url, DvrTypes::FilePath filePath)
{
	bool result = false;
	do
	{
		const char* dirName = (filePath == DvrTypes::FP_PHO ? "照片" : "视频");
		msleep(1000);
		char recvData[BUFF_SIZE] = { 0 };
		int recvLen = 0, tryAgainCount = 0;
		RUN_BREAK(!m_dvrClient.connect(), LC_TO_Q_STR(m_dvrClient.getLastError()));

	tryAgain:
		DEBUG_INFO_EX("发送获取%s文件列表报文", dirName);
		RUN_BREAK(!m_dvrClient.sendFrameDataEx({ (char)filePath, (char)filePath, 0x01, 0x01 },
			DvrTypes::NC_FILE_CONTROL, DvrTypes::NS_GET_FILE_LIST), LC_TO_Q_STR(m_dvrClient.getLastError()));

		memset(recvData, 0x00, BUFF_SIZE);
		RUN_BREAK(!m_dvrClient.recvFrameDataEx(recvData, &recvLen, DvrTypes::NC_FILE_CONTROL,
			DvrTypes::NS_GET_FILE_LIST), LC_TO_Q_STR(m_dvrClient.getLastError()));


		FileList dvrFileList = { 0 };
		memcpy(&dvrFileList.listCount, &recvData[2], sizeof(uint));

		dvrFileList.listCount = dvrFileList.listCount > 100 ? 100 : dvrFileList.listCount;

		DEBUG_INFO_EX("%s文件列表数量:%lu", dirName, dvrFileList.listCount);

		/*存在没有获取到情况,所以再次获取,原因可能是??未知*/
		if (dvrFileList.listCount == 0)
		{
			tryAgainCount++;
			if (tryAgainCount >= 15)
			{
				writeNetLog(filePath == DvrTypes::FP_EVT ? "getEvtUrl" : "getPhoUrl", recvData, recvLen);
				setLastError(Q_SPRINTF("%s文件列表为空,超过重试%d次,\n请确认SD卡中是否存在文件", dirName, tryAgainCount));
				break;
			}
			msleep(1000);
			goto tryAgain;
		}

		const char* pointer = &recvData[6];
		const char* dvrPath[] = { "NOR/", "EVT/", "PHO/" };
		const char* dvrType[] = { "NOR_", "EVT_", "PHO_", "_D1_" };
		const char* dvrSuffix[] = { ".mp4", ".jpg" };

		int maxDate = 0, flag = 0;

		for (int i = 0; i < dvrFileList.listCount; i++)
		{
			memcpy(&dvrFileList.fileInfo[i].index, pointer, 2);
			pointer += 2;
			memcpy(&dvrFileList.fileInfo[i].path, pointer, 1);
			pointer++;
			memcpy(&dvrFileList.fileInfo[i].type, pointer, 1);
			pointer++;
			memcpy(&dvrFileList.fileInfo[i].suffix, pointer, 1);
			pointer += 4;
			memcpy(&dvrFileList.fileInfo[i].size, pointer, 4);
			pointer += 4;
			memcpy(&dvrFileList.fileInfo[i].date, pointer, 4);
			pointer += 4;

			if (dvrFileList.fileInfo[i].date >= maxDate)
			{
				maxDate = dvrFileList.fileInfo[i].date;
				flag = i;
			}
		}

		int pathId = dvrFileList.fileInfo[flag].path;
		int typeId = dvrFileList.fileInfo[flag].type;

		RUN_BREAK((pathId < 0 || pathId > 2) || (typeId < 0 || typeId > 3),
			"获取DVR文件列表数据包异常,\n请检测网络连接是否有波动");

		url.sprintf("http://%s:%d/%s%s", Q_TO_C_STR(m_address), 8080, dvrPath[pathId], dvrType[typeId]);
		/*此处要减去时差*/
		time_t dvrSecond = dvrFileList.fileInfo[flag].date - 8 * 60 * 60;

		/*通过localtime将秒数转换为 年 月 日 时 分 秒*/
		struct DETECTION_DLL_EXPORT tm* dvrDate = localtime(&dvrSecond);
		RUN_BREAK(!dvrDate, "localtime触发一个nullptr异常");

		url.append(Q_SPRINTF("%04d%02d%02d_%02d%02d%02d_%05d",
			dvrDate->tm_year + 1900,
			dvrDate->tm_mon + 1,
			dvrDate->tm_mday,
			dvrDate->tm_hour,
			dvrDate->tm_min,
			dvrDate->tm_sec,
			dvrFileList.fileInfo[flag].index));
		url.append(dvrSuffix[dvrFileList.fileInfo[flag].suffix]);
		result = true;
	} while (false);
	m_dvrClient.disconnect();
	return result;
}

bool Dt::Dvr::downloadFile(const QString& url, bool isVideo)
{
	bool result = false, success = true;
	do
	{
		const QString dirName = isVideo ? "Video" : "Image";
		const QString path = QString("%1\\%2").arg(getLogPath(true), dirName);
		RUN_BREAK(!utility::makePath(path), utility::getLastError());

		const QString destFile = path + url.mid(url.lastIndexOf("/"));
		DeleteUrlCacheEntryW(Q_TO_WC_STR(url));
		const ulong startDownloadTime = GetTickCount64();
		const HRESULT downloadResult = URLDownloadToFileW(NULL, Q_TO_WC_STR(url), Q_TO_WC_STR(destFile), NULL, NULL);
		const float endDownloadTime = (GetTickCount64() - startDownloadTime) / 1000.0f;
		RUN_BREAK(downloadResult != S_OK, "URLDownloadToFile下载文件失败");

		struct DETECTION_DLL_EXPORT _stat64i32 stat = { 0 };
		_stat64i32(Q_TO_C_STR(destFile), &stat);
		const float fileSize = stat.st_size / 1024.0f / 1024.0f;
		const float networkSpeed = fileSize / endDownloadTime;
		QString downloadInfo = Q_SPRINTF("文件大小:%.2fMB,下载用时:%.2f秒,平均速度:%.2fM/秒", fileSize, endDownloadTime, networkSpeed);
		/*视频下载需要做网速处理*/
		if (isVideo)
		{
			auto& range = m_defConfig->range;
			success = (networkSpeed >= range.minNetworkSpeed && networkSpeed <= range.maxNetworkSpeed);
			downloadInfo.append(Q_SPRINTF(",网速范围:%.2fM~%.2fM %s", range.minNetworkSpeed, range.maxNetworkSpeed, OK_NG(success)));
			WRITE_LOG("网速,%.2f", networkSpeed);
		}
		addListItem(downloadInfo);
		RUN_BREAK(!success, "网速阈值不在范围之内");
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::downloadFile(const QString& url, DvrTypes::FileType types)
{
	//return downloadFile(url, types == DvrTypes::FT_VIDEO);
	bool result = false, success = true;
	do
	{
		int tryAgainCount = 1;
	tryAgain:
		const bool isVideo = (types == DvrTypes::FT_VIDEO);
		BaseTypes::DownloadInfo info;
		info.title.sprintf("下载DVR%s", isVideo ? "紧急录制视频" : "照片");
		info.url = url;
		info.path = QString("%1").arg(isVideo ? "Log\\Video" : "Log\\Image");
		//RUN_BREAK(!setDownloadDlg(&info), info.error);
		if (!setDownloadDlg(&info)/* && info.error == "Connection closed"*/)
		{
			addListItem(Q_SPRINTF("网络连接不稳定下载失败%s,重试第%d次", Q_TO_C_STR(info.error), tryAgainCount));
			RUN_BREAK(tryAgainCount++ >= 3, info.error);
			msleep(100);
			goto tryAgain;
		}

		info.speed /= 1024;
		QString downloadInfo = Q_SPRINTF("文件大小:%.2fMB,下载用时:%.2f秒,平均速度:%.2fM/秒", info.size, float(info.time) / 1000.00f, info.speed);
		/*视频下载需要做网速处理*/
		if (isVideo)
		{
			const auto& range = m_defConfig->range;
			success = (info.speed >= range.minNetworkSpeed && info.speed <= range.maxNetworkSpeed);
			downloadInfo.append(Q_SPRINTF(",网速范围:%.2fM~%.2fM %s", range.minNetworkSpeed, range.maxNetworkSpeed, OK_NG(success)));
			WRITE_LOG("网速,%.2f", info.speed);
		}
		addListItem(downloadInfo);
		RUN_BREAK(!success, "网速阈值不在范围之内");
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::checkRayAxis(const QString& url)
{
	setCurrentStatus("检测光轴");
	bool result = false;
	do
	{
		const QString localPath = utility::file::isFile(url) ? url :
			QString("%1\\Image\\%2").arg(getLogPath(true), utility::getFileNameByUrl(url));

		DEBUG_INFO() << "照片路径 " << localPath << endl;
		QByteArray byte = localPath.toLocal8Bit();
		cv::Mat mat = cv::imread(byte.constData(), 0);
		RUN_BREAK(mat.empty(), "加载图像失败");

		grayBuffer_t grayBuffer = { 0 };
		grayBuffer.buffer = mat.data;
		grayBuffer.height = mat.rows;
		grayBuffer.width = mat.cols;

		threshold_t threshold = { 0 };
		threshold.xAxis = 100;
		threshold.yAxis = 100;

		axisStandard_t axisStandard = { 0 };
		axisStandard.height = 40;
		axisStandard.width = 120;
		axisStandard.x = 100;
		axisStandard.y = 100;

		cross_t cross = { 0 };
		cross = calculateCross(&grayBuffer, &threshold, &axisStandard);

		RUN_BREAK(cross.iResult != 0, "计算光轴失败");

		auto& range = m_defConfig->range;

		bool tsX = false, tsY = false, tsA = false;
		tsX = (cross.x >= range.minRayAxisX && cross.x <= range.maxRayAxisX) ? true : result = false;
		addListItem(Q_SPRINTF("光轴X:%.2f,范围:%.2f~%.2f %s", cross.x, range.minRayAxisX, range.maxRayAxisX, tsX ? "OK" : "NG"));
		WRITE_LOG("光轴X,%.2f", cross.x);

		tsY = (cross.y >= range.minRayAxisY && cross.y <= range.maxRayAxisY) ? true : result = false;
		addListItem(Q_SPRINTF("光轴Y:%.2f,范围:%.2f~%.2f %s", cross.y, range.minRayAxisY, range.maxRayAxisY, tsY ? "OK" : "NG"));
		WRITE_LOG("光轴Y,%.2f", cross.y);

		tsA = (cross.angle >= range.minRayAxisA && cross.angle <= range.maxRayAxisA) ? true : result = false;
		addListItem(Q_SPRINTF("光轴角度:%.2f,范围:%.2f~%.2f %s", cross.angle, range.minRayAxisA, range.maxRayAxisA, tsA ? "OK" : "NG"));
		WRITE_LOG("光轴角度,%.2f", cross.angle);

		QString errorInfo;
		if (!tsX) errorInfo.append("光轴X,");
		if (!tsY) errorInfo.append("光轴Y,");
		if (!tsA) errorInfo.append("光轴角度,");
		RUN_BREAK(!tsX || !tsY || !tsA, errorInfo.append("阈值不在设定范围之内"));
		result = true;
	} while (false);
	addListItem(Q_SPRINTF("检测光轴 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Dvr::checkSfr(const QString& url)
{
	setCurrentStatus("检测解像度");
	bool result = false;
	do
	{
		const QString localPath = utility::file::isFile(url) ? url :
			QString("%1\\Image\\%2").arg(getLogPath(true), utility::getFileNameByUrl(url));
		DEBUG_INFO() << "照片路径 " << localPath << endl;

		QByteArray byte = localPath.toLocal8Bit();
		cv::Mat mat = cv::imread(byte.constData());
		RUN_BREAK(mat.empty(), "加载图像失败");

		QString destFile = localPath;
		destFile.replace("." + QFileInfo(destFile).suffix(), ".bmp");
		DEBUG_INFO() << "转换照片路径 " << destFile << endl;

		byte = destFile.toLocal8Bit();
		RUN_BREAK(!cv::imwrite(byte.constData(), mat), "转换图像保存失败");

		float value = 0.0f;
		RUN_BREAK(!m_sfrServer.getSfrValue(byte.constData(), value), m_sfrServer.getLastError());
		auto& range = m_defConfig->range;
		bool success = ((value >= range.minSfr) && (value <= range.maxSfr));
		addListItem(Q_SPRINTF("图像解像度:%.2f,范围:%.2f~%.2f %s", value, range.minSfr, range.maxSfr, OK_NG(success)));
		WRITE_LOG("解像度,%.2f", value);
		RUN_BREAK(!success, "解像度阈值不在范围之内");

		result = true;
	} while (false);
	addListItem(Q_SPRINTF("检测解像度 %s", OK_NG(result)), false);
	return result;
}

bool Dt::Dvr::checkRecordUseMsg(const can::Msg& msg, int id, int value, CanProc proc, ulong timeout)
{
	setCurrentStatus("检测紧急录制");
	bool result = false, success = false;
	do
	{
		addListItem("检测紧急录制中,请耐心等待...");
		RUN_BREAK(!setSoundLight(true), "打开音响和灯光失败");

		sendCanMsg(msg);
		success = autoProcessCanMsg(id, value, proc, timeout);
		addListItem(Q_SPRINTF("触发紧急录制 %s", OK_NG(success)));
		RUN_BREAK(!success, "触发紧急录制失败," + getLastError());
		result = true;
	} while (false);
	WRITE_LOG("检测紧急录制,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测紧急录制 %s", OK_NG(result)));
	return result;
}

bool Dt::Dvr::checkRecordUseMsg(int id0, int period, int start, int length, int data, can::SendType type,
	int count, int id1, int value, CanProc proc)
{
	return checkRecordUseMsg(generateCanMsg(id0, period, start, length, data, type, count), id1, value, proc);
}

bool Dt::Dvr::checkRecordUseMsg(int id0, int period, int start0, int length0, int data,
	can::SendType type, int count, int id1, int value, int start1, int length1)
{
	return checkRecordUseMsg(id0, period, start0, length0, data, type, count, id1, value,
		CAN_PROC_FNC(&){ return unpackCanMsg(FMSG, start1, length1) == FVAL; });
}

bool Dt::Dvr::checkRecordUseKey(int id, int value, CanProc proc, ulong time)
{
	setCurrentStatus("检测紧急录制");
	bool result = false, success = false;
	do
	{
		addListItem("检测紧急录制中,请耐心等待...");
		RUN_BREAK(!setSoundLight(true), "打开音响和灯光失败");

		RUN_BREAK(!m_relay->simulateKeystroke(m_defConfig->relay.keyLine, time), "继电器按键线闭合断开失败");

		success = autoProcessCanMsg(id, value, proc);
		addListItem(Q_SPRINTF("触发紧急录制 %s", OK_NG(success)));
		RUN_BREAK(!success, "触发紧急录制失败");
		result = true;
	} while (false);
	WRITE_LOG("检测紧急录制,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("检测紧急录制 %s", OK_NG(result)));
	return result;
}

bool Dt::Dvr::checkRecordUseKey(int id, int value, int start, int length, ulong time)
{
	return checkRecordUseKey(id, value, CAN_PROC_FNC(&){ return unpackCanMsg(FMSG, start, length) == FVAL; }, time);
}

bool Dt::Dvr::checkRayAxisSfrUseMsgEx(CanList list, int id, int value, CanProc proc)
{
	bool result = false;
	do
	{
		if (!setCanProcessFncEx("报文拍照", list, id, value, proc) || !checkRayAxisSfr())
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::checkRayAxisSfrUseMsg(can::Msg msg, int id, int value, CanProc proc)
{
	return checkRayAxisSfrUseMsgEx({ msg }, id, value, proc);
}

bool Dt::Dvr::checkRayAxisSfrUseMsg(int id0, int period, int start, int length, int data,
	can::SendType type, int count, int id1, int value, CanProc proc)
{
	return checkRayAxisSfrUseMsg(generateCanMsg(id0, period, start, length, data, type, count),
		id1, value, proc);
}

bool Dt::Dvr::checkRayAxisSfrUseMsg(int id0, int period, int start0, int length0, int data,
	can::SendType type, int count, int id1, int value, int start1, int length1)
{
	return checkRayAxisSfrUseMsg(generateCanMsg(id0, period, start0, length0, data, type, count),
		id1, value, CAN_PROC_FNC(&){ return unpackCanMsg(FMSG, start1, length1) == FVAL; });
}

bool Dt::Dvr::checkRayAxisSfrUseNet()
{
	bool result = false;
	do
	{
		if (!networkPhotoGraph() || !checkRayAxisSfr())
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::checkRayAxisSfrUseKey(int id, int value, CanProc proc, ulong time)
{
	bool result = false;
	do
	{
		RUN_BREAK(!m_relay->simulateKeystroke(m_defConfig->relay.keyLine, time), "继电器按键线闭合断开失败");

		RUN_BREAK(!autoProcessCanMsg(id, value, proc), "按键线触发拍照失败," + getLastError());

		if (!checkRayAxisSfr())
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::checkRayAxisSfrUseKey(int id, int value, int start, int length, ulong time)
{
	return checkRayAxisSfrUseKey(id, value, CAN_PROC_FNC(&){ return unpackCanMsg(FMSG, start, length) == FVAL; }, time);
}

bool Dt::Dvr::checkRayAxisSfr(bool local)
{
	bool result = false, success = false;
	do
	{
		QString url;
		addListItem("获取照片文件路径");
		if (local)
		{
			success = searchDiskFile(DvrTypes::FP_PHO, url);
			addListItem(QString("获取照片文件路径:%1").arg(success ? url : "无效的路径"));
			RUN_BREAK(!success, "获取照片文件路径失败,\n" + getLastError());
		}
		else
		{
			success = getFileUrl(url, DvrTypes::FP_PHO);
			addListItem(QString("获取照片文件路径:%1").arg(success ? url : "无效的路径"));
			RUN_BREAK(!success, "获取照片文件路径失败,\n" + getLastError());

			addListItem("下载DVR照片,请耐心等待...");
			success = downloadFile(url, DvrTypes::FileType::FT_PHOTO);
			addListItem(Q_SPRINTF("下载DVR照片 %s", OK_NG(success)));
			RUN_BREAK(!success, "下载DVR照片失败,\n" + getLastError());
		}

		addListItem("检测光轴");
		success = checkRayAxis(url);
		addListItem(Q_SPRINTF("检测光轴 %s", OK_NG(success)));
		RUN_BREAK(!success, "检测光轴失败,\n" + getLastError());

		addListItem("检测解像度");
		success = checkSfr(url);
		addListItem(Q_SPRINTF("检测解像度 %s", OK_NG(success)));
		RUN_BREAK(!success, "检测解像度失败,\n" + getLastError());
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::searchDiskFile(DvrTypes::FilePath path, QString& file, int timeout)
{
	bool result = false, success = false, notFound = false;
	do
	{
		if (path != DvrTypes::FP_EVT && path != DvrTypes::FP_PHO)
		{
			setLastError("仅支持搜索EVT和PHO文件夹");
			break;
		}

		const QString dir = path == DvrTypes::FP_EVT ? "EVT" : "PHO";
		const QString suffix = path == DvrTypes::FP_EVT ? ".mp4" : ".jpg";
		const auto startTime = GetTickCount64();
		while (true)
		{
			QStringList volume;
			utility::getRemovableDiskVolume(volume);

			for (const auto& x : volume)
			{
				QStringList data;
				utility::traversalDirectory(x, data, false);
				for (const auto& y : data)
				{
					if (utility::file::isDir(y) && y.contains(dir))
					{
						QStringList fileList = utility::getFileListBySuffixName(y, { suffix });
						if (!fileList.size())
						{
							notFound = true;
							break;
						}
						file = fileList.last();
						success = true;
						break;
					}
				}

				if (success || notFound)
				{
					break;
				}
			}

			if (success || notFound)
			{
				break;
			}

			if (GetTickCount64() - startTime > timeout)
			{
				success = false;
				break;
			}
			msleep(100);
		}

		RUN_BREAK(notFound, QString("没有找到%1\\%2文件").arg(dir, suffix));

		RUN_BREAK(!success, QString("搜索%1\\%2文件超时").arg(dir, suffix));
		result = true;
	} while (false);
	return result;
}

bool Dt::Dvr::formatSdCard(bool netFormat, bool ignoreResult, const QString& volume)
{
	setCurrentStatus("格式化SD卡");
	addListItem("正在进行格式化SD卡,请耐心等待...");
	bool result = false, success = false;
	if (netFormat)
	{
		int tryAgainCount = 0;
	tryAgain:
		do
		{
			msleep(1000);
			char data[BUFF_SIZE] = { 0 };
			int len = 0;
			RUN_BREAK(!m_dvrClient.connect(), LC_TO_Q_STR(m_dvrClient.getLastError()));
			DEBUG_INFO() << "发送暂停循环录制报文";
			addListItem("发送暂停循环录制报文");
			RUN_BREAK(!m_dvrClient.sendFrameDataEx({ 0x00 }, 0x02, 0x00), LC_TO_Q_STR(m_dvrClient.getLastError()));
			RUN_BREAK(!m_dvrClient.recvFrameDataEx(data, &len, DvrTypes::NC_FAST_CONTROL,
				DvrTypes::NS_FAST_CYCLE_RECORD), LC_TO_Q_STR(m_dvrClient.getLastError()));
			addListItem(Q_SPRINTF("暂停循环录制 %s", OK_NG(*(uint*)&data[2] == 0)));
			//writeNetLog("pauseRecord", data, len, *(int*)&data[2] == 0);
			RUN_BREAK(*(uint*)&data[2], Q_SPRINTF("暂停循环录制失败,操作错误代码:0x%X", *(uint*)&data[2]));
			msleep(1000);
			memset(data, 0x00, BUFF_SIZE);
			DEBUG_INFO() << "发送格式化SD卡报文";
			addListItem("发送格式化SD卡报文");
			RUN_BREAK(!m_dvrClient.sendFrameDataEx({ }, 0x12, 0x20), LC_TO_Q_STR(m_dvrClient.getLastError()));
			RUN_BREAK(!m_dvrClient.recvFrameDataEx(data, &len, 0x12, 0x20), LC_TO_Q_STR(m_dvrClient.getLastError()));
			//writeNetLog("formatSDCard", data, len, *(int*)&data[2] == 0);
			if (!ignoreResult)
			{
				RUN_BREAK(*(uint*)&data[2], Q_SPRINTF("格式化SD卡失败,操作错误代码:0x%X", *(uint*)&data[2]));
			}
			result = true;
		} while (false);
		m_dvrClient.disconnect();
		if (!result && ++tryAgainCount <= 3 && !ignoreResult)
		{
			addListItem(Q_SPRINTF("操作失败,重试第%d次", tryAgainCount));
			msleep(1000);
			goto tryAgain;
		}
	}
	else
	{
		do
		{
			if (!volume.isEmpty())
			{
				m_volume = volume;
			}
			success = utility::formatDisk(m_volume, "fat32", 1024);
			RUN_BREAK(!success, "格式化SD卡失败," + utility::getLastError());
			result = true;
		} while (false);
	}
	addListItemEx(Q_SPRINTF("格式化SD卡 %s", OK_NG(result)));
	WRITE_LOG("格式化SD卡,%s", OK_NG(result));
	return result;
}

bool Dt::Dvr::checkNetworkVersion(const QString& socJsonNode, const QString& mcuJsonNode, const QString& hwdJsonNode)
{
	setCurrentStatus("检测网络版本");
	bool result = false;
	do
	{
		char data[BUFF_SIZE] = { 0 };
		int len = 0;
		RUN_BREAK(!m_dvrClient.connect(), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(!m_dvrClient.sendFrameDataEx({ 0 }, 0x11, 0x20), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(!m_dvrClient.recvFrameDataEx(data, &len, 0x11, 0x20), LC_TO_Q_STR(m_dvrClient.getLastError()));

		auto v1 = UTIL_JSON->getOtherConfig1Value(socJsonNode);
		auto v2 = UTIL_JSON->getOtherConfig1Value(mcuJsonNode);
		auto v3 = UTIL_JSON->getOtherConfig1Value(hwdJsonNode);

		auto d1 = LC_TO_Q_STR(std::string(&data[2], 10).c_str());
		auto d2 = LC_TO_Q_STR(std::string(&data[12], 10).c_str());
		auto d3 = LC_TO_Q_STR(std::string(&data[22], 10).c_str());

		addListItem(QString("SOC版本:%1 %2").arg(d1, OK_NG(v1 == d1)));
		addListItem(QString("MCU版本:%1 %2").arg(d2, OK_NG(v2 == d2)));
		addListItem(QString("硬件版本:%1 %2").arg(d3, OK_NG(v3 == d3)));

		RUN_BREAK(v1 != d1 || v2 != d2 || v3 != d3, "网络版本号对比失败");
		result = true;
	} while (false);
	m_dvrClient.disconnect();
	addListItemEx(Q_SPRINTF("检测网络版本 %s", OK_NG(result)));
	WRITE_LOG("检测网络版本,%s", OK_NG(result));
	return result;
}

bool Dt::Dvr::umountSdCard()
{
	setCurrentStatus("卸载SD卡");
	bool result = false;
	do
	{
		msleep(1000);
		char data[BUFF_SIZE] = { 0 };
		int len = 0;
		DEBUG_INFO() << "发送卸载SD卡报文";
		RUN_BREAK(!m_dvrClient.connect(), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(!m_dvrClient.sendFrameDataEx({}, 0x12, 0x22), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(!m_dvrClient.recvFrameDataEx(data, &len, 0x12, 0x22), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(*(uint*)&data[2], Q_SPRINTF("卸载SD卡失败,操作错误代码:0x%X", *(uint*)&data[2]));
		result = true;
	} while (false);
	m_dvrClient.disconnect();
	addListItemEx(Q_SPRINTF("卸载SD卡 %s", OK_NG(result)));
	WRITE_LOG("卸载SD卡,%s", OK_NG(result));
	return result;
}

bool Dt::Dvr::changeWifiPassword()
{
	setCurrentStatus("修改WIFI密码");
	bool result = false, success = false;
	QString newPassword;
	do
	{
		QString oldPassword = m_wifiInfo.password;

		if (!getWifiInfo(true, false))
		{
			break;
		}

		const char word[62] =
		{
			'0','1','2','3','4','5','6','7','8','9',
			'q','w','e','r','t','y','u','i','o','p',
			'a','s','d','f','g','h','j','k','l',
			'z','x','c','v','b','n','m',
			'Q','W','E','R','T','Y','U','I','O','P',
			'A','S','D','F','G','H','J','K','L',
			'Z','X','C','V','B','N','M'
		};

		srand((uint)time(nullptr));
		for (int i = 0; i < 8; i++)
		{
			newPassword.append(word[rand() % 62]);
		}

		DEBUG_INFO() << "发送修改WIFI密码报文";
		addListItem("开始发送修改WIFI密码报文");
		char data[256] = { 0 };
		int len = 0;
		memcpy(&data[00], m_wifiInfo.account, 8);
		memcpy(&data[50], Q_TO_C_STR(newPassword), 8);
		RUN_BREAK(!m_dvrClient.connect(), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(!m_dvrClient.sendFrameDataEx(data, 100, 0x12, 0x05), LC_TO_Q_STR(m_dvrClient.getLastError()));
		memset(data, 0xff, sizeof(data));
		RUN_BREAK(!m_dvrClient.recvFrameDataEx(data, &len, 0x12, 0x05), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(*(uint*)&data[2], "修改WIFI密码失败");
		addListItem("正在校验WIFI密码,请耐心等待...");
		uint&& starTime = GetTickCount64();
		while (true)
		{
			if (!getWifiInfo(false, false))
			{
				break;
			}

			DEBUG_INFO_EX("WIFI矩阵新密码: %s", m_wifiInfo.password);
			if (newPassword == m_wifiInfo.password)
			{
				success = true;
				break;
			}
			RUN_BREAK(GetTickCount64() - starTime > 20000, "校验WIFI密码超时");
			msleep(100);
		}

		addListItem("WIFI矩阵旧密码: " + oldPassword);
		addListItem("WIFI随机新密码: " + newPassword);
		addListItem(Q_SPRINTF("WIFI矩阵新密码: %s", m_wifiInfo.password));
		addListItem(Q_SPRINTF("校验WIFI密码 %s", OK_NG(success)));
		RUN_BREAK(!success, "校验WIFI密码失败");
		result = true;
	} while (false);
	m_dvrClient.disconnect();
	WRITE_LOG("修改WIFI密码,%s", Q_TO_C_STR(newPassword));
	addListItemEx(Q_SPRINTF("修改WIFI密码 %s", OK_NG(result)));
	return result;
}

void Dt::Dvr::setAddressPort(const QString& address, const ushort& port)
{
	m_address = address;
	m_port = port;
	m_dvrClient.setAddressPort(Q_TO_C_STR(m_address), m_port);
}

bool Dt::Dvr::writeNetLog(const char* name, const char* data, uint size)
{
	bool result = false;
	do
	{
		QString path = QString(".\\%1\\Network\\%2\\").arg(getLogPath(), utility::getCurrentDate(true));
		utility::makePath(path);

		QString fileName(name);
		fileName.insert(0, utility::getCurrentTimeEx(true));
		path.append(fileName).append(".net");

		QFile file(path);
		if (!file.open(QFile::WriteOnly))
		{
			setLastError("写入网络日志文件失败," + file.errorString());
			break;
		}

		char buffer[0x10] = { 0 };
		sprintf(buffer, "%s\n", name);
		file.write(buffer, strlen(buffer));
		for (uint i = 0; i < size; i++)
		{
			memset(buffer, 0x00, sizeof(buffer));
			sprintf(buffer, "0x%02X\t", (uchar)data[i]);
			file.write(buffer, strlen(buffer));
			if ((i % 10 == 0) && i != 0)
			{
				file.write("\r\n", strlen("\r\n"));
			}
		}
		file.close();
		result = true;
	} while (false);
	return result;
}

int Dt::Dvr::getSystemStatus(const can::Msg& msg)
{
	setLastError("子类未实现获取系统状态");
	return -1;
}

int Dt::Dvr::getWifiStatus(const can::Msg& msg)
{
	setLastError("子类未实现获取wifi状态");
	return -1;
}

int Dt::Dvr::getSdCardStatus(const can::Msg& msg)
{
	setLastError("子类未实现获取SD卡状态");
	return -1;
}

bool Dt::Dvr::networkPhotoGraph()
{
	setCurrentStatus("网络拍照");
	bool result = false;
	do
	{
		addListItem("正在进行网络拍照,请耐心等待...");
		int len = 0;
		char data[32] = { 0 };
		RUN_BREAK(!m_dvrClient.connect(), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(!m_dvrClient.sendFrameDataEx(nullptr, 0, DvrTypes::NC_FAST_CONTROL,
			DvrTypes::NS_FAST_PHOTOGRAPHY), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(!m_dvrClient.recvFrameDataEx(data, &len, DvrTypes::NC_FAST_CONTROL,
			DvrTypes::NS_FAST_PHOTOGRAPHY), LC_TO_Q_STR(m_dvrClient.getLastError()));
		RUN_BREAK(*(uint*)&data[2], Q_SPRINTF("网络拍照失败,错误代码0x%X", *(uint*)&data[2]));
		result = true;
	} while (false);
	m_dvrClient.disconnect();
	addListItemEx(Q_SPRINTF("网络拍照 %s", OK_NG(result)));
	WRITE_LOG("网络拍照,%s", OK_NG(result));
	return result;
}

Nt::SfrServer* Dt::Dvr::getSfrServer() const
{
	return const_cast<Nt::SfrServer*>(&m_sfrServer);
}

int Dt::Dvr::setPlayQuestionBox(const QString& title, const QString& text, const QPoint& point)
{
	int result = DvrTypes::PR_NG;
	emit setPlayQuestionBoxSignal(title, text, &result, point);
	threadPause();
	return result;
}

bool Dt::Dvr::checkPlaybackByFile(bool checkState)
{
	setCurrentStatus("检测视频回放");
	bool result = false, success = false, hint = true, notFound = false;
	do
	{
		if (checkState)
		{
			setCustomDraw([&](cv::Mat& img) { ImageProcess::putTextOnCenter(img, "请将SD卡拔出", 60, CV_RGB(255, 0, 0), cv::Point(60, 0)); });
			addListItem("请将SD卡拔出");
			success = autoProcessStatus<DvrTypes::SdCardStatus>(DvrTypes::SCS_NO_SD, 60000);
			addListItem(Q_SPRINTF("SD卡拔出%s", SU_FA(success)));
			RUN_BREAK(!success, "SD卡拔出超时");
			success = false;
		}

		setCustomDraw([&](cv::Mat& img) { ImageProcess::putTextOnCenter(img, "请将SD卡插入电脑", 60, CV_RGB(255, 0, 0), cv::Point(130, 0)); });
		addListItem("请将SD卡插入电脑");
		RUN_BREAK(!utility::timeoutExpression([&]() {
			if (utility::getRemovableDiskCount())
			{
				addListItem("SD卡插入电脑成功");
				addListItem("正在搜索紧急录制视频,请耐心等待...");
				return true;
			}
			return false; }, 60000), "SD卡插入电脑超时");
		setCustomDraw(nullptr);

		QString fileName;
		success = searchDiskFile(DvrTypes::FP_EVT, fileName);
		addListItem(Q_SPRINTF("搜索紧急录制视频 %s", OK_NG(success)));
		RUN_BREAK(!success, "搜索紧急录制视频失败");
		m_volume = fileName.mid(0, fileName.indexOf(":") + 1);

		addListItem("播放紧急录制视频中...");
		int repalyCount = 1;
	replay1:
		success = vlcLocalStart(fileName);
		RUN_BREAK(!success, "播放紧急录制视频失败");

		int play = setPlayQuestionBox("提示", "紧急录制视频是否回放?", QPoint(130, 0));
		if (play == DvrTypes::PR_OK) success = true;
		else if (play == DvrTypes::PR_NG) success = false;
		else
		{
			vlcLocalStop();
			msleep(1000);
			addListItem(Q_SPRINTF("紧急录制视频重播第%d次", repalyCount));
			if (repalyCount++ >= 5) success = false; else goto replay1;
		}
		vlcLocalStop();
		RUN_BREAK(!success, "紧急录制视频回放失败");
		result = true;
	} while (false);
	setCustomDraw(nullptr);
	addListItemEx(Q_SPRINTF("检测视频回放 %s", OK_NG(result)));
	WRITE_LOG("检测视频回放,%s", OK_NG(result));
	return result;
}

bool Dt::Dvr::checkVideoUsePerson()
{
	bool result = false, success = false;
	setCurrentStatus("检测采集卡出画");
	do
	{
		addListItem("检测采集卡出画");
		success = setQuestionBoxEx("提示", "采集卡出画是否成功?", QPoint(80, 0));
		addListItemEx(Q_SPRINTF("检测采集卡出画 %s", OK_NG(success)));
		WRITE_LOG("采集卡出画,%s", OK_NG(success));
		RUN_BREAK(!success, "检测采集卡出画失败");
		result = true;
	} while (false);
	return result;
}

Dt::Module::Module(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Module);
	m_detectionType = BaseTypes::DetectionType::DT_MODULE;
}

Dt::Module::~Module()
{
	PRINT_CON_DESTRUCTION(Dt::Module);
}

bool Dt::Module::initialize()
{
	bool result = false;
	do
	{
		if (!Dt::Base::initialize())
		{
			break;
		}

		RUN_BREAK(!m_printer.init(PT_4503E, "./Config/PrintInfo.xml"), getPrinterError());
		result = true;
	} while (false);
	return result;
}

bool Dt::Module::printLabel(const std::function<bool(void)>& fnc)
{
	setCurrentStatus("打印产品标签");
	bool result = false;
	do
	{
		RUN_BREAK(!fnc(), getPrinterError());
		result = true;
	} while (false);
	WRITE_LOG("打印标签,%s", OK_NG(result));
	addListItemEx(Q_SPRINTF("打印标签 %s", OK_NG(result)));
	return result;
}

QString Dt::Module::getPrinterError() const
{
	return LC_TO_Q_STR(m_printer.getLastError());
}


Dt::Oms::Oms(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Oms);
	m_detectionType = BaseTypes::DetectionType::DT_OMS;
}

Dt::Oms::~Oms()
{
	PRINT_CON_DESTRUCTION(Dt::Oms);
}

bool Dt::Oms::checkJ2Version(const QString& ip, const QString& version)
{
	addListItem("检测J2版本");
	bool result = false, success = false;
	do
	{
		QString address = UTIL_JSON->getOtherConfig1Value(ip);
		if (address.isEmpty())
			address = ip;
		RUN_BREAK(address.isEmpty(), "IP地址未设置");

		QString j2Version = UTIL_JSON->getOtherConfig1Value(version);
		if (j2Version.isEmpty())
			j2Version = version;
		RUN_BREAK(j2Version.isEmpty(), "J2版本未设置");

		RUN_BREAK(!Misc::isOnline(address, 22, 5000), QString("未与%1连接,请检查网络").arg(address));

		const QString cmd = QString("scp.exe -o StrictHostKeyChecking=no "
			"root@%1:/etc/version ./version.tmp").arg(address);
		RUN_BREAK(!utility::execute(cmd), "scp执行失败,请到https://www.openssh.com下载安装");

		QString data;
		RUN_BREAK(!utility::file::read("version.tmp", data), utility::getLastError());
		data = data.simplified();
		success = j2Version == data;
		addListItem(Q_SPRINTF("J2版本与控制器内部版本对比 %s", OK_NG(success)));
		RUN_BREAK(!success, QString("对比J2版本失败,\n配置:[%1]\n实际:[%2]").arg(j2Version).arg(data));
		result = true;
	} while (false);
	addListItemEx(Q_SPRINTF("检测J2版本 %s", OK_NG(result)));
	WRITE_LOG("检测J2版本,%s", OK_NG(result));
	return result;
}

bool Dt::Oms::triggerQuadImage(const QString& address, ushort port, int count)
{
	setCurrentStatus("触发四分格图像");
	bool result = false, success = false;
	do
	{
		addListItem("正在触发四分格图像,请耐心等待...");

		RUN_BREAK(!m_omsClient.connect(Q_TO_C_STR(address), port),
			LC_TO_Q_STR(m_omsClient.getLastError()));

		for (int i = 0; i < count; ++i)
		{
			RUN_BREAK(m_omsClient.sendData({ 0x30,0x04 }) == SOCKET_ERROR,
				LC_TO_Q_STR(m_omsClient.getLastError()));

			auto startTime = GetTickCount64();
			while (true)
			{
				char data[BUFF_SIZE] = { 0 };
				RUN_BREAK(m_omsClient.recvData(data, sizeof(data)) == SOCKET_ERROR,
					LC_TO_Q_STR(m_omsClient.getLastError()));

				if (data[5] == 0x11 && data[8] == 0x04)
				{
					success = true;
					break;
				}

				if (GetTickCount64() - startTime > 3000)
				{
					setLastError("触发四分格出画超时");
					break;
				}
			}

			if (success)
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
	m_omsClient.disconnect();
	addListItemEx(Q_SPRINTF("触发四分格图像 %s", OK_NG(result)));
	WRITE_LOG("触发四分格图像,%s", OK_NG(result));
	return result;
}

bool Dt::Oms::triggerQuadImage(const QString& jsonAddress, const QString& jsonPort, int count)
{
	const QString address = UTIL_JSON->getOtherConfig1Value(jsonAddress);
	const int port = UTIL_JSON->getOtherConfig1Value(jsonPort).toInt();
	return triggerQuadImage(address, port, count);
}

bool Dt::Oms::checkAlgorithmStatus(const QString& address, ushort port, int timeout)
{
	static auto algOk = [&](int channel, int status)->int
	{
		QString name, text;
		switch (channel)
		{
		case OmsTypes::AC_DMS0:name = "DMS0"; break;
		case OmsTypes::AC_OMS0:name = "OMS0"; break;
		case OmsTypes::AC_OMS1:name = "OMS1"; break;
		case OmsTypes::AC_OMS2:name = "OMS2"; break;
		case OmsTypes::AC_GES0:name = "GES0"; break;
		case OmsTypes::AC_GES1:name = "GES1"; break;
		case OmsTypes::AC_GES2:name = "GES2"; break;
		case OmsTypes::AC_FID0:name = "FID0"; break;
		case OmsTypes::AC_FID1:name = "FID1"; break;
		default:name = "未知"; break;
		}

		int success = 0;
		do
		{
			if (status == OmsTypes::AS_STOPPED)
			{
				text = QString("%1算法状态STOPPED NG").arg(name);
			}
			else if (status == OmsTypes::AS_INITING)
			{
				text = QString("%1算法状态INITING NG").arg(name);
			}
			else if (status == OmsTypes::AS_FAILURE)
			{
				text = QString("%1算法状态FAILURE NG").arg(name);
			}
			else if (status == OmsTypes::AS_PAUSING)
			{
				if (channel == OmsTypes::AC_FID0 ||
					channel == OmsTypes::AC_FID1)
				{
					text = QString("%1算法状态STOPPED OK").arg(name);
					success = 1;
				}
				else
				{
					text = QString("%1算法状态PAUSING NG").arg(name);
				}
			}
			else if (status == OmsTypes::AS_RUNNING)
			{
				text = QString("%1算法状态RUNNING OK").arg(name);
				success = 1;
			}
			else
			{
				text = QString("%1算法状态ERROR NG").arg(name);
			}
			addListItem(text);
		} while (false);
		return success;
	};

	setCurrentStatus("检测算法状态");
	bool result = false, success = false, active = false;
	do
	{
		addListItem("正在检测算法状态,请耐心等待...");
		RUN_BREAK(!m_omsClient.connect(Q_TO_C_STR(address), port),
			LC_TO_Q_STR(m_omsClient.getLastError()));

		//0,1,2,3,4....代表算法通道
		//0->false,1->true,2->not use
		QMap<int, int> algMap = { {0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{6,2},{7,2},{8,2} };

		auto startTime = GetTickCount64();
		while (true)
		{
			char data[BUFF_SIZE] = { 0 };
			RUN_BREAK(m_omsClient.recvData(data, sizeof(data)) == SOCKET_ERROR,
				LC_TO_Q_STR(m_omsClient.getLastError()));

			if (data[5] == 0x21)
			{
				if (!active)
				{
					active = true;
				}

				int channel = data[6], status = data[7];
				if ((channel < 0) || (channel > algMap.size() - 1))
				{
					setLastError(Q_SPRINTF("所得算法通道%d,不在预期0~%d范围内", channel, OmsTypes::AC_FID1));
					break;
				}

				if (algMap[channel] == 2)
				{
					algMap[channel] = algOk(channel, status);
				}

				int okSum = 0, ngSum = 0;
				for (auto iter = algMap.cbegin(); iter != algMap.cend(); ++iter)
				{
					if (GetTickCount64() - startTime > timeout)
					{
						if (iter.value() == 1 || iter.value() == 2)
						{
							++okSum;
						}
						else
						{
							++ngSum;
						}
					}
					else
					{
						if (iter.value() == 1)
						{
							++okSum;
						}
						else if (iter.value() == 0)
						{
							++ngSum;
						}
					}
				}

				if (okSum == algMap.size())
				{
					success = true;
					break;
				}

				if (okSum + ngSum == algMap.size())
				{
					setLastError("检测算法状态失败,激活条件不满足");
					break;
				}
			}

			if (GetTickCount64() - startTime > ULONGLONG(timeout + 5000))
			{
				setLastError(Q_SPRINTF("检测算法状态超时,%s", active ? "激活条件不满足" : "此产品尚未激活"));
				break;
			}
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	m_omsClient.disconnect();
	addListItemEx(Q_SPRINTF("检测算法状态 %s", OK_NG(result)));
	WRITE_LOG("检测算法状态,%s", OK_NG(result));
	return result;
}

bool Dt::Oms::checkAlgorithmStatus(const QString& jsonAddress, const QString& jsonPort, int timeout)
{
	const QString address = UTIL_JSON->getOtherConfig1Value(jsonAddress);
	const int port = UTIL_JSON->getOtherConfig1Value(jsonPort).toInt();
	return checkAlgorithmStatus(address, port, timeout);
}

Dt::Aics::Aics(QObject* parent)
{
	PRINT_CON_DESTRUCTION(Dt::Aics);
	m_detectionType = BaseTypes::DetectionType::DT_AICS;
}

Dt::Aics::~Aics()
{
	PRINT_CON_DESTRUCTION(Dt::Aics);
	Lin::freeLinTransmit(m_linTransmit);
}

bool Dt::Aics::initialize()
{
	bool result = false;
	do
	{
		if (!Dt::Base::initialize())
		{
			break;
		}

		m_linTransmit = Lin::allocLinTransmit("Tms");
		RUN_BREAK(!m_linTransmit, "Lin卡分配内存失败");

		result = true;
	} while (false);
	return result;
}

bool Dt::Aics::openDevice()
{
	bool result = false;
	do
	{
		if (!Dt::Base::openDevice())
		{
			break;
		}

		m_boxPort.setSimpleTimeout(1000);
		m_boxPort.setPort(std::to_string(m_defConfig->hardware.expandCom1));
		m_boxPort.setBaudrate(m_defConfig->hardware.expandBaud1);
		if (!m_boxPort.open())
		{
			setDeviceFaultCode(DFC_ADAPTER_BOX);
			closeParentClassDevice();
			setLastError(Q_SPRINTF("打开转接盒串口%d失败", m_defConfig->hardware.expandCom1), false, m_faultHint);
			break;
		}

		if (!m_linTransmit->open())
		{
			setDeviceFaultCode(DFC_LIN_CARD);
			closeParentClassDevice();
			m_boxPort.close();
			setLastError(m_linTransmit->getLastError(), false, m_faultHint);
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Dt::Aics::closeDevice()
{
	bool result = false;
	do
	{
		if (!Dt::Base::closeDevice())
		{
			break;
		}

		m_boxPort.close();

		m_linTransmit->close();

		result = true;
	} while (false);
	return result;
}

bool Dt::Aics::setAdapterBox(AicsTypes::AdapterChannel channel, bool on)
{
	bool result = false;
	do
	{
		if (channel == AicsTypes::AdapterChannel::AC_ALL)
			memset(&m_boxData[4], on ? 1 : 0, sizeof(m_boxData) - 4);
		else
			m_boxData[4 + channel] = on ? 1 : 0;

		uchar sum = 0;
		for (size_t i = 0; i < 14; ++i)
			sum += m_boxData[4 + i];
		m_boxData[18] = sum;

		RUN_BREAK(m_boxPort.write(m_boxData, 19) != 19, "发送转接盒串口数据失败");

		uchar buffer[TINY_BUFF] = { 0 };
		RUN_BREAK(m_boxPort.read(buffer, 6) != 6, "接收转接盒串口数据失败");

		uchar temp[] = { 0xff,0xa5,0x5a,0x01,0x01,0x01 };
		RUN_BREAK(memcmp(buffer, temp, sizeof(temp)), "转接盒串口数据对比失败");

		result = true;
	} while (false);
	return result;
}

bool Dt::Aics::autoProcessLinMsg(int pid, int value, LinProc linProc, ulong timeout)
{
	bool result = false, success = false;
	do
	{
		const auto startTime = GetTickCount64();
		std::unique_ptr<LinMsg[]> msg(new LinMsg[MSG_BUFFER_SIZE]);
		while (true)
		{
			memset(msg.get(), 0, MSG_BUFFER_SIZE * sizeof(LinMsg));
			const int size = m_linTransmit->receive(msg.get(), MSG_BUFFER_SIZE);
			for (int i = 0; i < size; ++i)
			{
				if (msg[i].pid == pid)
				{
					if (linProc(value, msg[i]))
					{
						success = true;
						break;
					}
				}
			}

			if (success) break;

			RUN_BREAK(GetTickCount64() - startTime > timeout, "LIN报文处理失败");
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

void Dt::Aics::closeParentClassDevice()
{
	m_can->close();
	m_power->close();
	m_relay->close();
	m_voltmeter->close();
	m_amperemeter->close();
}

