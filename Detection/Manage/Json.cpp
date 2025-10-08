#include "Json.h"
#include "Types.h"

bool Json::m_userPath = false;

QString g_version = "1.0.0.1";

//#define DT_BETA

#if defined(DT_BETA)
QString g_model = "Beta";
#elif defined(QT_DEBUG)
QString g_model = "Debug";
#else
QString g_model = "Release";
#endif

Json::Json(QObject* parent)
	: QObject(parent)
{
	PRINT_CON_DESTRUCTION(Json);
}

Json::~Json()
{
	PRINT_CON_DESTRUCTION(Json);

	SAFE_DELETE_A(m_hwdConfig.workingCurrent);

	SAFE_DELETE_A(m_hwdConfig.componentResistor);

	SAFE_DELETE_A(m_hwdConfig.componentVoltage);

	SAFE_DELETE_A(m_udsConfig.version);

	SAFE_DELETE_A(m_udsConfig.dtc);
}

Json* Json::getInstance()
{
	static Json json;
	return &json;
}

void Json::setUsingUserPath(bool on)
{
	m_userPath = on;
}

bool Json::usingUserPath()
{
	return m_userPath;
}

QString Json::getUsingPath()
{
	return m_userPath ? QDir::homePath() : ".";
}

QString Json::getLastError() const
{
	return m_lastError;
}

QStringList Json::getErrorList() const
{
	return m_errorList;
}

void Json::setTypeName(const QString& typeName)
{
	m_typeName = typeName;
}

QString Json::getTypeName() const
{
	return m_typeName;
}

