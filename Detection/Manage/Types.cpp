#include "Types.h"

void Misc::setLastError(const QString& error)
{
	Var::lastError = error;
}

QString Misc::getLastError()
{
	return Var::lastError;
}

bool Misc::writeRunError(const QString& barCode, const QString& error)
{
	bool result = false;
	do
	{
		QString path = QString("%1\\Log\\Run").arg(Json::getUsingPath());
		if (!utility::makePath(path))
		{
			break;
		}

		QFile file(path.append("\\").append(utility::getCurrentDate(true)).append(".run"));
		if (!file.open(QFile::WriteOnly | QFile::Append | QFile::Text))
		{
			break;
		}
		QTextStream stream(&file);
		stream << utility::getCurrentTimeEx() << " " << (barCode.isEmpty() ? "未知" : barCode) << " " << error << endl;
		file.close();
		result = true;
	} while (false);
	return result;
}

bool Misc::cvImageToQtImage(IplImage* cv, QImage* qt)
{
	bool result = false;
	do
	{
		if (!cv || !qt)
			break;

		cvCvtColor(cv, cv, CV_BGR2RGB);
		*qt = QImage((uchar*)cv->imageData, cv->width, cv->height, cv->widthStep, QImage::Format_RGB888);
		result = true;
	} while (false);
	return result;
}

bool Misc::cvImageToQtImage(const cv::Mat& mat, QImage& image)
{
	if (mat.empty())
	{
		image = QImage();
		return false;
	}

	cv::Mat temp;
	cv::cvtColor(mat, temp, CV_BGR2RGB);

	image = QImage(temp.data, temp.cols, temp.rows, QImage::Format_RGB888).
		copy(0, 0, temp.cols, temp.rows);

	return true;
}

bool Misc::createShortcut()
{
	/*QString typeName = GET_TYPE_NAME();
	if (typeName == "未知")
		return false;

	QString fileName = typeName + Dt::Base::getDetectionName() + 
		QString("检测[") + utility::getAppVersion() + QString("].lnk");
	QString linkPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "\\" + fileName;

	if (QDir(linkPath).exists())
		return true;

	wchar_t temp[BUFF_SIZE] = { 0 };
	GetModuleFileNameW(nullptr, temp, sizeof(temp));
	QString fileFullPath = WC_TO_Q_STR(temp, sizeof(temp) / sizeof(wchar_t));
	QFile app(fileFullPath);
	return app.link(linkPath);*/
	return false;
}

bool Misc::isOnline(const QString& address, ushort port, int timeout)
{
	QTcpSocket tcp;
	tcp.connectToHost(address, port);
	return tcp.waitForConnected(timeout);
}

bool Misc::isOnline(const QString& source, const QString& destination, ushort port, int timeout)
{
	QTcpSocket tcp;
	tcp.bind(QHostAddress(source));
	tcp.connectToHost(destination, port);
	return tcp.waitForConnected(timeout);
}


Nt::DvrClient::DvrClient()
	:TcpClient()
{

}

Nt::DvrClient::DvrClient(const QString& address, ushort port)
	: TcpClient(Q_TO_C_STR(address), port)
{

}

Nt::DvrClient::~DvrClient()
{

}

bool Nt::DvrClient::sendFrameData(const char* buffer, int len, uchar cmd, uchar sub)
{
	uchar data[BUFF_SIZE] = { 0 };
	data[0] = 0xEE;
	data[1] = 0xAA;
	int sendLen = 2 + len;
	memcpy(&data[2], &sendLen, sizeof(int));
	data[6] = cmd;
	data[7] = sub;
	if (buffer != nullptr && len != 0)
		memcpy(&data[8], buffer, len);
	uint&& crc32 = unlockAlgorithm(&data[2], 4 + 2 + len, 0);
	for (int i = 0; i < 4; i++)
	{
		data[8 + len + i] = (crc32 >> i * 8) & 0xff;
	}
	return send((const char*)data, len + 12) == (len + 12);
}

bool Nt::DvrClient::sendFrameDataEx(const std::initializer_list<char>& buffer, uchar cmd, uchar sub)
{
	return sendFrameData(buffer.size() ? buffer.begin() : nullptr, buffer.size(), cmd, sub);
}

