#include "MainDlg.h"

Dt::MainDlg* Dt::MainDlg::m_self = nullptr;

Dt::MainDlg::MainDlg(Dt::Base* thread, const QString& typeName, QWidget* parent)
	:QWidget(parent),
	m_base(thread)
{
	PRINT_CON_DESTRUCTION(Dt::MainDlg);
	m_typeName = typeName;
	m_self = this;

	windowMaskPrint(false);
	setMainWindow<Dt::MainDlg>();

	if (!initialize())
	{
		utility::QMessageBoxEx::warning(this, "错误", getLastError());
		ui.connectButton->setEnabled(false);
	}
}

Dt::MainDlg::~MainDlg()
{
	PRINT_CON_DESTRUCTION(Dt::MainDlg);
	utility::ScanWidget::deleteInstance();
	destroyAllWindow<MainDlg>();
	m_self = nullptr;
}

bool Dt::MainDlg::initialize()
{
	bool result = false;
	do
	{
		ui.setupUi(this);

		setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

		utility::window::setTheme();

		utility::JQCPUMonitor::initialize();

		connect(&m_usageRateTimer, &QTimer::timeout, this, &Dt::MainDlg::usageRateTimerSlot);
		m_usageRateTimer.start(1000);

		connect(ui.settingButton, &QPushButton::clicked, this, &Dt::MainDlg::settingButtonSlot);

		connect(ui.connectButton, &QPushButton::clicked, this, &Dt::MainDlg::connectButtonSlot);

		connect(ui.exitButton, &QPushButton::clicked, this, &Dt::MainDlg::exitButtonSlot);

		connect(&m_disconnectTimer, &QTimer::timeout, this, &Dt::MainDlg::disconnectTimerSlot);

		if (m_typeName.isEmpty())
		{
			auto dlg = autoReleaseNewWindow<utility::UpdateDialog>(this, false);
			connect(dlg, &utility::UpdateDialog::restart, this, &Dt::MainDlg::restartSlot);
		}

		RUN_BREAK(!m_base.get(), "检测框架基类分配内存失败");

		connect(m_base.get(), &Base::addListItemSignal, this, &Dt::MainDlg::addListItemSlot);

		connect(m_base.get(), &Base::clearListItemSignal, this, &Dt::MainDlg::clearListItemSlot);

		connect(m_base.get(), &Base::setCurrentStatusSignal, this, &Dt::MainDlg::setCurrentStatusSlot);

		connect(m_base.get(), &Base::setMessageBoxSignal, this, &Dt::MainDlg::setMessageBoxSlot);

		connect(m_base.get(), &Base::setMessageBoxExSignal, this, &Dt::MainDlg::setMessageBoxExSlot);

		connect(m_base.get(), &Base::setQuestionBoxSignal, this, &Dt::MainDlg::setQuestionBoxSlot);

		connect(m_base.get(), &Base::setQuestionBoxExSignal, this, &Dt::MainDlg::setQuestionBoxExSlot);

		connect(m_base.get(), &Base::setTestResultSignal, this, &Dt::MainDlg::setTestResultSlot);

		connect(m_base.get(), &Base::updateImageSignal, this, &Dt::MainDlg::updateImageSlot);

		connect(m_base.get(), &Base::setUnlockDlgSignal, this, &Dt::MainDlg::setUnlockDlgSlot);

		connect(m_base.get(), &Base::setAuthDlgSignal, this, &Dt::MainDlg::setAuthDlgSlot);

		connect(m_base.get(), &Base::setDownloadDlgSignal, this, &Dt::MainDlg::setDownloadDlgSlot);

		connect(m_base.get(), &Base::startTestSignal, this, &Dt::MainDlg::startTestSlot);

		if (m_base->getDetectionType() == BaseTypes::DT_DVR)
		{
			Dt::Dvr* dvr = nullptr;
			RUN_BREAK(!(dvr = dynamic_cast<Dt::Dvr*>(m_base.get())), "父类不为Dt::Dvr,设置VLC句柄失败");
			dvr->setVlcMediaHwnd((HWND)ui.imageLabel->winId());
			connect(dvr, &Dvr::setPlayQuestionBoxSignal, this, &Dt::MainDlg::setPlayQuestionBoxSlot);
		}

		RUN_BREAK(!UTIL_JSON, "UTIL_JSON分配内存失败");
		UTIL_JSON->setTypeName(m_typeName);
		UTIL_JSON->addComponentVoltageConfigKeyValue(m_base->getComponentVoltageKeyValue());
		UTIL_JSON->addVersionConfigKeyValue(m_base->getVersionConfigKeyValue());
		UTIL_JSON->addDtcConfigKeyValue(m_base->getDtcConfigKeyValue());
		UTIL_JSON->addOtherConfig1KeyValue(m_base->getOtherConfig1KeyValue());
		UTIL_JSON->addOtherConfig2KeyValue(m_base->getOtherConfig2ParentKeyList(),
			m_base->getOtherConfig2ChildKeyValueList());
		RUN_BREAK(!UTIL_JSON->initialize(), "UTIL_JSON初始化失败," + UTIL_JSON->getLastError());

		const QString typeName = UTIL_JSON->getParsedDeviceConfig().typeName;
		const QString userName = UTIL_JSON->getUserConfigValue("用户名");
		const QString appName = QString("%1%2").arg(typeName, Dt::Base::getDetectionName());
		QString title; 
		if (m_typeName.isEmpty()) {
			title = appName + QString("[%1][权限:%2]").arg(utility::getVersion(), userName);
		}
		else {
			title = appName + QString("[%1][权限:%2][主程:%3][框架:%4][编译时间:%5][模式:%6]").arg(G_VERSION,
				userName,
				utility::getVersion(), Json::getLibVersion(),
				utility::getCompileDateTime(), G_MODEL);
		}
		setWindowTitle(title);
		if (m_typeName.isEmpty()) {
			utility::renameByVersion(this, appName);
		}

		const auto& device = UTIL_JSON->getParsedDefConfig()->device;
		const auto& enable = UTIL_JSON->getParsedDefConfig()->enable;

		utility::ScanWidget::Parameters params;
		params.dataHeader = &device.codeContent;
		params.dataLength = &device.codeLength;
		params.isJudgeData = (const bool*)&enable.judgeCode;
		params.isQueryMes = (const bool*)&enable.queryStation;
		params.autoClearDataList = true;

#ifdef MES_NETWORK_COMMUNICATION
		params.pluginSavePath = UTIL_JSON->getSharePath();

		connect(this, &MainDlg::changeWindowTitleSignal, this, &MainDlg::setWindowTitle);

		params.onConnection = [this](bool connected) {
			static QString title = windowTitle();
			if (connected)
			{
				const QString&& temp = title + "[与MES已连接]";
				changeWindowTitleSignal(temp);
			}
			else
			{
				const QString&& temp = title + "[与MES未连接]";
				changeWindowTitleSignal(temp);
			}
		};

		params.onMessage = [this](const QString& text)
		{
			m_base->addListItem(text);
		};
#else
		params.selfTitle = utility::ST_SD_HD_ECU_DVR_LINE;
#endif // !MES_NETWORK_COMMUNICATION

		utility::ScanWidget::getInstance();
		utility::ScanWidget::initialize(params);
		RUN_BREAK(!m_base->initialize(), m_base->getLastError());
		m_base->start();
		result = true;
	} while (false);
	return result;
}

