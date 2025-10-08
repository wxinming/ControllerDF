#pragma once
#pragma execution_character_set("utf-8")

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QTimer>
#include <QUrl>
#include <QVector>
#include "../Config.h"

namespace Dt
{

	enum DownloadResult
	{
		DR_SUCCESS,
		DR_FAILURE,
		DR_ERROR,
		DR_TIMEOUT,
	};

	/*下载器类*/
	class DETECTION_DLL_EXPORT Downloader : public QObject
	{
		Q_OBJECT
	public:
		/*构造*/
		Downloader();

		/*析构*/
		~Downloader();

		/*获取最终错误*/
		QString getLastError() const;

		/*获取文件大小*/
		qint64 getFileSize(const QUrl& url, int times = 0);

		/*下载*/
		void download(const QUrl& url);

		/*设置下载超时*/
		void setTimeout(int timeout);

		/*设置保存路径*/
		void setSavePath(const QString& savePath);

		/*获取应答*/
		QNetworkReply* getReply() const;

		/*所用时间,单位MS*/
		ulong getElapsedTime() const;

		/*平均网速,单位KB*/
		float getAverageSpeed() const;

		/*文件大小,单位MB*/
		float getFileSize() const;
	signals:
		/*结果信号*/
		void resultSignal(DownloadResult result, const QString& error);

		/*进度信号*/
		void progressSignal(qint64 receive, qint64 total, float speed);
	public slots:
		/*完成槽*/
		void finishedSlot(QNetworkReply* reply);

		/*SSL错误槽*/
		void sslErrorsSlot(const QList<QSslError>& errors);

		/*进度槽*/
		void progressSlot(qint64 recvBytes, qint64 totalBytes);

	protected:
		/*设置最终错误*/
		void setLastError(const QString& error);

		/*保存文件名*/
		QString saveFileName(const QUrl& url);

		/*保存到磁盘*/
		bool saveToDisk(const QString& filename, QIODevice* data);
	private:
		QString m_lastError = "未知错误";

		QNetworkAccessManager m_manager;

		QNetworkReply* m_reply = nullptr;

		int m_timeout = 120000;

		ulong m_startTime = 0;

		ulong m_endTime = 0;

		qint64 m_recvBytes = 0;

		qint64 m_fileSize = 0;

		QVector<float> m_speedV;

		QString m_savePath = "Downloader";
	};

	/*应答超时类*/
	class DETECTION_DLL_EXPORT QReplyTimeout : public QObject
	{
		Q_OBJECT
	public:
		QReplyTimeout(QNetworkReply* reply, int timeout) : QObject(reply)
		{
			if (reply && reply->isRunning())
			{
				QTimer::singleShot(timeout, this, &QReplyTimeout::onTimeout);
			}
		}
	private slots:
		void onTimeout()
		{
			QNetworkReply* reply = static_cast<QNetworkReply*>(parent());
			if (reply && reply->isRunning() && reply->size() == 0)
			{
				reply->abort();
				reply->deleteLater();
				emit timeout();
			}
		}
	signals:
		void timeout();
	};
}