bool Nt::DvrClient::sendFrameDataEx(const char* buffer, int len, uchar cmd, uchar sub)
{
	return sendFrameData(buffer, len, cmd, sub);
}

bool Nt::DvrClient::recvFrameData(char* buffer, int* const recvLen)
{
	bool success = false;
	uchar data[BUFF_SIZE] = { 0 };
	char* dataPtr = (char*)data;
	int tempLen = 0, dataLen = 0;
	ulong&& startTime = GetTickCount64();
	while (true)
	{
		int count = recv(dataPtr, 1);
		if (count == -1)
		{
			break;
		}

		if (count == 1)
		{
			tempLen++;
			dataPtr++;
		}

		//校验帧头
		if (tempLen == 2)
		{
			if (data[0] == 0xEE || data[1] == 0xAA)
			{
				dataPtr = (char*)&data[2];
			}
			else
			{
				tempLen = 0;
				dataPtr = (char*)data;
			}
		}

		//获取数据长度
		if (tempLen == 6)
		{
			memcpy(&dataLen, &data[2], 4);
		}

		//校验CRC
		if (tempLen == dataLen + 10)
		{
			uint crc32Recv = 0, crc32Result = 0;
			memcpy(&crc32Recv, &data[dataLen + 6], 4);
			crc32Result = unlockAlgorithm((uchar const*)(&data[2]), dataLen + 4, 0);
			if (crc32Result != crc32Recv)
			{
				tempLen = 0;
				dataPtr = (char*)data;
			}
			else
			{
				if (g_debug && *g_debug)
				{
					printf("\nReceive Start--------------------\n");
					for (int i = 0; i < tempLen; i++)
						printf("%02X ", data[i]);
					printf("\nReceive End  --------------------\n");
				}

				if (recvLen)
				{
					*recvLen = dataLen;
				}
				memcpy(buffer, &data[6], dataLen);
				success = true;
				break;
			}
		}

		if (GetTickCount64() - startTime > 5000)
		{
			break;
		}
		Sleep(1);
	}
	return success;
}