QString Dt::MainDlg::getLastError() const
{
	return m_lastError;
}

void Dt::MainDlg::setShowCloseHint(bool show)
{
	m_showCloseHint = show;
}

void Dt::MainDlg::setUnlockDlgSlot(bool show)
{
	if (!findWindow<utility::UnlockDialog>())
	{
		autoReleaseNewWindow<utility::UnlockDialog>();
		setWindowResident<utility::UnlockDialog>(true);
	}
	auto dlg = getWindow<utility::UnlockDialog>();
	dlg->setPassword(UTIL_JSON->getUserConfigValue("密码"));
	dlg->exec();
	m_base->threadContinue();
}

void Dt::MainDlg::setAuthDlgSlot(bool* result, bool threadCall)
{
	if (!findWindow<utility::AuthDialog>())
	{
		auto dlg = autoReleaseNewWindow<utility::AuthDialog>();
		setWindowResident<utility::AuthDialog>(true);
		dlg->setEnterButtonIcon(QIcon(":/images/Resources/images/connect.ico"));
		dlg->setExitButtonIcon(QIcon(":/images/Resources/images/exit.ico"));
	}
	auto dlg = getWindow<utility::AuthDialog>();
	dlg->setUserName(UTIL_JSON->getUserConfigValue("用户名"));
	dlg->setPassword(UTIL_JSON->getUserConfigValue("密码"));
	if (threadCall)
	{
		*result = dlg->exec() == QDialog::Accepted;
		m_base->threadContinue();
	}
	else
	{
		*result = !dlg->isAuthentication() ? (dlg->exec() == QDialog::Accepted) : true;
	}
	return;
}

