#include "DownloadDlg.h"

using namespace Dt;

DownloadDlg::DownloadDlg(QDialog* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	installEventFilter(this);
	utility::window::setBorderRadius(this);
	connect(ui.abortBtn, &QPushButton::clicked, this, [&]() {m_manager->getReply()->abort(); });
	resetNetwork();
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

DownloadDlg::~DownloadDlg()
{
	SAFE_DELETE(m_manager);
}

void DownloadDlg::download(const QString& title, const QString& url, const QString& path)
{
	ui.progressBar->setValue(0);
	ui.titleLabel->setText(title);
	ui.urlLabel->setText(url);
	m_manager->setSavePath(path);
	m_manager->download(url);
}

bool DownloadDlg::getResult() const
{
	return m_result == DR_SUCCESS;
}

ulong DownloadDlg::getElapsedTime() const
{
	return m_manager->getElapsedTime();
}

float DownloadDlg::getAverageSpeed() const
{
	return m_manager->getAverageSpeed();
}

float DownloadDlg::getFileSize() const
{
	return m_manager->getFileSize();
}

QString DownloadDlg::getLastError() const
{
	return m_lastError;
}

bool DownloadDlg::resetNetwork()
{
	//Network access is disabled.Qt的BUG
	SAFE_DELETE(m_manager);
	m_manager = NO_THROW_NEW Downloader;
	connect(m_manager, &Downloader::progressSignal, this, &DownloadDlg::progressSlot);
	connect(m_manager, &Downloader::resultSignal, this, &DownloadDlg::resultSlot);
	return m_manager != nullptr;
}

void DownloadDlg::setLastError(const QString& error)
{
	m_lastError = error;
}

void DownloadDlg::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isPress = true;
		m_point = event->globalPos();
	}
}

void DownloadDlg::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_isPress)
	{
		m_isPress = false;
	}
}

void DownloadDlg::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isPress)
	{
		int x = event->globalX() - m_point.x();
		int y = event->globalY() - m_point.y();
		m_point = event->globalPos();
		move(this->x() + x, this->y() + y);
	}
}

bool DownloadDlg::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		if (reinterpret_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
		{
			return true;
		}
	}
	return QObject::eventFilter(obj, event);
}

void DownloadDlg::resultSlot(DownloadResult result, const QString& error)
{
	setLastError(error);
	m_result = result;
	this->close();
}

void DownloadDlg::progressSlot(qint64 receive, qint64 total, float speed)
{
	ui.sizeLabel->setText(QString().sprintf("%.2fMB/%.2fMB", (float)total / 1024 / 1024,
		(float)receive / 1024 / 1024));
	ui.speedLabel->setText(QString().sprintf("%4.2fKb/s", speed));
	ui.progressBar->setValue(((float)receive / total) * 100);
}
