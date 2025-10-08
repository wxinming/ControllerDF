#include "Downloader.h"
#include <windows.h>
using namespace Dt;

#include "../Manage/Json.h"

Downloader::Downloader()
{
	PRINT_CON_DESTRUCTION(Downloader);
	connect(&m_manager, &QNetworkAccessManager::finished, this, &Downloader::finishedSlot);
}

Downloader::~Downloader()
{
	PRINT_CON_DESTRUCTION(Downloader);
}

QString Downloader::getLastError() const
{
	return m_lastError;
}

qint64 Downloader::getFileSize(const QUrl& url, int times)
{
	qint64 size = -1;
	do
	{
		QNetworkAccessManager manager;
		QEventLoop loop;
		QTimer timer;

		QNetworkReply* reply = manager.head(QNetworkRequest(url));
		if (!reply) continue;

		QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
		QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

		timer.start(3000);
		loop.exec();

		if (reply->error() != QNetworkReply::NoError)
		{
			setLastError(reply->errorString());
			continue;
		}
		QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
		size = var.toLongLong();
		reply->deleteLater();
		break;
	} while (times-- > 0);
	return size;
}

void Downloader::download(const QUrl& url)
{
	m_recvBytes = 0;
	m_speedV.clear();
	QNetworkRequest request(url);
	QNetworkReply* reply = m_manager.get(request);
	m_reply = reply;
	connect(reply, &QNetworkReply::downloadProgress, this, &Downloader::progressSlot);
	connect(reply, &QNetworkReply::sslErrors, this, &Downloader::sslErrorsSlot);
	connect(new QReplyTimeout(reply, 3000), &QReplyTimeout::timeout, this, [&]() {emit resultSignal(DR_TIMEOUT, "连接超时"); });
	m_startTime = GetTickCount64();
}

void Downloader::setTimeout(int timeout)
{
	m_timeout = timeout;
}

void Downloader::setSavePath(const QString& savePath)
{
	m_savePath = savePath;
}

QNetworkReply* Downloader::getReply() const
{
	return m_reply;
}

ulong Downloader::getElapsedTime() const
{
	return m_endTime;
}

float Downloader::getAverageSpeed() const
{
	float sum = 0.0f;
	for (auto& x : m_speedV)
		sum += x;
	return sum / m_speedV.size();
}

float Downloader::getFileSize() const
{
	return (float)m_fileSize / 1024.0f / 1024.0f;
}

void Downloader::sslErrorsSlot(const QList<QSslError>& sslErrors)
{
	foreach(const QSslError & error, sslErrors)
		setLastError(error.errorString());
}

void Downloader::progressSlot(qint64 recvBytes, qint64 totalBytes)
{
	m_fileSize = totalBytes;
	float speed = (recvBytes - m_recvBytes) / 1024.0 * 8;
	m_speedV.push_back(speed);
	emit progressSignal(recvBytes, totalBytes, speed);
	m_recvBytes = recvBytes;
}

void Downloader::setLastError(const QString& error)
{
#ifdef QT_DEBUG
	qDebug() << error;
#endif
	m_lastError = error;
}

QString Downloader::saveFileName(const QUrl& url)
{
	QString path = url.path();
	QString basename = QFileInfo(path).fileName();
	if (basename.isEmpty()) basename = "NoNameFile";

	QString savePath = m_savePath;
	if (!QDir(savePath).exists())
	{
		QDir dir;
		if (!dir.mkpath(savePath))
		{
			setLastError("创建下载目录失败");
			return basename;
		}
	}

	if (QFile::exists(savePath + basename))
	{
		int i = 0;
		basename += '.';
		while (QFile::exists(savePath + basename + QString::number(i))) ++i;
		basename += QString::number(i);
	}
	return basename;
}

bool Downloader::saveToDisk(const QString& filename, QIODevice* data)
{
	bool result = false;
	do
	{
		QString savePath = QString(".\\%1\\").arg(m_savePath);
		if (!QDir(savePath).exists())
		{
			QDir dir;
			if (!dir.mkpath(savePath))
			{
				setLastError("创建下载目录失败");
				break;
			}
		}
		QFile file(savePath + filename);
		if (!file.open(QIODevice::WriteOnly))
		{
			setLastError(file.errorString());
			break;
		}
		file.write(data->readAll());
		file.close();
		result = true;
	} while (false);
	return result;
}

void Downloader::finishedSlot(QNetworkReply* reply)
{
	m_endTime = GetTickCount64() - m_startTime;
	QUrl url = reply->url();
	if (reply->error())
	{
		emit resultSignal(DR_ERROR, reply->errorString());
	}
	else
	{
		if (saveToDisk(saveFileName(url), reply))
			emit resultSignal(DR_SUCCESS, "下载成功");
		else
			emit resultSignal(DR_FAILURE, "下载失败," + getLastError());
	}
	reply->deleteLater();
}