void Dt::MainDlg::setDownloadDlgSlot(BaseTypes::DownloadInfo* info)
{
	int tryAgainCount = 1;
	do
	{
		if (!findWindow<DownloadDlg>())
		{
			autoReleaseNewWindow<DownloadDlg>();
			setWindowResident<DownloadDlg>(true);
		}
		auto dlg = getWindow<DownloadDlg>();
	tryAgain:
		dlg->download(info->title, info->url, info->path);
		dlg->exec();
		info->result = dlg->getResult();
		info->size = dlg->getFileSize();
		info->time = dlg->getElapsedTime();
		info->speed = dlg->getAverageSpeed();
		info->error = dlg->getLastError();
		if (info->error == "Network access is disabled.")
		{
			dlg->resetNetwork();
			m_base->addListItem(Q_SPRINTF("网络访问权限被禁用已重置网络,重试第%d次", tryAgainCount));
			if (tryAgainCount++ >= 5) break;
			goto tryAgain;
		}
	} while (false);
	m_base->threadContinue();
	return;
}

void Dt::MainDlg::settingButtonSlot()
{
	do
	{
		if (findWindow<SettingDlg>())
		{
			showWindow<SettingDlg>();
			return;
		}

		bool result = false;
		setAuthDlgSlot(&result);
		if (!result)
		{
			break;
		}

		auto dlg = autoReleaseNewWindow<SettingDlg>(m_base.get());
		connect(dlg, &SettingDlg::restartSignal, this, &Dt::MainDlg::restartSlot);
		dlg->setIsConnect(&m_connected);
		if (!dlg->initialize())
		{
			utility::QMessageBoxEx::warning(this, "错误", dlg->getLastError());
		}
		dlg->show();
	} while (false);
	return;
}

void Dt::MainDlg::connectButtonSlot()
{
	do
	{
		auto task = [this]() {
			if (!m_connected)
			{
				if (!m_base->openDevice())
				{
					return false;
				}

				auto cast = dynamic_cast<Dt::Function*>(m_base.get());
				if (cast)
				{
					const auto touch = cast->getTouchScreenInfo();
					ui.imageLabel->setTouchScreenInfo(touch);
				}
			}
			else
			{
				m_base->closeDevice();
			}

			return true;
		};

		if (utility::LoadingDialog::doModal(task)) {
			ui.imageLabel->setLabelMode(!m_connected ? LM_TOUCH : LM_NONE);
			ui.connectButton->setIcon(!m_connected ?
				QIcon(":/images/Resources/images/disconnect.ico") :
				QIcon(":/images/Resources/images/connect.ico"));
			ui.exitButton->setEnabled(m_connected);
			ui.connectButton->setText(!m_connected ? "断开" : "连接");
			m_connected = !m_connected;
			m_base->setTestSequence(m_connected ? TS_SCAN_CODE : TS_NO);
			m_base->setConnectStatus(m_connected);
			startTestSlot(!m_connected);
		}

	} while (false);
	return;
}

void Dt::MainDlg::exitButtonSlot()
{
	this->close();
}