bool Json::initialize(bool update)
{
	bool result = false, success = true;
	do
	{
		m_jsonPath = QString("%1\\Config\\Json\\%2")
			.arg(getUsingPath(), m_typeName.isEmpty() ? "Default" : m_typeName);
		m_sharePath = QString("%1\\Config\\Share").arg(getUsingPath());

		const QStringList pathList = { m_jsonPath,m_sharePath };
		for (int i = 0; i < pathList.count(); i++)
		{
			if (!utility::makePath(pathList[i]))
			{
				success = false;
				setLastError(utility::getLastError());
				break;
			}
		}

		if (!success)
		{
			break;
		}

		bool (Json:: * readFnc[])(const QString&) =
		{
			&Json::readShareJsonFile,
			&Json::readDefJsonFile,
			&Json::readHwdJsonFile,
			&Json::readUdsJsonFile,
			&Json::readImgJsonFile,
			&Json::readOtherJsonFile
		};

		bool (Json:: * writeFnc[])(const QString&) =
		{
			&Json::writeShareJsonFile,
			&Json::writeDefJsonFile,
			&Json::writeHwdJsonFile,
			&Json::writeUdsJsonFile,
			&Json::writeImgJsonFile,
			&Json::writeOtherJsonFile
		};

		bool (Json:: * updateFnc[])(const QString&) =
		{
			&Json::updateShareJsonFile,
			&Json::updateDefJsonFile,
			&Json::updateHwdJsonFile,
			&Json::updateUdsJsonFile,
			&Json::updateImgJsonFile,
			&Json::updateOtherJsonFile
		};

		const QStringList fileNameList =
		{
			"share.json",
			"def.json",
			"hwd.json",
			"uds.json",
			"img.json",
			"other.json"
		};

		RUN_BREAK(fileNameList.size() != sizeof(readFnc) / sizeof(*readFnc) ||
			fileNameList.size() != sizeof(writeFnc) / sizeof(*writeFnc) ||
			fileNameList.size() != sizeof(updateFnc) / sizeof(*updateFnc),
			"文件名列表与函数指针数组大小不一致");

		for (int i = 0; i < fileNameList.size(); i++)
		{
			const QString file = QString("%1\\%2").arg(i ? m_jsonPath : m_sharePath, fileNameList[i]);

			if (!QFileInfo(file).exists() || update)
			{
				if (update ? !(this->*updateFnc[i])(file) : !(this->*writeFnc[i])(file))
				{
					success = false;
					break;
				}
			}

			if (!(this->*readFnc[i])(file))
			{
				success = false;
				break;
			}
		}

		if (!success)
		{
			break;
		}

		QStringList(Json:: * key1Fnc[])() const =
		{
			&Json::getDeviceConfigKeyList,
			&Json::getHardwareConfigKeyList,
			&Json::getRelayConfigKeyList,
			&Json::getUserConfigKeyList,
			&Json::getRangeConfigKeyList,
			&Json::getThresholdConfigKeyList,
			//&Json::getEnableConfigKeyList,
			&Json::getKeyVoltageConfigKeyList,
			&Json::getStaticCurrentConfigKeyList,
		};

		QStringList(Json:: * explain1Fnc[])() const =
		{
			&Json::getDeviceConfigExplain,
			&Json::getHardwareConfigExplain,
			&Json::getRelayConfigExplain,
			&Json::getUserConfigExplain,
			&Json::getRangeConfigExplain,
			&Json::getThresholdConfigExplain,
			//&Json::getEnableConfigExplain,
			&Json::getKeyVoltageConfigExplain,
			&Json::getStaticCurrentConfigExplain,
		};

		int (Json:: * countFnc[])() const =
		{
			&Json::getImageConfigCount,
			&Json::getComponentVoltageConfigCount,
			&Json::getWorkingCurrentConfigCount,
			&Json::getComponentResistorConfigCount,
			&Json::getVersionConfigCount,
			&Json::getDtcConfigCount
		};

		QStringList(Json:: * key2Fnc[])() const =
		{
			&Json::getChildImageConfigKeyList,
			&Json::getChildComponentVoltageConfigValueList,
			&Json::getChildComponentResistorConfigKeyList,
			&Json::getChildWorkingCurrentConfigKeyList,
			&Json::getChildVersionConfigKeyList,
			&Json::getChildDtcConfigKeyList
		};

		QStringList(Json:: * explain2Fnc[])() const =
		{
			&Json::getImageConfigExplain,
			&Json::getComponentVoltageConfigExplain,
			&Json::getComponentResistorConfigExplain,
			&Json::getWorkingCurrentConfigExplain,
			&Json::getVersionConfigExplain,
			&Json::getDtcConfigExplain
		};

		RUN_BREAK(sizeof(key1Fnc) / sizeof(*key1Fnc) != sizeof(explain1Fnc) / sizeof(*explain1Fnc),
			"键值1函数指针数组与说明1函数指针数组大小不一致");

		RUN_BREAK(sizeof(countFnc) / sizeof(*countFnc) != sizeof(key2Fnc) / sizeof(*key2Fnc),
			"数量函数指针数组与键值2函数指针数组大小不一致");

		RUN_BREAK(sizeof(countFnc) / sizeof(*countFnc) != sizeof(explain2Fnc) / sizeof(*explain2Fnc),
			"数量函数指针数组与说明2函数指针数组大小不一致");

		for (int i = 0; i < sizeof(key1Fnc) / sizeof(*key1Fnc); ++i)
		{
			if ((this->*key1Fnc[i])().size() != (this->*explain1Fnc[i])().size())
			{
				setLastError(Q_SPRINTF("key1Fnc[%d].size() != explain1Fnc[%d].size()", i, i));
				success = false;
				break;
			}
		}

		if (!success)
		{
			break;
		}

		for (int i = 0; i < sizeof(countFnc) / sizeof(*countFnc); ++i)
		{
			for (int j = 0; j < (this->*countFnc[i])(); ++j)
			{
				UTIL_JSON->setChildImageConfigKeyListIndex(j);
				if ((this->*key2Fnc[i])().size() != (this->*explain2Fnc[i])().size())
				{
					setLastError(Q_SPRINTF("key2Fnc[%d].size() != explain2Fnc[%d].size(),index = %d", i, i, j));
					success = false;
					break;
				}
			}

			if (!success)
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

QStringList Json::getAllMainKey() const
{
	const QStringList keys =
	{
		"设备配置",//0
		"硬件配置",//1
		"继电器配置",//2
		"范围配置",//3
		"阈值配置",//4
		"图像配置",//5
		"图像动态配置",//17
		"工作电流配置",//8
		"静态电流配置",//10
		"按键电压配置",//6
		"组件电压配置",//7
		"组件电阻配置",//9
		"版本配置",//11
		"诊断故障码配置",//12
		"启用配置",//13
		"逻辑配置",//17
		"用户配置",//14
		"其他配置1",//15
		"其他配置2"//16
	};
	return keys;
}

const char* Json::getLibVersion()
{
	return "2.0.0.9";
}

QString Json::getJsonPath() const
{
	return m_jsonPath;
}

QString Json::getSharePath() const
{
	return m_sharePath;
}

int Json::getDeviceConfigCount() const
{
	return m_deviceConfigObj.count();
}

QString Json::getDeviceConfigValue(const QString& key) const
{
	return m_deviceConfigObj[key].toString();
}

QStringList Json::getDeviceConfigKeyList() const
{
	return m_deviceConfigKeyList;
}

const DeviceConfig& Json::getParsedDeviceConfig() const
{
	return m_defConfig.device;
}

const QJsonObject& Json::getDeviceConfigObj() const
{
	return m_deviceConfigObj;
}

bool Json::setDeviceConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		QString temp = value;

		if (key == "采集卡通道数" || key == "采集卡通道号" ||
			key == "CAN卡类型" || key == "CAN卡设备号" ||
			key == "CAN卡通道号" || key == "CAN卡仲裁域" ||
			key == "CAN卡数据域" || key == "CAN卡拓展帧" ||
			key == "CAN卡对端端口") {
			const int number = temp.toInt(&convert);
			if (!convert) {
				setLastError(key + "必须为整数");
				break;
			}

			if (key == "采集卡通道号") {
				if (number < 0 || number > 1) {
					setLastError(getDeviceConfigValue("采集卡名称") + key + ",仅支持0或者1通道");
					break;
				}
			}
			else if (key == "CAN卡类型") {
				if (number < can::NULL_CAN || number > can::GC_USBCANFD) {
					setLastError("不支持的CAN卡类型");
					break;
				}
			}
			else if (key == "CAN卡设备号") {
				if (number < 0) {
					setLastError("CAN卡设备号必须大于等于0");
					break;
				}
			}
			else if (key == "CAN卡通道号") {
				if (number < 0 || number > 1) {
					setLastError("CAN卡通道号仅支持0或1");
					break;
				}
			}
			else if (key == "CAN卡对端端口") {
				if (number <= 0 || number > 65535) {
					setLastError("CAN卡对端端口无效");
					break;
				}
			}
		}
		else if (key == "采集卡名称") {
			const QStringList support = CaptureCard::getSupportCap();
			if (!support.contains(temp)) {
				setLastError(QString("采集卡仅支持[%1]").arg(support.join(",")));
				break;
			}
		}
		else if (key == "条码判断") {
			if (temp.isEmpty()) {
				setLastError("条码判断如果为空,请写NULL");
				break;
			}
		}
		else if (key == "条码长度") {
			const int number = temp.toInt(&convert);
			if (convert && number < 0) {
				setLastError("条码长度不可小于0");
				break;
			}
			temp = (!convert) ? N_TO_Q_STR(temp.length()) : temp;
		}
		else if (key == "机种名称" && !m_typeName.isEmpty() && value != m_typeName) {
			setLastError("软件集成模式下,无法进行更改机种名称");
			break;
		}
		else if (key == "CAN卡对端地址") {
			if (!utility::isIpAddressValid(value)) {
				setLastError("CAN卡对端地址无效");
				break;
			}
		}

		if (!m_deviceConfigObj.contains(key)) {
			setLastError(QString("%1非法的键").arg(key));
			break;
		}
		m_deviceConfigObj[key] = temp;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getDeviceConfigExplain() const
{
	const QStringList explain =
	{
		"界面标题",
		"鼠标悬停查看提示",
		"设备编号,默认:0",
		"通道编号,默认:0",
		"设备的IP地址",
		"设备的IP端口",
		"默认:500",
		"默认:2000",
		"0禁用,1启用",
		CaptureCard::getSupportCap().join(" "),
		"采集卡通道编号",
		"前N个字符串",
		"条码总长度,可扫码获取长度"
	};
	return explain;
}

QString Json::getHardwareConfigValue(const QString& key) const
{
	return m_hardwareConfigObj[key].toString();
}

int Json::getHardwareConfigCount() const
{
	return m_hardwareConfigObj.count();
}

QStringList Json::getHardwareConfigKeyList() const
{
	return m_hardwareConfigKeyList;
}

const HardwareConfig& Json::getParseHardwareConfig() const
{
	return m_defConfig.hardware;
}

bool Json::setHardwareConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		if (key == "继电器类型") {
			auto number = value.toInt(&convert);
			if (!convert) {
				setLastError(QString("%1必须为数字").arg(key));
				break;
			}
			
			if (number != relay::CW_MR_DO16_KN && number != relay::CW_EMR_DO16 && number != relay::ZQWL_2BZRC32) {
				setLastError(QString("%1不支持的继电器类型").arg(value));
				break;
			}
		}
		else if (key == "继电器串口") {
			auto type = getHardwareConfigValue("继电器类型");
			if (type.toInt() == relay::ZQWL_2BZRC32) {
				if (!utility::isIpAddressValid(value)) {
					setLastError(QString("%1无效的IP地址").arg(value));
					break;
				}
			}
			else {
				value.toInt(&convert);
				if (!convert)
				{
					setLastError(QString("%1必须为数字").arg(key));
					break;
				}
			}
		}
		else if (key == "继电器波特率") {
			auto type = getHardwareConfigValue("继电器类型");
			if (type.toInt() == relay::ZQWL_2BZRC32) {
				auto number = value.toInt(&convert);
				if (!convert)
				{
					setLastError(QString("%1无效的端口").arg(key));
					break;
				}

				if (number < 0 || number > 65535) {
					setLastError(QString("%1无效的端口").arg(key));
					break;
				}
			}
			else {
				value.toInt(&convert);
				if (!convert)
				{
					setLastError(QString("%1必须为数字").arg(key));
					break;
				}
			}
		}
		else {
			value.toInt(&convert);
			if (!convert)
			{
				setLastError(QString("%1必须为数字").arg(key));
				break;
			}
		}

		if (!m_hardwareConfigObj.contains(key))
		{
			setLastError(QString("%1非法的键").arg(key));
			break;
		}
		m_hardwareConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getHardwareConfigExplain() const
{
	const QStringList explain =
	{
		"串口编号",//1
		"默认:19200",//2
		"单位:伏(V)",//3
		"单位:安培(A)",//4
		"1:MRDO16KN;2:EMRDO16;3:2BZRC32",//5
		"串口编号/IP地址",//6
		"默认:19200/IP端口",//7
		"串口编号",//8
		"默认:9600",//9
		"串口编号",//10
		"默认:9600",//11
		"预留拓展",//12
		"预留拓展",//13
		"预留拓展",//14
		"预留拓展",//15
		"预留拓展",//16
		"预留拓展",//17
		"预留拓展",//18
		"预留拓展"//19
	};
	return explain;
}

QString Json::getRelayConfigValue(const QString& key) const
{
	return m_relayConfigObj[key].toString();
}

int Json::getRelayConfigCount() const
{
	return m_relayConfigObj.count();
}

const RelayConfig& Json::getParsedRelayConfig() const
{
	return m_defConfig.relay;
}

QStringList Json::getRelayConfigKeyList() const
{
	return m_relayConfigKeyList;
}

bool Json::setRelayConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		value.toInt(&convert);
		if (!convert)
		{
			setLastError(QString("%1必须为整数").arg(key));
			break;
		}

		if (!m_relayConfigObj.contains(key))
		{
			setLastError(QString("%1非法的键").arg(key));
			break;
		}
		m_relayConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getRelayConfigExplain() const
{
	const QStringList explain =
	{
		"用于上电,接正极",
		"用于上电,接负极",
		"用于检测按键电压",
		"用于检测静态电流",
		"用于图像转换出画",
		"用于检测紧急录制",
		"用于检测声音",
		"运行TS亮",
		"结果NG亮",
		"结果OK亮"
	};
	return explain;
}

QStringList Json::getUserConfigKeyList() const
{
	return m_userConfigKeyList;
}

QStringList Json::getUserConfigExplain() const
{
	const QStringList explain = { "用户名","密码" };
	return explain;
}

QString Json::getUserConfigValue(const QString& key) const
{
	return m_userConfigObj[key].toString();
}

int Json::getUserConfigCount() const
{
	return m_userConfigObj.count();
}

bool Json::setUserConfigValue(const QString& key, const QString& value)
{
	bool result = false;
	do
	{
		if (!m_userConfigObj.contains(key))
		{
			setLastError(QString("非法的键值[%1]").arg(key));
			break;
		}
		m_userConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

bool Json::getUserPrivileges() const
{
	const QString&& userName = getUserConfigValue("用户名").toUpper();
	return (userName == "ROOT" || userName == "INVO");
}

QString Json::getRangeConfigValue(const QString& key) const
{
	return m_rangeConfigObj[key].toString();
}

int Json::getRangeConfigCount() const
{
	return m_rangeConfigObj.count();
}

QStringList Json::getRangeConfigKeyList() const
{
	return m_rangeConfigKeyList;
}

const RangeConfig& Json::getParsedRangeConfig() const
{
	return m_defConfig.range;
}

bool Json::setRangeConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		const auto split = value.split("~", QString::SkipEmptyParts);
		if (split.size() != 2)
		{
			setLastError(QString("%1格式错误,范围格式规则为:[0.0~10.0]").arg(key));
			break;
		}

		for (int i = 0; i < split.size(); i++)
		{
			split[i].toFloat(&convert);
			if (!convert)
			{
				setLastError(QString("%1中的数据,必须为数字").arg(key));
				break;
			}
		}

		if (!convert)
		{
			break;
		}

		if (!m_rangeConfigObj.contains(key))
		{
			setLastError(QString("%1非法的键").arg(key));
			break;
		}
		m_rangeConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getRangeConfigExplain() const
{
	const QStringList explain =
	{
		"单位:MB(兆字节)",
		"单位:MM(毫米)",
		"单位:MM(毫米)",
		"单位:°(度)",
		"单位:PX(像素)",
		"单位:A(安培)",
		"单位:A(安培)"
	};
	return explain;
}

QString Json::getThresholdConfigValue(const QString& key) const
{
	return m_thresholdConfigObj[key].toString();
}

int Json::getThresholdConfigCount() const
{
	return m_thresholdConfigObj.count();
}

QStringList Json::getThresholdConfigKeyList() const
{
	return m_thresholdConfigKeyList;
}

const ThresholdConfig& Json::getParsedThresholdConfig() const
{
	return m_defConfig.threshold;
}

bool Json::setThresholdConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		const float data = value.toFloat(&convert);
		if (!convert)
		{
			setLastError(QString("%1,必须为数字").arg(key));
			break;
		}

		if (key == "启动延时" || key == "读取版本延时" || key == "重读版本次数" || key == "播放起始时间")
		{
			if (data < -1e-6)
			{
				setLastError(QString("%1,不可小于0").arg(key));
				break;
			}
		}

		if (!m_thresholdConfigObj.contains(key))
		{
			setLastError(QString("%1,非法的键").arg(key));
			break;
		}
		m_thresholdConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getThresholdConfigExplain() const
{
	const QStringList explain =
	{
		"单位:MS(毫秒)",//1
		"单位:A(安培)",//2
		"单位:A(安培)",//3
		"单位:MS(毫秒)",//4
		"单位:MS(毫秒)",//5
		"单位:次数",//6
		"单位:MS(毫秒)"//7
	};
	return explain;
}

const ImageConfig& Json::getParsedImageConfig() const
{
	return m_defConfig.image;
}

int Json::getImageConfigCount() const
{
	return m_imageConfigObj.count();
}

QStringList Json::getParentImageConfigKeyList() const
{
	return m_parentImageKeyList;
}

void Json::setChildImageConfigKeyListIndex(int index)
{
	m_childImageConfigIndex = index;
}

QStringList Json::getChildImageConfigKeyList(int index) const
{
	return m_childImageKeyList[index];
}

QStringList Json::getChildImageConfigKeyList() const
{
	return m_childImageKeyList[m_childImageConfigIndex];
}

QString Json::getImageConfigValue(const QString& parentKey, const QString& childKey) const
{
	return m_imageConfigObj.value(parentKey).toObject().value(childKey).toString();
}

void Json::setImageConfigKey(const QString& oldParentKey, const QString& newParentKey)
{
	return;
}

bool Json::setImageConfigValue(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		if (childKey != "颜色")
		{
			const int number = value.toInt(&convert);
			if (!convert)
			{
				setLastError(QString("[%1,%2]必须为整数").arg(parentKey, childKey));
				break;
			}

			if (childKey == "纯度" && (number < 0 || number > 100))
			{
				setLastError(QString("[%1,%2]范围必须在0~100之间").arg(parentKey, childKey));
				break;
			}
			else if (childKey == "R" || childKey == "G" || childKey == "B" || childKey == "误差")
			{
				if (number < 0 || number > 0xff)
				{
					setLastError(QString("[%1,%2]范围必须在0~255之间").arg(parentKey, childKey));
					break;
				}
			}
			else if (childKey == "X坐标" && (number < 0 || number > VIDEO_WIDTH))
			{
				setLastError(QString("[%1,%2]范围必须在0~%3之间").arg(parentKey, childKey).arg(VIDEO_WIDTH));
				break;
			}
			else if (childKey == "Y坐标" && (number < 0 || number > VIDEO_HEIGHT))
			{
				setLastError(QString("[%1,%2]范围必须在0~%3之间").arg(parentKey, childKey).arg(VIDEO_HEIGHT));
				break;
			}
			else if (childKey == "宽")
			{
				const int x = getImageConfigValue(parentKey, "X坐标").toInt();
				if (x + number < 0 || x + number > VIDEO_WIDTH)
				{
					setLastError(QString("[%1,X坐标]+[%2,%3]范围已超出图像宽度%4").arg(parentKey, parentKey, childKey).arg(VIDEO_WIDTH));
					break;
				}
			}
			else if (childKey == "高")
			{
				const int y = getImageConfigValue(parentKey, "Y坐标").toInt();
				if (y + number < 0 || y + number > VIDEO_HEIGHT)
				{
					setLastError(QString("[%1,Y坐标]+[%2,%3]范围已超出图像高度%4").arg(parentKey, parentKey, childKey).arg(VIDEO_HEIGHT));
					break;
				}
			}
		}

		if (!m_imageConfigObj.contains(parentKey))
		{
			setLastError(QString("%1非法的键").arg(parentKey));
			break;
		}

		QJsonObject object = m_imageConfigObj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("%1非法的键").arg(childKey));
			break;
		}
		object[childKey] = value;
		m_imageConfigObj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QStringList Json::getImageConfigExplain(int index) const
{
	const QStringList explain0 =
	{
		"0[均值],1[色彩]",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用"
	};

	const QStringList explain1 =
	{
		"判断语法:!=黑色,==黑色",
		"单位:百分比",
		"三原色:Red(红色)",
		"三原色:Green(绿色)",
		"三原色:Blue(蓝色)",
		"单位:PX(像素)",
		"单位:MM(毫米)",
		"单位:MM(毫米)",
		"单位:MM(毫米)",
		"单位:MM(毫米)",
		"0禁用,1启用"
	};

	if (index == getImageConfigCount() - 1)
	{
		return explain0;
	}
	return explain1;
}

QStringList Json::getImageConfigExplain() const
{
	const QStringList explain0 =
	{
		"0[均值],1[色彩]",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用",
		"0禁用,1启用"
	};

	const QStringList explain1 =
	{
		"判断语法,!=黑色(不等于黑色),==黑色(等于黑色)",
		"单位:百分比",
		"三原色:Red(红色)",
		"三原色:Green(绿色)",
		"三原色:Blue(蓝色)",
		"单位:PX(像素)",
		"单位:MM(毫米)",
		"单位:MM(毫米)",
		"单位:MM(毫米)",
		"单位:MM(毫米)",
		"0禁用,1启用"
	};

	if (m_childImageConfigIndex == getImageConfigCount() - 1)
	{
		return explain0;
	}
	return explain1;
}

int Json::getImageDynamicConfigCount() const
{
	return m_imageDynamicConfigKeyList.size();
}

bool Json::setImageDynamicConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do 
	{
		if (key == "判定次数" || key == "持续时间" || key == "动态频率")
		{
			value.toInt(&convert);
			RUN_BREAK(!convert, QString("%1必须为整数").arg(key));
		}
		else if (key == "动态占比" || key == "二值化阈值")
		{
			value.toDouble(&convert);
			RUN_BREAK(!convert, QString("%1必须为数字").arg(key));
		}

		RUN_BREAK(!m_imageDynamicConfigObj.contains(key), QString("非法的键[%1]").arg(key));
		m_imageDynamicConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getImageDynamicConfigValue(const QString& key) const
{
	return m_imageDynamicConfigObj.value(key).toString();
}

QStringList Json::getImageDynamicConfigKeyList() const
{
	return m_imageDynamicConfigKeyList;
}

QStringList Json::getImageDynamicConfigExplain() const
{
	return m_imageDynamicConfigExplainList;
}

ImageDynamicConfig* Json::getParsedImageDynamicConfig() const
{
	return const_cast<ImageDynamicConfig*>(&m_defConfig.imageDynamic);
}

int Json::getEnableConfigCount() const
{
	return m_enableConfigKeyList.count();
}

QStringList Json::getEnableConfigKeyList() const
{
	return m_enableConfigKeyList;
}

QStringList Json::getEnableConfigValueList() const
{
	return m_enableConfigValueList;
}

QString Json::getEnableConfigValue(const QString& key) const
{
	return m_enableConfigObj[key].toString();
}

bool Json::setEnableConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		value.toInt(&convert);
		if (!convert)
		{
			setLastError(QString("%1,必须为整数").arg(key));
			break;
		}

		if (value != "0" && value != "1")
		{
			setLastError("该值只能为0或1");
			break;
		}

		if (!m_enableConfigObj.contains(key))
		{
			setLastError(QString("%1,非法的键").arg(key));
			break;
		}
		m_enableConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getEnableConfigExplain() const
{
	QStringList explain = {};
	for (int i = explain.size(); i < 100; i++)
		explain.append("0禁用,1启用");
	return explain;
}

int Json::getLogicConfigCount() const
{
	return m_logicConfigObj.count();
}

QStringList Json::getLogicConfigKeyList() const
{
	return m_logicConfigKeyList;
}

QStringList Json::getLogicConfigValueList() const
{
	return m_logicConfigValueList;
}

QString Json::getLogicConfigValue(const QString& key) const
{
	return m_logicConfigObj.value(key).toString();
}

bool Json::setLogicConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		value.toInt(&convert);
		if (!convert)
		{
			setLastError(QString("%1,必须为整数").arg(key));
			break;
		}

		if (value != "0" && value != "1")
		{
			setLastError("该值只能为0或1");
			break;
		}

		if (!m_logicConfigObj.contains(key))
		{
			setLastError(QString("%1,非法的键").arg(key));
			break;
		}
		m_logicConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getLogicConfigExplain() const
{
	QStringList explain;
	for (int i = 0; i < m_logicConfigKeyList.size(); ++i)
	{
		explain.append("是否执行");
	}
	return explain;
}

DefConfig* Json::getParsedDefConfig() const
{
	return const_cast<DefConfig*>(&m_defConfig);
}

int Json::getComponentVoltageConfigCount() const
{
	return m_componentVoltageConfigObj.count();
}

QStringList Json::getChildComponentVoltageConfigKeyList() const
{
	return m_componentVoltageConfigKeyList;
}

QStringList Json::getChildComponentVoltageConfigValueList() const
{
	return m_componentVoltageConfigValueList;
}

QStringList Json::getParentComponentVoltageConfigKeyList() const
{
	QMultiMap<int, QString> map;
	auto&& keys = m_componentVoltageConfigObj.keys();
	for (const auto& x : keys)
	{
		auto obj = m_componentVoltageConfigObj[x].toObject();
		Q_ASSERT_X(obj.contains("继电器IO"), __FUNCTION__, "组件电压配置,子键[继电器IO]不存在");
		auto io = obj["继电器IO"].toString().toInt();
		map.insert(io, x);
	}

	QStringList list;
	for (auto iter = map.begin(); iter != map.end(); ++iter)
	{
		list.append(iter.value());
	}
	return list;
}

QString Json::getComponentVoltageConfigValue(const QString& parentKey, const QString& childKey) const
{
	return m_componentVoltageConfigObj.value(parentKey).toObject().value(childKey).toString();
}

void Json::setComponentVoltageConfigKey(const QString& oldParentKey, const QString& newParentKey)
{
	do
	{
		if (!m_componentVoltageConfigObj.contains(oldParentKey))
		{
			break;
		}

		QJsonObject object = m_componentVoltageConfigObj[oldParentKey].toObject();
		m_componentVoltageConfigObj.remove(oldParentKey);
		m_componentVoltageConfigObj.insert(newParentKey, object);
	} while (false);
	return;
}

bool Json::setComponentVoltageConfigValue(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		if (childKey != "继电器IO" && childKey != "延时")
		{
			value.toFloat(&convert);
			if (!convert)
			{
				setLastError(QString("%1,%2,必须为数字").arg(parentKey, childKey));
				break;
			}
		}
		else
		{
			int number = value.toInt(&convert);
			if (!convert)
			{
				setLastError(QString("%1,%2,必须为整数").arg(parentKey, childKey));
				break;
			}

			if (childKey == "延时")
			{
				RUN_BREAK(number < 0, QString("%1,%2,必须大于等于0").arg(parentKey, childKey));
			}
			else if (childKey == "继电器IO")
			{
				RUN_BREAK(number < 0 || number > 15, QString("%1,%2,必须在0~15范围内").arg(parentKey, childKey));
			}
		}

		if (!m_componentVoltageConfigObj.contains(parentKey))
		{
			setLastError(QString("%1,非法的键").arg(parentKey));
			break;
		}

		QJsonObject object = m_componentVoltageConfigObj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("%1,非法的键").arg(childKey));
			break;
		}
		object[childKey] = value;
		m_componentVoltageConfigObj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QJsonObject& Json::getComponentVoltageConfigObj() const
{
	return const_cast<QJsonObject&>(m_componentVoltageConfigObj);
}

QStringList Json::getComponentVoltageConfigExplain() const
{
	const QStringList explain = { "单位:V(伏)", "单位:V(伏)", "继电器接口编号", "单位:MS(毫秒)" };
	return explain;
}

void Json::addComponentVoltageConfigKeyValue(const QMap<QString, QString>& keyValue)
{
	const QStringList childKey = m_componentVoltageConfigKeyList;
	for (auto iter = keyValue.begin(); iter != keyValue.end(); ++iter)
	{
		QJsonObject obj;
		QStringList valueList = m_componentVoltageConfigValueList;

		QStringList split = iter.value().split(",", QString::SkipEmptyParts);
		if (split.size() == 1)
		{
			QStringList range = iter.value().split("~", QString::SkipEmptyParts);
			if (range.size() == 2)
			{
				valueList[0] = range[1];
				valueList[1] = range[0];
			}
		}
		else if (split.size() == 2)
		{
			QStringList range = split[0].split("~", QString::SkipEmptyParts);
			if (range.size() == 2)
			{
				//配置里的值为上限-下限,此处配置值为下限-上限,需要调换位置.
				valueList[0] = range[1];
				valueList[1] = range[0];
				valueList[2] = split[1];
			}
		}

		for (int i = 0; i < childKey.size(); ++i)
		{
			obj.insert(childKey[i], valueList[i]);
		}

		m_componentVoltageConfigObj.insert(iter.key().isEmpty() ?
			utility::getCurrentTime() : iter.key(), obj);
	}
}

int Json::getKeyVoltageConfigCount() const
{
	return m_keyVoltageConfigObj.count();
}

QStringList Json::getKeyVoltageConfigKeyList() const
{
	return m_keyVoltageConfigKeyList;
}

QStringList Json::getKeyVoltageConfigValueList() const
{
	return m_keyVoltageConfigValueList;
}

QString Json::getKeyVoltageConfigValue(const QString& key) const
{
	return m_keyVoltageConfigObj.value(key).toString();
}

bool Json::setKeyVoltageConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		value.toFloat(&convert);
		if (!convert)
		{
			setLastError(QString("%1,必须为数字").arg(key));
			break;
		}

		if (!m_keyVoltageConfigObj.contains(key))
		{
			setLastError(QString("%1,非法的键").arg(key));
			break;
		}
		m_keyVoltageConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getKeyVoltageConfigExplain() const
{
	const QStringList explain = { "单位:V(伏)","单位:V(伏)","单位:V(伏)","单位:V(伏)" };
	return explain;
}

int Json::getWorkingCurrentConfigCount() const
{
	return m_workingCurrentConfigObj.count();
}

QStringList Json::getParentWorkingCurrentConfigKeyList() const
{
	return m_workingCurrentConfigObj.keys();
}

QStringList Json::getChildWorkingCurrentConfigKeyList() const
{
	return m_workingCurrentConfigKeyList;
}

QStringList Json::getChildWorkingCurrentConfigValueList() const
{
	return m_workingCurrentConfigValueList;
}

QStringList Json::getChildWorkingCurrentConfigValueList(int index) const
{
	const QStringList list[2]{ { "0.38","0.23","12.0" } ,{"0.35","0.21","16.0"} };
	return list[index > 1 ? 1 : index];
}

QString Json::getWorkingCurrentConfigValue(const QString& parentKey, const QString& childKey) const
{
	return m_workingCurrentConfigObj.value(parentKey).toObject().value(childKey).toString();
}

void Json::setWorkingCurrentConfigKey(const QString& oldParentKey, const QString& newParentKey)
{
	do
	{
		if (!m_workingCurrentConfigObj.contains(oldParentKey))
		{
			break;
		}
		QJsonObject object = m_workingCurrentConfigObj[oldParentKey].toObject();
		m_workingCurrentConfigObj.remove(oldParentKey);
		m_workingCurrentConfigObj.insert(newParentKey, object);
	} while (false);
	return;
}

bool Json::setWorkingCurrentConfigValue(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		value.toFloat(&convert);
		if (!convert)
		{
			setLastError(QString("%1,%2必须为数字").arg(parentKey, childKey));
			break;
		}

		if (!m_workingCurrentConfigObj.contains(parentKey))
		{
			setLastError(QString("%1,非法的键").arg(parentKey));
			break;
		}

		QJsonObject object = m_workingCurrentConfigObj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("%1,非法的键").arg(childKey));
			break;
		}
		object[childKey] = value;
		m_workingCurrentConfigObj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QJsonObject& Json::getWorkingCurrentConfigObj() const
{
	return const_cast<QJsonObject&>(m_workingCurrentConfigObj);
}

QStringList Json::getWorkingCurrentConfigExplain() const
{
	const QStringList explain = { "单位:A(安培)", "单位:A(安培)", "单位:V(伏)" };
	return explain;
}

int Json::getStaticCurrentConfigCount() const
{
	return m_staticCurrentConfigObj.count();
}

QStringList Json::getStaticCurrentConfigKeyList() const
{
	return m_staticCurrentConfigKeyList;
}

QStringList Json::getStaticCurrentConfigValueList() const
{
	return m_staticCurrentConfigValueList;
}

QString Json::getStaticCurrentConfigValue(const QString& key) const
{
	return m_staticCurrentConfigObj.value(key).toString();
}

bool Json::setStaticCurrentConfigValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		value.toFloat(&convert);
		if (!convert)
		{
			setLastError(QString("%1,必须为数字").arg(key));
			break;
		}

		if (!m_staticCurrentConfigObj.contains(key))
		{
			setLastError(QString("%1,非法的键").arg(key));
			break;
		}
		m_staticCurrentConfigObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QStringList Json::getStaticCurrentConfigExplain() const
{
	const QStringList explain = { "单位:μA(微安)","单位:μA(微安)" };
	return explain;
}

QStringList Json::getParentComponentResistorConfigKeyList() const
{
	return m_componentResistorConfigObj.keys();
}

QStringList Json::getChildComponentResistorConfigKeyList() const
{
	return m_componentResistorConfigKeyList;
}

QStringList Json::getChildComponentResistorConfigValueList() const
{
	return m_componentResistorConfigValueList;
}

int Json::getComponentResistorConfigCount() const
{
	return m_componentResistorConfigObj.count();
}

QString Json::getComponentResistorConfigValue(const QString& parentKey, const QString& childKey) const
{
	return m_componentResistorConfigObj.value(parentKey).toObject().value(childKey).toString();
}

void Json::setComponentResistorConfigKey(const QString& oldParentKey, const QString& newParentKey)
{
	do
	{
		if (!m_componentResistorConfigObj.contains(oldParentKey))
		{
			break;
		}

		QJsonObject object = m_componentResistorConfigObj[oldParentKey].toObject();
		m_componentResistorConfigObj.remove(oldParentKey);
		m_componentResistorConfigObj.insert(newParentKey, object);
	} while (false);
	return;
}

bool Json::setComponentResistorConfigValue(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		if (childKey != "继电器IO")
		{
			value.toInt(&convert);
			if (!convert)
			{
				setLastError(QString("%1,%2,必须为整数").arg(parentKey, childKey));
				break;
			}
		}
		else
		{
			value.toFloat(&convert);
			if (!convert)
			{
				setLastError(QString("%1,%2,必须为整数").arg(parentKey, childKey));
				break;
			}
		}

		if (!m_componentResistorConfigObj.contains(parentKey))
		{
			setLastError(QString("%1,非法的键").arg(parentKey));
			break;
		}

		QJsonObject object = m_componentResistorConfigObj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("%1,非法的键").arg(childKey));
			break;
		}
		object[childKey] = value;
		m_componentResistorConfigObj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QJsonObject& Json::getComponentResistorConfigObj() const
{
	return const_cast<QJsonObject&>(m_componentResistorConfigObj);
}

QStringList Json::getComponentResistorConfigExplain() const
{
	const QStringList explain = { "单位:Ω(欧姆)","单位:Ω(欧姆)","继电器接口编号" };
	return explain;
}

HwdConfig* Json::getParsedHwdConfig() const
{
	return const_cast<HwdConfig*>(&m_hwdConfig);
}

int Json::getVersionConfigCount() const
{
	return m_versionConfigObj.count();
}

QStringList Json::getParentVersionConfigKeyList() const
{
	return m_versionConfigObj.keys();
}

QStringList Json::getChildVersionConfigKeyList() const
{
	return m_versionConfigKeyList;
}

QStringList Json::getChildVersionConfigValueList() const
{
	return m_versionConfigValueList;
}

QString Json::getVersionConfigValue(const QString& parentKey, const QString& childKey) const
{
	return m_versionConfigObj.value(parentKey).toObject().value(childKey).toString();
}

void Json::setVersionConfigKey(const QString& oldParentKey, const QString& newParentKey)
{
	do
	{
		if (!m_versionConfigObj.contains(oldParentKey))
		{
			break;
		}

		const QJsonObject object = m_versionConfigObj[oldParentKey].toObject();
		m_versionConfigObj.remove(oldParentKey);
		m_versionConfigObj.insert(newParentKey, object);
	} while (false);
	return;
}

bool Json::setVersionConfigValue(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		QString newValue = value;

		if (childKey == "DID")
		{
			if (!(newValue.contains("0x") || newValue.contains("0X")))
			{
				if (newValue.length() == 4)
				{
					newValue.insert(0, "0x");
				}
				else
				{
					setLastError(QString("%1,%2格式错误,参照格式0xF16C").arg(parentKey, childKey));
					break;
				}
			}
			else
			{
				if (!(newValue.length() == 6))
				{
					setLastError(QString("%1,%2格式错误,参照格式0xF16C").arg(parentKey, childKey));
					break;
				}
			}

			newValue.toInt(&convert, 16);
			if (!convert)
			{
				setLastError(QString("%1,%2格式错误,参照格式0xF16C").arg(parentKey, childKey));
				break;
			}
		}
		else if (childKey == "编码")
		{
			const QStringList data = { "HEX", "BCD", "ASCII", "INT", "ULL" };
			newValue = newValue.toUpper();
			if (newValue.front() == '$')
			{
				if (!udsEncodeExpression(newValue))
				{
					setLastError(QString("%1,%2编码格式不符合规则\n"
						"样例:$4A2B3H*A\n"
						"4A代表0~3字节为ASCII编码\n"
						"2B代表4~5字节为BCD编码\n"
						"3H代表6~8字节为HEX编码\n"
						"*A代表余下的字节为ASCII编码").arg(childKey, newValue));
					break;
				}
			}
			else
			{
				if (!data.contains(newValue))
				{
					setLastError(QString("%1,%2不支持的编码").arg(childKey, newValue));
					break;
				}
			}
		}

		if (!m_versionConfigObj.contains(parentKey))
		{
			setLastError(QString("%1,非法的键").arg(parentKey));
			break;
		}

		QJsonObject object = m_versionConfigObj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("%1,非法的键").arg(childKey));
			break;
		}
		object[childKey] = newValue;
		m_versionConfigObj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QJsonObject& Json::getVersionConfigObj() const
{
	return const_cast<QJsonObject&>(m_versionConfigObj);
}

QStringList Json::getVersionConfigExplain() const
{
	const QStringList explain =
	{
		"数据标识符",
		"支持编码[HEX BCD ASCII INT ULL $表达式]",
		"标识符数据",
		"0禁用,1启用"
	};
	return explain;
}

void Json::addVersionConfigKeyValue(const QMap<QString, QString>& keyValue)
{
	const QStringList childKey = m_versionConfigKeyList;
	for (auto iter = keyValue.begin(); iter != keyValue.end(); ++iter)
	{
		QJsonObject obj;
		QStringList valueList = m_versionConfigValueList;
		valueList[0] = iter.value();
		auto split = iter.value().split(",", QString::SkipEmptyParts);
		if (split.size() == 2)
		{
			valueList[0] = split[0];
			valueList[1] = split[1].toUpper();
		}
		else if (split.size() == 3)
		{
			valueList[0] = split[0];
			valueList[1] = split[1].toUpper();
			valueList[2] = split[2];
		}
		else if (split.size() == 4)
		{
			valueList = split;
			valueList[1] = split[1].toUpper();
		}

		const QStringList defaultList = { utility::getCurrentTime(),"XXXX","ASCII" };

		for (int i = 0; i < childKey.size(); ++i)
		{
			obj.insert(childKey[i], valueList[i].isEmpty() ?
				defaultList[i] : valueList[i]);
		}
		m_versionConfigObj.insert(iter.key().isEmpty() ?
			utility::getCurrentTime() : iter.key(), obj);
	}
}

void Json::addVersionConfigKeyValue(const QString& key, const QString& value)
{
	addVersionConfigKeyValue({ {key,value} });
}

int Json::getDtcConfigCount() const
{
	return m_dtcConfigObj.count();
}

QStringList Json::getParentDtcConfigKeyList() const
{
	return m_dtcConfigObj.keys();
}

QStringList Json::getChildDtcConfigKeyList() const
{
	return m_dtcConfigKeyList;
}

QStringList Json::getChildDtcConfigValueList() const
{
	return m_dtcConfigValueList;
}

QString Json::getDtcConfigValue(const QString& parentKey, const QString& childKey) const
{
	return m_dtcConfigObj.value(parentKey).toObject().value(childKey).toString();
}

void Json::setDtcConfigKey(const QString& oldParentKey, const QString& newParentKey)
{
	do
	{
		if (!m_dtcConfigObj.contains(oldParentKey))
		{
			break;
		}

		QJsonObject object = m_dtcConfigObj[oldParentKey].toObject();
		m_dtcConfigObj.remove(oldParentKey);
		m_dtcConfigObj.insert(newParentKey, object);
	} while (false);
	return;
}

bool Json::setDtcConfigValue(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		if (childKey == "DTC")
		{
			if (dtcCategoryConvert(value).isEmpty())
			{
				setLastError("DTC不符合编码规则," + getLastError());
				break;
			}
		}
		else
		{
			value.toInt(&convert);
			if (!convert)
			{
				setLastError("启用必须为整数");
				break;
			}
		}

		if (!m_dtcConfigObj.contains(parentKey))
		{
			setLastError(QString("%1,非法的键").arg(parentKey));
			break;
		}

		QJsonObject object = m_dtcConfigObj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("%1,非法的键").arg(childKey));
			break;
		}
		object[childKey] = value;
		m_dtcConfigObj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QJsonObject& Json::getDtcConfigObj() const
{
	return const_cast<QJsonObject&>(m_dtcConfigObj);
}