bool Nt::DvrClient::recvFrameDataEx(char* buffer, int* const len, uchar cmd, uchar sub)
{
	bool result = false, success = false;
	ulong&& startTime = GetTickCount64();
	do
	{
		while (true)
		{
			if (recvFrameData(buffer, len))
			{
				if ((uchar)buffer[0] == cmd && (uchar)buffer[1] == sub)
				{
					success = true;
					break;
				}
			}

			if (GetTickCount64() - startTime > 10000)
			{
				setLastError(LC_SPRINTF("CMD:%02X,SUB:%02X,接收数据超时", cmd, sub));
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

uint Nt::DvrClient::unlockAlgorithm(uchar const* memoryAddr, uint memoryLen, uint oldCrc32) const
{
	uint oldcrc32 = oldCrc32, length = memoryLen, crc32, oldcrc;
	uchar ccc, t;

	while (length--)
	{
		t = (uchar)(oldcrc32 >> 24U) & 0xFFU;
		oldcrc = m_crc32Table[t];
		ccc = *memoryAddr;
		oldcrc32 = (oldcrc32 << 8U | ccc);
		oldcrc32 = oldcrc32 ^ oldcrc;
		memoryAddr++;
	}
	crc32 = oldcrc32;
	return crc32;
}


void Nt::SfrServer::setLastError(const QString& error)
{
	DEBUG_INFO() << error;
	m_lastError = error;
}

Nt::SfrServer::SfrServer()
{
}

Nt::SfrServer::~SfrServer()
{
	closeListen();
}

void Nt::SfrServer::sfrProcThread(void* arg)
{
	Nt::SfrServer* sfrServer = static_cast<Nt::SfrServer*>(arg);
	while (!sfrServer->m_quit)
	{
		sockaddr_in clientAddr = { 0 };
		int addrLen = sizeof(sockaddr_in);
		SOCKET clientSocket = accept(sfrServer->m_socket, (sockaddr*)&clientAddr, &addrLen);
		if (clientSocket == -1)
		{
			break;
		}
		if (sfrServer->m_client != INVALID_SOCKET)
		{
			closesocket(sfrServer->m_client);
			sfrServer->m_client = INVALID_SOCKET;
		}
		sfrServer->m_client = clientSocket;
		Sleep(100);
	}
	return;
}

bool Nt::SfrServer::startListen(ushort port)
{
	bool result = false;
	do
	{
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(sockVersion, &wsaData) != 0)
		{
			setLastError(Q_SPRINTF("WSAStartup初始化失败,错误代码:%d", WSAGetLastError()));
			break;
		}

		m_socket = socket(AF_INET, SOCK_STREAM, 0);

		if (m_socket == INVALID_SOCKET)
		{
			setLastError(Q_SPRINTF("套接字初始化失败,错误代码:%d", WSAGetLastError()));
			break;
		}

		int timeout = 1000, optval = 1;
		setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));

		memset(&m_sockAddr, 0x00, sizeof(sockaddr_in));
		m_sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		m_sockAddr.sin_family = AF_INET;
		m_sockAddr.sin_port = htons(port);

		if (bind(m_socket, (const sockaddr*)&m_sockAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		{
			setLastError(Q_SPRINTF("SFR服务端绑定失败,错误代码:%d", WSAGetLastError()));
			break;
		}

		if (listen(m_socket, 128) == SOCKET_ERROR)
		{
			setLastError(Q_SPRINTF("SFR服务端监听失败,错误代码:%d", WSAGetLastError()));
			break;
		}

		_beginthread(Nt::SfrServer::sfrProcThread, 0, this);
		result = true;
	} while (false);
	return result;
}

bool Nt::SfrServer::getSfrValue(const char* filePath, float& sfr)
{
	bool result = false;
	do
	{
		int sendLen = 208;
		char sendData[256] = { 0 };

		sprintf(sendData, "$THC001%s", filePath);
		sendData[sendLen - 1] = '$';
		int count = send(sendData, sendLen);
		if (count != sendLen)
		{
			break;
		}

		int recvLen = 208;
		char recvData[256] = { 0 };
		count = recv(recvData, recvLen);
		//检测SFR的那个软件写的有问题,检测失败会返回SOCKET_ERROR
		if (count == SOCKET_ERROR || recvData[0] != '$')
		{
			sfr = 0.0f;
		}
		else
		{
			if (strncmp(recvData, "$HTR000", 7))
			{
				setLastError("SFR客户端数据异常,$HTR000");
				break;
			}

			if (sscanf(&recvData[7], "%f", &sfr) != 1)
			{
				setLastError("SFR客户端数据异常,结果值不为1");
				break;
			}
		}
		result = true;
	} while (false);
	return result;
}

int Nt::SfrServer::send(const char* buffer, int len)
{
	int count = len, result = 0;
	while (count > 0)
	{
		result = ::send(m_client, buffer, count, 0);
		if (result == SOCKET_ERROR)
		{
			setLastError(Q_SPRINTF("发送失败,套接字错误,错误代码:%d", WSAGetLastError()));
			return -1;
		}

		if (result == 0)
		{
			setLastError(Q_SPRINTF("发送失败,数据包丢失,错误代码:%d", WSAGetLastError()));
			return len - count;
		}

		buffer += result;
		count -= result;
	}
	return len;
}

int Nt::SfrServer::recv(char* buffer, int len)
{
	int count = len, result = 0;
	while (count > 0)
	{
		result = ::recv(m_client, buffer, count, 0);
		if (result == SOCKET_ERROR)
		{
			setLastError(Q_SPRINTF("接收失败,套接字错误,错误代码:%d", WSAGetLastError()));
			return -1;
		}

		if (result == 0)
		{
			setLastError(Q_SPRINTF("接收失败,数据包丢失,错误代码:%d", WSAGetLastError()));
			return len - count;
		}

		buffer += result;
		count -= result;
	}
	return len;
}

void Nt::SfrServer::closeListen()
{
	m_quit = true;
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	if (m_client != INVALID_SOCKET)
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
	WSACleanup();
}

QString Nt::SfrServer::getLastError() const
{
	return m_lastError;
}


Nt::OmsClient::OmsClient()
{

}

Nt::OmsClient::~OmsClient()
{

}

int Nt::OmsClient::sendData(const std::initializer_list<uchar>& data)
{
	return sendData((const char*)data.begin(), data.size());
}

int Nt::OmsClient::sendData(const char* data, int size)
{
	size += 4;
	uchar buffer[BUFF_SIZE]{ 0xff,0xa5,0x5a,uchar(size >> 8),uchar(size & 0xff) };
	size -= 4;

	for (size_t i = 0; i < size; i++)
		buffer[HEAD_SIZE + i] = data[i];

	size += HEAD_SIZE;
	uint crc = getCrc(buffer, size, CRC_INIT);

	for (size_t i = 0; i < 4; i++)
		buffer[size + i] = (crc >> ((4 - i - 1) * 8)) & 0xff;

	size += CRC_SIZE;
	return send((const char*)buffer, size);
}

int Nt::OmsClient::recvData(char* data, int size)
{
	uchar buffer[BUFF_SIZE]{};
	if (recv((char*)buffer, HEAD_SIZE) == SOCKET_ERROR)
	{
		return -1;
	}

	if (buffer[0] != 0xff || buffer[1] != 0xa5 || buffer[2] != 0x5a)
	{
		setLastError(U_2_A("帧头校验失败"));
		return -1;
	}

	int length = (((ushort)buffer[3]) << 8) | buffer[4];

	if (recv((char*)buffer + HEAD_SIZE, length) == SOCKET_ERROR)
	{
		return -1;
	}
	length += HEAD_SIZE;
	uint crc0 = getCrc(buffer, length - CRC_SIZE, CRC_INIT);
	uint crc1 = ((uint)buffer[length - 4] << 24) | ((uint)buffer[length - 3] << 16) |
		((uint)buffer[length - 2] << 8) | ((uint)buffer[length - 1] << 0);

	if (crc0 != crc1)
	{
		setLastError(U_2_A("校验码对比失败"));
		return -1;
	}
	memcpy(data, buffer, std::min<int>(size, length));
	return length;
}

uint Nt::OmsClient::getCrc(const uchar* data, uint size, uint hist)
{
	uint crc32val = hist ^ 0xFFFFFFFFU;
	for (uint i = 0; i < size; i++)
	{
		crc32val = m_crc32table[(crc32val ^ data[i]) & 0xff] ^ (crc32val >> 8);
	}
	return crc32val ^ 0xFFFFFFFFU;
}


Nt::AicsClient::AicsClient()
{
}

Nt::AicsClient::~AicsClient()
{
}

bool Nt::AicsClient::diagnosticSessionControl(uchar id)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 2 + 4;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x10;
		data[6] = id;

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));

		if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (2 != data[3] * 256 + data[4] - 4) ||
			(0x50 != data[5]) || (id != data[6]))
		{
			setLastError(U_2_A("诊断会话控制校验数据失败"));
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::ecuReset(uchar id)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		memset(data, 0, sizeof(data));
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 2 + 4;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x11;
		data[6] = id;

		getCrc(data, size + 1, &data[size + 1]);

		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));
		if (recv((char*)data,sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (2 != data[3] * 256 + data[4] - 4) ||
			(0x51 != data[5]) || (id != data[6]))
		{
			setLastError(U_2_A("控制器重置校验数据失败"));
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::readDataByIdentifier(uchar id0, uchar id1, uchar* _data, int* _size)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		memset(data, 0, sizeof(data));
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 3 + 4;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x22;
		data[6] = id0;
		data[7] = id1;

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));

		if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (0x62 != data[5]) ||
			(id0 != data[6]) || (id1 != data[7]))
		{
			setLastError(U_2_A("读取数据标识符校验失败"));
			break;
		}

		size = data[3] * 256 + data[4] - 3 - 4;
		if (_size)
			*_size = size;

		memcpy(_data, &data[8], size);

		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::securityAccess(uchar id)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		memset(data, 0, sizeof(data));
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 2 + 4;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x27;
		data[6] = id;

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));
		if (recv((char*)data,sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (6 != data[3] * 256 + data[4] - 4) ||
			(0x67 != data[5]) || (id != data[6]))
		{
			setLastError(U_2_A("安全访问数据校验失败[0]"));
			break;
		}

		memset(data, 0, sizeof(data));
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		size = 6 + 4;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x27;
		data[6] = id + 1;

		getKey(id, &data[7], 4, &data[7]);

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));
		if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (2 != data[3] * 256 + data[4] - 4) ||
			(0x67 != data[5]) || (id + 1 != data[6]))
		{
			setLastError(U_2_A("安全访问数据校验失败[1]"));
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::routineControl(uchar id0, uchar id1, uchar id2, const uchar* sendData, int sendSize, uchar* recvData, int* recvSize)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 4 + 4 + sendSize;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x31;
		data[6] = id0;
		data[7] = id1;
		data[8] = id2;

		if (sendSize > 0)
			memcpy(&data[9], sendData, sendSize);

		getCrc(data, size + 1, &data[size + 1]);

		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));
		if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (0x71 != data[5]) ||
			(id0 != data[6]) || (id1 != data[7]) || (id2 != data[8]))
		{
			setLastError(U_2_A("例程控制校验数据失败"));
			break;
		}

		if (data[3] * 256 + data[4] > 4)
		{
			if (recvSize)
				*recvSize = data[3] * 256 + data[4] - 4;

			if (recvData)
				memcpy(recvData, &data[5], data[3] * 256 + data[4] - 4);
		}
		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::requestDownload(quint64 address, quint64 size)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		memset(data, 0, sizeof(data));
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 18 + 4;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x34;
		data[6] = 0x88;

		for (int i = 0; i < 8; i++)
		{
			data[7 + i] = (address >> ((8 - i - 1) * 8)) & 0xff;
			data[15 + i] = (address >> ((8 - i - 1) * 8)) & 0xff;
		}

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));

		if (recv((char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) || (0x5a != data[2]) ||
			(0x04 != data[3] * 256 + data[4] - 4) || (0x74 != data[5]) ||
			(0x08 != data[6]) || (0x00 != data[7]) || (0x01 != data[8]))
		{
			setLastError(U_2_A("请求下载校验数据失败"));
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::transferData(uchar id, const char* _data, int _size)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 2 + 4 + _size;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x36;
		data[6] = id;

		if (_size > 0)
		{
			memcpy(&data[7], _data, _size);
		}

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));

		if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (0x02 != data[3] * 256 + data[4] - 4) ||
			(0x76 != data[5]) || (id != data[6]))
		{
			setLastError(U_2_A("传输数据校验失败"));
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::requestTransferExit(uchar* _data, int* _size)
{
	bool result = false;
	do
	{
		uchar data[256] = { 0 };
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;

		ushort size = 1 + 4;
		data[3] = 0xff & (size >> 8);
		data[4] = 0xff & (size >> 0);
		data[5] = 0x37;

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		memset(data, 0, sizeof(data));

		if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
		{
			break;
		}

		if ((0xff != data[0]) || (0xa5 != data[1]) ||
			(0x5a != data[2]) || (0x01 != data[3] * 256 + data[4] - 4) ||
			(0x77 != data[5]))
		{
			setLastError(U_2_A("请求传输校验数据失败"));
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::readRegister(uint8_t devices, uint16_t page, uint32_t address, uint32_t* value)
{
	bool result = false, success = false;
	do 
	{
		uint8_t data[256] = { 0 };
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;
		uint16_t size = 1 + 8 + 4;//1 is cmd size, 8 is data size, 4 is crc size
		data[3] = static_cast<uint8_t>((size >> 8) & 0xff);
		data[4] = static_cast<uint8_t>((size >> 0) & 0xff);
		data[5] = 0xd0;//is request rw register cmd
		data[6] = 0;// 0 is read cmd
		data[7] = devices;
		data[8] = static_cast<uint8_t>((page >> 8) & 0xff);
		data[9] = static_cast<uint8_t>((page >> 0) & 0xff);
		data[10] = static_cast<uint8_t>((address >> 24) & 0xff);
		data[11] = static_cast<uint8_t>((address >> 16) & 0xff);
		data[12] = static_cast<uint8_t>((address >> 8) & 0xff);
		data[13] = static_cast<uint8_t>((address >> 0) & 0xff);

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		auto time = GetTickCount64();
		while (true)
		{
			memset(data, 0, sizeof(data));
			if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
			{
				break;
			}

			if (data[0] == 0xff && data[1] == 0xa5 && data[2] == 0x5a && data[5] == 0xd1)
			{
				success = true;
				break;
			}

			if (GetTickCount64() - time > 3000)
			{
				setLastError(U_2_A("读取寄存器校验数据失败"));
				break;
			}
		}

		if (!success)
		{
			break;
		}

		//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
		//ff a5 5a 00 11 d1 00 01 00 00 00 00 00 09 00 00 08 00 21 80 5f 7a
		memcpy(value, &data[14], 4);
		result = true;
	} while (false);
	return result;
}

bool Nt::AicsClient::writeRegistr(uint8_t devices, uint16_t page, uint32_t address, uint32_t value)
{
	bool result = false, success = false;
	do 
	{
		uint8_t data[256] = { 0 };
		data[0] = 0xff;
		data[1] = 0xa5;
		data[2] = 0x5a;
		uint16_t size = 1 + 12 + 4;//1 is cmd size, 8 is data size, 4 is crc size
		data[3] = static_cast<uint8_t>((size >> 8) & 0xff);
		data[4] = static_cast<uint8_t>((size >> 0) & 0xff);
		data[5] = 0xd0;//is request rw register cmd
		data[6] = 1;// 1 is write cmd
		data[7] = devices;
		data[8] = static_cast<uint8_t>((page >> 8) & 0xff);
		data[9] = static_cast<uint8_t>((page >> 0) & 0xff);
		data[10] = static_cast<uint8_t>((address >> 24) & 0xff);
		data[11] = static_cast<uint8_t>((address >> 16) & 0xff);
		data[12] = static_cast<uint8_t>((address >> 8) & 0xff);
		data[13] = static_cast<uint8_t>((address >> 0) & 0xff);

		data[14] = static_cast<uint8_t>((value >> 24) & 0xff);
		data[15] = static_cast<uint8_t>((value >> 16) & 0xff);
		data[16] = static_cast<uint8_t>((value >> 8) & 0xff);
		data[17] = static_cast<uint8_t>((value >> 0) & 0xff);

		getCrc(data, size + 1, &data[size + 1]);
		size += 5;

		if (send((const char*)data, size) == SOCKET_ERROR)
		{
			break;
		}

		auto time = GetTickCount64();
		while (true)
		{
			memset(data, 0, sizeof(data));
			if (recv((char*)data, sizeof(data)) == SOCKET_ERROR)
			{
				break;
			}

			if (data[0] == 0xff && data[1] == 0xa5 && data[2] == 0x5a && data[5] == 0xd1)
			{
				success = true;
				break;
			}

			if (GetTickCount64() - time > 3000)
			{
				setLastError(U_2_A("写入寄存器校验数据失败"));
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

bool Nt::AicsClient::getKey(int level, const uchar* seedData, int seedSize, uchar* keyData, int* keySize)
{
	unsigned int key = 0;
	for (int i = 0; i < seedSize; i++)
		key += ((unsigned int)seedData[i] << (3 - i) * 8);
	key = ~key + (0x11 == level ? 0x67616569 : 0x47414549);

	keyData[0] = (key >> 24);
	keyData[1] = (key >> 16);
	keyData[2] = (key >> 8);
	keyData[3] = (key >> 0);

	if (keySize)
		*keySize = 4;
	return true;
}

uint Nt::AicsClient::getCrc(const uchar* data, int size) const
{
	unsigned int crc32 = 0xFFFFFFFF;
	for (unsigned int i = 0; i < size; i++)
	{
		crc32 = m_crc32Table[(crc32 ^ data[i]) & 0xff] ^ (crc32 >> 8);
	}
	crc32 = crc32 ^ 0xFFFFFFFF;
	return crc32;
}

void Nt::AicsClient::getCrc(const uchar* data, int size, uchar* value) const
{
	auto result = getCrc(data, size);

	value[0] = (result >> 24) & 0xff;
	value[1] = (result >> 16) & 0xff;
	value[2] = (result >> 8) & 0xff;
	value[3] = (result >> 0) & 0xff;
	return;
}

int Nt::AicsClient::recv(char* buffer, int size)
{
	uchar head[5]{};
	if (__super::recv((char*)head, sizeof(head)) != 5)
	{
		setLastError(LC_SPRINTF("接收数据包帧头失败,%s", LC_TO_C_STR(getLastError())));
		return -1;
	}

	uchar temp[3]{ 0xff,0xa5,0x5a };
	if (memcmp(head, temp, sizeof(temp)))
	{
		setLastError(U_2_A("帧头校验失败"));
		return -1;
	}

	memcpy(buffer, head, sizeof(head));
	ushort length = ((((ushort)head[3]) << 8) | (head[4]));
	return __super::recv(buffer + sizeof(head), length);
}