void Dt::MainDlg::setMessageBoxSlot(const QString& title, const QString& text)
{
	utility::QMessageBoxEx::warning(this, title, text);
	m_base->threadContinue();
}

void Dt::MainDlg::setMessageBoxExSlot(const QString& title, const QString& text, const QPoint& point)
{
	QMessageBox msgBox(QMessageBox::Warning, title, text, QMessageBox::Yes | QMessageBox::No);
	msgBox.move(point);
	msgBox.exec();
	m_base->threadContinue();
}

void Dt::MainDlg::setQuestionBoxSlot(const QString& title, const QString& text, bool modal, bool* result)
{
	if (modal)
	{
		*result = utility::QMessageBoxEx::question(this, title, text) == QMessageBox::Yes;
		m_base->threadContinue();
	}
	else
	{
		new utility::QMessageBoxEx(QMessageBox::Question, title, text,
			QMessageBox::Yes | QMessageBox::No, true, false, [&](int value, void* args)
			{
				*static_cast<bool*>(args) = (value == QMessageBox::Yes);
				m_base->threadContinue();
			},
			static_cast<void*>(result), this);
	}
}

void Dt::MainDlg::setQuestionBoxExSlot(const QString& title, const QString& text, bool* result, const QPoint& point)
{
	QMessageBox msgBox(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::No);
	msgBox.setParent(ui.recordList);
	msgBox.setStyleSheet("color:red");
	int width = ui.recordList->width() - msgBox.widthMM();
	msgBox.move(width - point.x(), point.y());
	*result = (msgBox.exec() == QMessageBox::Yes);
	m_base->threadContinue();
}

void Dt::MainDlg::setPlayQuestionBoxSlot(const QString& title, const QString& text, int* result, const QPoint& point)
{
	QMessageBox msgBox(QMessageBox::Question, title, text);
	auto replay = msgBox.addButton("重播", QMessageBox::HelpRole);
	auto yes = msgBox.addButton("是", QMessageBox::YesRole);
	auto no = msgBox.addButton("否", QMessageBox::NoRole);
	msgBox.setParent(ui.recordList);
	msgBox.setStyleSheet("color:red");
	int width = ui.recordList->width() - msgBox.widthMM();
	msgBox.move(width - point.x(), point.y());
	yes->setFocus();
	msgBox.exec();
	if (msgBox.clickedButton() == yes) *result = DvrTypes::PR_OK;
	else if (msgBox.clickedButton() == no) *result = DvrTypes::PR_NG;
	else *result = DvrTypes::PR_REPLAY;
	m_base->threadContinue();
	return;
}

void Dt::MainDlg::setCurrentStatusSlot(const QString& status, bool systemStatus)
{
	systemStatus ? ui.systemLabel->setText(status) : ui.statusLabel->setText(status);
}

void Dt::MainDlg::setTestResultSlot(BaseTypes::TestResult testResult)
{
	QString result = "NO", color = "black";
	switch (testResult)
	{
	case BaseTypes::TR_TS:result = "TS"; color = "rgb(255,255,0)"; break;
	case BaseTypes::TR_OK:result = "OK"; color = "rgb(0,255,0)"; break;
	case BaseTypes::TR_NG:result = "NG"; color = "rgb(255,0,0)"; break;
	case BaseTypes::TR_NO:result = "NO"; color = "rgb(0,0,0)"; break;
	default:break;
	}
	ui.resultLabel->setText(result);
	ui.resultLabel->setStyleSheet(QString("background-color:%1;color:%2;").arg(color).arg("rgb(0,0,0)"));
}