QStringList Json::getDtcConfigExplain() const
{
	const QStringList explain = { "诊断故障码","0禁用,1启用" };
	return explain;
}

void Json::addDtcConfigKeyValue(const QMap<QString, QString>& keyValue)
{
	const QStringList childKey = m_dtcConfigKeyList;
	for (auto iter = keyValue.begin(); iter != keyValue.end(); ++iter)
	{
		QJsonObject obj;
		QStringList valueList = m_dtcConfigValueList;
		valueList[0] = iter.value();
		auto split = iter.value().split(",", QString::SkipEmptyParts);
		if (split.size() == 2)
		{
			valueList = split;
		}

		const QStringList defaultList = { utility::getCurrentTime(),"0" };

		for (int i = 0; i < childKey.size(); ++i)
		{
			obj.insert(childKey[i], valueList[i].isEmpty() ?
				defaultList[i] : valueList[i]);
		}
		m_dtcConfigObj.insert(iter.key().isEmpty() ?
			utility::getCurrentTime() : iter.key(), obj);
	}
}

void Json::addDtcConfigKeyValue(const QString& key, const QString& value)
{
	addDtcConfigKeyValue({ { key , value } });
}

UdsConfig* Json::getParsedUdsConfig() const
{
	return const_cast<UdsConfig*>(&m_udsConfig);
}

