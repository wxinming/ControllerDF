#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QDialog>
#include <QMouseEvent>

#ifdef _WIN64
#ifdef QT_DEBUG
#include "../x64/Debug/qt/uic/ui_DownloadDlg.h"
#else
#include "../x64/Release/qt/uic/ui_DownloadDlg.h"
#endif//QT_DEBUG
#else
#ifdef QT_DEBUG
#include "../Win32/Debug/qt/uic/ui_DownloadDlg.h"
#else
#include "../Win32/Release/qt/uic/ui_DownloadDlg.h"
#endif//QT_DEBUG
#endif//_WIN64

#include "../Network/Downloader.h"
#include "../Manage/Json.h"

namespace Dt {
	class DETECTION_DLL_EXPORT DownloadDlg : public QDialog
	{
		Q_OBJECT
	public:
		DownloadDlg(QDialog* parent = Q_NULLPTR);
		~DownloadDlg();
		void download(const QString& title, const QString& url, const QString& path);
		bool getResult() const;
		ulong getElapsedTime() const;
		float getAverageSpeed() const;
		float getFileSize() const;
		QString getLastError() const;
		bool resetNetwork();

	protected:
		void setLastError(const QString& error);
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		bool eventFilter(QObject* obj, QEvent* event) override;

	private slots:
		void resultSlot(DownloadResult result, const QString& error);
		void progressSlot(qint64 receive, qint64 total, float speed);

	private:
		Ui::DownloadDlg ui;
		QString m_lastError = "未知错误";
		Downloader* m_manager = nullptr;
		DownloadResult m_result = DR_ERROR;
		bool m_isPress = false;
		QPoint m_point;
	};
}