void Dt::MainDlg::addListItemSlot(const QString& item, bool logItem)
{
	QString icon = ":/images/Resources/images/";

	if (item.contains(" OK") && item.contains(" NG"))
	{
		icon.append("ng.ico");
	}
	else if (item.contains(" OK"))
	{
		icon.append("ok.ico");
	}
	else if (item.contains(" NG"))
	{
		icon.append("ng.ico");
	}
	else if (item.contains("OK") && item.contains("NG"))
	{
		icon.append("ng.ico");
	}
	else if (item.contains("成功") && item.contains("失败"))
	{
		icon.append("ng.ico");
	}
	else if (item.contains("成功") && item.contains("NG"))
	{
		icon.append("ng.ico");
	}
	else if (item.contains("OK") || item.contains("成功"))
	{
		icon.append("ok.ico");
	}
	else if (item.contains("NG") || item.contains("失败"))
	{
		icon.append("ng.ico");
	}
	else
	{
		icon.append("star.ico");
	}

	logItem ? ui.recordList->addItem(new QListWidgetItem(QIcon(icon), item))
		: ui.resultList->addItem(new QListWidgetItem(QIcon(icon), item));
	ui.recordList->setCurrentRow(ui.recordList->count() - 1);
	ui.resultList->setCurrentRow(ui.resultList->count() - 1);
}

void Dt::MainDlg::clearListItemSlot()
{
	ui.resultList->clear();
	ui.recordList->clear();
}

void Dt::MainDlg::updateImageSlot(const QPixmap& pixmap)
{
	ui.imageLabel->setPixmap(pixmap);
}

void Dt::MainDlg::usageRateTimerSlot()
{
	static MEMORYSTATUSEX memoryStatus = { 0 };
	memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memoryStatus);

	qreal&& rate = utility::JQCPUMonitor::cpuUsagePercentage() * 100;
	static QString cpuStyle = "color:rgb(0,0,0);";
	static QString memStyle = "color:rgb(0,0,0);";

	static auto processFnc = [](int usageRate, QString& styleSheet)->void {
		if (usageRate <= 50)
		{
			styleSheet = "color:rgb(34,139,34);";
		}
		else if (usageRate > 50 && usageRate <= 80)
		{
			styleSheet = "color:rgb(218,165,32);";
		}
		else if (usageRate > 80)
		{
			styleSheet = "color:rgb(165,42,42);";
		}
	};

	processFnc(rate, cpuStyle);
	processFnc(memoryStatus.dwMemoryLoad, memStyle);

	ui.cpuLabel->setText(QString().sprintf("%6.2f%%", rate));
	ui.cpuLabel->setStyleSheet(cpuStyle);

	ui.memLabel->setText(QString().sprintf("%02d%%", memoryStatus.dwMemoryLoad));
	ui.memLabel->setStyleSheet(memStyle);
}

void Dt::MainDlg::restartSlot(const QString& name)
{
	if (m_connected)
	{
		connectButtonSlot();
	}

	m_showCloseHint = false;
	if (m_typeName.isEmpty())
	{
		//QProcess* process(new QProcess);
		//process->setWorkingDirectory(utility::getCurrentDir());
		//const QString cmd = QString("cmd.exe /c start %1").arg(name.isEmpty() ?
		//	windowTitle().mid(0, windowTitle().indexOf(']') + 1) + ".exe" : name);
		//process->start(cmd);
		//process->waitForStarted();
		//QApplication::exit(0);
		utility::restartProcess(reinterpret_cast<HWND>(winId()), name, QString());
	}
	else
	{
		this->close();
	}
}

void Dt::MainDlg::disconnectTimerSlot()
{
	if (UTIL_JSON->getParsedDefConfig()->enable.autoDisconnect)
		connectButtonSlot();

	if (m_disconnectTimer.isActive())
		m_disconnectTimer.stop();
}

void Dt::MainDlg::startTestSlot(bool testing)
{
	if (testing)
	{
		if (m_disconnectTimer.isActive())
			m_disconnectTimer.stop();
	}
	else
	{
		if (!m_disconnectTimer.isActive())
			m_disconnectTimer.start(10 * 60000);
	}
}

void Dt::MainDlg::closeEvent(QCloseEvent* event)
{
	if (m_showCloseHint)
	{
		if (utility::QMessageBoxEx::question(this, "友情提示", "你确定要退出吗?") == QMessageBox::Yes)
		{
			emit applicationExitSignal();
			event->accept();
		}
		else
		{
			event->ignore();
		}
	}
	else
	{
		emit applicationExitSignal();
		event->accept();
	}
}

void Dt::MainDlg::setLastError(const QString& error)
{
	m_lastError = error;
}

