#include "QLabelEx.h"

using namespace Dt;

QLabelEx::QLabelEx(QWidget* parent)
	: QLabel(parent)
{

}

QLabelEx::~QLabelEx()
{

}

void QLabelEx::setLabelMode(LabelMode mode)
{
	m_mode = mode;
}

void QLabelEx::getCoordinate(QVector<QPoint>* start, QVector<QPoint>* end)
{
	*start = m_startCoord;
	*end = m_endCoord;
}

void QLabelEx::clearCoordinate()
{
	m_startCoord.clear();
	m_endCoord.clear();
}

void QLabelEx::setTouchScreenInfo(const TouchScreenInfo& touch)
{
	m_touch = touch;
}

void QLabelEx::setShowCoordSizeLabel(QLabel* label)
{
	m_label = label;
}

void QLabelEx::mousePressEvent(QMouseEvent* event)
{
	if (m_mode == LabelMode::LM_NONE)
	{
		return;
	}

	if (m_mode == LabelMode::LM_PAINT)
	{
		if (event->button() == Qt::LeftButton)
		{
			m_startX = event->pos().x();
			m_startY = event->pos().y();
			QCursor cursor;
			cursor.setShape(Qt::ClosedHandCursor);
			QApplication::setOverrideCursor(cursor);
			m_press = false;
		}
	}
	else if (m_mode == LabelMode::LM_TOUCH)
	{
		if (m_touch.canBase && *m_touch.canBase && m_touch.canMatrix)
		{
			can::Msg msg = { 0 };
			msg.id = m_touch.msgId;
			msg.dlc = 8;
			msg.sendCycle = m_touch.msgSendPeriod;
			msg.sendType = can::SendType::EVENT;
			msg.sendCount = m_touch.msgSendCount;

			const int x = float(m_touch.screenWidth * event->pos().x()) / m_width;
			const int y = float(m_touch.screenHeight * event->pos().y()) / m_height;
			qDebug("Press x %d,y %d", x, y);

			m_touch.canMatrix->pack(msg.data, m_touch.touchStart, m_touch.touchLength, m_touch.touchPress);
			m_touch.canMatrix->pack(msg.data, m_touch.xStart, m_touch.xLength, x);
			m_touch.canMatrix->pack(msg.data, m_touch.yStart, m_touch.yLength, y);

			(*m_touch.canBase)->addMsg(msg);
			(*m_touch.canBase)->startAsyncSendMsg();
		}
	}
}

void QLabelEx::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_mode == LabelMode::LM_NONE)
	{
		return;
	}

	if (m_mode == LabelMode::LM_PAINT)
	{
		if (event->button() == Qt::LeftButton)
		{
			m_endX = event->pos().x();
			m_endY = event->pos().y();
			QApplication::restoreOverrideCursor();
			//if (m_endX - m_startX <= 10 || m_endY - m_startY <= 10)
			//{
			//	return;
			//}
			m_startCoord.push_back(QPoint(m_startX, m_startY));
			m_endCoord.push_back(QPoint(m_endX, m_endY));

			update();
		}

		if (event->button() == Qt::RightButton)
		{
			if (m_startCoord.size() && m_endCoord.size())
			{
				m_press = true;
				m_startCoord.pop_back();
				m_endCoord.pop_back();
				update();
			}
		}
	}
	else if (m_mode == LabelMode::LM_TOUCH)
	{
		if (m_touch.canBase && *m_touch.canBase && m_touch.canMatrix)
		{
			can::Msg msg = { 0 };
			msg.id = m_touch.msgId;
			msg.dlc = 8;
			msg.sendCycle = m_touch.msgSendPeriod;
			msg.sendType = can::SendType::EVENT;
			msg.sendCount = m_touch.msgSendCount;

			const int x = float(m_touch.screenWidth * event->pos().x()) / m_width;
			const int y = float(m_touch.screenHeight * event->pos().y()) / m_height;

			qDebug("Release x %d,y %d", x, y);

			m_touch.canMatrix->pack(msg.data, m_touch.touchStart, m_touch.touchLength, m_touch.touchRelease);
			m_touch.canMatrix->pack(msg.data, m_touch.xStart, m_touch.xLength, x);
			m_touch.canMatrix->pack(msg.data, m_touch.yStart, m_touch.yLength, y);
			(*m_touch.canBase)->addMsg(msg);
		}
	}
}

void QLabelEx::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mode == LabelMode::LM_NONE)
	{
		return;
	}

	if (m_mode == LabelMode::LM_PAINT)
	{
		if ((event->buttons() & Qt::LeftButton))
		{
			m_endX = event->pos().x();
			m_endY = event->pos().y();
			update();
		}
	}
	else if (m_mode == LabelMode::LM_TOUCH)
	{
		if ((event->buttons() & Qt::LeftButton))
		{
			if (m_touch.canBase && *m_touch.canBase && m_touch.canMatrix)
			{
				can::Msg msg = { 0 };
				msg.id = m_touch.msgId;
				msg.dlc = 8;
				msg.sendCycle = m_touch.msgSendPeriod;
				msg.sendType = can::SendType::EVENT;
				msg.sendCount = m_touch.msgSendCount;

				const int x = float(m_touch.screenWidth * event->pos().x()) / m_width;
				const int y = float(m_touch.screenHeight * event->pos().y()) / m_height;

				qDebug("Slither x %d,y %d", x, y);

				m_touch.canMatrix->pack(msg.data, m_touch.touchStart,
					m_touch.touchLength, m_touch.touchSlither);
				m_touch.canMatrix->pack(msg.data, m_touch.xStart, m_touch.xLength, x);
				m_touch.canMatrix->pack(msg.data, m_touch.yStart, m_touch.yLength, y);
				(*m_touch.canBase)->addMsg(msg);
			}

		}
	}
}

void QLabelEx::paintEvent(QPaintEvent* event)
{
	QLabel::paintEvent(event);

	if (m_mode == LabelMode::LM_NONE)
	{
		return;
	}

	if (m_mode == LabelMode::LM_PAINT)
	{
		QPainter painter(this);
		painter.setPen(QPen(Qt::red, 1));
		for (int i = 0; i < m_endCoord.size(); i++)
		{
			painter.drawRect(QRect(m_startCoord.at(i).x(), m_startCoord.at(i).y(),
				m_endCoord.at(i).x() - m_startCoord.at(i).x(), m_endCoord.at(i).y() - m_startCoord.at(i).y()));
		}

		if (!m_press)
		{
			QRect rect(m_startX, m_startY, m_endX - m_startX, m_endY - m_startY);
			painter.drawRect(rect);
			if (m_label)
				m_label->setText(QString().sprintf("X:%d,Y:%d,W:%d,H:%d",
					rect.x(), rect.y(), rect.width(), rect.height()));
		}
	}
}