bool Json::setOtherConfig1Value(const QString& key, const QString& value)
{
	bool result = false;
	do
	{
		if (!m_otherConfig1Obj.contains(key))
		{
			setLastError(QString("非法的键[%1]").arg(key));
			break;
		}
		m_otherConfig1Obj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getOtherConfig1Value(const QString& key) const
{
	return m_otherConfig1Obj.value(key).toString();
}

int Json::getOtherConfig1Count() const
{
	return m_otherConfig1Obj.count();
}

QStringList Json::getOtherConfig1KeyList() const
{
	return m_otherConfig1Obj.keys();
}

QStringList Json::getOtherConfig1Explain() const
{
	QStringList explain = {};
	for (int i = explain.size(); i < 100; i++)
		explain.append("无");
	return explain;
}

void Json::addOtherConfig1KeyValue(const QMap<QString, QString>& keyValue)
{
	for (auto iter = keyValue.begin(); iter != keyValue.end(); ++iter)
	{
		m_otherConfig1Obj.insert(iter.key().isEmpty() ?
			utility::getCurrentTime() : iter.key(), iter.value().isEmpty() ?
			utility::getCurrentTime() : iter.value());
	}
}

void Json::addOtherConfig1KeyValue(const QString& key, const QString& value)
{
	addOtherConfig1KeyValue({ {key , value } });
}

QJsonObject& Json::getOtherConfig1Obj() const
{
	return const_cast<QJsonObject&>(m_otherConfig1Obj);
}

void Json::setOtherConfig2Key(const QString& parentKey, const QString& value)
{
	return;//不允许修改父键
}

bool Json::setOtherConfig2Value(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false;
	do
	{
		QJsonObject object = m_otherConfig2Obj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("%1,非法的键").arg(childKey));
			break;
		}
		object[childKey] = value;
		m_otherConfig2Obj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QString Json::getOtherConfig2Value(const QString& parentKey, const QString& childKey) const
{
	return m_otherConfig2Obj.value(parentKey).toObject().value(childKey).toString();
}

int Json::getOtherConfig2Count() const
{
	return m_otherConfig2Obj.count();
}

QStringList Json::getParentOtherConfig2KeyList() const
{
	return m_otherConfig2Obj.keys();
}

QStringList Json::getChildOtherConfig2KeyList() const
{
	const QString key = m_otherConfig2Obj.keys()[m_other2ConfigIndex];
	return m_otherConfig2Obj.value(key).toObject().keys();
}

void Json::setChildOtherConfig2KeyListIndex(int index)
{
	m_other2ConfigIndex = index;
}

QStringList Json::getOtherConfig2Explain() const
{
	return getOtherConfig1Explain();
}

void Json::addOtherConfig2KeyValue(const QString& parentKey, const QMap<QString, QString>& keyValue)
{
	QJsonObject obj;
	for (auto iter = keyValue.begin(); iter != keyValue.end(); ++iter)
	{
		obj.insert(iter.key().isEmpty() ?
			utility::getCurrentTime() : iter.key(), iter.value().isEmpty() ?
			utility::getCurrentTime() : iter.value());
	}
	m_otherConfig2Obj.insert(parentKey, obj);
}

void Json::addOtherConfig2KeyValue(const QStringList& parentKeyList, const QList<QMap<QString, QString>>& keyValueList)
{
	const int size = std::min(parentKeyList.size(), keyValueList.size());
	for (int i = 0; i < size; ++i)
	{
		addOtherConfig2KeyValue(parentKeyList[i], keyValueList[i]);
	}
}

QJsonObject& Json::getOtherConfig2Obj() const
{
	return const_cast<QJsonObject&>(m_otherConfig2Obj);
}

bool Json::getEnableItem(int item) const
{
	bool result = true;
	switch (item)
	{
	case TS_DEBUG_MODE:
		result = m_defConfig.enable.debugMode;
		break;
	case TS_RETEST_MODE:
		result = m_defConfig.enable.retestMode;
		break;
	case TS_JUDGE_CODE:
		result = m_defConfig.enable.judgeCode;
		break;
	case TS_QUERY_STATION:
		result = m_defConfig.enable.queryStation;
		break;
	case TS_CHECK_SN:
	case TS_WRITE_SN:
		result = m_defConfig.enable.snReadWrite;
		break;
	case TS_CHECK_DATE:
	case TS_WRITE_DATE:
		result = m_defConfig.enable.dateReadWrite;
		break;
	case TS_CHECK_COMPONENT_VOLTAGE:
		result = m_defConfig.enable.checkComponentVoltage;
		break;
	case TS_CHECK_CAN_ROUSE_SLEEP:
		result = m_defConfig.enable.checkCanRouseSleep;
		break;
	case TS_CHECK_STATIC_CURRENT:
		result = m_defConfig.enable.checkStaticCurrent;
		break;
	case TS_CHECK_VERSION:
		result = m_defConfig.enable.checkVersion;
		break;
	case TS_CHECK_DTC:
		result = m_defConfig.enable.checkDtc;
		break;
	case TS_TRIGGER_IMAGE:
	case TS_CHECK_IMAGE:
		result = m_defConfig.enable.checkImage;
		break;
	case TS_CHECK_WORKING_CURRENT:
		result = m_defConfig.enable.checkWorkingCurrent;
		break;
	case TS_CLEAR_DTC:
		result = m_defConfig.enable.clearDtc;
		break;
	case TS_OUTER_NET_COMMUNICATE:
		result = m_defConfig.enable.outerNetCommunicate;
		break;
	default:
		break;
	}
	return result;
}

bool Json::udsEncodeConvert(VersonConfig* config)
{
	bool result = false;
	do
	{
		const QString encode = QString(config->encode).simplified();
		if (encode == "HEX" || encode == "BCD")
		{
			QString value;
			for (int i = 0; i < config->size; i++)
				value.append(Q_SPRINTF("%02x", static_cast<uchar>(config->read[i])));
			strcpy_s(config->read, Q_TO_C_STR(value));
		}
		else if (encode == "ASCII")
		{

		}
		else if (encode == "INT")
		{
			int temp = config->read[0];
			config->read[0] = config->read[3];
			config->read[3] = (temp & 0xff);

			temp = config->read[1];
			config->read[1] = config->read[2];
			config->read[2] = (temp & 0xff);

			temp = *(int*)&config->read[0];
			sprintf(config->read, "%d", temp);
		}
		else if (encode == "ULL")
		{
			quint64 value = 0;
			char temp[sizeof(value)] = { 0 };
			for (int i = 0; i < config->size; i++)
				temp[i] = config->read[config->size - i - 1];
			memcpy(&value, temp, config->size);
			strcpy_s(config->read, Q_TO_C_STR(QString::number(value)));
		}
		else if (encode.left(1) == "$")
		{
			if (!udsEncodeExpression(encode, config->read, config->size))
			{
				break;
			}
		}
		else
		{
			setLastError(Q_SPRINTF("0x%02X%02X,编码转换失败,不支持的编码格式%s",
				config->did[0], config->did[1], config->encode));
			break;
		}

		strcpy_s(config->read, Q_TO_C_STR(QString(config->read).trimmed()));
		result = true;
	} while (false);
	return result;
}

bool Json::udsEncodeExpression(const QString& expression, char* data, int size, const char* append)
{
	bool result = false, convert = true, head = true;
	do
	{
		if (expression.front() != '$')
		{
			break;
		}

		QList<int> length, type;
		for (int i = 1; i < expression.size(); ++i)
		{
			if (head)
			{
				if (!expression[i].isDigit() && expression[i].toLatin1() != '*')
				{
					setLastError(Q_SPRINTF("表达式中%c不为数字或*", expression[i].toLatin1()));
					convert = false;
					break;
				}

				QString number;
				int j = i;
				for (; j < expression.size(); ++j)
				{
					if (expression[j].isDigit())
					{
						number.append(expression[j]);
					}
					else if (expression[j] == '*')
					{
						int special = expression[i].toLatin1() - 48;
						number.sprintf("%d", special);
						j++;
						break;
					}
					else
					{
						break;
					}
				}
				i = j - 1;
				length.append(number.toInt());
				head = false;
			}
			else
			{
				if (!expression[i].isUpper())
				{
					convert = false;
					setLastError(Q_SPRINTF("表达式中%c不为编码类型", expression[i].toLatin1()));
					break;
				}

				if (expression[i].toLatin1() != 'A' &&
					expression[i].toLatin1() != 'B' &&
					expression[i].toLatin1() != 'H')
				{
					convert = false;
					setLastError(Q_SPRINTF("不支持的编码类型%c", expression[i].toLatin1()));
					break;
				}
				type.append(expression[i].toLatin1());
				head = true;
			}
		}

		if (!convert)
		{
			break;
		}

		if (length.size() != type.size())
		{
			setLastError("字节长度列表与编码类型列表不一致");
			break;
		}

		if (!data || !size)
		{
			result = true;
			break;
		}

		int sum = 0, index = 0;;
		char buffer[4096] = { 0 };

		for (int i = 0; i < length.size(); ++i)
		{
			int step = length[i];
			if (step < 0)
			{
				step = size - index;
			}
			sum += step;

			if (sum > size && i != length.size() - 1)
			{
				setLastError(Q_SPRINTF("%d%c超出最大总长度范围", length[i], type[i]));
				convert = false;
				break;
			}
			else if (sum > size)
			{
				step -= sum - size;
			}

			if (type[i] == 'A')//ASCII
			{
				char* ptr = strrchr(buffer, '\0');
				for (int x = 0; x < step; ++x)
				{
					uchar ch = (uchar)(data + index)[x];
					ptr[x] = char(isalnum(ch) || ispunct(ch) || isspace(ch) ? ch : '?');
				}
				ptr[step] = '\0';
			}
			else if (type[i] == 'B')//BCD
			{
				for (int j = 0; j < step; ++j)
				{
					sprintf(strrchr(buffer, '\0'), "%02X", (uchar)(data + index)[j]);
				}
			}
			else if (type[i] == 'H')//HEX
			{
				for (int j = 0; j < step; ++j)
				{
					sprintf(strrchr(buffer, '\0'), "%02X", (uchar)(data + index)[j]);
				}
			}
			else
			{
				break;
			}

			if (sum < size && append)
			{
				strcat(strrchr(buffer, '\0'), append);
			}

			index += step;
		}

		if (!convert)
		{
			break;
		}
		strcpy(data, buffer);
		result = true;
	} while (false);
	return result;
}

void Json::setLastError(const QString& error)
{
	m_lastError = error;
	if (m_errorList.size() > 1024)
	{
		m_errorList.clear();
	}
	m_errorList.append(error);
}

bool Json::readShareJsonFile(const QString& name)
{
	bool result = false, success = true;
	do
	{
		const QStringList keyList =
		{
			"硬件配置",
			"继电器配置",
			"用户配置"
		};

		RUN_BREAK(!utility::file::repairJson1LevelNode(name,
			{ keyList },
			{
				m_hardwareConfigKeyList,
				m_relayConfigKeyList,
				m_userConfigKeyList
			},
			{
				m_hardwareConfigValueList,
				m_relayConfigValueList,
				m_userConfigValueList
			}),
			"修复share.json配置文件失败," + utility::getLastError());

		QJsonObject rootObj;
		RUN_BREAK(!utility::file::readJson(name, rootObj),
			"读取share.json配置文件失败," + utility::getLastError());

		QList<QJsonObject*>objList =
		{
			&m_hardwareConfigObj,
			&m_relayConfigObj,
			&m_userConfigObj
		};

		bool (Json:: * parseFnc[])() =
		{
			&Json::parseHardwareConfigData,
			&Json::parseRelayConfigData,
			&Json::parseUserConfigData
		};

		for (int i = 0; i < keyList.count(); i++)
		{
			if (rootObj.contains(keyList[i]))
			{
				*objList[i] = rootObj.value(keyList[i]).toObject();
				if (!(this->*parseFnc[i])())
				{
					success = false;
					break;
				}
			}
			else
			{
				setLastError(QString("%1配置文件未找到对象名%2").arg(name, keyList[i]));
				success = false;
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

bool Json::writeShareJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj, hardwareConfigObj,
			relayIoConfigObj, userConfigObj;

		/*写硬件配置*/
		for (int i = 0; i < m_hardwareConfigKeyList.length(); i++)
		{
			hardwareConfigObj.insert(m_hardwareConfigKeyList[i], m_hardwareConfigValueList[i]);
		}

		/*写继电器IO端口配置*/
		for (int i = 0; i < m_relayConfigKeyList.length(); i++)
		{
			relayIoConfigObj.insert(m_relayConfigKeyList[i], m_relayConfigValueList[i]);
		}

		/*写用户配置*/
		for (int i = 0; i < m_userConfigKeyList.length(); i++)
		{
			userConfigObj.insert(m_userConfigKeyList[i], m_userConfigValueList[i]);
		}

		rootObj.insert("硬件配置", hardwareConfigObj);
		rootObj.insert("用户配置", userConfigObj);
		rootObj.insert("继电器配置", relayIoConfigObj);
		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"写入share.json配置失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateShareJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;
		rootObj.insert("硬件配置", m_hardwareConfigObj);
		rootObj.insert("用户配置", m_userConfigObj);
		rootObj.insert("继电器配置", m_relayConfigObj);
		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"更新share.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::readDefJsonFile(const QString& name)
{
	bool result = false, success = true;
	do
	{
		const QStringList keyList =
		{
			"设备配置",
			"范围配置",
			"阈值配置",
			"启用配置",
			"逻辑配置"
		};

		RUN_BREAK(!utility::file::repairJson1LevelNode(name,
			{ keyList },
			{
				m_deviceConfigKeyList,
				m_rangeConfigKeyList,
				m_thresholdConfigKeyList,
				m_enableConfigKeyList,
				m_logicConfigKeyList
			},
			{
				m_deviceConfigValueList,
				m_rangeConfigValueList,
				m_thresholdConfigValueList,
				m_enableConfigValueList,
				m_logicConfigValueList
			}),
			"修复def.json配置文件失败," + utility::getLastError());

		QJsonObject rootObj;
		RUN_BREAK(!utility::file::readJson(name, rootObj),
			"读取def.json配置文件失败," + utility::getLastError());

		QList<QJsonObject*>objList =
		{
			&m_deviceConfigObj,
			&m_rangeConfigObj,
			&m_thresholdConfigObj,
			&m_enableConfigObj,
			&m_logicConfigObj
		};

		bool (Json:: * parseFnc[])() =
		{
			&Json::parseDeviceConfigData,
			&Json::parseRangeConfigData,
			&Json::parseThresholdConfigData,
			&Json::parseEnableConfigData,
			&Json::parseLogicConfigData
		};

		for (int i = 0; i < keyList.count(); i++)
		{
			if (rootObj.contains(keyList[i]))
			{
				*objList[i] = rootObj.value(keyList[i]).toObject();
				if (!(this->*parseFnc[i])())
				{
					success = false;
					break;
				}
			}
			else
			{
				setLastError(QString("%1配置文件未找到对象名%2").arg(name, keyList[i]));
				success = false;
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

bool Json::writeDefJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj, deviceConfigObj, rangeConfigObj,
			thresholdConfigObj, enableConfigObj, logicConfigObj;

		/*写设备配置*/
		for (int i = 0; i < m_deviceConfigKeyList.length(); i++)
		{
			deviceConfigObj.insert(m_deviceConfigKeyList[i], m_deviceConfigValueList[i]);
		}

		/*范围配置*/
		for (int i = 0; i < m_rangeConfigKeyList.length(); i++)
		{
			rangeConfigObj.insert(m_rangeConfigKeyList[i], m_rangeConfigValueList[i]);
		}

		/*阈值配置*/
		for (int i = 0; i < m_thresholdConfigKeyList.length(); i++)
		{
			thresholdConfigObj.insert(m_thresholdConfigKeyList[i], m_thresholdConfigValueList[i]);
		}

		/*启用配置*/
		for (int i = 0; i < m_enableConfigKeyList.length(); i++)
		{
			enableConfigObj.insert(m_enableConfigKeyList[i], m_enableConfigValueList[i]);
		}

		for (int i = 0; i < m_logicConfigKeyList.length(); i++)
		{
			logicConfigObj.insert(m_logicConfigKeyList[i], m_logicConfigValueList[i]);
		}

		rootObj.insert("设备配置", deviceConfigObj);
		rootObj.insert("范围配置", rangeConfigObj);
		rootObj.insert("阈值配置", thresholdConfigObj);
		rootObj.insert("启用配置", enableConfigObj);
		rootObj.insert("逻辑配置", logicConfigObj);
		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"写入def.json配置失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateDefJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;
		rootObj.insert("设备配置", m_deviceConfigObj);
		rootObj.insert("范围配置", m_rangeConfigObj);
		rootObj.insert("阈值配置", m_thresholdConfigObj);
		rootObj.insert("启用配置", m_enableConfigObj);
		rootObj.insert("逻辑配置", m_logicConfigObj);
		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"更新def.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::readHwdJsonFile(const QString& name)
{
	bool result = false, success = true;
	do
	{
		const QStringList level1Key =
		{
			"静态电流配置",
			"按键电压配置"
		};


		RUN_BREAK(!utility::file::repairJson1LevelNode(name,
			{
				level1Key
			},
			{
				m_staticCurrentConfigKeyList,
				m_keyVoltageConfigKeyList
			},
			{
				m_staticCurrentConfigValueList,
				m_keyVoltageConfigValueList
			}),
			"修复hwd.json配置文件失败," + utility::getLastError());

		const QStringList leve2Key =
		{
			"工作电流配置",
			"组件电压配置",
			"组件电阻配置"
		};

		RUN_BREAK(!utility::file::renameJsonMasterNode(name, { "电流配置","电压配置","电阻配置" },
			leve2Key), "重命名hwd.json配置文件主节点失败," + utility::getLastError());

		RUN_BREAK(!utility::file::repairJson2LevelNodeEx(name,
			{
				leve2Key
			},
			{
				{"12V电流","16V电流"},
				{"1.8V电压","3.3V电压"},
				{"12V电阻","16V电阻"}
			},
			{
				m_workingCurrentConfigKeyList,
				m_componentVoltageConfigKeyList,
				m_componentResistorConfigKeyList
			},
			{
				m_workingCurrentConfigValueList,
				m_componentVoltageConfigValueList,
				m_componentResistorConfigValueList
			}),
			"修复hwd.json配置文件失败," + utility::getLastError());

		QJsonObject root;
		RUN_BREAK(!utility::file::readJson(name, root),
			"读取hwd.json配置文件失败," + utility::getLastError());

		QList<QJsonObject*> objList =
		{
			&m_workingCurrentConfigObj,
			&m_staticCurrentConfigObj,
			&m_keyVoltageConfigObj,
			&m_componentVoltageConfigObj,
			&m_componentResistorConfigObj
		};

		bool (Json:: * parseFnc[])() =
		{
			&Json::parseWorkingCurrentConfigData,
			&Json::parseStaticCurrentConfigData,
			&Json::parseKeyVoltageConfigData,
			&Json::parseComponentVoltageConfigData,
			&Json::parseComponentResistorConfigData
		};

		QStringList keyList =
		{
			"工作电流配置",
			"静态电流配置",
			"按键电压配置",
			"组件电压配置",
			"组件电阻配置"
		};

		for (int i = 0; i < keyList.count(); i++)
		{
			if (root.contains(keyList[i]))
			{
				*objList[i] = root.value(keyList[i]).toObject();
				if (!(this->*parseFnc[i])())
				{
					success = false;
					break;
				}
			}
			else
			{
				setLastError(QString("%1配置文件未找到对象名%2").arg(name, keyList[i]));
				success = false;
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

bool Json::writeHwdJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj, voltageObj, keyVolObj, currentObj, staticObj, resObj;

		/*电压默认配置*/
		QList<QJsonObject> voltageList;
		for (int i = 0; i < 2; i++)
		{
			QJsonObject obj;
			for (int j = 0; j < getChildComponentVoltageConfigKeyList().count(); j++)
			{
				obj.insert(getChildComponentVoltageConfigKeyList()[j],
					getChildComponentVoltageConfigValueList()[j]);
			}
			voltageList.append(obj);
		}

		voltageObj.insert("1.8V电压", voltageList[0]);
		voltageObj.insert("3.3V电压", voltageList[1]);

		/*按键默认配置*/
		for (int i = 0; i < getKeyVoltageConfigKeyList().size(); i++)
		{
			keyVolObj.insert(getKeyVoltageConfigKeyList()[i], getKeyVoltageConfigValueList()[i]);
		}

		/*电流默认配置*/
		QList<QJsonObject> currentList;
		for (int i = 0; i < 2; i++)
		{
			QJsonObject obj;
			for (int j = 0; j < getChildWorkingCurrentConfigKeyList().count(); j++)
			{
				obj.insert(getChildWorkingCurrentConfigKeyList()[j],
					getChildWorkingCurrentConfigValueList(i)[j]);
			}
			currentList.append(obj);
		}
		currentObj.insert("12V电流", currentList[0]);
		currentObj.insert("16V电流", currentList[1]);

		/*静态电流默认配置*/
		for (int i = 0; i < getStaticCurrentConfigKeyList().count(); i++)
		{
			staticObj.insert(getStaticCurrentConfigKeyList()[i], getStaticCurrentConfigValueList()[i]);
		}

		/*RES电阻配置*/
		QList<QJsonObject> resList;
		for (int i = 0; i < 2; i++)
		{
			QJsonObject obj;
			for (int j = 0; j < getChildComponentResistorConfigKeyList().count(); j++)
			{
				obj.insert(getChildComponentResistorConfigKeyList()[j],
					getChildComponentResistorConfigValueList()[j]);
			}
			resList.append(obj);
		}
		resObj.insert("12V电阻", resList[0]);
		resObj.insert("16V电阻", resList[1]);

		/*根节点默认配置*/
		rootObj.insert("工作电流配置", currentObj);
		rootObj.insert("静态电流配置", staticObj);
		rootObj.insert("按键电压配置", keyVolObj);
		rootObj.insert("组件电压配置", m_componentVoltageConfigObj.isEmpty() ?
			voltageObj : m_componentVoltageConfigObj);
		rootObj.insert("组件电阻配置", resObj);

		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"写入hwd.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateHwdJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;
		rootObj.insert("工作电流配置", m_workingCurrentConfigObj);
		rootObj.insert("静态电流配置", m_staticCurrentConfigObj);
		rootObj.insert("按键电压配置", m_keyVoltageConfigObj);
		rootObj.insert("组件电压配置", m_componentVoltageConfigObj);
		rootObj.insert("组件电阻配置", m_componentResistorConfigObj);

		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"更新hwd.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::readImgJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		RUN_BREAK(!utility::file::repairJson2LevelNode(name,
			{ "图像配置" },
			{ m_parentImageKeyList },
			{ m_childImageKeyList },
			{ m_childImageValueList }),
			"修复图像配置失败," + utility::getLastError());

		RUN_BREAK(!utility::file::repairJson1LevelNode(name,
			{ "图像动态配置" },
			{ m_imageDynamicConfigKeyList },
			{ m_imageDynamicConfigValueList }),
			"修复图像动态配置失败," + utility::getLastError());

		QJsonObject rootObj;
		RUN_BREAK(!utility::file::readJson(name, rootObj),
			"读取img.json配置文件失败," + utility::getLastError());

		if (!rootObj.contains("图像配置"))
		{
			setLastError(QString("%1配置文件未找到对象名图像配置").arg(name));
			break;
		}

		m_imageConfigObj = rootObj.value("图像配置").toObject();
		if (!parseImageConfigData())
		{
			break;
		}

		if (!rootObj.contains("图像动态配置"))
		{
			setLastError(QString("%1配置文件未找到对象名图像动态配置").arg(name));
			break;
		}

		m_imageDynamicConfigObj = rootObj.value("图像动态配置").toObject();
		if (!parseImageDynamicConfigData())
		{
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool Json::writeImgJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;

		QJsonObject childImageCheckObj[IMAGE_CHECK_COUNT];
		for (int i = 0; i < IMAGE_CHECK_COUNT; i++)
		{
			for (int j = 0; j < m_childImageKeyList[i].size(); j++)
			{
				childImageCheckObj[i].insert(m_childImageKeyList[i].value(j), m_childImageValueList[i].value(j));
			}
		}

		QJsonObject imageConfigObj;
		for (int i = 0; i < m_parentImageKeyList.size(); i++)
		{
			imageConfigObj.insert(m_parentImageKeyList.value(i), childImageCheckObj[i]);
		}

		rootObj.insert("图像配置", imageConfigObj);

		QJsonObject imageDynamicConfigObj;
		for (int i = 0; i < m_imageDynamicConfigKeyList.size(); ++i)
		{
			imageDynamicConfigObj.insert(m_imageDynamicConfigKeyList[i], m_imageDynamicConfigValueList[i]);
		}
		rootObj.insert("图像动态配置", imageDynamicConfigObj);

		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"写入img.json配置失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateImgJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;
		rootObj.insert("图像配置", m_imageConfigObj);
		rootObj.insert("图像动态配置", m_imageDynamicConfigObj);

		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"更新img.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::readOtherJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		const QStringList keyList = m_otherConfig1Obj.keys();
		QStringList valueList;
		for (int i = 0; i < keyList.size(); ++i)
		{
			auto obj = m_otherConfig1Obj.value(keyList[i]).toString();;
			valueList.append(obj);
		}
		RUN_BREAK(!utility::file::repairJson1LevelNode(name,
			{ "其他配置1" }, { keyList }, { valueList }),
			"修复other.json配置文件失败," + utility::getLastError());

		QStringList parentKeyList = m_otherConfig2Obj.keys();
		RUN_BREAK(parentKeyList.size() > 100, "其他配置2,节点数超过最大上限100");

		QStringList childKeyList[100], childValueList[100];
		for (int i = 0; i < parentKeyList.size(); ++i)
		{
			auto obj = m_otherConfig2Obj[parentKeyList[i]].toObject();
			auto keys = obj.keys();
			childKeyList[i].append(keys);
			QStringList valueList;
			for (int j = 0; j < keys.size(); ++j)
				valueList.append(obj.value(keys[j]).toString());
			childValueList[i].append(valueList);
		}
		RUN_BREAK(!utility::file::repairJson2LevelNode(name,
			{ "其他配置2" }, { parentKeyList },
			{ childKeyList }, { childValueList }),
			"修复other.json配置文件失败," + utility::getLastError());

		QJsonObject rootObj;
		RUN_BREAK(!utility::file::readJson(name, rootObj),
			"读取other.json配置文件失败," + utility::getLastError());
		RUN_BREAK(!rootObj.contains("其他配置1"), "丢失对象[其他配置1]");
		m_otherConfig1Obj = rootObj.value("其他配置1").toObject();

		RUN_BREAK(!rootObj.contains("其他配置2"), "丢失对象[其他配置2]");
		m_otherConfig2Obj = rootObj.value("其他配置2").toObject();
		result = true;
	} while (false);
	return result;
}

bool Json::writeOtherJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;
		rootObj.insert("其他配置1", m_otherConfig1Obj);
		rootObj.insert("其他配置2", m_otherConfig2Obj);
		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"写入other.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateOtherJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;
		rootObj.insert("其他配置1", m_otherConfig1Obj);
		rootObj.insert("其他配置2", m_otherConfig2Obj);
		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"更新other.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::readUdsJsonFile(const QString& name)
{
	bool result = false, success = false;
	do
	{
		RUN_BREAK(!utility::file::repairJson2LevelNode(name,
			{ "版本配置","诊断配置" },
			{
				m_versionConfigKeyList,
				m_dtcConfigKeyList
			},
			{
				m_versionConfigValueList,
				m_dtcConfigValueList
			}
			), "修复uds.json配置文件失败," + utility::getLastError());

		QJsonObject root;
		RUN_BREAK(!utility::file::readJson(name, root),
			"读取uds.json配置文件失败," + utility::getLastError());

		if (root.contains("版本配置"))
		{
			m_versionConfigObj = root.value("版本配置").toObject();
			if (!parseVersionConfigData())
			{
				break;
			}
		}

		if (root.contains("诊断配置"))
		{
			m_dtcConfigObj = root.value("诊断配置").toObject();
			if (!parseDtcConfigData())
			{
				break;
			}
		}

		result = true;
	} while (false);
	return result;
}

bool Json::writeUdsJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj, verObj0, verObj1, verObj2,
			dtcObj0, dtcObj1, dtcObj2;
		verObj0.insert("DID", "0xF187");
		verObj0.insert("编码", "ASCII");
		verObj0.insert("值", "A00087710");

		verObj1.insert("DID", "0xF193");
		verObj1.insert("编码", "ASCII");
		verObj1.insert("值", "0.03");

		verObj2.insert("广汽ECU硬件版本号", verObj0);
		verObj2.insert("应用程序版本号", verObj1);

		dtcObj0.insert("DTC", "U100900");
		dtcObj0.insert("忽略", "0");

		dtcObj1.insert("DTC", "U100587");
		dtcObj1.insert("忽略", "0");

		dtcObj2.insert("蓄电池供电电压低于工作阈值", dtcObj0);
		dtcObj2.insert("蓄电池供电电压高于工作阈值", dtcObj1);

		rootObj.insert("版本配置", m_versionConfigObj.isEmpty() ?
			verObj2 : m_versionConfigObj);
		rootObj.insert("诊断配置", m_dtcConfigObj.isEmpty() ?
			dtcObj2 : m_dtcConfigObj);

		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"写入uds.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateUdsJsonFile(const QString& name)
{
	bool result = false;
	do
	{
		QJsonObject rootObj;
		rootObj.insert("版本配置", m_versionConfigObj);
		rootObj.insert("诊断配置", m_dtcConfigObj);

		RUN_BREAK(!utility::file::writeJson(name, rootObj),
			"更新uds.json配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::parseRangeValue(const QString& value, float& min, float& max)
{
	bool result = false;
	do
	{
		const QStringList split = value.split("~", QString::SkipEmptyParts);
		if (split.size() != 2)
		{
			break;
		}

		bool convert = false;
		min = split[0].toFloat(&convert);
		if (!convert)
		{
			break;
		}
		max = split[1].toFloat(&convert);
		if (!convert)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseDeviceConfigData()
{
	bool result = false, success = true, convert = false;
	do
	{
		int structSize = sizeof(DeviceConfig) / sizeof(QString);

		if (structSize != m_deviceConfigKeyList.size())
		{
			setLastError("设备配置结构体大小与键列表不匹配");
			break;
		}

		if (structSize != m_deviceConfigValueList.size())
		{
			setLastError("设备配置结构体大小与值列表不匹配");
			break;
		}

		QString* valuePtr = reinterpret_cast<QString*>(&m_defConfig.device);
		for (int i = 0; i < m_deviceConfigKeyList.length(); i++, valuePtr++)
		{
			*valuePtr = getDeviceConfigValue(m_deviceConfigKeyList.value(i));
			if (valuePtr->isEmpty())
			{
				success = false;
				setLastError(QString("设备配置[%1]格式错误").arg(m_deviceConfigKeyList.value(i)));
				break;
			}
		}

		if (!success)
		{
			break;
		}

		if (!m_typeName.isEmpty())
		{
			if (m_defConfig.device.typeName != m_typeName)
			{
				m_defConfig.device.typeName = m_typeName;
				setDeviceConfigValue("机种名称", m_typeName);
			}
		}

		result = true;
	} while (false);
	return result;
}

bool Json::parseHardwareConfigData()
{
	bool result = false, success = true, convert = false;
	do
	{
		//硬件配置结构体大小,存在数据格式不一致,无法做出校验,如新增键值,需自行校验
		int* valuePtr = reinterpret_cast<int*>(&m_defConfig.hardware);
		for (int i = 0; i < m_hardwareConfigKeyList.length(); i++, valuePtr++)
		{
			if (i == 2)
			{
				m_defConfig.hardware.powerVoltage = getHardwareConfigValue(m_hardwareConfigKeyList.value(i)).toFloat(&convert);
			}
			else if (i == 3)
			{
				m_defConfig.hardware.powerCurrent = getHardwareConfigValue(m_hardwareConfigKeyList.value(i)).toFloat(&convert);
			}
			else
			{
				auto ipAddr = getHardwareConfigValue("继电器串口");
				bool validIp = false;
				if (m_hardwareConfigKeyList[i] == "继电器串口") {
					if (utility::isIpAddressValid(ipAddr)) {
						validIp = true;
					}
				}

				if (validIp) {
					*valuePtr = inet_addr(ipAddr.toStdString().c_str());
				}
				else {
					*valuePtr = getHardwareConfigValue(m_hardwareConfigKeyList.value(i)).toInt(&convert);
				}
			}

			if (!convert)
			{
				success = false;
				setLastError(QString("硬件配置[%1]格式错误").arg(m_hardwareConfigKeyList.value(i)));
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

bool Json::parseRelayConfigData()
{
	bool result = false, success = true, convert = false;
	do
	{
		const int structSize = sizeof(RelayConfig) / sizeof(int);
		if (structSize != m_relayConfigKeyList.size())
		{
			setLastError("继电器配置结构体大小与键列表不匹配");
			break;
		}

		if (structSize != m_relayConfigValueList.size())
		{
			setLastError("继电器配置结构体大小与值列表不匹配");
			break;
		}

		int* valuePtr = reinterpret_cast<int*>(&m_defConfig.relay);
		for (int i = 0; i < m_relayConfigKeyList.length(); i++, valuePtr++)
		{
			*valuePtr = getRelayConfigValue(m_relayConfigKeyList.value(i)).toInt(&convert);
			if (!convert)
			{
				success = false;
				setLastError(QString("继电器配置[%1]格式错误").arg(m_relayConfigKeyList.value(i)));
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

bool Json::parseUserConfigData()
{
	return true;
}

bool Json::parseImageConfigData()
{
	bool result = false, convert = true;
	do
	{
		int* valuePtr = nullptr;

		for (int i = 0; i < IMAGE_CHECK_COUNT; i++)
		{
			if (i == IMAGE_CHECK_COUNT - 1)
			{
				valuePtr = reinterpret_cast<int*>(&m_defConfig.image);
			}
			else if (i >= SMALL_RECT_ && i <= SMALL_RECT_ + BIG_RECT_ - 1)
			{
				valuePtr = reinterpret_cast<int*>(&(m_defConfig.image.bigRect[i - SMALL_RECT_]));
			}
			else
			{
				valuePtr = reinterpret_cast<int*>(&(m_defConfig.image.smallRect[i]));
			}

			for (int j = 0; j < m_childImageKeyList[i].count(); j++, (j || i == IMAGE_CHECK_COUNT - 1) ? valuePtr++ : valuePtr += sizeof(QString))
			{
				if (j || i == IMAGE_CHECK_COUNT - 1)
				{
					*valuePtr = getImageConfigValue(m_parentImageKeyList[i], m_childImageKeyList[i][j]).toInt(&convert);
				}
				else
				{
					const QString& color = m_defConfig.image.smallRect[i].color = getImageConfigValue(m_parentImageKeyList[i], m_childImageKeyList[i][j]);
					if ((!color.contains("!=") && !color.contains("==")))
					{
						convert = false;
						setLastError(QString("%1[%2],语法错误,\n判断语法,!=黑色(不等于黑色),==黑色(等于黑色)").arg(m_parentImageKeyList[i], m_childImageKeyList[i][j]));
						break;
					}
				}

				if (!convert)
				{
					setLastError(QString("%1[%2],格式错误").arg(m_parentImageKeyList[i], m_childImageKeyList[i][j]));
					break;
				}
			}

			if (!convert)
			{
				break;
			}
		}

		if (!convert)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseImageDynamicConfigData()
{
	bool result = false;
	do 
	{
		m_defConfig.imageDynamic.determineCount = getImageDynamicConfigValue("判定次数").toInt();

		m_defConfig.imageDynamic.durationTime = getImageDynamicConfigValue("持续时间").toInt();

		m_defConfig.imageDynamic.dynamicFrequency = getImageDynamicConfigValue("动态频率").toInt();

		m_defConfig.imageDynamic.dynamicPercent = getImageDynamicConfigValue("动态占比").toDouble();

		m_defConfig.imageDynamic.binaryThreshold = getImageDynamicConfigValue("二值化阈值").toDouble();

		result = true;
	} while (false);
	return result;
}

bool Json::parseRangeConfigData()
{
	bool result = false, success = true;
	do
	{
		const int structSize = sizeof(RangeConfig) / (sizeof(float) * 2);
		if (structSize != m_rangeConfigKeyList.size())
		{
			setLastError("范围配置结构体大小与键不匹配");
			break;
		}

		if (structSize != m_rangeConfigValueList.size())
		{
			setLastError("范围配置结构体大小与值不匹配");
			break;
		}

		float* valuePtr = reinterpret_cast<float*>(&m_defConfig.range);
		for (int i = 0; i < m_rangeConfigKeyList.length(); i++, valuePtr++, valuePtr++)
		{
			if (!parseRangeValue(getRangeConfigValue(m_rangeConfigKeyList.value(i)), *valuePtr, *(valuePtr + 1)))
			{
				setLastError(QString("范围配置[%1]格式错误").arg(m_rangeConfigKeyList.value(i)));
				success = false;
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

bool Json::parseThresholdConfigData()
{
	bool result = false, success = true, convert = false;
	do
	{
		const int structSize = sizeof(ThresholdConfig) / sizeof(float);
		if (structSize != m_thresholdConfigKeyList.size())
		{
			setLastError("阈值配置结构体大小与键不匹配");
			break;
		}

		if (structSize != m_thresholdConfigValueList.size())
		{
			setLastError("阈值配置结构体大小与值不匹配");
			break;
		}

		float* valuePtr = reinterpret_cast<float*>(&m_defConfig.threshold);
		for (int i = 0; i < m_thresholdConfigKeyList.length(); i++, valuePtr++)
		{
			*valuePtr = getThresholdConfigValue(m_thresholdConfigKeyList.value(i)).toFloat(&convert);
			if (!convert)
			{
				setLastError(QString("阈值配置[%1]格式错误").arg(m_thresholdConfigKeyList.value(i)));
				success = false;
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

bool Json::parseEnableConfigData()
{
	bool result = false, convert = false, success = true;
	do
	{
		const int structSize = sizeof(EnableConfig) / sizeof(int);
		if (structSize != m_enableConfigKeyList.size())
		{
			setLastError("启用配置结构体大小与键不匹配");
			break;
		}

		if (structSize != m_enableConfigValueList.size())
		{
			setLastError("启用配置结构体大小与值不匹配");
			break;
		}

		int* valuePtr = reinterpret_cast<int*>(&m_defConfig.enable);
		for (int i = 0; i < m_enableConfigKeyList.length(); i++, valuePtr++)
		{
			*valuePtr = getEnableConfigValue(m_enableConfigKeyList[i]).toInt(&convert);
			if (!convert)
			{
				setLastError(QString("启用配置[%1]格式错误").arg(m_enableConfigKeyList[i]));
				success = false;
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

bool Json::parseLogicConfigData()
{
	bool result = false;
	do 
	{
		if (sizeof(LogicConfig) * 8 != m_logicConfigKeyList.size())
		{
			setLastError("逻辑配置结构体大小与键不匹配");
			break;
		}

		memset(&m_defConfig.logic, 0, sizeof(LogicConfig));

		for (int i = 0; i < m_logicConfigKeyList.size(); ++i)
		{
			int v = getLogicConfigValue(m_logicConfigKeyList[i]).toInt();
			utility::setBit(reinterpret_cast<int*>(&m_defConfig.logic), i, static_cast<bool>(v));
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseComponentVoltageConfigData()
{
	bool result = false, convert = false, success = true;
	do
	{
		SAFE_DELETE_A(m_hwdConfig.componentVoltage);

		const int size = getComponentVoltageConfigCount() + 1;
		m_hwdConfig.componentVoltage = NO_THROW_NEW ComponentVoltageConfig[size];
		ComponentVoltageConfig* componentVoltage = m_hwdConfig.componentVoltage;
		RUN_BREAK(!componentVoltage, "组件电压配置分配内存失败");

		memset(componentVoltage, 0x00, sizeof(ComponentVoltageConfig) * size);

		for (int i = 0; i < getComponentVoltageConfigCount(); i++)
		{
			const QString key = getParentComponentVoltageConfigKeyList()[i];
			strcpy_s(componentVoltage[i].name, Q_TO_C_STR(key));

			componentVoltage[i].high = getComponentVoltageConfigValue(key, "上限").toFloat(&convert);
			if (!convert)
			{
				success = false;
				setLastError("组件电压配置[上限]格式错误");
				break;
			}

			componentVoltage[i].low = getComponentVoltageConfigValue(key, "下限").toFloat(&convert);
			if (!convert)
			{
				success = false;
				setLastError("组件电压配置[下限]格式错误");
				break;
			}

			componentVoltage[i].relay = getComponentVoltageConfigValue(key, "继电器IO").toInt(&convert);
			if (!convert)
			{
				success = false;
				setLastError("组件电压配置[继电器IO]格式错误");
				break;
			}

			componentVoltage[i].delay = getComponentVoltageConfigValue(key, "延时").toInt(&convert);
			if (!convert)
			{
				success = false;
				setLastError("组件电压配置[延时]格式错误");
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

bool Json::parseKeyVoltageConfigData()
{
	bool result = false, convert = false, success = true;
	do
	{
		m_hwdConfig.keyVoltage.hULimit = getKeyVoltageConfigValue("高电平上限").toFloat(&convert);
		if (!convert)
		{
			setLastError("按键电压配置[高电平上限]格式错误");
			break;
		}

		m_hwdConfig.keyVoltage.hLLimit = getKeyVoltageConfigValue("高电平下限").toFloat(&convert);
		if (!convert)
		{
			setLastError("按键电压配置[高电平下限]格式错误");
			break;
		}

		m_hwdConfig.keyVoltage.lULimit = getKeyVoltageConfigValue("低电平上限").toFloat(&convert);
		if (!convert)
		{
			setLastError("按键电压配置[低电平上限]格式错误");
			break;
		}

		m_hwdConfig.keyVoltage.lLLimit = getKeyVoltageConfigValue("低电平下限").toFloat(&convert);
		if (!convert)
		{
			setLastError("按键电压配置[低电平下限]格式错误");
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool Json::parseWorkingCurrentConfigData()
{
	bool result = false, convert = false, success = true;
	do
	{
		SAFE_DELETE_A(m_hwdConfig.workingCurrent);

		const int size = getWorkingCurrentConfigCount() + 1;

		m_hwdConfig.workingCurrent = NO_THROW_NEW WorkingCurrentConfig[size];

		WorkingCurrentConfig* current = m_hwdConfig.workingCurrent;

		RUN_BREAK(!current, "工作电流配置分配内存失败");

		memset(current, 0x00, sizeof(WorkingCurrentConfig) * size);

		for (int i = 0; i < getWorkingCurrentConfigCount(); i++)
		{
			const QString key = getParentWorkingCurrentConfigKeyList()[i];

			strcpy_s(current[i].name, Q_TO_C_STR(key));

			current[i].high = getWorkingCurrentConfigValue(key, "上限").toFloat(&convert);
			if (!convert)
			{
				success = false;
				setLastError("工作电流配置[上限]格式错误");
				break;
			}

			current[i].low = getWorkingCurrentConfigValue(key, "下限").toFloat(&convert);
			if (!convert)
			{
				success = false;
				setLastError("工作电流配置[下限]格式错误");
				break;
			}

			current[i].voltage = getWorkingCurrentConfigValue(key, "电压").toFloat(&convert);
			if (!convert)
			{
				success = false;
				setLastError("工作电流配置[电压]格式错误");
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

bool Json::parseStaticCurrentConfigData()
{
	bool result = false, convert = false;
	do
	{
		m_hwdConfig.staticCurrent.high = getStaticCurrentConfigValue("上限").toFloat(&convert);
		if (!convert)
		{
			setLastError("静态电流配置[上限]格式错误");
			break;
		}

		m_hwdConfig.staticCurrent.low = getStaticCurrentConfigValue("下限").toFloat(&convert);
		if (!convert)
		{
			setLastError("静态电流配置[下限]格式错误");
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseComponentResistorConfigData()
{
	bool result = false, convert = false, success = true;
	do
	{
		SAFE_DELETE_A(m_hwdConfig.componentResistor);

		const int size = getComponentResistorConfigCount() + 1;

		m_hwdConfig.componentResistor = NO_THROW_NEW ComponentResistorConfig[size];

		ComponentResistorConfig* componentResistor = m_hwdConfig.componentResistor;

		RUN_BREAK(!componentResistor, "组件电阻配置分配内存失败");

		memset(componentResistor, 0x00, sizeof(ComponentResistorConfig) * size);

		for (int i = 0; i < getComponentResistorConfigCount(); i++)
		{
			const QString key = getParentComponentResistorConfigKeyList()[i];
			strcpy_s(m_hwdConfig.componentResistor[i].name, Q_TO_C_STR(key));
			m_hwdConfig.componentResistor[i].high = getComponentResistorConfigValue(key, "上限").toFloat(&convert);
			if (!convert)
			{
				setLastError("组件电阻[上限]格式错误");
				success = false;
				break;
			}

			m_hwdConfig.componentResistor[i].low = getComponentResistorConfigValue(key, "下限").toFloat(&convert);
			if (!convert)
			{
				setLastError("组件电阻[下限]格式错误");
				success = false;
				break;
			}

			m_hwdConfig.componentResistor[i].relay = getComponentResistorConfigValue(key, "继电器IO").toInt(&convert);
			if (!convert)
			{
				setLastError("组件电阻[继电器IO]格式错误");
				success = false;
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

bool Json::parseVersionConfigData()
{
	bool result = false, convert = false, success = false;
	do
	{
		const char* code[] = { "HEX", "BCD", "ASCII", "INT", "ULL" };

		SAFE_DELETE_A(m_udsConfig.version);
		const int size = getVersionConfigCount() + 1;
		m_udsConfig.version = NO_THROW_NEW VersonConfig[size];
		VersonConfig* version = m_udsConfig.version;
		RUN_BREAK(!version, "版本配置分配内存失败");

		memset(version, 0x00, sizeof(VersonConfig) * size);
		for (int i = 0; i < getVersionConfigCount(); i++)
		{
			const QString key = getParentVersionConfigKeyList()[i];
			strcpy_s(version[i].name, Q_TO_C_STR(key));

			success = true;
			ushort did = getVersionConfigValue(key, "DID").toUShort(&convert, 16);

			if (!convert)
			{
				success = false;
				setLastError(QString("版本配置[%1]:[DID]格式错误").arg(key));
				break;
			}

			version[i].did[0] = uchar((did >> 8) & 0xff);
			version[i].did[1] = uchar((did >> 0) & 0xff);

			strcpy_s(version[i].setup, Q_TO_C_STR(getVersionConfigValue(key, "值")));
			strcpy_s(version[i].encode, Q_TO_C_STR(getVersionConfigValue(key, "编码").toUpper()));
			version[i].enable = getVersionConfigValue(key, "启用").toInt(&convert);

			if (!convert)
			{
				success = false;
				setLastError(QString("版本配置[%1]:[启用]格式错误").arg(key));
				break;
			}

			success = false;
			for (int j = 0; j < sizeof(code) / sizeof(char*); j++)
			{
				if (*version[i].encode == '$')
				{
					if (udsEncodeExpression(version[i].encode))
					{
						success = true;
						break;
					}
				}
				else
				{
					if (!strcmp(version[i].encode, code[j]))
					{
						success = true;
						break;
					}
				}
			}

			if (!success)
			{
				setLastError(QString("版本配置[%1]:[编码]格式错误").arg(key));
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

bool Json::parseDtcConfigData()
{
	bool result = false, convert = false, success = true;
	do
	{
		SAFE_DELETE_A(m_udsConfig.dtc);
		const int size = getDtcConfigCount() + 1;
		m_udsConfig.dtc = NO_THROW_NEW DtcConfig[size];
		DtcConfig* config = m_udsConfig.dtc;
		RUN_BREAK(!config, "诊断故障码配置分配内存失败");

		memset(config, 0x00, sizeof(DtcConfig) * size);

		for (int i = 0; i < getDtcConfigCount(); i++)
		{
			const QString key = getParentDtcConfigKeyList()[i];
			strcpy_s(config[i].name, Q_TO_C_STR(key));

			const QString dtcValue = dtcCategoryConvert(getDtcConfigValue(key, "DTC"));
			if (dtcValue.isEmpty())
			{
				success = false;
				setLastError(QString("诊断配置[%1]:[DTC]为空").arg(key));
				break;
			}

			const uint bytes = dtcValue.toUInt(&convert, 16);
			if (!convert)
			{
				success = false;
				setLastError(QString("诊断配置[%1]:[DTC]格式错误").arg(key));
				break;
			}

			config[i].dtc[0] = uchar((bytes >> 16) & 0xff);
			config[i].dtc[1] = uchar((bytes >> 8) & 0xff);
			config[i].dtc[2] = uchar((bytes >> 0) & 0xff);
			config[i].enable = getDtcConfigValue(key, "启用").toInt(&convert);
			if (!convert)
			{
				success = false;
				setLastError(QString("诊断配置[%1]:[启用]格式错误").arg(key));
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

QString Json::dtcCategoryConvert(const QString& dtc)
{
	QString result = "";
	bool convert = false;
	do
	{
		int category = -1;
		switch (dtc[0].toLatin1())
		{
		case 'B':category = 0x8; break;
		case 'C':category = 0x4; break;
		case 'P':category = 0x0; break;
		case 'U':category = 0xC; break;
		default:break;
		}

		if (category == -1)
		{
			setLastError("诊断故障码,代码种类不符合规则");
			break;
		}

		category += dtc.mid(1, 1).toInt(&convert);
		if (!convert)
		{
			setLastError("诊断故障码常规转换失败");
			break;
		}
		result = dtc;
		result.replace(dtc.mid(0, 2), QString::number(category, 16).toUpper());
	} while (false);
	return result;
}

